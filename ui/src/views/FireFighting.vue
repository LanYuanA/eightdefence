<template>
  <div class="fire-root">
    <ParticleBackground :particle-count="60" color="#ef4444" :opacity="0.5" />
    <AppNavbar title="智能消防系统" subtitle="Fire Fighting System" :menu-items="menuItems" />

    <main class="fire-main">
      <!-- 火灾风险概览 -->
      <section class="risk-section">
        <div class="section-header">
          <h2 class="section-title">
            <svg width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M8.5 14.5A2.5 2.5 0 0 0 11 12c0-1.38-.5-2-1-3-1.072-2.143-.224-4.054 2-6 .5 2.5 2 4.9 4 6.5 2 1.6 3 3.5 3 5.5a7 7 0 1 1-14 0c0-1.153.433-2.294 1-3a2.5 2.5 0 0 0 2.5 2.5z"/></svg>
            火灾风险评估
          </h2>
          <div class="header-actions">
            <CyberButton variant="danger" size="sm" @click="emergencyAction">
              <svg width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M10.29 3.86L1.82 18a2 2 0 0 0 1.71 3h16.94a2 2 0 0 0 1.71-3L13.71 3.86a2 2 0 0 0-3.42 0z"/><line x1="12" y1="9" x2="12" y2="13"/><line x1="12" y1="17" x2="12.01" y2="17"/></svg>
              应急联动
            </CyberButton>
          </div>
        </div>

        <!-- 区域选择和风险指示 -->
        <div class="region-risk-grid">
          <div class="region-selector">
            <div class="selector-label">监测区域</div>
            <div class="region-tabs">
              <button v-for="r in regions" :key="r.id" class="region-tab" :class="{ active: currentRegion === r.id }" @click="currentRegion = r.id">
                {{ r.name }}
              </button>
            </div>
          </div>
          <div class="risk-indicator" :class="riskLevel">
            <div class="risk-icon">{{ riskIcon }}</div>
            <div class="risk-info">
              <div class="risk-level">风险等级: {{ riskText }}</div>
              <div class="risk-desc">{{ riskDescription }}</div>
            </div>
            <div class="risk-bar">
              <div class="risk-fill" :style="{ width: riskPercent + '%' }"></div>
            </div>
          </div>
        </div>
      </section>

      <!-- 实时监测数据 -->
      <section class="realtime-section">
        <div class="realtime-grid">
          <DataCard v-for="(card, i) in fireCards" :key="card.label" v-bind="card" :delay="i * 100" />
        </div>
      </section>

      <!-- 图表和控制 -->
      <section class="charts-control-section">
        <!-- 趋势图表 -->
        <div class="charts-area">
          <BaseCard title="温度趋势" subtitle="火灾风险预测" icon-color="red">
            <template #header>
              <div class="flex items-center gap-3">
                <div class="card-icon icon-red">
                  <svg width="20" height="20" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M14 14.76V3.5a2.5 2.5 0 0 0-5 0v11.26a4.5 4.5 0 1 0 5 0z"/></svg>
                </div>
                <div><h3 class="card-title">温度趋势</h3><p class="card-subtitle">火灾风险预测</p></div>
              </div>
            </template>
            <div class="chart-box"><canvas ref="tempChartRef"></canvas></div>
          </BaseCard>
          <BaseCard title="烟雾浓度" subtitle="实时监测" icon-color="yellow">
            <template #header>
              <div class="flex items-center gap-3">
                <div class="card-icon icon-yellow">
                  <svg width="20" height="20" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M18.5 8c.83 0 1.5-.67 1.5-1.5S19.33 5 18.5 5c-.17 0-.34.03-.5.08A2.5 2.5 0 0 0 16 3a2.5 2.5 0 0 0-2 4v.02A1.5 1.5 0 0 0 13 8.5c0 .83.67 1.5 1.5 1.5h4z"/></svg>
                </div>
                <div><h3 class="card-title">烟雾浓度</h3><p class="card-subtitle">实时监测</p></div>
              </div>
            </template>
            <div class="chart-box"><canvas ref="smokeChartRef"></canvas></div>
          </BaseCard>
        </div>

        <!-- 设备控制 -->
        <div class="control-area">
          <BaseCard title="设备联动控制" subtitle="消防设备管理" icon-color="cyan">
            <template #header>
              <div class="flex items-center gap-3">
                <div class="card-icon icon-cyan">
                  <svg width="20" height="20" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M12 22s8-4 8-10V5l-8-3-8 3v7c0 6 8 10 8 10z"/></svg>
                </div>
                <div><h3 class="card-title">设备联动控制</h3><p class="card-subtitle">消防设备管理</p></div>
              </div>
            </template>
            <div class="device-controls">
              <div v-for="dev in fireDevices" :key="dev.id" class="device-ctrl" :class="{ active: dev.active }">
                <span class="dev-icon">{{ dev.icon }}</span>
                <div class="dev-info">
                  <div class="dev-name">{{ dev.name }}</div>
                  <div class="dev-desc">{{ dev.desc }}</div>
                </div>
                <div class="dev-switch" :class="{ on: dev.active }" @click="toggleDevice(dev)">
                  <div class="switch-thumb"></div>
                </div>
              </div>
            </div>
          </BaseCard>

          <!-- 阈值设置 -->
          <BaseCard title="预警阈值设置" subtitle="报警参数配置" icon-color="purple">
            <template #header>
              <div class="flex items-center gap-3">
                <div class="card-icon icon-purple">
                  <svg width="20" height="20" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><circle cx="12" cy="12" r="3"/><path d="M19.4 15a1.65 1.65 0 0 0 .33 1.82l.06.06a2 2 0 0 1 0 2.83 2 2 0 0 1-2.83 0l-.06-.06a1.65 1.65 0 0 0-1.82-.33 1.65 1.65 0 0 0-1 1.51V21a2 2 0 0 1-2 2 2 2 0 0 1-2-2v-.09A1.65 1.65 0 0 0 9 19.4a1.65 1.65 0 0 0-1.82.33l-.06.06a2 2 0 0 1-2.83 0 2 2 0 0 1 0-2.83l.06-.06A1.65 1.65 0 0 0 4.68 15a1.65 1.65 0 0 0-1.51-1H3a2 2 0 0 1-2-2 2 2 0 0 1 2-2h.09A1.65 1.65 0 0 0 4.6 9a1.65 1.65 0 0 0-.33-1.82l-.06-.06a2 2 0 0 1 0-2.83 2 2 0 0 1 2.83 0l.06.06A1.65 1.65 0 0 0 9 4.68a1.65 1.65 0 0 0 1-1.51V3a2 2 0 0 1 2-2 2 2 0 0 1 2 2v.09a1.65 1.65 0 0 0 1 1.51 1.65 1.65 0 0 0 1.82-.33l.06-.06a2 2 0 0 1 2.83 0 2 2 0 0 1 0 2.83l-.06.06A1.65 1.65 0 0 0 19.4 9a1.65 1.65 0 0 0 1.51 1H21a2 2 0 0 1 2 2 2 2 0 0 1-2 2h-.09a1.65 1.65 0 0 0-1.51 1z"/></svg>
                </div>
                <div><h3 class="card-title">预警阈值设置</h3><p class="card-subtitle">报警参数配置</p></div>
              </div>
            </template>
            <div class="threshold-list">
              <div v-for="t in thresholds" :key="t.key" class="threshold-item">
                <div class="th-label">{{ t.label }} <span class="th-peak">峰值: {{ t.peak }}{{ t.unit }}</span></div>
                <div class="th-input-row">
                  <input type="number" v-model.number="t.value" class="th-input" />
                  <span class="th-unit">{{ t.unit }}</span>
                  <CyberButton variant="ghost" size="sm" @click="saveThreshold(t)">保存</CyberButton>
                </div>
              </div>
            </div>
          </BaseCard>
        </div>
      </section>

      <!-- 数据流向和事件流 -->
      <section class="flow-stream-section">
        <div class="flow-panel">
          <BaseCard title="消防数据流向" subtitle="实时数据传输" icon-color="red">
            <template #header>
              <div class="flex items-center gap-3">
                <div class="card-icon icon-red">
                  <svg width="20" height="20" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M22 12h-4l-3 9L9 3l-3 9H2"/></svg>
                </div>
                <div><h3 class="card-title">消防数据流向</h3><p class="card-subtitle">实时数据传输</p></div>
              </div>
            </template>
            <canvas ref="flowCanvasRef" class="flow-canvas"></canvas>
          </BaseCard>
        </div>
        <div class="stream-panel">
          <BaseCard title="消防事件流" subtitle="实时告警监控" icon-color="yellow">
            <template #header>
              <div class="flex items-center gap-3">
                <div class="card-icon icon-yellow">
                  <svg width="20" height="20" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M13 2L3 14h9l-1 8 10-12h-9l1-8z"/></svg>
                </div>
                <div><h3 class="card-title">消防事件流</h3><p class="card-subtitle">实时告警监控</p></div>
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
import { ref, reactive, computed, onMounted, onUnmounted } from 'vue'
import { ElMessage } from 'element-plus'
import { Chart, registerables } from 'chart.js'
import ParticleBackground from '../components/ParticleBackground.vue'
import AppNavbar from '../components/AppNavbar.vue'
import DataCard from '../components/DataCard.vue'
import BaseCard from '../components/BaseCard.vue'
import CyberButton from '../components/CyberButton.vue'
import LiveDataStream from '../components/LiveDataStream.vue'

