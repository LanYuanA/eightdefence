<template>
  <div class="security-root">
    <ParticleBackground :particle-count="50" color="#3b82f6" :opacity="0.4" />
    <AppNavbar title="智能安防系统" subtitle="Security Monitoring System" :menu-items="menuItems" />

    <main class="security-main">
      <!-- 状态概览 -->
      <section class="status-section">
        <div class="section-header">
          <h2 class="section-title">
            <svg width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M12 22s8-4 8-10V5l-8-3-8 3v7c0 6 8 10 8 10z"/></svg>
            安防系统概览
          </h2>
        </div>
        <div class="status-grid">
          <div class="status-card" :class="systemStatus">
            <div class="status-icon">🛡️</div>
            <div class="status-info">
              <div class="status-label">系统状态</div>
              <div class="status-value">{{ systemStatus === 'normal' ? '运行正常' : '存在异常' }}</div>
            </div>
            <StatusDot :status="systemStatus === 'normal' ? 'online' : 'warning'" size="lg" />
          </div>
          <div class="status-card" :class="alarmActive ? 'danger' : 'normal'">
            <div class="status-icon">🔔</div>
            <div class="status-info">
              <div class="status-label">报警状态</div>
              <div class="status-value">{{ alarmActive ? '报警中' : '待机中' }}</div>
            </div>
            <StatusDot :status="alarmActive ? 'danger' : 'online'" size="lg" />
          </div>
          <div class="status-card">
            <div class="status-icon">📹</div>
            <div class="status-info">
              <div class="status-label">在线设备</div>
              <div class="status-value">{{ onlineCount }}/{{ totalCount }}</div>
            </div>
            <StatusDot status="online" size="lg" />
          </div>
          <div class="status-card">
            <div class="status-icon">⚡</div>
            <div class="status-info">
              <div class="status-label">今日事件</div>
              <div class="status-value">{{ todayEvents }} 条</div>
            </div>
            <StatusDot status="online" size="lg" />
          </div>
        </div>
      </section>

      <!-- 安防场景卡片 -->
      <section class="scenarios-section">
        <div class="section-header">
          <h2 class="section-title">安防场景监控</h2>
        </div>
        <div class="scenarios-grid">
          <!-- 水浸检测 -->
          <BaseCard title="水浸风险安防" subtitle="水浸隐患监测与处置" icon-color="blue">
            <template #header>
              <div class="flex items-center gap-3">
                <div class="card-icon icon-blue">
                  <svg width="20" height="20" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M12 2.69l5.66 5.66a8 8 0 1 1-11.31 0z"/></svg>
                </div>
                <div><h3 class="card-title">水浸风险安防</h3><p class="card-subtitle">水浸隐患监测与处置</p></div>
              </div>
            </template>
            <div class="scenario-data">
              <div class="data-row"><span class="data-label">传感器状态</span><span class="data-value" :class="waterOnline ? 'text-success' : 'text-danger'">{{ waterOnline ? '在线' : '离线' }}</span></div>
              <div class="data-row"><span class="data-label">水浸检测</span><span class="data-value" :class="waterDetected ? 'text-danger' : ''">{{ waterDetected ? '检测到水浸' : '正常' }}</span></div>
              <div class="data-row"><span class="data-label">水位高度</span><span class="data-value">{{ waterLevel }}cm</span></div>
              <div class="data-row"><span class="data-label">风险等级</span><span class="data-value" :class="waterRiskClass">{{ waterRisk }}</span></div>
            </div>
            <div class="scenario-actions">
              <CyberButton variant="danger" size="sm" @click="simulateWater">模拟异常</CyberButton>
              <CyberButton variant="ghost" size="sm" @click="resetWater">恢复正常</CyberButton>
            </div>
          </BaseCard>

          <!-- 入侵检测 -->
          <BaseCard title="人员入侵检测" subtitle="红外+雷达双鉴探测" icon-color="purple">
            <template #header>
              <div class="flex items-center gap-3">
                <div class="card-icon icon-purple">
                  <svg width="20" height="20" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M20 21v-2a4 4 0 0 0-4-4H8a4 4 0 0 0-4 4v2"/><circle cx="12" cy="7" r="4"/></svg>
                </div>
                <div><h3 class="card-title">人员入侵检测</h3><p class="card-subtitle">红外+雷达双鉴探测</p></div>
              </div>
            </template>
            <div class="scenario-data">
              <div class="data-row"><span class="data-label">探测器状态</span><span class="data-value" :class="intrusionOnline ? 'text-success' : 'text-danger'">{{ intrusionOnline ? '在线' : '离线' }}</span></div>
              <div class="data-row"><span class="data-label">红外检测</span><span class="data-value" :class="infraredDetected ? 'text-danger' : ''">{{ infraredDetected ? '检测到移动' : '正常' }}</span></div>
              <div class="data-row"><span class="data-label">雷达检测</span><span class="data-value" :class="radarDetected ? 'text-danger' : ''">{{ radarDetected ? '检测到目标' : '正常' }}</span></div>
              <div class="data-row"><span class="data-label">风险等级</span><span class="data-value" :class="intrusionRiskClass">{{ intrusionRisk }}</span></div>
            </div>
            <div class="scenario-actions">
              <CyberButton variant="danger" size="sm" @click="simulateIntrusion">模拟入侵</CyberButton>
              <CyberButton variant="ghost" size="sm" @click="resetIntrusion">恢复正常</CyberButton>
            </div>
          </BaseCard>

          <!-- 气体检测 -->
          <BaseCard title="有害气体防护" subtitle="TVOC/CH2O/CO2监测" icon-color="yellow">
            <template #header>
              <div class="flex items-center gap-3">
                <div class="card-icon icon-yellow">
                  <svg width="20" height="20" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M18.5 8c.83 0 1.5-.67 1.5-1.5S19.33 5 18.5 5c-.17 0-.34.03-.5.08A2.5 2.5 0 0 0 16 3a2.5 2.5 0 0 0-2 4v.02A1.5 1.5 0 0 0 13 8.5c0 .83.67 1.5 1.5 1.5h4z"/></svg>
                </div>
                <div><h3 class="card-title">有害气体防护</h3><p class="card-subtitle">TVOC/CH2O/CO2监测</p></div>
              </div>
            </template>
            <div class="scenario-data">
              <div class="data-row"><span class="data-label">TVOC浓度</span><span class="data-value">{{ tvocValue }}ppb</span></div>
              <div class="data-row"><span class="data-label">甲醛浓度</span><span class="data-value">{{ ch2oValue }}ppb</span></div>
              <div class="data-row"><span class="data-label">CO₂浓度</span><span class="data-value">{{ co2Value }}ppm</span></div>
              <div class="data-row"><span class="data-label">风险等级</span><span class="data-value" :class="gasRiskClass">{{ gasRisk }}</span></div>
            </div>
            <div class="scenario-actions">
              <CyberButton variant="danger" size="sm" @click="simulateGas">模拟异常</CyberButton>
              <CyberButton variant="ghost" size="sm" @click="resetGas">恢复正常</CyberButton>
            </div>
          </BaseCard>
        </div>
      </section>

      <!-- 数据流向和实时监控 -->
      <section class="monitor-section">
        <div class="flow-panel">
          <BaseCard title="安防数据流向" subtitle="实时数据传输" icon-color="blue">
            <template #header>
              <div class="flex items-center gap-3">
                <div class="card-icon icon-blue">
                  <svg width="20" height="20" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M22 12h-4l-3 9L9 3l-3 9H2"/></svg>
                </div>
                <div><h3 class="card-title">安防数据流向</h3><p class="card-subtitle">实时数据传输</p></div>
              </div>
            </template>
            <canvas ref="flowCanvasRef" class="flow-canvas"></canvas>
          </BaseCard>
        </div>
        <div class="stream-panel">
          <BaseCard title="安防事件流" subtitle="实时告警监控" icon-color="red">
            <template #header>
              <div class="flex items-center gap-3">
                <div class="card-icon icon-red">
                  <svg width="20" height="20" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M10.29 3.86L1.82 18a2 2 0 0 0 1.71 3h16.94a2 2 0 0 0 1.71-3L13.71 3.86a2 2 0 0 0-3.42 0z"/><line x1="12" y1="9" x2="12" y2="13"/><line x1="12" y1="17" x2="12.01" y2="17"/></svg>
                </div>
                <div><h3 class="card-title">安防事件流</h3><p class="card-subtitle">实时告警监控</p></div>
              </div>
            </template>
            <LiveDataStream :max-items="20" />
          </BaseCard>
        </div>
      </section>
    </main>
  </div>
