<script setup lang="ts">
import { computed, onMounted, onUnmounted, ref } from 'vue'
import { confirmRealActuatorSwitch, connectActuator, createActuatorDemo, mergeRealMotorTelemetry, syncRealRemovalState } from '../marine/actuators'
import type { ActuatorDevice } from '../marine/actuators'
import { marineApi } from '../marine/api'
import '../styles/hardware-topology.css'

const demo = ref(createActuatorDemo())
const gatewayOnline = ref(false)
const selectedExecutorId = ref('FIRE-PUMP-01')
const targetRpm = ref(200)
const targetDirection = ref<'forward' | 'reverse'>('forward')
const motorBusy = ref(false)
const motorMessage = ref('请选择在线电机并设置运行参数')
const executorByAddress: Record<string, string> = { '0x02': 'EXHAUST-FAN-01', '0x0E': 'FIRE-PUMP-01', '0x0F': 'DRAIN-PUMP-01' }
const selectedMotor = computed(() => demo.value.devices.find(device => executorByAddress[device.address] === selectedExecutorId.value))
const candidate = computed(() => demo.value.devices.find(d => d.id === demo.value.candidateId))
const canProtect = computed(() => ['running', 'recovered'].includes(demo.value.phase))
const phaseLabel = computed(() => ({
  running: '冷却任务运行中', protected: '请选择替换水泵',
  'awaiting-confirmation': '等待确认切换', switching: '正在恢复控制', recovered: '冷却任务已恢复',
}[demo.value.phase]))
const positions = [420, 780, 1140]
const protocols = ['CAN', 'Modbus', 'EtherCAT']
function selectable(d: ActuatorDevice) {
  return ['protected', 'awaiting-confirmation'].includes(demo.value.phase) && d.id !== demo.value.logicalExecutor.boundDeviceId && d.compatible
}
function select(d: ActuatorDevice) {
  selectedExecutorId.value = executorByAddress[d.address]!
  if (selectable(d)) demo.value = connectActuator(demo.value, d.id)
}
async function startMotor() {
  if (!selectedMotor.value?.status || selectedMotor.value.status === 'offline') { motorMessage.value = '所选电机当前离线，无法启动'; return }
  if (targetRpm.value < 1 || targetRpm.value > 500) { motorMessage.value = 'RPM 必须设置在 1–500 之间'; return }
  motorBusy.value = true; motorMessage.value = '控制指令下发中…'
  try {
    await marineApi.startMotor(selectedExecutorId.value, { speedRpm: targetRpm.value, direction: targetDirection.value, acceleration: 10, deceleration: 10, operator: '现场演示员' })
    motorMessage.value = `已下发 ${targetDirection.value === 'reverse' ? '反转' : '正转'} ${targetRpm.value} RPM`
    await refreshRealMotors()
  } catch (error) { motorMessage.value = error instanceof Error ? error.message : '电机启动失败' }
  finally { motorBusy.value = false }
}
async function stopMotor() {
  motorBusy.value = true; motorMessage.value = '停止指令下发中…'
  try { await marineApi.stopDirectMotor(selectedExecutorId.value, '现场演示员'); motorMessage.value = '电机已停止'; await refreshRealMotors() }
  catch (error) { motorMessage.value = error instanceof Error ? error.message : '电机停止失败' }
  finally { motorBusy.value = false }
}
function routeState(d: ActuatorDevice) {
  if (d.id === demo.value.candidateId) return 'pending'
  if (d.id !== demo.value.logicalExecutor.boundDeviceId) return 'standby'
  return demo.value.phase === 'switching' ? 'pending' : d.status === 'offline' ? 'broken' : 'live'
}
function branchPath(x: number) {
  if (x === 780) return 'M 780 420 L 780 532'
  return `M 780 420 L 780 444 Q 780 462 ${x < 780 ? 762 : 798} 462 L ${x < 780 ? x + 18 : x - 18} 462 Q ${x} 462 ${x} 480 L ${x} 532`
}
let timer: ReturnType<typeof setInterval>
let refreshBusy = false
async function refreshRealMotors() {
  if (refreshBusy) return
  refreshBusy = true
  try {
    const motors = await marineApi.listMotors()
    gatewayOnline.value = true
    demo.value = syncRealRemovalState({ ...demo.value, mode: 'real', devices: mergeRealMotorTelemetry(demo.value.devices, motors) })
  } catch { gatewayOnline.value = false }
  finally { refreshBusy = false }
}
async function checkRemovedMotor() {
  motorMessage.value = '正在读取当前绑定电机…'
  await refreshRealMotors()
  motorMessage.value = demo.value.phase === 'protected'
    ? '已检测到电机离线，请选择在线备用设备'
    : '当前绑定电机仍在线，请先断开其 RS485 或电源'
}
function confirmSwitch() {
  demo.value = confirmRealActuatorSwitch(demo.value)
  const bound = demo.value.devices.find(device => device.id === demo.value.logicalExecutor.boundDeviceId)
  if (bound) selectedExecutorId.value = executorByAddress[bound.address]!
  motorMessage.value = demo.value.lastMessage
}
onMounted(() => { refreshRealMotors(); timer = setInterval(refreshRealMotors, 5000) })
onUnmounted(() => clearInterval(timer))
</script>