Chart.register(...registerables)

const menuItems = [
  { path: '/', label: '主控台' },
  { path: '/security', label: '安防系统' },
  { path: '/environment', label: '环境监测' },
  { path: '/devices', label: '设备管理' },
  { path: '/logs', label: '系统日志' }
]

const currentRegion = ref('A')
const regions = [
  { id: 'A', name: '区域A' },
  { id: 'B', name: '区域B' },
  { id: 'C', name: '区域C' }
]

// 风险计算
const riskLevel = computed(() => {
  const temp = fireCards[0].value
  if (temp > 60) return 'critical'
  if (temp > 45) return 'high'
  if (temp > 35) return 'medium'
  return 'low'
})
const riskText = computed(() => ({ critical: '极高', high: '高', medium: '中', low: '低' }[riskLevel.value]))
const riskIcon = computed(() => ({ critical: '🔴', high: '🟠', medium: '🟡', low: '🟢' }[riskLevel.value]))
const riskDescription = computed(() => ({ critical: '立即疏散！火情确认', high: '高度警戒，准备应急', medium: '加强监控，预防为主', low: '安全状态，正常运行' }[riskLevel.value]))
const riskPercent = computed(() => ({ critical: 95, high: 70, medium: 40, low: 15 }[riskLevel.value]))

