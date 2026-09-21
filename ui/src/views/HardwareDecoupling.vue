<script setup lang="ts">
import { computed, onMounted, onUnmounted, ref } from 'vue'
import { confirmRealActuatorSwitch, injectActuatorFault, motorPresentation, createActuatorDemo, hardwareDisplayProtocol, mergeRealMotorTelemetry, syncRealRemovalState } from '../marine/actuators'
import type { ActuatorDevice } from '../marine/actuators'
import { marineApi } from '../marine/api'
import DecouplingSwitch from '../components/DecouplingSwitch.vue'
import motorImage from '../assets/motor-blue-3d.png'


const demo = ref(createActuatorDemo())
const gatewayOnline = ref(false)
const selectedExecutorId = ref('FIRE-PUMP-01')
const targetRpm = ref(50)
const targetDirection = ref<'forward' | 'reverse'>('forward')
const motorBusy = ref(false)
const motorMessage = ref('请选择在线电机并设置运行参数')
const executorByAddress: Record<string, string> = { '0x02': 'EXHAUST-FAN-01', '0x0E': 'FIRE-PUMP-01', '0x0F': 'DRAIN-PUMP-01' }
const selectedMotor = computed(() => demo.value.devices.find(device => executorByAddress[device.address] === selectedExecutorId.value))
const live = computed(() => motorPresentation(selectedMotor.value, gatewayOnline.value))
const interfacesActive = computed(() => gatewayOnline.value && demo.value.devices.some(device => motorPresentation(device, true).online))
const validRpm = computed(() => Number.isInteger(targetRpm.value) && targetRpm.value >= 1 && targetRpm.value <= 500)
const canStart = computed(() => gatewayOnline.value && live.value.online && validRpm.value && !motorBusy.value)
const candidate = computed(() => demo.value.devices.find(d => d.id === demo.value.candidateId))
const canProtect = computed(() => ['running', 'recovered'].includes(demo.value.phase))
const phaseLabel = computed(() => ({
  running: '冷却任务运行中', protected: '请选择替换水泵',
  'awaiting-confirmation': '等待确认切换', switching: '正在恢复控制', recovered: '冷却任务已恢复',
}[demo.value.phase]))
function selectable(d: ActuatorDevice) {
  return ['protected', 'awaiting-confirmation'].includes(demo.value.phase) && d.id !== demo.value.logicalExecutor.boundDeviceId && d.compatible && d.status !== 'offline' && gatewayOnline.value
}
function select(d: ActuatorDevice) {
  selectedExecutorId.value = executorByAddress[d.address]!
  if (selectable(d)) demo.value = { ...demo.value, candidateId: d.id, phase: 'awaiting-confirmation', lastMessage: `已选择 ${d.name}，等待确认切换` }
}
const screen = ref<HTMLElement>()
const packetPath = ref('')
const packetColor = ref('#53f5ef')
const packetKey = ref(0)
const heartbeat = ref(false)
let disposed = false
let heartbeatTimer: ReturnType<typeof setTimeout> | undefined
function pathBetween(from: string, to: string, color: string) {
  const root = screen.value
  const a = root?.querySelector(from)?.getBoundingClientRect()
  const b = root?.querySelector(to)?.getBoundingClientRect()
  if (!root || !a || !b) { packetPath.value = ''; return }
  const box = root.getBoundingClientRect()
  const down = b.top >= a.top
  const x1 = a.left + a.width / 2 - box.left
  const y1 = (down ? a.bottom : a.top) - box.top
  const x2 = b.left + b.width / 2 - box.left
  const y2 = (down ? b.top : b.bottom) - box.top
  const dx = x2 - x1
  const dy = y2 - y1
  const distance = Math.hypot(dx, dy) || 1
  const sway = Math.min(24, distance * .16)
  const nx = -dy / distance * sway
  const ny = dx / distance * sway
  // Opposing control points create a gentle sideways sway with exact endpoints.
  packetPath.value = `M ${x1} ${y1} C ${x1 + dx / 3 + nx} ${y1 + dy / 3 + ny}, ${x1 + dx * 2 / 3 - nx} ${y1 + dy * 2 / 3 - ny}, ${x2} ${y2}`
  packetColor.value = color
  packetKey.value++
}
const pause = (ms: number) => new Promise(resolve => setTimeout(resolve, ms))
async function showRoute(command: string, target: string, returning = false) {
  const generation = flowGeneration
  const motor = `[data-device="${target}"]`
  const source = command === 'get_status' ? '.query button' : `[data-command="${command}"]`
  const route = returning ? [motor, '.standalone-adapter', '.query button', '.live-parameters'] : [source, '.standalone-adapter', motor]
  for (let i = 0; i < route.length - 1; i++) {
    if (disposed || generation !== flowGeneration) return
    flowStage.value = returning ? 'return' : i === 0 ? 'function' : 'adapter'
    motorMessage.value = returning ? '设备已响应 → 状态反馈 → 运行参数' : i === 0 ? `下发 ${command} → 协议转换` : `协议转换 → ${demo.value.devices.find(d => d.id === target)?.name ?? '目标电机'}`
    pathBetween(route[i]!, route[i + 1]!, returning || command === 'get_status' ? '#c68aff' : '#53f5ef')
    await pause(450)
  }
  if (generation === flowGeneration && !disposed) { packetPath.value = ''; flowStage.value = returning ? 'feedback' : 'motor' }
}
type FlowCommand = 'start' | 'stop' | 'set_speed' | 'set_direction' | 'get_status'
const flowStage = ref('')
const flowCommand = ref<FlowCommand>('start')
const flowTarget = ref('')
let flowGeneration = 0
const flowTimers: ReturnType<typeof setTimeout>[] = []
function clearFlow() {
  flowGeneration++
  flowTimers.splice(0).forEach(clearTimeout)
  flowStage.value = ''
  packetPath.value = ''
}
function previewParameter(command: FlowCommand) {
  if (motorBusy.value) return
  clearFlow()
  flowCommand.value = command
  flowStage.value = 'function'
  motorMessage.value = '参数已调整，点击启动电机下发'
  flowTimers.push(setTimeout(() => { flowStage.value = '' }, 1000))
}
async function traceControl(command: FlowCommand, target: string, send: () => Promise<unknown>) {
  clearFlow()
  const generation = flowGeneration
  flowCommand.value = command
  flowTarget.value = target
  // Send immediately, especially for stop; the route is an explanatory animation.
  const result = send().then(value => ({ value, error: null }), error => ({ value: null, error }))
  await showRoute(command, target)
  if (generation === flowGeneration) motorMessage.value = '指令已下发，等待设备响应…'
  const outcome = await result
  if (generation === flowGeneration && !disposed) {
    if (!outcome.error) await showRoute(command, target, true)
    else flowStage.value = 'error'
    flowTimers.push(setTimeout(() => { flowStage.value = '' }, 1000))
  }
  if (outcome.error) throw outcome.error
  return outcome.value
}
async function startMotor() {
  if (motorBusy.value) return
  if (!gatewayOnline.value || !live.value.online) { motorMessage.value = '所选电机当前离线，无法启动'; return }
  if (!validRpm.value) { motorMessage.value = 'RPM 必须设置在 1–500 之间'; return }
  motorBusy.value = true; motorMessage.value = '控制指令下发中…'
  try {
    await traceControl(flowCommand.value === 'set_speed' || flowCommand.value === 'set_direction' ? flowCommand.value : 'start', selectedMotor.value!.id, () => marineApi.startMotor(selectedExecutorId.value, { speedRpm: targetRpm.value, direction: targetDirection.value, acceleration: 10, deceleration: 10, operator: '现场演示员' }))
    motorMessage.value = `已下发 ${targetDirection.value === 'reverse' ? '反转' : '正转'} ${targetRpm.value} RPM`
    await refreshRealMotors()
  } catch (error) { motorMessage.value = error instanceof Error ? error.message : '电机启动失败' }
  finally { motorBusy.value = false }
}
async function stopMotor() {
  if (motorBusy.value || !gatewayOnline.value) return
  motorBusy.value = true; motorMessage.value = '停止指令下发中…'
  try { await traceControl('stop', selectedMotor.value!.id, () => marineApi.stopDirectMotor(selectedExecutorId.value, '现场演示员')); motorMessage.value = '电机已停止'; await refreshRealMotors() }
  catch (error) { motorMessage.value = error instanceof Error ? error.message : '电机停止失败' }
  finally { motorBusy.value = false }
}
let timer: ReturnType<typeof setInterval>
const queryBusy = ref(false)
const queryStage = ref('')
const queryTarget = ref('')
let queryGeneration = 0
const queryTimers: ReturnType<typeof setTimeout>[] = []
function clearQueryFlow() {
  queryGeneration++
  queryTimers.splice(0).forEach(clearTimeout)
  queryStage.value = ''
}
let refreshBusy = false
async function refreshRealMotors(manual = false) {
  if (refreshBusy || (manual && motorBusy.value)) return
  refreshBusy = true
  queryBusy.value = true
  if (manual) { motorBusy.value = true; clearFlow(); flowCommand.value = 'get_status' }
  try {
    const request = marineApi.listMotors().then(value => ({ value, error: null }), error => ({ value: null, error }))
    const onlineDevice = demo.value.devices.find(d => motorPresentation(d, gatewayOnline.value).online)
    if (manual && onlineDevice) { flowTarget.value = onlineDevice.id; await showRoute('get_status', onlineDevice.id) }
    else if (manual) motorMessage.value = '正在扫描接入设备…'
    const result = await request
    if (disposed) return
    if (result.error) throw result.error
    const motors = result.value!
    gatewayOnline.value = true
    demo.value = syncRealRemovalState({ ...demo.value, mode: 'real', devices: mergeRealMotorTelemetry(demo.value.devices, motors) })
    heartbeat.value = true
    clearTimeout(heartbeatTimer)
    heartbeatTimer = setTimeout(() => { heartbeat.value = false }, 250)
    if (manual) {
      const online = motors.find(motor => motor.online)
      const target = demo.value.devices.find(d => executorByAddress[d.address] === online?.executorId)
      if (target) { flowTarget.value = target.id; await showRoute('get_status', target.id, true) }
      motorMessage.value = target ? `${target.name} 状态已更新` : '未检测到在线电机，继续自动扫描'
      flowTimers.push(setTimeout(() => { flowStage.value = '' }, 700))
    }
  } catch (error) {
    if (disposed) return
    gatewayOnline.value = false
    if (manual) { flowStage.value = 'error'; motorMessage.value = `查询失败：${error instanceof Error ? error.message : '网关无响应'}` }
  } finally { refreshBusy = false; queryBusy.value = false; if (manual) motorBusy.value = false }
}
async function simulateFault() {
  if (!canProtect.value || motorBusy.value || !gatewayOnline.value) return
  motorBusy.value = true
  motorMessage.value = '正在模拟故障并停止当前绑定电机…'
  try {
    await traceControl('stop', demo.value.logicalExecutor.boundDeviceId, () => marineApi.faultHardwareDemo('现场演示员'))
    demo.value = injectActuatorFault(demo.value)
    await refreshRealMotors()
    motorMessage.value = '当前绑定电机已停止并恢复为 0 RPM，请选择在线备用电机'
  } catch (error) { motorMessage.value = error instanceof Error ? error.message : '模拟故障失败' }
  finally { motorBusy.value = false }
}
async function confirmSwitch() {
  if (!gatewayOnline.value || !candidate.value || candidate.value.status === 'offline') return
  const selectedCandidate = candidate.value
  motorBusy.value = true
  motorMessage.value = `正在更新逻辑绑定并启动 ${selectedCandidate.name}…`
  try {
    await traceControl('start', selectedCandidate.id, () => marineApi.switchHardwareDemo(selectedCandidate.id, { speedRpm: targetRpm.value, direction: targetDirection.value, operator: '现场演示员' }))
    demo.value = confirmRealActuatorSwitch(demo.value)
    selectedExecutorId.value = executorByAddress[selectedCandidate.address]!
    await refreshRealMotors()
    motorMessage.value = `${selectedCandidate.name} 已接管，应用软件与控制服务保持不变`
  } catch (error) { motorMessage.value = error instanceof Error ? error.message : '设备切换失败' }
  finally { motorBusy.value = false }
}
async function resetReplacementDemo() {
  if (motorBusy.value || !gatewayOnline.value) return
  motorBusy.value = true
  motorMessage.value = '正在恢复当前在线电机并初始化运行状态…'
  try {
    const result = await marineApi.resetHardwareDemo('现场演示员')
    const fresh = createActuatorDemo()
    const executorId = result?.motor?.executorId as string | undefined
    const boundDeviceId = result?.binding?.deviceId as ActuatorDevice['id'] | undefined
    const onlineDevice = demo.value.devices.find(device => executorByAddress[device.address] === executorId)
    demo.value = {
      ...fresh,
      mode: 'real',
      devices: demo.value.devices,
      logicalExecutor: { ...fresh.logicalExecutor, boundDeviceId: boundDeviceId ?? onlineDevice?.id ?? fresh.logicalExecutor.boundDeviceId },
    }
    if (executorId) selectedExecutorId.value = executorId
    targetRpm.value = 50
    targetDirection.value = 'forward'
    await refreshRealMotors()
    motorMessage.value = `演示已重置：${onlineDevice?.name ?? '当前在线电机'}以 50 RPM 正转`
  } catch (error) { motorMessage.value = error instanceof Error ? error.message : '重置演示失败' }
  finally { motorBusy.value = false }
}
async function loadCurrentBinding() {
  try {
    const bindings = await marineApi.listExecutors()
    const binding = bindings.find(item => item.logicalExecutorId === 'COOL-01')
    const deviceId = binding?.deviceId as ActuatorDevice['id'] | undefined
    if (deviceId && demo.value.devices.some(device => device.id === deviceId)) {
      demo.value = { ...demo.value, logicalExecutor: { ...demo.value.logicalExecutor, boundDeviceId: deviceId } }
      const bound = demo.value.devices.find(device => device.id === deviceId)!
      selectedExecutorId.value = executorByAddress[bound.address]!
    }
  } catch { /* 电机刷新会显示网关连接状态 */ }
}
onMounted(async () => { await loadCurrentBinding(); await refreshRealMotors(); timer = setInterval(() => { if (!motorBusy.value) void refreshRealMotors() }, 2000) })
onUnmounted(() => { disposed = true; clearTimeout(heartbeatTimer); clearInterval(timer); clearFlow(); clearQueryFlow() })
</script>

