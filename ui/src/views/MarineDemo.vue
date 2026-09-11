<script setup lang="ts">
import { ref, shallowRef, computed, onMounted, onUnmounted, watch } from 'vue'
import { useRoute } from 'vue-router'
import { services, serviceById, createLeadershipDemo, createPreset, createStep, createNode, uid, areas, validateApp, plannedDuration, parseSavedApps } from '../marine/model'
import type { MarineApp, TaskNode } from '../marine/model'
import { MarineRunner, emptyRun } from '../marine/runner'
import type { RunState } from '../marine/runner'
import { demoSensorSnapshot, normalizeSensorSnapshot } from '../marine/sensors'
import type { SensorSnapshot } from '../marine/sensors'
import { realtimeApi } from '../api/realtime'
import { marineApi } from '../marine/api'
import ShipDiagram from '../components/marine/ShipDiagram.vue'
import ApplicationFlow from '../components/marine/ApplicationFlow.vue'
import ApplicationCommandCenter from '../components/marine/ApplicationCommandCenter.vue'
import '../styles/marine.css'

const route = useRoute()
const tab = ref<'compose' | 'run' | 'catalog'>(route.path === '/atomic-services' ? 'catalog' : 'compose')
watch(() => route.path, path => { tab.value = path === '/atomic-services' ? 'catalog' : 'compose' })
const app = ref<MarineApp>(createPreset('D'))
const original = ref(JSON.stringify(app.value))
const selectedId = ref(app.value.steps[0]!.nodes[0]!.id)
const selectedStep = ref(app.value.steps[0]!.id)
const search = ref('')
const catalogFilter = ref('全部服务')
const savedApps = ref<MarineApp[]>([])
const libraryExpanded = ref(false)
const awarenessExpanded = ref(true)
const actionExpanded = ref(true)
const notice = ref('')
const noticeError = ref(false)
const confirmation = ref<HTMLDialogElement | null>(null)
const savedDialog = ref<HTMLDialogElement | null>(null)
const journeyStage = ref(1)
const leadershipBusy = ref(false)
const generationPreview = ref<{ mode: 'single' | 'leadership'; apps: MarineApp[]; reusedServiceIds: string[] } | null>(null)
let pendingAction: (() => void) | undefined
let toastTimer: ReturnType<typeof setTimeout> | undefined
let generationTimer: ReturnType<typeof setTimeout> | undefined
interface RuntimeSession {
  id: string
  app: MarineApp
  runner: MarineRunner
  state: RunState
  lastTick: number
}
const runtimeSessions = shallowRef<RuntimeSession[]>([])
const activeRuntimeId = ref('')
const idleRun = emptyRun()
const activeSession = computed(() => runtimeSessions.value.find(session => session.id === activeRuntimeId.value))
const editingSession = computed(() => runtimeSessions.value.find(session => session.id === app.value.id))
const run = computed(() => runtimeSessions.value.find(session => session.id === activeRuntimeId.value)?.state ?? idleRun)
let timer: ReturnType<typeof setInterval> | undefined
let sensorTimer: ReturnType<typeof setInterval> | undefined
let gatewayTimer: ReturnType<typeof setInterval> | undefined
const fullscreen = ref(false)
const gatewayConnected = ref(false)
const motorMode = ref<'real' | 'simulation'>('simulation')
const emergencyStopped = ref(false)
const motors = ref<any[]>([])
const gatewayRunIds = new Map<string, string>()
const sensorSnapshot = ref<SensorSnapshot>(demoSensorSnapshot())
const sensorApiConnected = ref(false)
const busy = computed(() => ['running', 'paused'].includes(run.value.status))
const editingBusy = computed(() => ['running', 'paused'].includes(runtimeSessions.value.find(session => session.id === app.value.id)?.state.status || 'idle'))
const activeRuntimeCount = computed(() => runtimeSessions.value.filter(session => ['running', 'paused'].includes(session.state.status)).length)
const applicationPages = computed(() => {
  const pages = new Map<string, MarineApp>()
  savedApps.value.forEach(saved => pages.set(saved.id, saved))
  runtimeSessions.value.forEach(session => pages.set(session.id, session.app))
  return [...pages.values()]
})
const dirty = computed(() => JSON.stringify(app.value) !== original.value)
const nodes = computed(() => app.value.steps.flatMap(step => step.nodes))
const selected = computed(() => nodes.value.find(node => node.id === selectedId.value))
const errors = computed(() => validateApp(app.value))
const duration = computed(() => plannedDuration(app.value))
const runNodes = computed(() => Object.values(run.value.nodes))
const awarenessServices = computed(() => services.filter(service => service.available && service.kind === 'awareness'))
const actionServices = computed(() => services.filter(service => service.available && service.kind === 'action'))
const onlineSensorCount = computed(() => Object.values(sensorSnapshot.value.values).filter(sensor => sensor.online).length)
const completedCount = computed(() => runNodes.value.filter(node => node.status === 'completed').length)
const progress = computed(() => runNodes.value.length ? Math.round(runNodes.value.reduce((sum, node) => sum + node.progress, 0) / runNodes.value.length * 100) : 0)
const filteredServices = computed(() => services.filter(service => (service.name + service.description + service.id).includes(search.value) && (catalogFilter.value === '全部服务' || (catalogFilter.value === '可运行' ? service.available : !service.available))))
const reused = computed(() => [...new Set(nodes.value.map(node => node.serviceId))].filter(id => ['01', '04', '05'].includes(id)))
const runReused = computed(() => [...new Set(runNodes.value.map(item => item.node.serviceId))].filter(id => ['01', '04', '05'].includes(id)))
const generationReuseNames = computed(() => generationPreview.value?.reusedServiceIds.map(id => `${id} ${serviceById(id).name}`).join('　＋　') || '')
const statusLabels = { idle: '等待运行', running: '正在执行', paused: '已暂停', completed: '任务完成', cancelled: '已停止', failed: '执行异常' }
const nodeLabels = { waiting: '等待执行', running: '运行中', completed: '已完成', cancelled: '已取消', failed: '异常' }
const storageKey = 'marine-demo.apps.v1'
function notify(message: string, error = false) {
  notice.value = message; noticeError.value = error
  clearTimeout(toastTimer); toastTimer = setTimeout(() => { notice.value = '' }, 5000)
}
function replaceApp(next: MarineApp) {
  if (editingBusy.value) return
  app.value = JSON.parse(JSON.stringify(next)); original.value = JSON.stringify(app.value)
  selectedId.value = app.value.steps[0]?.nodes[0]?.id || ''; selectedStep.value = app.value.steps[0]?.id || ''
  tab.value = 'compose'
}
function requestReplace(next: MarineApp) {
  if (editingBusy.value) return
  if (!dirty.value) { replaceApp(next); return }
  pendingAction = () => replaceApp(next); confirmation.value?.showModal()
}
function confirmReplace() { confirmation.value?.close(); pendingAction?.(); pendingAction = undefined }
function newApp() { journeyStage.value = 1; requestReplace({ id: uid(), name: '未命名应用', description: '请填写任务目标，并从左侧添加需要的服务。', steps: [createStep()] }) }
function selectPreset(which: 'A' | 'B' | 'C' | 'D' | 'E') { journeyStage.value = 2; requestReplace(createPreset(which)) }
function chooseNode(node: TaskNode, stepId: string) { selectedId.value = node.id; selectedStep.value = stepId }
function addService(serviceId: string, targetId = selectedStep.value) {
  if (editingBusy.value) return
  if (!serviceById(serviceId)?.available) { notify('这项服务尚未实现，当前仅展示能力规划。'); return }
  let step = app.value.steps.find(item => item.id === targetId)
  if (!step) { step = createStep(); app.value.steps.push(step) }
  const node = createNode(serviceId); step.nodes.push(node); chooseNode(node, step.id)
  journeyStage.value = 2
}
function addServiceFromSelect(event: Event, targetId: string) {
  const select = event.target as HTMLSelectElement
  if (select.value) addService(select.value, targetId)
  select.value = ''
}
function addStep() { if (!editingBusy.value) { const step = createStep(); app.value.steps.push(step); selectedStep.value = step.id; selectedId.value = '' } }
function removeNode(id: string) {
  if (editingBusy.value) return
  for (const step of app.value.steps) step.nodes = step.nodes.filter(node => node.id !== id)
  if (selectedId.value === id) selectedId.value = ''
}
function removeStep(id: string) {
  if (editingBusy.value) return
  app.value.steps = app.value.steps.filter(step => step.id !== id)
  if (selectedStep.value === id) { selectedStep.value = app.value.steps[0]?.id || ''; selectedId.value = '' }
}
function moveStep(id: string, offset: number) {
  if (editingBusy.value) return
  const index = app.value.steps.findIndex(step => step.id === id); const target = index + offset
  if (index < 0 || target < 0 || target >= app.value.steps.length) return
  const [step] = app.value.steps.splice(index, 1); app.value.steps.splice(target, 0, step!)
}
function moveNode(id: string, targetId: string) {
  if (editingBusy.value) return
  const source = app.value.steps.find(step => step.nodes.some(node => node.id === id))
  if (!source || source.id === targetId) return
  const node = source.nodes.find(node => node.id === id)!
  let target = app.value.steps.find(step => step.id === targetId)
  if (!target) { target = createStep(); app.value.steps.push(target) }
  source.nodes = source.nodes.filter(item => item.id !== id); target.nodes.push(node)
  if (!source.nodes.length) app.value.steps = app.value.steps.filter(step => step.id !== source.id)
  chooseNode(node, target.id)
}
function drag(event: DragEvent, kind: string, id: string) {
  if (editingBusy.value) { event.preventDefault(); return }
  event.dataTransfer?.setData('application/x-marine-service', JSON.stringify({ kind, id }))
  if (event.dataTransfer) event.dataTransfer.effectAllowed = kind === 'service' ? 'copy' : 'move'
}
function drop(event: DragEvent, targetId = '') {
  if (editingBusy.value) return
  try {
    const { kind, id } = JSON.parse(event.dataTransfer?.getData('application/x-marine-service') || '{}')
    if (typeof id !== 'string') return
    if (kind === 'service') addService(id, targetId)
    if (kind === 'node') moveNode(id, targetId)
    if (kind === 'step') {
      const index = app.value.steps.findIndex(step => step.id === id)
      const target = targetId ? app.value.steps.findIndex(step => step.id === targetId) : app.value.steps.length - 1
      if (index >= 0 && target >= 0) moveStep(id, target - index)
    }
  } catch { notify('未识别到可添加的服务，请从服务库拖入。', true) }
}
function saveApp() {
  if (editingBusy.value) return
  if (errors.value.length) { notify(errors.value[0]!, true); return }
  const snapshot: MarineApp = JSON.parse(JSON.stringify(app.value))
  const list = savedApps.value.filter(item => item.id !== snapshot.id); list.unshift(snapshot)
  try { localStorage.setItem(storageKey, JSON.stringify(list)); savedApps.value = list; original.value = JSON.stringify(app.value); const session = ensureRuntime(snapshot); if (!activeRuntimeId.value) activeRuntimeId.value = session.id; window.dispatchEvent(new CustomEvent('marine-apps-updated')); journeyStage.value = 3; showGeneration([snapshot], 'single', []); notify('应用已生成：软件基座新增了一个独立应用页面。') }
  catch { notify('本机存储不可用或空间不足，应用尚未保存。', true) }
  if (gatewayConnected.value) marineApi.createApp(snapshot).catch(() => notify('网关保存失败，已保留本机应用。', true))
}
function loadApp(saved: MarineApp) { savedDialog.value?.close(); requestReplace(saved) }
function cloneApp(target: MarineApp): MarineApp { return JSON.parse(JSON.stringify(target)) }
function ensureRuntime(target: MarineApp) {
  let session = runtimeSessions.value.find(item => item.id === target.id)
  if (!session) {
    session = { id: target.id, app: cloneApp(target), runner: new MarineRunner(), state: emptyRun(), lastTick: performance.now() }
    runtimeSessions.value = [...runtimeSessions.value, session]
  } else if (!['running', 'paused'].includes(session.state.status)) {
    session.app = cloneApp(target)
    runtimeSessions.value = [...runtimeSessions.value]
  }
  return session
}
function syncRun(session: RuntimeSession) { session.state = JSON.parse(JSON.stringify(session.runner.state)); runtimeSessions.value = [...runtimeSessions.value] }
function runtimeStatus(id: string): RunState['status'] { return runtimeSessions.value.find(session => session.id === id)?.state.status || 'idle' }
function selectApplicationPage(target: MarineApp) {
  const session = ensureRuntime(target)
  activeRuntimeId.value = session.id
  tab.value = 'run'
}
function gatewayRunToState(remote: any, target: MarineApp): RunState {
  const nodes: RunState['nodes'] = {}
  for (const node of target.steps.flatMap(step => step.nodes)) {
    const source = remote.nodes?.[node.id] || {}
    nodes[node.id] = { node, status: ['waiting', 'running', 'completed', 'cancelled', 'failed'].includes(source.status) ? source.status : 'waiting', elapsed: Number(source.elapsed || 0), progress: Number(source.progress || 0), result: source.result || { metric: '', value: '', unit: '', detail: '', source: '' } }
  }
  return { status: ['idle', 'running', 'paused', 'completed', 'cancelled', 'failed'].includes(remote.status) ? remote.status : 'failed', app: target, stepIndex: Number(remote.stepIndex || 0), elapsed: Number(remote.elapsed || 0), nodes, events: Array.isArray(remote.events) ? remote.events : [] }
}
async function syncGatewayRuns() {
  if (!gatewayConnected.value) return
  try {
    const runs = await marineApi.listRuns()
    for (const remote of runs) {
      const target = savedApps.value.find(item => item.id === remote.appId) || (app.value.id === remote.appId ? app.value : undefined)
      if (!target) continue
      const session = ensureRuntime(target)
      session.state = gatewayRunToState(remote, target)
      gatewayRunIds.set(target.id, remote.id)
    }
    runtimeSessions.value = [...runtimeSessions.value]
    motors.value = await marineApi.listMotors()
  } catch { gatewayConnected.value = false; notify('网关连接中断，后续操作将使用本地演示。', true) }
}
const executorNames: Record<string,string> = { 'EXHAUST-FAN-01': '机舱排烟风机', 'FIRE-PUMP-01': '消防水泵', 'DRAIN-PUMP-01': '舱底排水泵' }
async function applyMotor(node: TaskNode) { const runId = activeSession.value ? gatewayRunIds.get(activeSession.value.app.id) : ''; if (!runId || !node.motor) return; try { const remote = await marineApi.updateMotor(runId, node.id, node.motor); activeSession.value!.state = gatewayRunToState(remote, activeSession.value!.app); motors.value = await marineApi.listMotors(); runtimeSessions.value = [...runtimeSessions.value]; notify('电机参数已下发。') } catch (error) { notify(error instanceof Error ? error.message : '电机参数下发失败', true) } }
async function stopMotorNode(node: TaskNode) { const runId = activeSession.value ? gatewayRunIds.get(activeSession.value.app.id) : ''; if (!runId) return; try { await marineApi.stopMotor(runId, node.id); motors.value = await marineApi.listMotors(); notify('电机停止指令已下发。') } catch (error) { notify(error instanceof Error ? error.message : '电机停止失败', true) } }
async function triggerEmergency() {
  if (!confirm('确认立即停止三台电机并锁定执行器？')) return
  emergencyStopped.value = true
  try { await marineApi.emergencyStop('演示员'); notify('执行器急停已触发。', true) }
  catch (error) { notify(error instanceof Error ? `${error.message} 急停保持锁定，请检查现场。` : '急停未全部确认，请检查现场。', true) }
  try { motors.value = await marineApi.listMotors(); const health: any = await marineApi.health(); emergencyStopped.value = health?.emergencyStopped !== false }
  catch { emergencyStopped.value = true }
}
async function resetEmergency() { await marineApi.emergencyReset('演示员'); emergencyStopped.value = false; notify('执行器急停已复位。') }
async function startApplication(target: MarineApp) {
  try {
    const session = ensureRuntime(target)
    if (gatewayConnected.value) {
      await marineApi.createApp(target)
      const remote = await marineApi.startRun(target.id)
      gatewayRunIds.set(target.id, remote.id)
      session.state = gatewayRunToState(remote, target)
    } else session.runner.start(target, sensorSnapshot.value)
    session.app = cloneApp(target)
    session.lastTick = performance.now()
    if (!gatewayConnected.value) syncRun(session)
    activeRuntimeId.value = session.id
    tab.value = 'run'
    journeyStage.value = 4
  } catch (error) { notify(error instanceof Error ? error.message : '任务无法启动', true) }
}
function showGeneration(apps: MarineApp[], mode: 'single' | 'leadership', reusedServiceIds: string[]) {
  clearTimeout(generationTimer)
  generationPreview.value = { mode, apps: apps.map(cloneApp), reusedServiceIds }
  generationTimer = setTimeout(() => {
    generationPreview.value = null
    if (mode === 'leadership') { leadershipBusy.value = false; tab.value = 'run' }
  }, mode === 'leadership' ? 2800 : 1900)
}
async function runLeadershipDemo() {
  if (leadershipBusy.value) return
  leadershipBusy.value = true
  try {
    const demo = createLeadershipDemo()
    const demoNames = new Set(demo.apps.map(item => item.name))
    const list = [...demo.apps, ...savedApps.value.filter(item => !demoNames.has(item.name))]
    localStorage.setItem(storageKey, JSON.stringify(list))
    savedApps.value = list
    runtimeSessions.value.forEach(session => {
      if (demoNames.has(session.app.name)) session.runner.cancel()
    })
    runtimeSessions.value = runtimeSessions.value.filter(session => !demoNames.has(session.app.name))
    for (const target of demo.apps) {
      const session = ensureRuntime(target)
      if (gatewayConnected.value) {
        await marineApi.createApp(target)
        const remote = await marineApi.startRun(target.id)
        gatewayRunIds.set(target.id, remote.id)
        session.state = gatewayRunToState(remote, target)
      } else session.runner.start(target, sensorSnapshot.value)
      session.app = cloneApp(target)
      session.lastTick = performance.now()
      if (!gatewayConnected.value) syncRun(session)
    }
    activeRuntimeId.value = demo.apps[0].id
    app.value = cloneApp(demo.apps[0])
    original.value = JSON.stringify(app.value)
    journeyStage.value = 4
    window.dispatchEvent(new CustomEvent('marine-apps-updated'))
    showGeneration(demo.apps, 'leadership', demo.reusedServiceIds)
  } catch {
    leadershipBusy.value = false
    notify('本机存储不可用，一键演示未能启动。', true)
  }
}
function startRun() {
  void startApplication(app.value)
}
async function pauseResume() {
  const session = activeSession.value
  if (!session) return
  if (gatewayConnected.value) {
    const runId = gatewayRunIds.get(session.app.id)
    if (!runId) return
    const remote = session.state.status === 'paused' ? await marineApi.resumeRun(runId) : await marineApi.pauseRun(runId)
    session.state = gatewayRunToState(remote, session.app); runtimeSessions.value = [...runtimeSessions.value]
  } else { if (session.state.status === 'paused') { session.runner.resume(); session.lastTick = performance.now() } else session.runner.pause(); syncRun(session) }
}
async function stopRun() { const session = activeSession.value; if (!session) return; if (gatewayConnected.value) { const runId = gatewayRunIds.get(session.app.id); if (!runId) return; const remote = await marineApi.cancelRun(runId); session.state = gatewayRunToState(remote, session.app); runtimeSessions.value = [...runtimeSessions.value] } else { session.runner.cancel(); syncRun(session) } }
function injectFailure() {
  const session = activeSession.value
  const active = runNodes.value.find(node => node.status === 'running')
  if (session && active) { session.runner.fail(active.node.id, '演示故障'); syncRun(session) }
}
async function toggleFullscreen() {
  try { if (document.fullscreenElement) await document.exitFullscreen(); else await document.documentElement.requestFullscreen() }
  catch { notify('当前环境不支持全屏，请使用浏览器全屏功能。', true) }
}
function syncFullscreen() { fullscreen.value = Boolean(document.fullscreenElement) }
function unload(event: BeforeUnloadEvent) { if (dirty.value || activeRuntimeCount.value) event.preventDefault() }
async function pollSensors() {
  try {
    const response = await realtimeApi.getAllData() as unknown as { data?: unknown }
    sensorSnapshot.value = normalizeSensorSnapshot(response?.data ?? response)
    sensorApiConnected.value = true
  } catch {
    sensorApiConnected.value = false
  }
}
onMounted(() => {
  try {
    const raw = localStorage.getItem(storageKey)
    savedApps.value = parseSavedApps(raw)
    if (savedApps.value[0]) activeRuntimeId.value = ensureRuntime(savedApps.value[0]).id
    if (raw && JSON.parse(raw).length !== savedApps.value.length) notify('部分旧应用记录无效，已跳过。', true)
  }
  catch { notify('未能读取本机应用记录，仍可正常编排和演示。', true) }
  void marineApi.health().then(async (health: any) => {
    gatewayConnected.value = Boolean(health)
    motorMode.value = health?.motorMode === 'real' ? 'real' : 'simulation'; emergencyStopped.value = Boolean(health?.emergencyStopped)
    if (!gatewayConnected.value) return
    try {
      const apps = await marineApi.listApps()
      const valid = parseSavedApps(JSON.stringify(apps))
      if (valid.length) { savedApps.value = valid; localStorage.setItem(storageKey, JSON.stringify(valid)) }
      await syncGatewayRuns()
    } catch { gatewayConnected.value = false }
  })
  timer = setInterval(() => {
    if (gatewayConnected.value) return
    const now = performance.now()
    runtimeSessions.value.forEach(session => {
      if (session.runner.state.status === 'running') { session.runner.tick((now - session.lastTick) / 1000); syncRun(session) }
      session.lastTick = now
    })
  }, 100)
  pollSensors()
  sensorTimer = setInterval(pollSensors, 2000)
  gatewayTimer = setInterval(() => { void syncGatewayRuns() }, 500)
  document.addEventListener('fullscreenchange', syncFullscreen); window.addEventListener('beforeunload', unload)
})
onUnmounted(() => { clearInterval(timer); clearInterval(sensorTimer); clearInterval(gatewayTimer); clearTimeout(toastTimer); clearTimeout(generationTimer); runtimeSessions.value.forEach(session => session.runner.cancel()); document.removeEventListener('fullscreenchange', syncFullscreen); window.removeEventListener('beforeunload', unload) })
</script>