const fireCards = reactive([
  { label: '当前温度', value: 28.5, unit: '℃', icon: '<path d="M14 14.76V3.5a2.5 2.5 0 0 0-5 0v11.26a4.5 4.5 0 1 0 5 0z"/>', iconColor: 'red' as const, status: 'normal' as const, trend: '上升', trendDirection: 'up' as const, miniChartData: [26,27,27.5,28,28.2,28.5,28.5] },
  { label: '烟雾浓度', value: 0.3, unit: '%obs/m', icon: '<path d="M18.5 8c.83 0 1.5-.67 1.5-1.5S19.33 5 18.5 5c-.17 0-.34.03-.5.08A2.5 2.5 0 0 0 16 3a2.5 2.5 0 0 0-2 4v.02A1.5 1.5 0 0 0 13 8.5c0 .83.67 1.5 1.5 1.5h4z"/>', iconColor: 'yellow' as const, status: 'normal' as const, trend: '稳定', trendDirection: 'stable' as const, miniChartData: [0.2,0.25,0.3,0.28,0.3,0.32,0.3] },
  { label: 'CO₂浓度', value: 520, unit: 'ppm', icon: '<circle cx="12" cy="12" r="10"/><path d="M12 6v6l4 2"/>', iconColor: 'purple' as const, status: 'normal' as const, trend: '正常', trendDirection: 'stable' as const, miniChartData: [510,515,520,518,522,520,520] },
  { label: '湿度', value: 45, unit: '%', icon: '<path d="M12 2.69l5.66 5.66a8 8 0 1 1-11.31 0z"/>', iconColor: 'blue' as const, status: 'normal' as const, trend: '下降', trendDirection: 'down' as const, miniChartData: [50,48,47,46,45.5,45,45] }
])