</template>

<script setup lang="ts">
import { ref, computed, onMounted, onUnmounted } from 'vue'
import { ElMessage } from 'element-plus'
import ParticleBackground from '../components/ParticleBackground.vue'
import AppNavbar from '../components/AppNavbar.vue'
import { realtimeApi } from '../api/realtime'
import BaseCard from '../components/BaseCard.vue'
import CyberButton from '../components/CyberButton.vue'
import StatusDot from '../components/StatusDot.vue'
import LiveDataStream from '../components/LiveDataStream.vue'

const menuItems = [
  { path: '/', label: '主控台' },
  { path: '/environment', label: '环境监测' },
  { path: '/fire', label: '消防系统' },
  { path: '/devices', label: '设备管理' },
  { path: '/logs', label: '系统日志' }
]

// 状态数据
const systemStatus = ref('normal')
const alarmActive = ref(false)
const onlineCount = ref(8)
const totalCount = ref(10)
const todayEvents = ref(12)

// 水浸检测
const waterOnline = ref(true)
const waterDetected = ref(false)
const waterLevel = ref(0)
const waterRisk = ref('低')
const waterRiskClass = computed(() => waterRisk.value === '高' ? 'text-danger' : waterRisk.value === '中' ? 'text-warning' : '')

// 入侵检测
const intrusionOnline = ref(true)
const infraredDetected = ref(false)
const radarDetected = ref(false)
const intrusionRisk = ref('低')
const intrusionRiskClass = computed(() => intrusionRisk.value === '高' ? 'text-danger' : intrusionRisk.value === '中' ? 'text-warning' : '')