<template>
  <div class="cooling-screen">
    <header class="cooling-header">
      <div class="cooling-heading"><span class="cooling-mark">◈</span><h1>中央冷却泵替换</h1><span class="cooling-submark">软硬解耦</span></div>
      <span class="cooling-mode">{{ gatewayOnline ? '真实网关 · RS485' : '网关连接中' }}</span>
    </header>
    <main class="cooling-canvas">
      <svg class="cooling-topology" viewBox="0 0 1400 780" role="img" aria-label="中央冷却应用通过原子服务和设备资源池连接三台水泵">
        <defs>
          <radialGradient id="cool-node"><stop stop-color="#183e51"/><stop offset="1" stop-color="#0b2030"/></radialGradient>
          <linearGradient id="cool-pool" x2="0" y2="1"><stop stop-color="#102738" stop-opacity=".5"/><stop offset="1" stop-color="#081623" stop-opacity=".2"/></linearGradient>
          <filter id="cool-glow" x="-100%" y="-100%" width="300%" height="300%"><feGaussianBlur stdDeviation="3"/></filter>
        </defs>

        <g class="tier-label"><text x="40" y="100" class="tier-number">04</text><text x="40" y="133">应用层</text><path d="M40 152H172"/></g>
        <g class="tier-label"><text x="40" y="251" class="tier-number">03</text><text x="40" y="284">原子服务层</text><path d="M40 303H172"/></g>
        <g class="tier-label"><text x="40" y="399" class="tier-number">02</text><text x="40" y="432">设备资源池</text><path d="M40 451H172"/></g>
        <g class="tier-label"><text x="40" y="590" class="tier-number">01</text><text x="40" y="623">执行器硬件层</text><path d="M40 642H172"/></g>

        <g class="upper-wires">
          <path d="M780 141V219 M780 301V377" class="wire"/>
          <path d="M780 141V219 M780 301V377" class="wire-stream"/>
          <path d="m776 187 4 5 4-5 m-8 151 4 5 4-5" class="arrow"/>
        </g>
        <text x="815" y="182" class="link-caption">任务调用</text>
        <text x="815" y="345" class="link-caption">统一能力接口</text>

        <g class="topology-node">
          <circle cx="780" cy="100" r="41" fill="url(#cool-node)"/>
          <path d="M761 95h38l-6 17h-26z M770 94V82h20v12 M780 82v-8" class="node-icon"/>
          <circle cx="780" cy="141" r="4" class="port"/>
          <text x="845" y="99" class="node-title">中央冷却应用</text>
          <text x="845" y="126" class="node-detail">应用定义保持不变</text>
        </g>
        <g class="topology-node">
          <circle cx="780" cy="260" r="41" fill="url(#cool-node)"/>
          <path d="m780 238 19 11v22l-19 11-19-11v-22z m-19 11 19 11 19-11 m-19 11v22" class="node-icon"/>
          <circle cx="780" cy="219" r="4" class="port"/><circle cx="780" cy="301" r="4" class="port"/>
          <text x="845" y="257" class="node-title">冷却泵控制服务</text>
          <text x="845" y="285" class="node-detail">启停 · 调速 · 状态反馈</text>
        </g>

        <rect x="225" y="363" width="1110" height="393" rx="24" fill="url(#cool-pool)" class="pool-outline"/>
        <g class="resource-hub"><rect x="660" y="377" width="240" height="43" rx="21.5"/><text x="780" y="405" text-anchor="middle">冷却泵 · 统一资源入口</text></g>
        <text x="265" y="405" class="pool-caption">可替换设备 · 3</text>
        <g v-for="(device, index) in demo.devices" :key="'wire-'+device.id" :class="['pump-branch', routeState(device)]">
          <path :d="branchPath(positions[index]!)" class="wire"/>
          <path v-if="['live','pending'].includes(routeState(device))" :d="branchPath(positions[index]!)" class="wire-stream"/>
          <circle :cx="positions[index]" cy="532" r="4" class="port"/>
        </g>

        <g v-for="(device, index) in demo.devices" :key="device.id"
          :transform="`translate(${positions[index]}, 580)`"
          :class="['pump-node', routeState(device), { selectable: selectable(device), 'control-selected': executorByAddress[device.address] === selectedExecutorId }]"
          role="button" :tabindex="selectable(device) ? 0 : -1" :aria-disabled="!selectable(device)" :aria-label="'选择'+device.name"
          @click="select(device)" @keydown.enter.prevent="select(device)" @keydown.space.prevent="select(device)">
          <rect x="-161" y="-62" width="322" height="229" rx="16" class="pump-hit"/>
          <circle r="45" class="pump-ring"/><circle r="34" fill="url(#cool-node)"/>
          <g class="pump-rotor" :class="{ spinning: device.status === 'running', reverse: device.speed < 0 }">
            <path d="M0-22C18-22 22-5 7 0C22 15 7 25 0 7C-18 23-25 5-7 0C-23-17-5-25 0-7Z"/>
            <circle r="5"/>
          </g>
          <text x="-126" y="-23" class="pump-name">{{ device.name }}</text>
          <text x="63" y="-23" class="pump-status">{{ device.status === 'offline' ? '离线' : device.status === 'running' ? '在线运行' : '在线待机' }}</text>
          <text x="63" y="3" class="pump-role">{{ routeState(device) === 'live' ? '当前绑定' : routeState(device) === 'pending' ? '待切换' : selectable(device) ? '点击选择' : '备用设备' }}</text>
          <text x="0" y="88" text-anchor="middle" class="pump-rpm">{{ Math.abs(device.speed) }}<tspan class="rpm-unit"> RPM</tspan></text>
          <text x="0" y="112" text-anchor="middle" class="pump-direction">{{ !device.speed ? '已停止' : device.speed < 0 ? '反转' : '正转' }}</text>
          <text x="0" y="137" text-anchor="middle" class="rs485-label">RS485 · {{ device.address }}</text>
          <text x="0" y="157" text-anchor="middle" class="protocol-options">{{ protocols.join('  /  ') }}</text>
        </g>
        <text x="1310" y="778" text-anchor="end" class="protocol-note">灰色接口：扩展能力示意，尚未接入</text>
      </svg>
    </main>
    <footer class="cooling-toolbar">
      <div class="motor-control-panel">
        <span>真实电机控制</span>
        <select v-model="selectedExecutorId">
          <option value="EXHAUST-FAN-01">水泵 A · 0x02</option>
          <option value="FIRE-PUMP-01">水泵 B · 0x0E</option>
          <option value="DRAIN-PUMP-01">水泵 C · 0x0F</option>
        </select>
        <label><input v-model.number="targetRpm" type="number" min="1" max="500"/> RPM</label>
        <select v-model="targetDirection"><option value="forward">正转</option><option value="reverse">反转</option></select>
        <button class="start-motor" :disabled="motorBusy || selectedMotor?.status === 'offline' || targetRpm < 1 || targetRpm > 500" @click="startMotor">启动</button>
        <button class="stop-motor" :disabled="motorBusy" @click="stopMotor">停止</button>
        <small>{{ motorMessage }}</small>
      </div>
      <div class="cooling-operation">
        <span class="operation-state">{{ phaseLabel }}</span>
        <button :disabled="!canProtect || !gatewayOnline" @click="checkRemovedMotor">检测拔出</button>
        <button class="confirm-pump" :disabled="!candidate || demo.phase !== 'awaiting-confirmation'" @click="confirmSwitch">确认切换</button>
      </div>
      <nav><RouterLink to="/">软件组合 ↗</RouterLink><RouterLink to="/legacy-dashboard">系统总图 ↗</RouterLink></nav>
    </footer>
  </div>
</template>