const fireDevices = reactive([
  { id: 1, name: '舱门控制', desc: '火情联动紧急开启', icon: '🚪', active: false },
  { id: 2, name: '排烟风机', desc: '火情确认后自动开启', icon: '🌀', active: false },
  { id: 3, name: '喷淋系统', desc: '温度过高自动启动', icon: '💦', active: false },
  { id: 4, name: '声光报警', desc: '检测到火情立即报警', icon: '🔔', active: false }
])

const thresholds = reactive([
  { key: 'temp', label: '温度报警阈值', value: 55, unit: '℃', peak: 42.5 },
  { key: 'smoke', label: '烟雾报警阈值', value: 2.0, unit: '%obs/m', peak: 1.2 },
  { key: 'co2', label: 'CO₂报警阈值', value: 1000, unit: 'ppm', peak: 680 }
])

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
  canvas.height = 380

  const ox = 220
  const layers = [
    { name: '设备层', y: 320, nodes: [
      { id: 'sensor-smoke', x: 100 + ox, icon: '🔥', label: '烟雾报警', color: '#ef4444' },
      { id: 'sensor-temp', x: 260 + ox, icon: '🌡️', label: '温度传感', color: '#f59e0b' },
      { id: 'sensor-spray', x: 420 + ox, icon: '💦', label: '喷淋控制', color: '#06b6d4' },
      { id: 'sensor-fan', x: 560 + ox, icon: '🌀', label: '排烟风机', color: '#22c55d' }
    ]},
    { name: '设备抽象层', y: 240, nodes: [
      { id: 'abs-fire', x: 330 + ox, icon: '🔥', label: '消防抽象', color: '#ef4444' }
    ]},
    { name: '原子服务下层', y: 160, nodes: [
      { id: 'lower-collect', x: 200 + ox, icon: '📥', label: '数据采集', color: '#3b82f6' },
      { id: 'lower-alarm', x: 400 + ox, icon: '🔔', label: '报警判断', color: '#ef4444' },
      { id: 'lower-control', x: 550 + ox, icon: '🎮', label: '设备控制', color: '#22c55d' }
    ]},
    { name: '原子服务上层', y: 80, nodes: [
      { id: 'upper-fire', x: 350 + ox, icon: '🔥', label: '消防预警服务', color: '#ef4444' }
    ]},
    { name: '应用层', y: 20, nodes: [
      { id: 'app-fire', x: 350 + ox, icon: '🔥', label: '消防系统', color: '#ef4444' }
    ]}
  ]
  const connections = [
    { from: 'sensor-smoke', to: 'abs-fire' }, { from: 'sensor-temp', to: 'abs-fire' },
    { from: 'sensor-spray', to: 'abs-fire' }, { from: 'sensor-fan', to: 'abs-fire' },
    { from: 'abs-fire', to: 'lower-collect' },
    { from: 'lower-collect', to: 'lower-alarm' }, { from: 'lower-alarm', to: 'lower-control' },
    { from: 'lower-collect', to: 'upper-fire' }, { from: 'lower-alarm', to: 'upper-fire' }, { from: 'lower-control', to: 'upper-fire' },
    { from: 'upper-fire', to: 'app-fire' }
  ]

  let time = 0
  function animate() {
    if (!ctx) return
    ctx.clearRect(0, 0, canvas.width, canvas.height)
    time += 0.02

    layers.forEach((layer, i) => {
      ctx.fillStyle = i % 2 === 0 ? 'rgba(239,68,68,0.03)' : 'rgba(245,158,11,0.03)'
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

      ctx.beginPath(); ctx.moveTo(fromNode.x, fromLayer.y + 30); ctx.lineTo(toNode.x, toLayer.y + 30)
      ctx.strokeStyle = '#ef4444'; ctx.lineWidth = 2; ctx.globalAlpha = 0.5; ctx.stroke()

      const t = (time % 2) / 2
      const px = fromNode.x + (toNode.x - fromNode.x) * t
      const py = fromLayer.y + 30 + (toLayer.y + 30 - fromLayer.y - 30) * t
      ctx.beginPath(); ctx.arc(px, py, 4, 0, Math.PI * 2)
      ctx.fillStyle = '#ef4444'; ctx.globalAlpha = 0.9; ctx.fill()
      ctx.beginPath(); ctx.arc(px, py, 8, 0, Math.PI * 2)
      ctx.fillStyle = '#ef4444'; ctx.globalAlpha = 0.25; ctx.fill()
    })
    ctx.globalAlpha = 1

    layers.forEach(layer => {
      layer.nodes.forEach(node => {
        ctx.beginPath(); ctx.arc(node.x, layer.y + 30, 20, 0, Math.PI * 2)
        ctx.fillStyle = node.color + '30'; ctx.strokeStyle = node.color; ctx.lineWidth = 2
        ctx.fill(); ctx.stroke()
        ctx.font = '16px serif'; ctx.textAlign = 'center'; ctx.textBaseline = 'middle'
        ctx.fillStyle = '#ffffff'; ctx.fillText(node.icon, node.x, layer.y + 30)
        ctx.font = '10px sans-serif'; ctx.fillStyle = node.color
        ctx.fillText(node.label, node.x, layer.y + 55)
      })
    })
    flowAnimId = requestAnimationFrame(animate)
  }
  animate()
}