// 气体检测
const tvocValue = ref(120)
const ch2oValue = ref(30)
const co2Value = ref(450)
const gasRisk = ref('低')
const gasRiskClass = computed(() => gasRisk.value === '高' ? 'text-danger' : gasRisk.value === '中' ? 'text-warning' : '')

// Canvas 数据流向图
const flowCanvasRef = ref<HTMLCanvasElement | null>(null)
let flowAnimId = 0

function drawFlowChart() {
  if (!flowCanvasRef.value) return
  const canvas = flowCanvasRef.value
  const ctx = canvas.getContext('2d')
  if (!ctx) return
  const container = canvas.parentElement
  if (!container) return
  canvas.width = container.clientWidth
  canvas.height = 370

  const ox = 220
  const layers = [
    { name: '应用层', y: 20, nodes: [
      { id: 'app-security', x: 400 + ox, icon: '🛡️', label: '安防系统应用', color: '#8b5cf6' }
    ]},
    { name: '原子服务上层', y: 85, nodes: [
      { id: 'upper-security', x: 400 + ox, icon: '🛡️', label: '安防监控服务', color: '#8b5cf6' }
    ]},
    { name: '原子服务下层', y: 155, nodes: [
      { id: 'lower-collect', x: 200 + ox, icon: '📥', label: '数据采集服务', color: '#3b82f6' },
      { id: 'lower-alarm', x: 450 + ox, icon: '🔔', label: '报警判断服务', color: '#ef4444' },
      { id: 'lower-control', x: 650 + ox, icon: '🎮', label: '联动控制服务', color: '#22c55d' },
    ]},
    { name: '设备抽象层', y: 230, nodes: [
      { id: 'abs-water', x: 160 + ox, icon: '💧', label: '虚拟水浸', color: '#06b6d4' },
      { id: 'abs-infrared', x: 350 + ox, icon: '👤', label: '虚拟红外', color: '#8b5cf6' },
      { id: 'abs-gas', x: 540 + ox, icon: '☁️', label: '虚拟气体', color: '#f59e0b' },
      { id: 'abs-door', x: 700 + ox, icon: '🔌', label: '虚拟门禁', color: '#22c55d' },
    ]},
    { name: '设备层', y: 310, nodes: [
      { id: 'sensor-water', x: 160 + ox, icon: '💧', label: '水浸传感器', color: '#06b6d4' },
      { id: 'sensor-infrared', x: 350 + ox, icon: '👤', label: '红外探测器', color: '#8b5cf6' },
      { id: 'sensor-gas', x: 540 + ox, icon: '☁️', label: '气体传感器', color: '#f59e0b' },
      { id: 'sensor-door', x: 700 + ox, icon: '🔌', label: '门禁控制器', color: '#22c55d' },
    ]},
  ]
  // 向上流: 设备→抽象→服务→上层→应用 (仅联动控制向下)
  const connections = [
    { from: 'sensor-water', to: 'abs-water' }, { from: 'sensor-infrared', to: 'abs-infrared' },
    { from: 'sensor-gas', to: 'abs-gas' }, { from: 'sensor-door', to: 'abs-door' },
    { from: 'abs-water', to: 'lower-collect' }, { from: 'abs-infrared', to: 'lower-collect' },
    { from: 'abs-gas', to: 'lower-collect' }, { from: 'abs-door', to: 'lower-collect' },
    { from: 'abs-water', to: 'lower-alarm' }, { from: 'abs-infrared', to: 'lower-alarm' },
    { from: 'lower-collect', to: 'upper-security' }, { from: 'lower-alarm', to: 'upper-security' },
    { from: 'upper-security', to: 'app-security' },
    // 设备控制向下
    { from: 'app-security', to: 'lower-control' },
    { from: 'lower-control', to: 'abs-water' }, { from: 'lower-control', to: 'abs-infrared' }, { from: 'lower-control', to: 'abs-door' },
    { from: 'abs-water', to: 'sensor-water' }, { from: 'abs-door', to: 'sensor-door' },
  ]

  let time = 0
  function animate() {
    if (!ctx) return
    ctx.clearRect(0, 0, canvas.width, canvas.height)
    time += 0.02

    layers.forEach((layer, i) => {
      ctx.fillStyle = i % 2 === 0 ? 'rgba(59,130,246,0.03)' : 'rgba(139,92,246,0.03)'
      ctx.fillRect(0, layer.y - 12, canvas.width, 58)
      ctx.font = '11px sans-serif'; ctx.fillStyle = '#64748b'; ctx.textAlign = 'left'
      ctx.fillText(layer.name, 10, layer.y + 22)
    })

    connections.forEach(conn => {
      const fromLayer = layers.find(l => l.nodes.some(n => n.id === conn.from))
      const toLayer = layers.find(l => l.nodes.some(n => n.id === conn.to))
      if (!fromLayer || !toLayer) return
      const fromNode = fromLayer.nodes.find(n => n.id === conn.from)
      const toNode = toLayer.nodes.find(n => n.id === conn.to)
      if (!fromNode || !toNode) return

      const fy = fromLayer.y + 25; const ty = toLayer.y + 25
      ctx.beginPath(); ctx.moveTo(fromNode.x, fy); ctx.lineTo(toNode.x, ty)
      ctx.strokeStyle = '#8b5cf670'; ctx.lineWidth = 1.8; ctx.stroke()

      const prog = ((time * 0.3 + fromNode.x * 0.01) % 1 + 1) % 1
      const px = fromNode.x + (toNode.x - fromNode.x) * prog
      const py = fy + (ty - fy) * prog
      ctx.beginPath(); ctx.arc(px, py, 3.5, 0, Math.PI * 2)
      ctx.fillStyle = '#8b5cf6'; ctx.fill()
    })

    layers.forEach(layer => {
      layer.nodes.forEach(node => {
        const ny = layer.y + 25; const r = 17
        const g = ctx.createRadialGradient(node.x, ny, r*0.5, node.x, ny, r*2)
        g.addColorStop(0, node.color + '25'); g.addColorStop(1, 'transparent')
        ctx.beginPath(); ctx.arc(node.x, ny, r*2, 0, Math.PI*2); ctx.fillStyle = g; ctx.fill()
        ctx.beginPath(); ctx.arc(node.x, ny, r, 0, Math.PI*2)
        ctx.fillStyle = node.color + '35'; ctx.strokeStyle = node.color; ctx.lineWidth = 2
        ctx.fill(); ctx.stroke()
        ctx.font = '13px serif'; ctx.textAlign = 'center'; ctx.textBaseline = 'middle'
        ctx.fillStyle = '#fff'; ctx.fillText(node.icon, node.x, ny)
        ctx.font = '9px sans-serif'; ctx.fillStyle = node.color
        ctx.fillText(node.label, node.x, ny + r + 11)
      })
    })
    flowAnimId = requestAnimationFrame(animate)
  }
  animate()
}

