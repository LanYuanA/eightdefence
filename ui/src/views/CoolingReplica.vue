<script setup lang="ts">
import { computed, ref } from 'vue'
import motorImage from '../assets/motor-blue-3d.png'

// This visual replica runs entirely locally; it never sends hardware commands.
const motors = ['A', 'B', 'C'] as const
const protocols = ['RS485', 'CAN', 'RS232']
const selected = ref('C')
const bound = ref('C')
const speed = ref(50)
const direction = ref('forward')
const running = ref(false)
const removed = ref(false)
const message = ref('请确定在确认的环境中进行参数设置')
const validSpeed = computed(() => Number.isFinite(speed.value) && speed.value >= 1 && speed.value <= 500 && Number.isInteger(speed.value))
const canSwitch = computed(() => removed.value && selected.value !== bound.value)
function start() {
  if (!validSpeed.value) { message.value = '请输入 1–500 之间的整数转速'; return }
  if (removed.value && selected.value === bound.value) { message.value = '当前电机已拔出，请选择备用电机'; return }
  running.value = true
  message.value = `演示：电机 ${selected.value} 已启动 · ${speed.value} RPM · ${direction.value === 'forward' ? '正转' : '反转'}`
}
function stop() {
  running.value = false
  message.value = `演示：电机 ${selected.value} 已停止`
}
function remove() {
  removed.value = true
  running.value = false
  message.value = `演示：电机 ${bound.value} 已拔出，请选择备用电机并确认切换`
}
function switchMotor() {
  if (!canSwitch.value) return
  bound.value = selected.value
  removed.value = false
  message.value = `演示：已切换至电机 ${bound.value}，上层应用保持不变`
}
function status() {
  message.value = `演示：绑定电机 ${bound.value} · ${removed.value ? '已拔出' : running.value ? '运行中' : '已停止'} · 设定 ${speed.value} RPM`
}
</script>

<template>
  <div class="pump-replica">
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
      <span title="本页为本地交互演示，不连接真实设备">现场演示</span>
    </header>

    <main class="replica-architecture" aria-label="中央冷却泵四层架构">
      <section class="replica-tier app-tier">
        <div class="replica-tier-label"><b>04</b><h2>应用软件</h2></div>
        <div class="replica-panel upper-panel">
          <div class="replica-node"><span class="replica-icon"><svg><use href="#pump-ship"/></svg></span><div><h3>中央冷却应用</h3><p>应用定义保持不变</p></div></div>
        </div>
      </section>
      <section class="replica-tier service-tier">
        <div class="replica-tier-label"><b>03</b><h2>软件模块</h2></div>
        <div class="replica-panel upper-panel">
          <div class="replica-node"><span class="replica-icon"><svg><use href="#pump-cube"/></svg></span><div><h3>冷却泵控制服务</h3><p>启停 · 调速 · 状态反馈</p></div></div>
        </div>
      </section>
      <section class="replica-tier abstract-tier">
        <div class="replica-tier-label"><b>02</b><h2>设备抽象</h2></div>
        <div class="replica-panel abstraction-box">
          <div class="abstraction-heading"><span class="replica-icon"><svg><use href="#pump-cube"/></svg></span><h3>冷却泵抽象</h3></div>
          <div class="replica-capability parameters">
            <h4><svg class="gear"><use href="#pump-gear"/></svg>运行参数</h4>
            <label class="replica-speed"><span>转速</span><input v-model.number="speed" type="number" min="1" max="500" aria-label="运行转速"/><span>RPM</span></label>
            <div class="replica-directions"><strong>转向</strong><button :class="{ active: direction === 'forward' }" :aria-pressed="direction === 'forward'" @click="direction = 'forward'">正转</button><button :class="{ active: direction === 'reverse' }" :aria-pressed="direction === 'reverse'" @click="direction = 'reverse'">反转</button></div>
          </div>
          <div class="replica-capability driver">
            <h4><svg class="gear"><use href="#pump-gear"/></svg>电机驱动能力</h4>
            <div class="replica-api"><h5>控制接口</h5><div class="api-buttons"><button @click="start">start <span>启动</span></button><button @click="stop">stop <span>停止</span></button><button @click="message = validSpeed ? `演示：转速已设置为 ${speed} RPM` : '请输入 1–500 之间的整数转速'">set_speed <span>设置转速</span></button><button @click="message = `演示：转向已设置为${direction === 'forward' ? '正转' : '反转'}`">set_direction <span>设置转向</span></button></div></div>
            <div class="replica-api query"><h5>查询接口</h5><button @click="status">get_status <span>获取状态</span></button></div>
          </div>
          <div class="replica-adapter"><span class="adapter-arrows" aria-hidden="true">⇆</span><div><h4>协议适配标准中间件（翻译官）</h4><p>统一指令与状态 ↔ 不同设备协议</p></div></div>
        </div>
      </section>
      <section class="replica-tier motors-tier">
        <div class="replica-tier-label"><b>01</b><h2>现场硬件设备</h2></div>
        <div class="replica-motors">
          <button v-for="(motor, index) in motors" :key="motor" class="replica-motor" :class="{ candidate: removed && selected === motor && motor !== bound, disconnected: removed && bound === motor }" :aria-label="`选择电机 ${motor}`" :aria-pressed="selected === motor" @click="selected = motor">
            <span class="motor-connector" aria-hidden="true"><i></i></span>
            <span class="replica-motor-picture"><img :src="motorImage" alt="蓝色工业电机"/><strong>电机 {{ motor }}</strong></span><span class="replica-protocol">{{ protocols[index] }}</span>
          </button>
        </div>
      </section>
    </main>

    <footer class="replica-toolbar">
      <div class="replica-controls"><strong title="本页仅模拟控制">真实电机控制</strong><select v-model="selected" aria-label="选择控制电机"><option v-for="motor in motors" :key="motor" :value="motor">电机 {{ motor }}</option></select><label><input v-model.number="speed" type="number" min="1" max="500" aria-label="设定转速"/><span>RPM</span></label><select v-model="direction" aria-label="电机转向"><option value="forward">正转</option><option value="reverse">反转</option></select><button class="green-button" :disabled="!validSpeed" @click="start">启动</button><button class="red-button" @click="stop">停止</button></div>
      <p class="replica-message" role="status" :title="message">{{ message }}</p>
      <div class="replica-switch"><button :disabled="removed" @click="remove">检测退出</button><button class="green-button" :disabled="!canSwitch" @click="switchMotor">确认切换</button></div>
      <nav><RouterLink to="/">软件组合</RouterLink><span>/</span><RouterLink to="/hardware-decoupling">系统总图</RouterLink><button class="replica-help" aria-label="演示说明" @click="message = '本页仅为本地演示：启动、停止和切换不会控制真实设备'">ⓘ</button></nav>
    </footer>
  </div>
</template>

<style scoped src="../styles/cooling-replica.css"></style>