<template>
  <div ref="screen" class="pump-replica live-cooling">
    <svg v-if="packetPath" class="packet-overlay" aria-hidden="true" :style="{ color: packetColor }">
      <circle :key="packetKey" r="5" fill="currentColor"><animateMotion :path="packetPath" dur=".45s" fill="freeze" /></circle>
    </svg>
    <svg class="ambient-texture" aria-hidden="true"><filter id="pump-cloud"><feTurbulence type="fractalNoise" baseFrequency=".012 .019" numOctaves="3" seed="8"/><feColorMatrix type="saturate" values="0"/></filter><rect width="100%" height="100%" filter="url(#pump-cloud)"/></svg>
    <svg class="symbol-library" aria-hidden="true">
      <defs>
        <symbol id="pump-ship" viewBox="0 0 64 64"><path d="M10 29h44l-6 21H16zM21 29V15h23v14M32 15V5M13 35h38"/></symbol>
        <symbol id="pump-cube" viewBox="0 0 64 64"><path d="m10 20 22-12 22 12v25L32 58 10 45zM10 20l22 13 22-13M32 33v25"/></symbol>
        <symbol id="pump-gear" viewBox="0 0 64 64"><path d="m27 3 10 0 2 8 5 3 8-3 6 8-5 6 1 6 8 4-2 10-9 0-4 5 1 8-9 4-5-7-6-1-6 6-9-5 2-9-4-4-8-1-1-10 8-3 3-5-3-8 8-6 6 5 6-1z"/><circle cx="32" cy="33" r="12" fill="#001d29"/></symbol>
      </defs>
    </svg>

    <header class="replica-header">
      <div><span class="brand-diamond">◈</span><h1>中央冷却泵替换</h1><span class="brand-divider"></span><span>软硬解耦</span></div>
      <span :class="{ 'gateway-connected': gatewayOnline }">{{ gatewayOnline ? '现场演示 · 网关在线' : '网关连接中 / 数据不可用' }}</span>
    </header>

    <main class="replica-architecture" aria-label="中央冷却泵四层架构">
      <section class="replica-tier app-tier">
        <div class="replica-tier-label"><b>04</b><h2>应用软件</h2></div>
        <div class="replica-panel upper-panel">
          <div class="replica-node"><span class="replica-icon"><svg><use href="#pump-ship"/></svg></span><div><h3>中央冷却应用</h3><p>应用定义保持不变</p></div></div>
        </div>
      </section>
      <section class="replica-tier service-tier" :class="{ 'flow-lit': flowStage === 'service' }">
        <div class="replica-tier-label"><b>03</b><h2>软件模块</h2></div>
        <div class="replica-panel upper-panel">
          <div class="replica-node"><span class="replica-icon"><svg><use href="#pump-cube"/></svg></span><div><h3>冷却泵控制服务</h3><p>启停 · 调速 · 状态反馈</p></div></div>
        </div>
      </section>
      <section class="replica-tier abstract-tier">
        <div class="replica-tier-label"><b>02</b><h2>设备抽象</h2></div>
        <div class="replica-panel abstraction-box">
          <div class="abstraction-heading"><span class="replica-icon"><svg><use href="#pump-cube"/></svg></span><h3>冷却泵抽象</h3></div>
          <div class="replica-capability parameters live-parameters" :class="{ 'feedback-updated': flowStage === 'feedback' }" aria-label="设备实时运行参数">
            <h4><svg class="gear"><use href="#pump-gear"/></svg><span>运行参数<small>{{ selectedMotor?.name }} · {{ selectedMotor?.address }}</small></span></h4>
            <div class="live-metric"><span>实时转速</span><strong data-testid="actual-rpm">{{ live.rpm ?? '--' }} <small>RPM</small></strong></div>
            <div class="live-metric"><span>实时转向</span><strong data-testid="actual-direction">{{ live.direction }}</strong></div>
            <div class="live-metric"><span>运行状态</span><strong data-testid="running-state" :class="{ 'status-running': live.state === '运行中' }">{{ live.state }}</strong></div>
            <div class="live-metric"><span>是否在线</span><strong data-testid="online-state" class="connection-status" :class="{ online: live.online, offline: live.connection === '离线' }"><i></i>{{ live.connection }}</strong></div>
            <div class="live-motor-actions" aria-label="电机启停控制">
              <button class="start-button" :disabled="!canStart" @click="startMotor">启动电机</button>
              <button class="stop-button" :disabled="motorBusy || !gatewayOnline || !live.online" @click="stopMotor">停止电机</button>
            </div>
          </div>
          <div class="replica-capability driver" :class="{ 'interfaces-inactive': !interfacesActive }">
            <h4><svg class="gear"><use href="#pump-gear"/></svg>电机驱动能力</h4>
            <div class="replica-api"><h5>控制接口</h5><div class="api-buttons"><span v-for="command in (['start', 'stop', 'set_speed', 'set_direction'] as const)" :key="command" :data-command="command" :class="{ 'flow-lit': interfacesActive && flowStage === 'function' && flowCommand === command }">{{ command }} <small>{{ { start: '启动', stop: '停止', set_speed: '设置转速', set_direction: '设置转向' }[command] }}</small></span></div></div>
            <div class="replica-api query"><h5>查询接口 <i class="heartbeat-dot" :class="{ received: heartbeat }" title="心跳响应" /></h5><button :disabled="motorBusy || queryBusy" :class="{ 'query-lit': interfacesActive && flowCommand === 'get_status' && ['function', 'return'].includes(flowStage), 'flow-error': interfacesActive && queryStage === 'error' }" @click="refreshRealMotors(true)">get_status <span>获取状态</span></button></div>
          </div>
        </div>
      </section>
      <section class="replica-tier middleware-tier" aria-label="协议适配中间件">
          <div class="replica-adapter standalone-adapter" :class="{ 'flow-lit': flowStage === 'adapter', 'query-lit': ['adapter', 'return'].includes(queryStage) }"><span class="adapter-arrows" aria-hidden="true">⇆</span><div><h4>协议适配标准中间件（翻译官）</h4><p>统一指令与状态 ↔ 不同设备协议</p></div></div>
      </section>
      <section class="replica-tier motors-tier">
        <div class="replica-tier-label"><b>01</b><h2>现场硬件设备</h2></div>
        <div class="replica-motors">
          <button v-for="device in demo.devices" :key="device.id" class="replica-motor" :data-device="device.id" :class="{ 'device-online': motorPresentation(device, gatewayOnline).online, 'device-offline': !motorPresentation(device, gatewayOnline).online, candidate: demo.candidateId === device.id, selected: selectedMotor?.id === device.id, 'flow-lit': flowTarget === device.id && ['motor', 'complete'].includes(flowStage), 'flow-error': flowTarget === device.id && flowStage === 'error', 'flow-travel': flowTarget === device.id && flowStage === 'motor', 'query-lit': queryTarget === device.id && queryStage === 'motor', 'query-outbound': queryTarget === device.id && queryStage === 'motor', 'query-return': queryTarget === device.id && queryStage === 'return' }" :aria-label="`选择${device.name}`" :aria-pressed="selectedMotor?.id === device.id" @click="select(device)">
            <span class="motor-connector" aria-hidden="true"><i></i></span>
            <span class="motor-live-status" :class="{ online: motorPresentation(device, gatewayOnline).online }"><i></i>{{ motorPresentation(device, gatewayOnline).connection }}<small v-if="demo.logicalExecutor.boundDeviceId === device.id"> · 当前绑定</small></span>
            <span class="replica-motor-picture"><img :src="motorImage" alt="蓝色工业电机"/><strong>{{ device.name }}</strong></span><span class="replica-protocol">{{ hardwareDisplayProtocol(device.address) }}</span>
          </button>
        </div>
      </section>
    </main>

    <footer class="replica-toolbar">
      <div class="replica-controls">
        <strong>真实电机控制</strong>
        <select v-model="selectedExecutorId" :disabled="motorBusy" aria-label="选择控制电机" @change="selectedMotor && select(selectedMotor)"><option v-for="device in demo.devices" :key="device.id" :value="executorByAddress[device.address]">{{ device.name }}</option></select>
        <label><input v-model.number="targetRpm" :disabled="motorBusy" @change="previewParameter('set_speed')" type="number" min="1" max="500" aria-label="设定转速"/><span>RPM</span></label>
        <select v-model="targetDirection" :disabled="motorBusy" @change="previewParameter('set_direction')" aria-label="设定转向"><option value="forward">正转</option><option value="reverse">反转</option></select>
      </div>
      <p class="replica-message" role="status" :title="motorMessage">{{ motorMessage }}</p>
      <div class="replica-switch" :title="phaseLabel"><button class="red-button" :disabled="!canProtect || !gatewayOnline || motorBusy" @click="simulateFault">模拟故障</button><button class="green-button" :disabled="!gatewayOnline || !candidate || candidate.status === 'offline' || demo.phase !== 'awaiting-confirmation' || motorBusy" @click="confirmSwitch">确认切换</button><button :disabled="!gatewayOnline || motorBusy" @click="resetReplacementDemo">重置演示</button></div>
      <nav><RouterLink to="/">软件组合</RouterLink><span>/</span><RouterLink to="/legacy-dashboard">系统总图</RouterLink></nav>
    </footer>
    <DecouplingSwitch :disabled="motorBusy" />
  </div>
</template>

<style scoped src="../styles/cooling-replica.css"></style>
<style scoped src="../styles/cooling-live.css"></style>