onMounted(() => { drawFlowChart(); fetchSecurityData() })
onUnmounted(() => { if (flowAnimId) cancelAnimationFrame(flowAnimId); if (secTimer) clearInterval(secTimer) })

// C++ 实时安防数据轮询
let secTimer: ReturnType<typeof setInterval> | null = null
const secCppOnline = ref(false)

async function fetchSecurityData() {
  try {
    const res = await realtimeApi.getAllData() as any
    const d = res.data || res
    if (d) {
      secCppOnline.value = true
      waterOnline.value = d.water_online !== undefined ? d.water_online : waterOnline.value
      waterDetected.value = (d.water || 0) > 0
      intrusionOnline.value = d.ir_online !== undefined ? d.ir_online : intrusionOnline.value
      infraredDetected.value = (d.ir || 0) > 0
      radarDetected.value = (d.radar || 0) > 0
      if (d.tvoc !== undefined) tvocValue.value = d.tvoc
      if (d.ch2o !== undefined) ch2oValue.value = d.ch2o
      if (d.co2 !== undefined) co2Value.value = d.co2
      // 更新在线计数
      const allOnline = [d.pm25_online,d.humidity_online,d.temperature_online,d.co2_online,d.smoke_online,d.water_online,d.ir_online,d.light_online].filter(Boolean).length
      onlineCount.value = allOnline
    }
  } catch {
    secCppOnline.value = false
  }
}
secTimer = setInterval(fetchSecurityData, 2000)