<template>
  <div class="marine">
    <header class="marine-header">
      <div class="marine-brand"><span class="brand-mark"><svg viewBox="0 0 32 32" width="30" height="30" fill="none" stroke="currentColor" stroke-width="1.5"><path d="M5 19L16 15L27 19L23 26H9ZM10 17V9H22V17M16 9V4M4 28Q8 25 12 28T20 28T28 28"/></svg></span><div><strong>船舶软件定义平台</strong><small>SOFTWARE DEFINED VESSEL</small></div></div>
      <nav aria-label="演示导航"><button :class="{active:tab === 'compose'}" @click="tab = 'compose'">任务编排</button><button :class="{active:tab === 'run'}" @click="tab = 'run'">应用运行<span v-if="activeRuntimeCount" class="live-dot"/> <small v-if="activeRuntimeCount">{{ activeRuntimeCount }}</small></button><button :class="{active:tab === 'catalog'}" @click="tab = 'catalog'">服务目录</button><RouterLink class="scene-link" to="/hardware-decoupling">硬件解耦 <span>↗</span></RouterLink></nav>
      <span class="mode-pill"><i/> 演示模式</span><button class="fullscreen-button" @click="toggleFullscreen" :aria-label="fullscreen ? '退出全屏' : '进入全屏'">⛶ <span>{{ fullscreen ? '退出全屏' : '全屏演示' }}</span></button>
    </header>

    <main class="marine-main">
      <template v-if="tab === 'compose'">
        <section class="page-heading compose-heading"><div><p class="eyebrow">MISSION COMPOSER</p><h1>任务编排<span>.</span></h1></div><div class="button-group"><button class="leadership-button" :disabled="leadershipBusy" @click="runLeadershipDemo"><span class="leadership-play">▶</span> {{ leadershipBusy ? '正在生成应用…' : '一键演示' }}</button><button :disabled="editingBusy" @click="savedDialog?.showModal()">我的应用 <span class="count">{{ savedApps.length }}</span></button><button :disabled="editingBusy" @click="newApp">＋ 新建应用</button></div></section>
        <section class="software-story-bar" aria-label="软件定义演示路径">
          <div class="journey-track">
            <div v-for="(item,index) in ['任务选择','服务编排','应用生成','应用运行']" :key="item" class="journey-step" :class="{active:journeyStage === index + 1,done:journeyStage > index + 1}"><span>{{ journeyStage > index + 1 ? '✓' : index + 1 }}</span><div><small>STEP {{ String(index + 1).padStart(2,'0') }}</small><strong>{{ item }}</strong></div><i v-if="index < 3">→</i></div>
          </div>
          <div class="soft-value-metrics"><div><strong>{{ savedApps.length }}</strong><span>应用</span></div><div><strong>{{ activeRuntimeCount }}</strong><span>运行中</span></div></div>
        </section>
        <section class="order-strip reuse-orders" aria-label="示例任务订单与服务复用对比">
          <div class="reuse-orders-title"><p class="eyebrow">PRESET SCENARIOS</p><strong>预设场景</strong></div>
          <button class="reuse-order-card" :disabled="editingBusy" @click="selectPreset('D')"><b>01</b><span class="order-name">夜间机舱检修保障</span><span class="order-service service-01">人员 / 照度</span><i>＋</i><span class="order-service shared-service">通风 / 报警</span><i>＋</i><span class="order-service service-04">排水</span></button>
          <div class="reuse-bridge"><span>01</span><span>05</span><strong>复用</strong></div>
          <button class="reuse-order-card" :disabled="editingBusy" @click="selectPreset('E')"><b>02</b><span class="order-name">检修异常安全处置</span><span class="order-service service-01">烟雾 / 水浸</span><i>＋</i><span class="order-service shared-service">报警 / 通风</span><i>＋</i><span class="order-service service-04">排水</span></button>
        </section>
        <section class="panel composer-ship-panel">
          <div class="panel-heading"><div><p class="eyebrow">SHIP CAPABILITY MAP</p><h2>任务与船舶能力对应关系</h2></div><span>{{ run.status === 'running' ? '当前任务能力流动中' : '编排服务将映射到船舶能力' }}</span></div>
          <ShipDiagram :nodes="runNodes" :paused="run.status !== 'running'"/>
        </section>
        <div v-if="editingBusy" class="info-banner">应用正在{{ editingSession?.state.status === 'paused' ? '暂停' : '运行' }}，编排已锁定。<button @click="tab = 'run'">查看运行 →</button></div>
        <div class="composer-layout">
          <aside class="panel library"><div class="panel-heading"><h2>原子服务库</h2><span>{{ services.length }} 项</span></div>
            <button class="library-label library-toggle" :aria-expanded="awarenessExpanded" @click="awarenessExpanded = !awarenessExpanded"><span><i class="sensor-dot" :class="{live:sensorApiConnected}"/> 态势感知服务</span><span>07 {{ awarenessExpanded ? '−' : '+' }}</span></button>
            <div v-if="awarenessExpanded"><article v-for="service in awarenessServices" :key="service.id" class="service-tile sensing-tile" :style="{'--service-color':service.color}" :draggable="!editingBusy" @dragstart="drag($event, 'service', service.id)"><span class="service-number">{{ service.id }}</span><div><h3>{{ service.name }}</h3><p>{{ service.description }}</p></div><button :disabled="editingBusy" class="tile-add" :aria-label="`添加${service.name}到选中步骤`" @click="addService(service.id)">＋</button></article></div>
            <button class="library-label library-toggle" :aria-expanded="actionExpanded" @click="actionExpanded = !actionExpanded"><span><i class="action-dot"/> 执行控制服务</span><span>{{ String(actionServices.length).padStart(2,'0') }} {{ actionExpanded ? '−' : '+' }}</span></button>
            <div v-if="actionExpanded"><article v-for="service in actionServices" :key="service.id" class="service-tile" :style="{'--service-color':service.color}" :draggable="!editingBusy" @dragstart="drag($event, 'service', service.id)"><span class="service-number">{{ service.id }}</span><div><h3>{{ service.name }}</h3><p>{{ service.description }}</p></div><button :disabled="editingBusy" class="tile-add" :aria-label="`添加${service.name}到选中步骤`" @click="addService(service.id)">＋</button></article></div>
            <button class="expand-library" :aria-expanded="libraryExpanded" @click="libraryExpanded = !libraryExpanded">扩展服务 <span>10 项 · {{ libraryExpanded ? '收起 −' : '展开 ＋' }}</span></button>
            <div v-if="libraryExpanded" class="future-library"><div v-for="service in services.filter(item => !item.available)" :key="service.id"><span>{{ service.id }} {{ service.name }}</span><span>待实现</span></div></div>
            <div class="library-footer"><span class="interface-icon">◇</span><div>统一服务契约<p>任务参数 + 态势输入 → 服务结果</p></div></div>
          </aside>
          <section class="panel canvas"><div class="panel-heading"><div><p class="eyebrow">APPLICATION / 应用编排</p><h2>{{ app.name }}</h2></div><span class="draft-state">{{ dirty ? '未保存更改' : savedApps.some(item => item.id === app.id) ? '已保存到本机' : '示例 / 草稿' }}</span></div>
            <div class="canvas-toolbar"><span>{{ nodes.length }} 项服务 <i/> {{ app.steps.length }} 个步骤 <i/> 预计 {{ duration }} 秒</span><div class="quick-actions"><button :disabled="editingBusy" class="text-button" @click="addStep">＋ 步骤</button><button :disabled="editingBusy || !!errors.length" @click="saveApp">生成应用</button><button class="primary" :disabled="editingBusy || !!errors.length" @click="startRun">▶ 运行</button></div></div>
            <div class="flow-steps">
              <article v-for="(step,index) in app.steps" :key="step.id" class="step" :class="{ 'selected-step':selectedStep === step.id, parallel:step.nodes.length > 1 }" :draggable="!editingBusy" @dragstart.self="drag($event, 'step', step.id)" @dragover.prevent @drop.prevent.stop="drop($event, step.id)">
                <div class="step-header"><button class="step-select" @click="selectedStep = step.id; selectedId = ''"><span class="step-index">{{ String(index + 1).padStart(2,'0') }}</span>{{ step.nodes.length > 1 ? '并行组' : '顺序步骤' }}</button><div class="step-actions"><button :disabled="editingBusy || index === 0" @click="moveStep(step.id,-1)" aria-label="步骤前移">←</button><button :disabled="editingBusy || index === app.steps.length-1" @click="moveStep(step.id,1)" aria-label="步骤后移">→</button><button :disabled="editingBusy" @click="removeStep(step.id)" aria-label="删除步骤">×</button></div></div>
                <div class="step-service-picker"><label>添加原子服务</label><select :disabled="editingBusy" @change="addServiceFromSelect($event, step.id)"><option value="">从服务库选择…</option><option v-for="service in services" :key="service.id" :value="service.id" :disabled="!service.available">{{ service.id }} · {{ service.name }}{{ service.available ? '' : '（待实现）' }}</option></select></div>
                <p v-if="step.nodes.length > 1" class="parallel-caption">并行执行 · 与关系：全部完成后进入下一阶段</p>
                <div v-for="node in step.nodes" :key="node.id" class="flow-node" :class="{selected:selectedId === node.id}" :style="{'--service-color':serviceById(node.serviceId).color}" :draggable="!editingBusy" @dragstart.stop="drag($event,'node',node.id)">
                  <button class="node-main" @click="chooseNode(node,step.id)"><div class="node-top"><span class="service-number">{{ node.serviceId }}</span><span v-if="serviceById(node.serviceId).kind === 'awareness'" class="reuse-badge">态势输入</span><span v-else-if="['02','03'].includes(node.serviceId)" class="reuse-badge">可跨应用复用</span><span class="drag-handle">⠿</span></div><h3>{{ serviceById(node.serviceId).name }}</h3><p>{{ node.area }} · {{ serviceById(node.serviceId).kind === 'awareness' ? `${serviceById(node.serviceId).metric} ${node.thresholdOperator === 'gte' ? '≥' : '≤'} ${node.threshold}${serviceById(node.serviceId).unit}` : `强度 ${node.intensity}%` }}</p><div class="node-footer"><span>◷ {{ node.duration }} 秒</span><span>参数配置 ↗</span></div></button>
                  <button class="node-remove" :disabled="editingBusy" :aria-label="`移除${serviceById(node.serviceId).name}`" @click="removeNode(node.id)">×</button>
                </div>
                <button class="parallel-drop" :disabled="editingBusy" @click="selectedStep = step.id; selectedId = ''">＋ 添加服务</button>
                <span v-if="index < app.steps.length-1" class="step-connector" aria-label="全部完成后进入下一阶段">↓<small>全部完成</small></span>
              </article>
              <button class="new-step-drop" :disabled="editingBusy" @dragover.prevent @drop.prevent.stop="drop($event)" @click="addStep"><span>＋</span>新步骤<small>拖入服务，顺序执行</small></button>
            </div>
            <div v-if="errors.length" class="validation-list" role="status"><p v-for="error in errors" :key="error">{{ error }}</p></div>
            <div class="canvas-bottom"><span>任务步骤</span><div class="button-group"><button :disabled="editingBusy || !!errors.length" @click="saveApp">生成并保存应用</button><button class="primary" :disabled="editingBusy || !!errors.length" @click="startRun">▶ 运行应用</button></div></div>
          </section>
          <aside class="panel inspector"><p class="eyebrow">MISSION ORDER</p><h2>{{ selected ? '服务参数' : '任务需求' }}</h2>
          <template v-if="selected"><div class="inspector-service" :style="{'--service-color':serviceById(selected.serviceId).color}"><span class="service-number">{{ selected.serviceId }}</span><h3>{{ serviceById(selected.serviceId).name }}</h3></div><p>{{ serviceById(selected.serviceId).description }}</p><div v-if="serviceById(selected.serviceId).kind === 'awareness'" class="sensor-source-card"><span :class="{live:sensorApiConnected}"/><div><strong>{{ sensorApiConnected ? '实时数据已接入' : '当前使用演示数据' }}</strong><p>{{ sensorApiConnected ? `${onlineSensorCount} / 12 路数据在线` : '启动网关后自动切换为真实数据' }}</p></div></div><label class="field">任务区域<select v-model="selected.area" :disabled="editingBusy"><option v-for="area in areas" :key="area">{{ area }}</option></select></label><template v-if="serviceById(selected.serviceId).kind === 'awareness'"><div class="threshold-title">态势判断条件 <span>阈值随应用保存</span></div><div class="threshold-fields"><label class="field">指标<strong>{{ serviceById(selected.serviceId).metric }}{{ serviceById(selected.serviceId).unit ? `（${serviceById(selected.serviceId).unit}）` : '' }}</strong></label><label class="field">关系<select v-model="selected.thresholdOperator" :disabled="editingBusy"><option value="gte">大于或等于</option><option value="lte">小于或等于</option></select></label><label class="field">阈值<input type="number" min="0" step="1" v-model.number="selected.threshold" :disabled="editingBusy" /></label></div><label class="field">采样窗口（秒）<input type="number" min="2" max="30" v-model.number="selected.duration" :disabled="editingBusy" /></label></template><template v-else-if="selected.serviceId === 'M01' && selected.motor"><label class="field">逻辑执行器<select v-model="selected.motor.executorId" :disabled="editingBusy"><option v-for="(name,id) in executorNames" :key="id" :value="id">{{ name }}</option></select></label><label class="field">目标转速 <strong>{{ selected.motor.speedRpm }} rpm</strong><input v-model.number="selected.motor.speedRpm" type="range" min="1" max="500" step="1" :disabled="editingBusy"/><input v-model.number="selected.motor.speedRpm" type="number" min="1" max="500" :disabled="editingBusy"/></label><label class="field">旋转方向<select v-model="selected.motor.direction" :disabled="editingBusy"><option value="forward">正转</option><option value="reverse">反转</option></select></label><details class="motor-advanced"><summary>高级参数</summary><label class="field">加速度<input v-model.number="selected.motor.acceleration" type="number" min="1" max="100"/></label><label class="field">减速度<input v-model.number="selected.motor.deceleration" type="number" min="1" max="100"/></label></details><label class="field">持续时间（秒）<input type="number" min="2" max="30" v-model.number="selected.duration" :disabled="editingBusy" /></label></template><template v-else><label class="field">运行强度 <strong>{{ selected.intensity }}%</strong><input type="range" min="10" max="100" step="5" v-model.number="selected.intensity" :disabled="editingBusy" /></label><label class="field">持续时间（秒）<input type="number" min="2" max="30" v-model.number="selected.duration" :disabled="editingBusy" /></label></template><label class="field">移动到步骤<select :value="selectedStep" :disabled="editingBusy" @change="moveNode(selected.id, ($event.target as HTMLSelectElement).value)"><option v-for="(step,index) in app.steps" :key="step.id" :value="step.id">步骤 {{ index+1 }}{{ step.nodes.length > 1 ? ' · 并行组' : '' }}</option><option value="">新建顺序步骤</option></select></label><button class="text-button" @click="selectedId = ''">← 查看任务需求</button></template>
            <template v-else><label class="field">应用名称<input v-model="app.name" maxlength="40" :disabled="editingBusy" /></label><label class="field">任务目标<textarea v-model="app.description" rows="4" maxlength="240" :disabled="editingBusy" /></label><div class="order-summary"><span>执行方式</span><strong>顺序 / 并行</strong><span>预计运行</span><strong>{{ duration }} 秒</strong><span>运行环境</span><strong>软件模拟</strong></div></template>
            <div class="reuse-note">当前场景服务<strong v-for="id in reused" :key="id">{{ id }} {{ serviceById(id).name }}</strong></div>
          </aside>
        </div>
        <section class="architecture-strip" aria-label="控制执行顺序"><div><span>01</span><strong>任务需求</strong><p>选择并生成任务应用</p></div><i>→</i><div><span>02</span><strong>应用层</strong><p>下发应用执行计划</p></div><i>→</i><div><span>03</span><strong>原子服务层</strong><p>步骤间顺序 · 步骤内并行</p></div><i>→</i><div><span>04</span><strong>设备抽象层</strong><p>转换为统一设备指令</p></div><i>→</i><div><span>05</span><strong>物理资源层</strong><p>态势感知与执行器硬件</p></div></section>
      </template>

      <template v-else-if="tab === 'run'">
        <section class="page-heading"><div><p class="eyebrow">SOFTWARE BASE</p><h1>{{ activeSession?.app.name || '应用运行' }}<span>.</span></h1></div><div class="button-group"><button @click="tab = 'compose'">＋ 生成新应用</button><template v-if="busy"><button @click="pauseResume">{{ run.status === 'paused' ? '▶ 继续' : 'Ⅱ 暂停' }}</button><button class="danger-button" @click="stopRun">■ 停止当前应用</button></template><button v-else class="primary" :disabled="!activeSession" @click="activeSession && startApplication(activeSession.app)">▶ {{ run.app ? '重新运行' : '启动当前应用' }}</button></div></section>
        <section class="application-base panel">
          <div class="application-base-heading"><div><p class="eyebrow">APPLICATION PAGES / 应用页面</p><h2>一个基座，承载多个独立应用</h2></div><div class="base-runtime-summary"><strong>{{ applicationPages.length }}</strong><span>个应用</span><i/><strong>{{ activeRuntimeCount }}</strong><span>个正在运行</span></div></div>
          <div v-if="applicationPages.length" class="application-page-tabs">
            <article v-for="(page,index) in applicationPages" :key="page.id" class="application-page-tab" :class="[runtimeStatus(page.id), {selected:activeRuntimeId === page.id}]">
              <button class="application-page-main" @click="selectApplicationPage(page)"><span>APP {{ String(index+1).padStart(2,'0') }}</span><strong>{{ page.name }}</strong><small>{{ page.steps.flatMap(step => step.nodes).length }} 项服务 · {{ page.steps.length }} 个步骤</small></button>
              <button class="application-page-action" :disabled="['running','paused'].includes(runtimeStatus(page.id))" @click.stop="startApplication(page)">{{ ['running','paused'].includes(runtimeStatus(page.id)) ? statusLabels[runtimeStatus(page.id)] : '▶ 启动' }}</button>
            </article>
            <button class="application-page-new" @click="tab = 'compose'"><span>＋</span><strong>生成新应用</strong><small>新增独立页面</small></button>
          </div>
          <div v-else class="application-base-empty"><span>＋</span><div><strong>还没有生成应用</strong><p>返回任务编排，组合服务并点击“生成并保存应用”。</p></div><button class="primary" @click="tab = 'compose'">前往任务编排</button></div>
        </section>
        <section v-if="activeSession" class="motor-safety-bar" :class="{stopped:emergencyStopped}"><div><strong>{{ motorMode === 'real' ? '真实设备控制' : '模拟演示模式' }}</strong><span>{{ emergencyStopped ? '执行器已急停锁定' : '三台电机控制可用' }}</span></div><button v-if="!emergencyStopped" @click="triggerEmergency">执行器急停</button><button v-else class="reset" @click="resetEmergency">复位急停</button></section>
        <ApplicationCommandCenter v-if="activeSession" :app="activeSession.app" :nodes="runNodes" :snapshot="sensorSnapshot" :status="run.status" />
        <ApplicationFlow v-if="activeSession" :app="activeSession.app" :nodes="runNodes" :snapshot="sensorSnapshot" :status="run.status" />
        <div class="runtime-stats"><div><span>任务状态</span><strong :class="run.status">{{ statusLabels[run.status] }}</strong></div><div><span>整体进度</span><strong>{{ progress }}<small>%</small></strong></div><div><span>服务完成</span><strong>{{ completedCount }}<small>/ {{ runNodes.length }}</small></strong></div><div><span>执行用时</span><strong>{{ run.elapsed.toFixed(1) }}<small>秒</small></strong></div><div class="runtime-mode"><span class="mode-pill">混合运行</span><p>感知读取数据 · 电机{{ motorMode === 'real' ? '真实控制' : '模拟执行' }}</p></div></div>
        <aside class="panel timeline"><div class="panel-heading"><h2>执行时间线</h2><span>{{ run.events.length }} 条事件</span></div><div class="timeline-list" role="log" aria-live="polite"><p v-if="!run.events.length" class="muted">任务开始后，这里显示服务执行过程。</p><article v-for="(event,index) in [...run.events].reverse()" :key="run.events.length-index" :class="event.kind"><time>+{{ event.time.toFixed(1) }}s</time><p>{{ event.message }}</p></article></div><button class="text-button fault-button" :disabled="!busy" @click="injectFailure">模拟服务异常</button></aside>
        <section v-if="run.app" class="panel execution-panel"><div class="panel-heading"><h2>应用执行链</h2><span>并行服务同时运行，全部完成后进入下一步</span></div><div class="execution-chain"><article v-for="(step,index) in run.app.steps" :key="step.id" class="execution-step" :class="{current:run.stepIndex === index && busy}"><header><span>步骤 {{ String(index+1).padStart(2,'0') }}</span><span>{{ step.nodes.length > 1 ? '并行执行' : '顺序执行' }}</span></header><div v-for="node in step.nodes" :key="node.id" class="execution-node" :style="{'--service-color':serviceById(node.serviceId).color}"><div><h3>{{ node.serviceId }} {{ serviceById(node.serviceId).name }}</h3><span :class="run.nodes[node.id]!.status">{{ run.status === 'paused' && run.nodes[node.id]!.status === 'running' ? '已暂停' : nodeLabels[run.nodes[node.id]!.status] }}</span></div><p>{{ node.area }} · {{ node.serviceId === 'M01' && node.motor ? `${executorNames[node.motor.executorId]} · ${node.motor.direction === 'reverse' ? '反转' : '正转'} · ${node.motor.speedRpm} rpm` : serviceById(node.serviceId).kind === 'awareness' ? `${node.duration} 秒采样` : `${node.intensity}% 强度 · ${node.duration} 秒` }}</p><div class="progress-track"><span :style="{width:`${run.nodes[node.id]!.progress*100}%`}"/></div><div class="metric-result"><span>{{ run.nodes[node.id]!.result.metric }}<small class="result-source">{{ run.nodes[node.id]!.result.source }}</small></span><strong>{{ run.nodes[node.id]!.status === 'waiting' ? '—' : run.nodes[node.id]!.result.value }} <small>{{ run.nodes[node.id]!.result.unit }}</small></strong></div><div v-if="node.serviceId === 'M01' && node.motor && run.nodes[node.id]!.status === 'running'" class="motor-live-controls"><input v-model.number="node.motor.speedRpm" type="range" min="1" max="500"/><strong>{{ node.motor.speedRpm }} rpm</strong><button @click="node.motor.direction = node.motor.direction === 'forward' ? 'reverse' : 'forward'; applyMotor(node)">{{ node.motor.direction === 'forward' ? '切换反转' : '切换正转' }}</button><button @click="applyMotor(node)">下发转速</button><button class="stop" @click="stopMotorNode(node)">停止电机</button></div><p class="result-detail">{{ run.nodes[node.id]!.result.detail }}</p></div></article></div></section>
        <section v-if="run.status === 'completed'" class="result-banner" role="status"><span class="result-check">✓</span><div><h2>任务执行完成</h2><p>{{ runNodes.length }} 项服务全部完成，耗时 {{ run.elapsed.toFixed(1) }} 秒。{{ runReused.length ? `复用服务：${runReused.map(id => serviceById(id).name).join('、')}。` : '' }}</p></div><button @click="tab = 'compose'">重新组合，生成新应用 →</button></section>
        <section v-else-if="run.status === 'failed' || run.status === 'cancelled'" class="info-banner" role="status">{{ run.status === 'failed' ? '服务出现演示异常，后续步骤已停止。可重新运行验证恢复。' : '任务已停止，未完成的服务已取消。' }}</section>
      </template>

      <template v-else>
        <section class="page-heading"><div><p class="eyebrow">CAPABILITY CATALOG</p><h1>原子服务库<span>.</span></h1></div><button class="primary" @click="tab = 'compose'">前往任务编排 →</button></section>
        <div class="catalog-toolbar"><div class="filter-tabs"><button v-for="filter in ['全部服务','可运行','待实现']" :key="filter" :class="{active:catalogFilter === filter}" @click="catalogFilter = filter">{{ filter }}</button></div><input v-model="search" aria-label="搜索服务" placeholder="搜索名称、编号或能力…" /></div>
        <div class="catalog-grid"><article v-for="service in filteredServices" :key="service.id" class="panel catalog-card" :style="{'--service-color':service.color}"><header><span class="service-number">{{ service.id }}</span><span :class="service.available ? 'available-tag' : 'pending-tag'">{{ service.available ? (service.kind === 'awareness' ? (sensorApiConnected ? '可运行 · 实时数据' : '可运行 · 演示数据') : service.id === 'M01' ? `可运行 · ${motorMode === 'real' ? '真实设备' : '模拟设备'}` : '可运行 · 软件模拟') : '待实现' }}</span></header><span class="catalog-category">{{ service.category }}</span><h2>{{ service.name }}</h2><p>{{ service.description }}</p><footer><span>{{ service.available ? (service.kind === 'awareness' ? '传感器数据 → 态势结果' : '任务参数 → 执行结果') : '扩展能力规划' }}</span><button v-if="service.available" :disabled="busy" :aria-label="`将${service.name}加入编排`" @click="addService(service.id); tab = 'compose'">加入编排 ↗</button><span v-else>—</span></footer></article></div><p v-if="!filteredServices.length" class="empty-state">没有匹配的服务，请尝试其他关键词。</p>
      </template>
      <footer class="marine-footer"><span>SD VESSEL <i/> 民用船舶软件定义演示</span><span>软软解耦 · 业务服务自由组合</span><span><RouterLink to="/hardware-decoupling">硬件替换演示 ↗</RouterLink><RouterLink to="/legacy-dashboard">系统总图 ↗</RouterLink></span></footer>
    </main>
    <Transition name="generation">
      <div v-if="generationPreview" class="generation-overlay" role="status" aria-live="assertive">
        <section class="generation-stage">
          <p class="eyebrow">SOFTWARE DEFINED / 服务编排生成应用</p>
          <h2>{{ generationPreview.mode === 'leadership' ? '正在生成两个应用' : '正在生成应用' }}</h2>
          <div class="generation-apps">
            <article v-for="(previewApp,index) in generationPreview.apps" :key="previewApp.id" class="generation-app-card">
              <div class="generation-app-icon">APP<span>{{ String(index + 1).padStart(2,'0') }}</span></div>
              <div><small>独立应用页面</small><h3>{{ previewApp.name }}</h3><div class="generation-service-row"><span v-for="node in previewApp.steps.flatMap(step => step.nodes).filter(item => serviceById(item.serviceId).kind === 'action')" :key="node.id" :class="{shared:generationPreview.reusedServiceIds.includes(node.serviceId)}">{{ node.serviceId }} {{ serviceById(node.serviceId).name }}</span></div></div>
              <i class="generation-pulse"/>
            </article>
          </div>
          <div v-if="generationPreview.mode === 'leadership'" class="generation-reuse-line"><span/><strong>{{ generationReuseNames }}</strong><span/></div>
          <div class="generation-metrics"><div><strong>0</strong><span>新增服务开发</span></div><div><strong>{{ generationPreview.reusedServiceIds.length }}</strong><span>共享原子服务</span></div><div><strong>{{ generationPreview.apps.length }}</strong><span>新应用页面</span></div><div><strong>{{ generationPreview.mode === 'leadership' ? generationPreview.apps.length : activeRuntimeCount }}</strong><span>可并行运行</span></div></div>
          <p class="generation-status"><i/> {{ generationPreview.mode === 'leadership' ? '应用已装载到统一软件基座，即将进入并行运行视图' : '应用页面已加入软件基座' }}</p>
        </section>
      </div>
    </Transition>
    <div v-if="notice" class="marine-toast" :class="{error:noticeError}" role="status">{{ notice }}</div>
    <dialog ref="confirmation" class="marine-dialog" @cancel="pendingAction = undefined"><h2>当前编排有未保存的更改</h2><p>切换后将放弃这些更改。你也可以返回，先保存当前应用。</p><div class="button-group"><button autofocus @click="confirmation?.close(); pendingAction = undefined">返回编排</button><button class="primary" @click="confirmReplace">放弃更改并切换</button></div></dialog>
    <dialog ref="savedDialog" class="marine-dialog saved-dialog"><div class="panel-heading"><h2>我的应用</h2><button autofocus @click="savedDialog?.close()" aria-label="关闭我的应用">×</button></div><p class="muted">保存在当前浏览器中，刷新后仍可继续使用。</p><p v-if="!savedApps.length" class="empty-state">还没有保存的应用。完成编排后点击“保存应用”。</p><button v-for="saved in savedApps" :key="saved.id" class="saved-app" @click="loadApp(saved)"><div><h3>{{ saved.name }}</h3><p>{{ saved.steps.flatMap(step => step.nodes).map(node => node.serviceId).join(' + ') }} · {{ saved.steps.length }} 个步骤</p></div><span>打开 →</span></button></dialog>
  </div>
</template>
