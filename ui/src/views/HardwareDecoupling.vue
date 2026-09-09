<script setup lang="ts">
import { computed, onMounted, onUnmounted, ref } from 'vue'
import {
  advanceActuatorDemo,
  confirmActuatorSwitch,
  connectActuator,
  createActuatorDemo,
  disconnectActive,
  injectActuatorFault,
} from '../marine/actuators'
import type { ActuatorDevice, ActuatorPhase } from '../marine/actuators'
import '../styles/hardware-decoupling.css'

const demo = ref(createActuatorDemo())
const fullscreen = ref(false)
let timer: ReturnType<typeof setInterval> | undefined

const activeDevice = computed(() => demo.value.devices.find(device => device.id === demo.value.logicalExecutor.boundDeviceId))
const candidate = computed(() => demo.value.devices.find(device => device.id === demo.value.candidateId))
const phaseLabels: Record<ActuatorPhase, string> = {
  running: '任务运行中',
  protected: '任务保护暂停',
  'awaiting-confirmation': '等待人工确认',
  switching: '正在切换绑定',
  recovered: '任务已恢复',
}
const phaseStep = computed(() => ({ running: 1, protected: 2, 'awaiting-confirmation': 3, switching: 4, recovered: 5 }[demo.value.phase]))
const canProtect = computed(() => ['running', 'recovered'].includes(demo.value.phase))
const statusLabels = { offline: '离线', standby: '在线待机', running: '在线运行', faulted: '设备异常' }

function disconnect() { demo.value = disconnectActive(demo.value) }
function fault() { demo.value = injectActuatorFault(demo.value) }
function connect(deviceId: ActuatorDevice['id']) { demo.value = connectActuator(demo.value, deviceId) }
function confirmSwitch() { demo.value = confirmActuatorSwitch(demo.value) }
function reset() { demo.value = createActuatorDemo() }
function syncFullscreen() { fullscreen.value = Boolean(document.fullscreenElement) }
async function toggleFullscreen() {
  if (document.fullscreenElement) await document.exitFullscreen()
  else await document.documentElement.requestFullscreen()
}

onMounted(() => {
  timer = setInterval(() => { demo.value = advanceActuatorDemo(demo.value, 100) }, 100)
  document.addEventListener('fullscreenchange', syncFullscreen)
})
onUnmounted(() => {
  clearInterval(timer)
  document.removeEventListener('fullscreenchange', syncFullscreen)
})
</script>