// 模拟操作
function simulateWater() {
  waterDetected.value = true
  waterLevel.value = 5.2
  waterRisk.value = '高'
  alarmActive.value = true
  ElMessage.warning('模拟水浸异常')
}

function resetWater() {
  waterDetected.value = false
  waterLevel.value = 0
  waterRisk.value = '低'
  alarmActive.value = false
  ElMessage.success('水浸状态已重置')
}

function simulateIntrusion() {
  infraredDetected.value = true
  radarDetected.value = true
  intrusionRisk.value = '高'
  alarmActive.value = true
  ElMessage.warning('模拟人员入侵')
}

function resetIntrusion() {
  infraredDetected.value = false
  radarDetected.value = false
  intrusionRisk.value = '低'
  alarmActive.value = false
  ElMessage.success('入侵状态已重置')
}

function simulateGas() {
  tvocValue.value = 580
  ch2oValue.value = 120
  co2Value.value = 1200
  gasRisk.value = '高'
  alarmActive.value = true
  ElMessage.warning('模拟气体异常')
}

function resetGas() {
  tvocValue.value = 120
  ch2oValue.value = 30
  co2Value.value = 450
  gasRisk.value = '低'
  alarmActive.value = false
  ElMessage.success('气体状态已重置')
}
</script>

<style scoped>
.security-root { min-height: 100vh; background: var(--bg-primary); position: relative; }
.security-main { position: relative; z-index: 1; padding: var(--spacing-xl); max-width: 1920px; margin: 0 auto; }

.section-header { margin-bottom: var(--spacing-lg); }
.section-title { display: flex; align-items: center; gap: var(--spacing-sm); font-size: 20px; font-weight: 600; color: var(--text-primary); margin: 0; }
.section-title svg { color: var(--accent-primary); }

.status-section { margin-bottom: var(--spacing-xl); }
.status-grid { display: grid; grid-template-columns: repeat(4, 1fr); gap: var(--spacing-lg); }
.status-card { display: flex; align-items: center; gap: var(--spacing-md); padding: var(--spacing-lg); background: var(--bg-card); border: 1px solid var(--border-primary); border-radius: var(--radius-lg); transition: all var(--duration-fast); }
.status-card:hover { transform: translateY(-2px); box-shadow: var(--shadow-md); }
.status-card.normal { border-left: 4px solid var(--status-success); }
.status-card.warning { border-left: 4px solid var(--status-warning); }
.status-card.danger { border-left: 4px solid var(--status-danger); animation: alarm-border 2s infinite; }
.status-icon { font-size: 32px; }
.status-info { flex: 1; }
.status-label { font-size: 12px; color: var(--text-secondary); text-transform: uppercase; }
.status-value { font-size: 18px; font-weight: 600; color: var(--text-primary); }

.scenarios-section { margin-bottom: var(--spacing-xl); }
.scenarios-grid { display: grid; grid-template-columns: repeat(3, 1fr); gap: var(--spacing-xl); }

.scenario-data { margin-bottom: var(--spacing-md); }
.data-row { display: flex; justify-content: space-between; align-items: center; padding: var(--spacing-sm) 0; border-bottom: 1px solid var(--border-primary); }
.data-row:last-child { border-bottom: none; }
.data-label { font-size: 13px; color: var(--text-secondary); }
.data-value { font-size: 14px; font-weight: 600; color: var(--text-primary); font-family: var(--font-mono); }
.text-success { color: var(--status-success); }
.text-warning { color: var(--status-warning); }
.text-danger { color: var(--status-danger); }

.scenario-actions { display: flex; gap: var(--spacing-sm); padding-top: var(--spacing-md); border-top: 1px solid var(--border-primary); }

.monitor-section { display: grid; grid-template-columns: 1.2fr 1fr; gap: var(--spacing-xl); }
.flow-canvas { width: 100%; height: 370px; background: rgba(0,0,0,0.2); border-radius: var(--radius-md); }

@media (max-width: 1200px) {
  .status-grid { grid-template-columns: repeat(2, 1fr); }
  .scenarios-grid { grid-template-columns: 1fr; }
  .monitor-section { grid-template-columns: 1fr; }
}
@media (max-width: 768px) {
  .security-main { padding: var(--spacing-md); }
  .status-grid { grid-template-columns: 1fr; }
}
</style>