const tempChartRef = ref<HTMLCanvasElement | null>(null)
const smokeChartRef = ref<HTMLCanvasElement | null>(null)
let tempChart: Chart | null = null
let smokeChart: Chart | null = null

function initCharts() {
  if (tempChartRef.value) {
    tempChart = new Chart(tempChartRef.value, {
      type: 'line',
      data: { labels: ['00:00','04:00','08:00','12:00','16:00','20:00'], datasets: [{ label: '温度', data: [26,27,27.5,28,28.2,28.5], borderColor: '#ef4444', backgroundColor: 'rgba(239,68,68,0.1)', borderWidth: 2, tension: 0.4, fill: true }] },
      options: { responsive: true, maintainAspectRatio: false, plugins: { legend: { display: false } }, scales: { y: { min: 20, max: 40 } } }
    })
  }
  if (smokeChartRef.value) {
    smokeChart = new Chart(smokeChartRef.value, {
      type: 'line',
      data: { labels: ['00:00','04:00','08:00','12:00','16:00','20:00'], datasets: [{ label: '烟雾', data: [0.2,0.25,0.3,0.28,0.3,0.32], borderColor: '#f59e0b', backgroundColor: 'rgba(245,158,11,0.1)', borderWidth: 2, tension: 0.4, fill: true }] },
      options: { responsive: true, maintainAspectRatio: false, plugins: { legend: { display: false } }, scales: { y: { min: 0, max: 2 } } }
    })
  }
}

function emergencyAction() { ElMessage.warning('应急联动处置已启动') }
function toggleDevice(dev: any) { dev.active = !dev.active; ElMessage.success(`${dev.name} 已${dev.active ? '开启' : '关闭'}`) }
function saveThreshold(t: any) { ElMessage.success(`${t.label} 已保存`) }

onMounted(() => { initCharts(); drawFlowChart() })
onUnmounted(() => { tempChart?.destroy(); smokeChart?.destroy(); if (flowAnimId) cancelAnimationFrame(flowAnimId) })
</script>

<style scoped>
.fire-root { min-height: 100vh; background: var(--bg-primary); position: relative; }
.fire-main { position: relative; z-index: 1; padding: var(--spacing-xl); max-width: 1920px; margin: 0 auto; }

.section-header { display: flex; align-items: center; justify-content: space-between; margin-bottom: var(--spacing-lg); }
.section-title { display: flex; align-items: center; gap: var(--spacing-sm); font-size: 20px; font-weight: 600; color: var(--text-primary); margin: 0; }
.section-title svg { color: var(--status-danger); }
.header-actions { display: flex; gap: var(--spacing-sm); }

.risk-section { margin-bottom: var(--spacing-xl); }
.region-risk-grid { display: grid; grid-template-columns: auto 1fr; gap: var(--spacing-xl); }
.region-selector { display: flex; flex-direction: column; gap: var(--spacing-sm); }
.selector-label { font-size: 13px; color: var(--text-secondary); }
.region-tabs { display: flex; gap: var(--spacing-xs); }
.region-tab { padding: 8px 16px; background: transparent; border: 1px solid var(--border-primary); border-radius: var(--radius-md); color: var(--text-secondary); cursor: pointer; transition: all var(--duration-fast); }
.region-tab:hover { border-color: var(--accent-primary); color: var(--text-primary); }
.region-tab.active { background: rgba(59,130,246,0.2); border-color: var(--accent-primary); color: var(--accent-primary); }