<template>
  <div class="hardware-demo" :class="`phase-${demo.phase}`">
    <div class="hardware-grid-bg" aria-hidden="true"/>
    <header class="hardware-header">
      <RouterLink class="hardware-brand" to="/">
        <span class="hardware-brand-mark">
          <svg viewBox="0 0 42 42" fill="none" stroke="currentColor" stroke-width="1.5"><path d="M6 25l15-6 15 6-5 10H11L6 25Z"/><path d="M14 21V11h14v10M21 11V5M5 37q5-4 10 0t12 0 10 0"/></svg>
        </span>
        <span><strong>船舶软件定义平台</strong><small>SOFTWARE DEFINED VESSEL</small></span>
      </RouterLink>
      <nav aria-label="演示场景导航">
        <RouterLink to="/">软件组合</RouterLink>
        <RouterLink class="active" to="/hardware-decoupling">硬件解耦</RouterLink>
        <RouterLink to="/legacy-dashboard">系统总图</RouterLink>
      </nav>
      <div class="hardware-header-actions">
        <span class="hardware-mode"><i/> 演示模式</span>
        <button @click="toggleFullscreen">⛶ {{ fullscreen ? '退出全屏' : '全屏演示' }}</button>
      </div>
    </header>

    <main class="hardware-main">
      <section class="hardware-title-row">
        <div>
          <p class="hardware-eyebrow">靠港夜间机舱检修 / HARDWARE REPLACEMENT</p>
          <h1>检修通风设备替换<span>。</span></h1>
          <p>机舱通风机 1 号异常或厂家更换时，切换至兼容备用驱动设备。</p>
        </div>
        <div class="change-metrics" aria-label="软硬解耦变更统计">
          <article><span>应用修改</span><strong>{{ demo.applicationChanges }}</strong><small>任务页面保持不变</small></article>
          <article><span>原子服务修改</span><strong>{{ demo.serviceChanges }}</strong><small>能力接口保持不变</small></article>
          <article class="binding-metric"><span>设备绑定更新</span><strong>{{ demo.bindingUpdates }}</strong><small>差异收敛在抽象层</small></article>
        </div>
      </section>

      <section class="hardware-stage">
        <div class="software-chain">
          <header class="stage-heading"><span>稳定的软件调用链</span><small>切换全程保持不变</small></header>
          <div class="chain-card application-card">
            <span class="chain-level">L4 · 应用层</span>
            <div class="chain-icon">APP</div>
            <div><strong>{{ demo.application.name }}</strong><small>{{ demo.application.id }} · 正在运行</small></div>
            <b>0 修改</b>
          </div>
          <div class="vertical-flow control-flow"><i/><span>任务控制</span></div>
          <div class="chain-card service-card">
            <span class="chain-level">L3 · 原子服务层</span>
            <div class="chain-icon">01</div>
            <div><strong>{{ demo.atomicService.name }}</strong><small>统一调用逻辑执行能力</small></div>
            <b>0 修改</b>
          </div>
          <div class="vertical-flow control-flow"><i/><span>标准能力接口</span></div>
          <div class="chain-card abstraction-card">
            <span class="chain-level">L2 · 设备抽象层</span>
            <div class="chain-icon">API</div>
            <div><strong>{{ demo.logicalExecutor.id }} · {{ demo.logicalExecutor.name }}</strong><small>启停 · 调速 · 状态 · 故障</small></div>
            <b>稳定接口</b>
          </div>
        </div>

        <div class="binding-core">
          <header class="stage-heading"><span>运行时逻辑绑定</span><small>唯一发生变化的位置</small></header>
          <div class="binding-orbit" :class="{ switching: demo.phase === 'switching' }">
            <div class="orbit-ring ring-one"/><div class="orbit-ring ring-two"/>
            <div class="binding-center">
              <span>设备抽象层</span>
              <strong>机舱通风</strong>
              <small>当前连接设备</small>
              <b>{{ activeDevice?.address }}</b>
            </div>
            <span class="orbit-node node-a">能力匹配</span><span class="orbit-node node-b">协议适配</span><span class="orbit-node node-c">状态复检</span>
          </div>
          <div class="binding-route" :class="{ interrupted: demo.phase === 'protected', pending: demo.phase === 'awaiting-confirmation', active: ['switching','recovered'].includes(demo.phase) }">
            <div><span>原绑定</span><strong>0x02</strong></div><i><b/></i><div><span>{{ candidate || demo.bindingUpdates ? '目标绑定' : '等待接入' }}</span><strong>{{ candidate?.address || (demo.bindingUpdates ? activeDevice?.address : '—') }}</strong></div>
          </div>
          <div class="register-strip"><span><b>6041h</b> 状态心跳</span><span><b>603Fh</b> 故障码</span><span><b>2009h</b> 运行状态</span></div>
        </div>

        <div class="physical-pool">
          <header class="stage-heading"><span>L1 · 设备抽象电机</span><small>同一 RS485 总线 · 不同地址</small></header>
          <article v-for="device in demo.devices" :key="device.id" class="motor-card" :class="[device.status, { bound: device.id === demo.logicalExecutor.boundDeviceId, candidate: device.id === demo.candidateId }]">
            <div class="motor-visual">
              <span class="motor-axis"/><span class="motor-body"><i/><i/><i/></span><span class="motor-tail"/>
            </div>
            <div class="motor-copy">
              <div><span class="motor-status"><i/>{{ statusLabels[device.status] }}</span><small>{{ device.type }}</small></div>
              <h3>{{ device.name }}</h3>
              <p>{{ device.id }} <i/> RS485 <strong>{{ device.address }}</strong></p>
            </div>
            <div class="motor-telemetry"><span class="speed-reading">实时转速 <b>{{ device.speed || '—' }}</b><small> rpm</small></span><span>故障码 <b>{{ device.faultCode }}</b></span></div>
            <div class="motor-badges"><span v-if="device.id === demo.logicalExecutor.boundDeviceId">当前绑定</span><span v-if="device.id === demo.candidateId">推荐替换</span><span v-else-if="device.compatible">能力兼容</span></div>
            <button v-if="device.compatible && device.status === 'offline' && demo.phase === 'protected' && device.id !== demo.logicalExecutor.boundDeviceId" @click="connect(device.id)">＋ 模拟接入</button>
          </article>
        </div>
      </section>

      <section class="operation-deck">
        <div class="scenario-control">
          <header><div><span>现场演示控制</span><small>所有操作仅改变前端模拟状态</small></div><button class="reset-button" @click="reset">↻ 重置场景</button></header>
          <div class="control-buttons">
            <button class="danger-control" :disabled="!canProtect" @click="disconnect"><span>01</span><div><strong>拔出当前电机</strong><small>模拟连续通信超时</small></div></button>
            <button class="warning-control" :disabled="!canProtect" @click="fault"><span>02</span><div><strong>注入运行异常</strong><small>模拟 0x8311 过载</small></div></button>
            <button class="confirm-control" :disabled="demo.phase !== 'awaiting-confirmation'" @click="confirmSwitch"><span>03</span><div><strong>操作员确认切换</strong><small>{{ candidate ? `将 ${demo.logicalExecutor.id} 绑定到 ${candidate.address}` : '等待兼容电机上线' }}</small></div></button>
          </div>
        </div>

        <div class="switch-progress">
          <header><span>切换进程</span><strong :class="demo.phase">{{ phaseLabels[demo.phase] }}</strong></header>
          <div class="progress-track"><i :style="{ width: `${phaseStep * 20}%` }"/></div>
          <div class="progress-steps"><span :class="{done:phaseStep>=1}">正常运行</span><span :class="{done:phaseStep>=2}">故障隔离</span><span :class="{done:phaseStep>=3}">发现设备</span><span :class="{done:phaseStep>=4}">更新绑定</span><span :class="{done:phaseStep>=5}">任务恢复</span></div>
          <div class="recovery-readout"><span>恢复计时</span><strong>{{ (demo.recoveryMs / 1000).toFixed(1) }}</strong><small>秒</small><b>人工确认模式 A</b></div>
        </div>

        <div class="event-timeline">
          <header><span>系统事件</span><small>{{ demo.events.length }} 条</small></header>
          <div class="event-list">
            <article v-for="(item, index) in [...demo.events].reverse()" :key="item.id" :class="item.tone"><span>T+{{ (demo.events.length - index - 1).toString().padStart(2,'0') }}</span><i/><div><strong>{{ item.title }}</strong><small>{{ item.detail }}</small></div></article>
          </div>
        </div>
      </section>
    </main>

    <footer class="hardware-footer"><span><i/> SD VESSEL · 民用船舶软件定义演示</span><strong>应用不感知地址变化，硬件差异由设备抽象层吸收</strong><span>SIMULATION / NO DEVICE COMMANDS</span></footer>
  </div>
</template>