.risk-indicator { display: flex; align-items: center; gap: var(--spacing-lg); padding: var(--spacing-lg); background: var(--bg-card); border: 1px solid var(--border-primary); border-radius: var(--radius-lg); }
.risk-indicator.low { border-left: 4px solid var(--status-success); }
.risk-indicator.medium { border-left: 4px solid var(--status-warning); }
.risk-indicator.high { border-left: 4px solid #f97316; }
.risk-indicator.critical { border-left: 4px solid var(--status-danger); animation: alarm-border 1s infinite; }
.risk-icon { font-size: 48px; }
.risk-info { flex: 1; }
.risk-level { font-size: 18px; font-weight: 600; color: var(--text-primary); }
.risk-desc { font-size: 13px; color: var(--text-secondary); margin-top: 4px; }
.risk-bar { width: 200px; height: 8px; background: var(--bg-primary); border-radius: 4px; overflow: hidden; }
.risk-fill { height: 100%; background: var(--status-danger); border-radius: 4px; transition: width 0.5s ease; }

.realtime-section { margin-bottom: var(--spacing-xl); }
.realtime-grid { display: grid; grid-template-columns: repeat(4, 1fr); gap: var(--spacing-lg); }

.charts-control-section { display: grid; grid-template-columns: 1fr 1fr; gap: var(--spacing-xl); margin-bottom: var(--spacing-xl); }
.charts-area { display: flex; flex-direction: column; gap: var(--spacing-xl); }
.chart-box { height: 200px; padding: var(--spacing-md); }
.control-area { display: flex; flex-direction: column; gap: var(--spacing-xl); }

.device-controls { display: flex; flex-direction: column; gap: var(--spacing-sm); }
.device-ctrl { display: flex; align-items: center; gap: var(--spacing-md); padding: var(--spacing-md); background: rgba(0,0,0,0.2); border: 1px solid var(--border-primary); border-radius: var(--radius-md); transition: all var(--duration-fast); }
.device-ctrl.active { border-color: var(--status-success); background: rgba(34,197,94,0.05); }
.dev-icon { font-size: 24px; }
.dev-info { flex: 1; }
.dev-name { font-size: 14px; font-weight: 500; color: var(--text-primary); }
.dev-desc { font-size: 12px; color: var(--text-secondary); }
.dev-switch { width: 48px; height: 24px; background: var(--bg-primary); border: 1px solid var(--border-primary); border-radius: 12px; cursor: pointer; position: relative; transition: all var(--duration-fast); }
.dev-switch.on { background: var(--status-success); border-color: var(--status-success); }
.switch-thumb { position: absolute; top: 2px; left: 2px; width: 18px; height: 18px; background: white; border-radius: 50%; transition: transform var(--duration-fast); }
.dev-switch.on .switch-thumb { transform: translateX(24px); }

.threshold-list { display: flex; flex-direction: column; gap: var(--spacing-md); }
.threshold-item { padding: var(--spacing-md); background: rgba(0,0,0,0.2); border-radius: var(--radius-md); }
.th-label { font-size: 14px; color: var(--text-primary); margin-bottom: var(--spacing-sm); }
.th-peak { font-size: 12px; color: var(--status-danger); margin-left: var(--spacing-sm); }
.th-input-row { display: flex; align-items: center; gap: var(--spacing-sm); }
.th-input { width: 80px; padding: 6px; background: var(--bg-primary); border: 1px solid var(--border-primary); border-radius: var(--radius-sm); color: var(--text-primary); text-align: center; }
.th-unit { font-size: 12px; color: var(--text-secondary); }

.flow-stream-section { display: grid; grid-template-columns: 1.2fr 1fr; gap: var(--spacing-xl); }

.flow-canvas { width: 100%; height: 380px; background: rgba(0,0,0,0.2); border-radius: var(--radius-md); }

@media (max-width: 1200px) {
  .region-risk-grid { grid-template-columns: 1fr; }
  .realtime-grid { grid-template-columns: repeat(2, 1fr); }
  .charts-control-section { grid-template-columns: 1fr; }
  .flow-stream-section { grid-template-columns: 1fr; }
}
@media (max-width: 768px) {
  .fire-main { padding: var(--spacing-md); }
  .realtime-grid { grid-template-columns: 1fr; }
}
</style>
