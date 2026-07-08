<template>
  <div class="env-root">
    <ParticleBackground :particle-count="40" color="#22c55d" :opacity="0.3" />
    <AppNavbar title="环境监测系统" subtitle="Environment Monitoring System" :menu-items="menuItems" />

    <main class="env-main">
      <!-- 概览卡片 -->
      <section class="overview-section">
        <div class="section-header">
          <h2 class="section-title">
            <svg width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M14 14.76V3.5a2.5 2.5 0 0 0-5 0v11.26a4.5 4.5 0 1 0 5 0z"/></svg>
            环境数据概览
          </h2>
          <div class="header-actions">
            <CyberButton variant="ghost" size="sm" @click="refreshData">刷新数据</CyberButton>
            <CyberButton variant="primary" size="sm" @click="showThresholds = true">阈值设置</CyberButton>
          </div>
        </div>
        <div class="cards-grid">
          <DataCard v-for="(card, i) in envCards" :key="card.label" v-bind="card" :delay="i * 100" />
        </div>
      </section>

      <!-- 图表和数据流 -->
      <section class="charts-flow-section">
        <div class="charts-area">
          <BaseCard title="温度趋势" subtitle="24小时监测" icon-color="red">
            <template #header>
              <div class="flex items-center gap-3">
                <div class="card-icon icon-red">
                  <svg width="20" height="20" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M14 14.76V3.5a2.5 2.5 0 0 0-5 0v11.26a4.5 4.5 0 1 0 5 0z"/></svg>
                </div>
                <div><h3 class="card-title">温度趋势</h3><p class="card-subtitle">24小时监测</p></div>
              </div>
            </template>
            <div class="chart-box"><canvas ref="tempChartRef"></canvas></div>
          </BaseCard>
          <BaseCard title="湿度趋势" subtitle="24小时监测" icon-color="blue">
            <template #header>
              <div class="flex items-center gap-3">
                <div class="card-icon icon-blue">
                  <svg width="20" height="20" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M12 2.69l5.66 5.66a8 8 0 1 1-11.31 0z"/></svg>
                </div>
                <div><h3 class="card-title">湿度趋势</h3><p class="card-subtitle">24小时监测</p></div>
              </div>
            </template>
            <div class="chart-box"><canvas ref="humiChartRef"></canvas></div>
          </BaseCard>
        </div>
        <div class="flow-area">
          <BaseCard title="数据流向" subtitle="实时传输可视化" icon-color="purple">
            <template #header>
              <div class="flex items-center gap-3">
                <div class="card-icon icon-purple">
                  <svg width="20" height="20" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M22 12h-4l-3 9L9 3l-3 9H2"/></svg>
                </div>
                <div><h3 class="card-title">数据流向</h3><p class="card-subtitle">实时传输可视化</p></div>
              </div>
            </template>
            <canvas ref="flowCanvasRef" class="flow-canvas"></canvas>
          </BaseCard>
        </div>
      </section>

      <!-- 设备列表和控制 -->
      <section class="devices-control-section">
        <BaseCard title="在线设备" subtitle="环境监测设备" icon-color="green">
          <template #header>
            <div class="flex items-center justify-between w-full">
              <div class="flex items-center gap-3">
                <div class="card-icon icon-green">
                  <svg width="20" height="20" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><rect x="2" y="3" width="20" height="14" rx="2"/><path d="M8 21h8m-4-4v4"/></svg>
                </div>
                <div><h3 class="card-title">在线设备</h3><p class="card-subtitle">环境监测设备</p></div>
              </div>
              <CyberButton variant="ghost" size="sm" @click="showAddDevice = true">添加设备</CyberButton>
            </div>
          </template>
          <div class="device-list">
            <div v-for="dev in devices" :key="dev.id" class="device-row" :class="{ offline: !dev.online }">
              <span class="dev-icon">{{ dev.icon }}</span>
              <StatusDot :status="dev.online ? 'online' : 'offline'" size="sm" :pulse="dev.online" />
              <div class="dev-info"><div class="dev-name">{{ dev.name }}</div><div class="dev-type">{{ dev.type }}</div></div>
              <div class="dev-val">{{ dev.value }}</div>
              <div class="dev-actions">
                <button class="act-btn" @click="viewDev(dev)">详情</button>
                <button class="act-btn" @click="refreshDev(dev)">刷新</button>
              </div>
            </div>
          </div>
        </BaseCard>

        <BaseCard title="智能联动" subtitle="环境调控" icon-color="cyan">
          <template #header>
            <div class="flex items-center gap-3">
              <div class="card-icon icon-cyan">
                <svg width="20" height="20" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M12 22s8-4 8-10V5l-8-3-8 3v7c0 6 8 10 8 10z"/></svg>
              </div>
              <div><h3 class="card-title">智能联动</h3><p class="card-subtitle">环境调控</p></div>
            </div>
          </template>
          <div class="control-list">
            <div v-for="ctrl in controls" :key="ctrl.id" class="ctrl-item" :class="{ active: ctrl.active }">
              <span class="ctrl-icon">{{ ctrl.icon }}</span>
              <div class="ctrl-info"><div class="ctrl-name">{{ ctrl.name }}</div><div class="ctrl-desc">{{ ctrl.desc }}</div></div>
              <span class="ctrl-status" :class="ctrl.active ? 'on' : 'off'">{{ ctrl.active ? '运行' : '待机' }}</span>
              <button class="ctrl-btn" :class="ctrl.active ? 'btn-off' : 'btn-on'" @click="toggleCtrl(ctrl)">{{ ctrl.active ? '关闭' : '开启' }}</button>
            </div>
          </div>
        </BaseCard>
      </section>

      <!-- 实时数据流 -->
      <section class="stream-section">
        <BaseCard title="实时数据流" subtitle="系统事件监控" icon-color="yellow">
          <template #header>
            <div class="flex items-center gap-3">
              <div class="card-icon icon-yellow">
                <svg width="20" height="20" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M13 2L3 14h9l-1 8 10-12h-9l1-8z"/></svg>
              </div>
              <div><h3 class="card-title">实时数据流</h3><p class="card-subtitle">系统事件监控</p></div>
            </div>
          </template>
            <div class="realtime-events">
              <div v-if="envEvents.length === 0" class="empty-stream">等待数据流...</div>
              <div v-for="evt in envEvents.slice(0, 15)" :key="evt.id" class="realtime-event-row">
                <span class="evt-time">{{ evt.time }}</span>
                <span class="evt-src" :style="{color: evt.color}">{{ evt.source }}</span>
                <span class="evt-val">{{ evt.value }}{{ evt.unit }}</span>
                <span class="evt-status" :class="evt.online ? 'online' : 'offline'">{{ evt.online ? '在线' : '离线' }}</span>
              </div>
            </div>
        </BaseCard>
      </section>
    </main>

    <!-- 阈值设置弹窗 -->
    <Transition name="modal">
      <div v-if="showThresholds" class="modal-overlay" @click.self="showThresholds = false">
        <div class="modal-content">
          <div class="modal-header">
            <h3>环境阈值设置</h3>
            <button class="modal-close" @click="showThresholds = false">×</button>
          </div>
          <div class="modal-body">
            <div v-for="t in thresholdList" :key="t.key" class="threshold-row">
              <span class="th-label">{{ t.label }}</span>
              <div class="th-inputs">
                <input type="number" v-model.number="t.min" class="th-input" />
                <span>~</span>
                <input type="number" v-model.number="t.max" class="th-input" />
                <span class="th-unit">{{ t.unit }}</span>
              </div>
            </div>
          </div>
          <div class="modal-footer">
            <CyberButton variant="ghost" @click="showThresholds = false">取消</CyberButton>
            <CyberButton variant="primary" @click="saveThresholds">保存</CyberButton>
          </div>
        </div>
      </div>
    </Transition>

    <!-- 添加设备弹窗 -->
    <Transition name="modal">
      <div v-if="showAddDevice" class="modal-overlay" @click.self="showAddDevice = false">
        <div class="modal-content">
          <div class="modal-header">
            <h3>添加环境监测设备</h3>
            <button class="modal-close" @click="showAddDevice = false">×</button>
          </div>
          <div class="modal-body">
            <div class="form-grid">
              <div class="form-group">
                <label>设备名称 *</label>
                <input v-model="newDevice.name" placeholder="输入设备名称" />
              </div>
              <div class="form-group">
                <label>设备类型</label>
                <select v-model="newDevice.type">
                  <option value="sensor">传感器</option>
                  <option value="controller">控制器</option>
                  <option value="alarm">报警器</option>
                </select>
              </div>
              <div class="form-group">
                <label>设备型号 *</label>
                <input v-model="newDevice.model" placeholder="输入设备型号" />
              </div>
              <div class="form-group">
                <label>Modbus地址</label>
                <input v-model="newDevice.address" placeholder="0x30" />
              </div>
              <div class="form-group">
                <label>数据单位</label>
                <input v-model="newDevice.unit" placeholder="℃, %, ppm" />
              </div>
              <div class="form-group">
                <label>所属应用</label>
                <select v-model="newDevice.app">
                  <option value="环境监测">环境监测</option>
                  <option value="安防系统">安防系统</option>
                  <option value="消防系统">消防系统</option>
                </select>
              </div>
            </div>
          </div>
          <div class="modal-footer">
            <CyberButton variant="ghost" @click="showAddDevice = false">取消</CyberButton>
            <CyberButton variant="primary" @click="addDevice">添加设备</CyberButton>
          </div>
        </div>
      </div>
    </Transition>
  </div>
</template>

<script setup lang="ts">
import { ref, reactive, onMounted, onUnmounted } from 'vue'
import axios from "axios"
import { ElMessage } from 'element-plus'
import { Chart, registerables } from 'chart.js'
import ParticleBackground from '../components/ParticleBackground.vue'
import AppNavbar from '../components/AppNavbar.vue'
import BaseCard from '../components/BaseCard.vue'
import CyberButton from '../components/CyberButton.vue'
import StatusDot from '../components/StatusDot.vue'
import { realtimeApi } from '../api/realtime'

Chart.register(...registerables)

const menuItems = [
  { path: '/', label: '主控台' },
  { path: '/security', label: '安防系统' },
  { path: '/fire', label: '消防系统' },
  { path: '/devices', label: '设备管理' },
  { path: '/logs', label: '系统日志' }
]

const showThresholds = ref(false)
const showAddDevice = ref(false)

const newDevice = reactive({
  name: '',
  type: 'sensor',
  model: '',
  address: '',
  unit: '',
  app: '环境监测'
})

const envCards = reactive([
  { label: '当前温度', value: 24.5, unit: '℃', icon: '<path d="M14 14.76V3.5a2.5 2.5 0 0 0-5 0v11.26a4.5 4.5 0 1 0 5 0z"/>', iconColor: 'red' as const, status: 'normal' as const, trend: '稳定', trendDirection: 'stable' as const, miniChartData: [24.2,24.5,24.3,24.6,24.4,24.5,24.5] },
  { label: '当前湿度', value: 65, unit: '%', icon: '<path d="M12 2.69l5.66 5.66a8 8 0 1 1-11.31 0z"/>', iconColor: 'blue' as const, status: 'normal' as const, trend: '下降', trendDirection: 'down' as const, miniChartData: [68,67,66,65.5,65,64.5,65] },
  { label: 'PM2.5', value: 42, unit: 'μg/m³', icon: '<path d="M18.5 8c.83 0 1.5-.67 1.5-1.5S19.33 5 18.5 5c-.17 0-.34.03-.5.08A2.5 2.5 0 0 0 16 3a2.5 2.5 0 0 0-2 4v.02A1.5 1.5 0 0 0 13 8.5c0 .83.67 1.5 1.5 1.5h4z"/>', iconColor: 'yellow' as const, status: 'normal' as const, trend: '正常', trendDirection: 'stable' as const, miniChartData: [40,42,45,43,41,42,42] },
  { label: 'CO₂浓度', value: 520, unit: 'ppm', icon: '<circle cx="12" cy="12" r="10"/><path d="M12 6v6l4 2"/>', iconColor: 'purple' as const, status: 'normal' as const, trend: '正常', trendDirection: 'stable' as const, miniChartData: [510,515,520,518,522,520,520] }
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
  canvas.height = 370

  const ox = 220
  const layers = [
    { name: '应用层', y: 20, nodes: [
      { id: 'app-env', x: 400 + ox, icon: '🌡️', label: '环境监测应用', color: '#3b82f6' }
    ]},
    { name: '原子服务上层', y: 85, nodes: [
      { id: 'upper-monitor', x: 400 + ox, icon: '📊', label: '环境监测服务', color: '#3b82f6' }
    ]},
    { name: '原子服务下层', y: 155, nodes: [
      { id: 'lower-collect', x: 200 + ox, icon: '📥', label: '数据采集服务', color: '#3b82f6' },
      { id: 'lower-process', x: 450 + ox, icon: '⚙️', label: '数据处理服务', color: '#8b5cf6' },
      { id: 'lower-alarm', x: 650 + ox, icon: '🔔', label: '报警判断服务', color: '#ef4444' },
    ]},
    { name: '设备抽象层（云测仪→7个虚拟传感器）', y: 230, nodes: [
      { id: 'abs-temp', x: 25+ox, icon:'🌡️', label:'温度', color:'#3b82f6' },
      { id: 'abs-humi', x: 110+ox, icon:'💧', label:'湿度', color:'#06b6d4' },
      { id: 'abs-pm25', x: 195+ox, icon:'💨', label:'PM2.5', color:'#f59e0b' },
      { id: 'abs-co2', x: 280+ox, icon:'☁️', label:'CO₂', color:'#8b5cf6' },
      { id: 'abs-tvoc', x: 365+ox, icon:'🧪', label:'TVOC', color:'#ec4899' },
      { id: 'abs-ch2o', x: 450+ox, icon:'⚗️', label:'甲醛', color:'#14b8a6' },
      { id: 'abs-pm10', x: 535+ox, icon:'💨', label:'PM10', color:'#f59e0b' },
    ]},
    { name: '设备层', y: 310, nodes: [
      { id: 'sensor-cloud', x: 350 + ox, icon: '☁️', label: '云测仪(SD123)', color: '#3b82f6' },
    ]},
  ]
  // 数据流: 设备→抽象(上) + 控制流: 应用→服务→设备(下)
  // 向上流: 设备→抽象→数据采集→上层→应用
  const connections: Array<{from:string;to:string;color?:string}> = [
    { from: 'sensor-cloud', to: 'abs-temp', color: '#3b82f6' }, { from: 'sensor-cloud', to: 'abs-humi', color: '#06b6d4' },
    { from: 'sensor-cloud', to: 'abs-pm25', color: '#f59e0b' }, { from: 'sensor-cloud', to: 'abs-co2', color: '#8b5cf6' },
    { from: 'sensor-cloud', to: 'abs-tvoc', color: '#ec4899' }, { from: 'sensor-cloud', to: 'abs-ch2o', color: '#14b8a6' },
    { from: 'sensor-cloud', to: 'abs-pm10', color: '#f59e0b' },
    { from: 'abs-temp', to: 'lower-collect', color: '#3b82f6' }, { from: 'abs-humi', to: 'lower-collect', color: '#06b6d4' },
    { from: 'abs-pm25', to: 'lower-collect', color: '#f59e0b' }, { from: 'abs-co2', to: 'lower-collect', color: '#8b5cf6' },
    { from: 'abs-tvoc', to: 'lower-collect', color: '#ec4899' }, { from: 'abs-ch2o', to: 'lower-collect', color: '#14b8a6' },
    { from: 'abs-pm10', to: 'lower-collect', color: '#f59e0b' },
    { from: 'lower-collect', to: 'lower-process' }, { from: 'lower-process', to: 'lower-alarm' },
    { from: 'lower-collect', to: 'upper-monitor', color: '#3b82f6' }, { from: 'lower-process', to: 'upper-monitor', color: '#8b5cf6' },
    { from: 'upper-monitor', to: 'app-env', color: '#3b82f6' },
  ]

  let time = 0
  function animate() {
    if (!ctx) return
    ctx.clearRect(0, 0, canvas.width, canvas.height)
    time += 0.02

    layers.forEach((layer: any, i: number) => {
      const colors = ['rgba(15,23,42,0.5)','rgba(59,130,246,0.05)','rgba(34,197,94,0.04)','rgba(139,92,246,0.04)','rgba(59,130,246,0.04)']
      const h = 48
      ctx.fillStyle = colors[i] || colors[0]
      ctx.fillRect(0, layer.y - 10, canvas.width, h)
      ctx.font = 'bold 10px sans-serif'; ctx.fillStyle = '#94a3b8'; ctx.textAlign = 'left'
      ctx.fillText(layer.name, 8, layer.y + 6)
    })

    connections.forEach((conn: any) => {
      const fromLayer = layers.find((l: any) => l.nodes.some((n: any) => n.id === conn.from))
      const toLayer = layers.find((l: any) => l.nodes.some((n: any) => n.id === conn.to))
      if (!fromLayer || !toLayer) return
      const fromNode = fromLayer.nodes.find((n: any) => n.id === conn.from)
      const toNode = toLayer.nodes.find((n: any) => n.id === conn.to)
      if (!fromNode || !toNode) return

      const col = conn.color || '#3b82f6'
      const fy = fromLayer.y + 24; const ty = toLayer.y + 24
      ctx.beginPath(); ctx.moveTo(fromNode.x, fy); ctx.lineTo(toNode.x, ty)
      ctx.strokeStyle = col + '60'; ctx.lineWidth = 1.5; ctx.stroke()

      const prog = ((time * 0.3 + fromNode.x * 0.01) % 1 + 1) % 1
      const px = fromNode.x + (toNode.x - fromNode.x) * prog
      const py = fy + (ty - fy) * prog
      ctx.beginPath(); ctx.arc(px, py, 3, 0, Math.PI * 2)
      ctx.fillStyle = col; ctx.fill()
    })

    layers.forEach((layer: any) => {
      layer.nodes.forEach((node: any) => {
        const ny = layer.y + 24; const r = 16
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

const devices = reactive([
  { id: 1, name: '温度传感器', type: '云测仪 SD123', icon: '🌡️', value: '24.5℃', online: true },
  { id: 2, name: '湿度传感器', type: '云测仪 SD123', icon: '💧', value: '65%', online: true },
  { id: 3, name: 'PM2.5传感器', type: '云测仪 SD123', icon: '💨', value: '42μg/m³', online: true },
  { id: 4, name: 'CO₂传感器', type: '云测仪 SD123', icon: '☁️', value: '520ppm', online: true },
  { id: 5, name: '空调控制器', type: '控制设备', icon: '❄️', value: '24℃', online: true },
  { id: 6, name: '恒湿净化机', type: '净化设备', icon: '🌀', value: '运行中', online: false }
])

const controls = reactive([
  { id: 1, name: '空调控制', desc: '温度过高自动调温', icon: '❄️', active: true },
  { id: 2, name: '恒湿净化', desc: '湿度异常自动启动', icon: '💧', active: false },
  { id: 3, name: '空气净化', desc: '空气质量异常净化', icon: '🌀', active: false },
  { id: 4, name: '光照调节', desc: '光照不足自动补光', icon: '💡', active: false }
])

const thresholdList = reactive([
  { key: 'temp', label: '温度阈值', min: 10, max: 35, unit: '℃' },
  { key: 'humi', label: '湿度阈值', min: 30, max: 70, unit: '%' },
  { key: 'pm25', label: 'PM2.5阈值', min: 0, max: 75, unit: 'μg/m³' },
  { key: 'co2', label: 'CO₂阈值', min: 400, max: 1000, unit: 'ppm' }
])

const tempChartRef = ref<HTMLCanvasElement | null>(null)
const humiChartRef = ref<HTMLCanvasElement | null>(null)
let tempChart: Chart | null = null
let humiChart: Chart | null = null

function initCharts() {
  if (tempChartRef.value) {
    tempChart = new Chart(tempChartRef.value, {
      type: 'line',
      data: { labels: ['00:00','04:00','08:00','12:00','16:00','20:00'], datasets: [{ label: '温度', data: [24.2,24.5,24.3,24.6,24.4,24.5], borderColor: '#ef4444', backgroundColor: 'rgba(239,68,68,0.1)', borderWidth: 2, tension: 0.4, fill: true }] },
      options: { responsive: true, maintainAspectRatio: false, plugins: { legend: { display: false } }, scales: { y: { min: 20, max: 30 } } }
    })
  }
  if (humiChartRef.value) {
    humiChart = new Chart(humiChartRef.value, {
      type: 'line',
      data: { labels: ['00:00','04:00','08:00','12:00','16:00','20:00'], datasets: [{ label: '湿度', data: [62,60,58,56,57,55], borderColor: '#3b82f6', backgroundColor: 'rgba(59,130,246,0.1)', borderWidth: 2, tension: 0.4, fill: true }] },
      options: { responsive: true, maintainAspectRatio: false, plugins: { legend: { display: false } }, scales: { y: { min: 40, max: 70 } } }
    })
  }
}

function refreshData() { ElMessage.success('数据已刷新') }
function viewDev(dev: any) { ElMessage.info(`查看: ${dev.name}`) }
function refreshDev(dev: any) { ElMessage.success(`${dev.name} 已刷新`) }
function toggleCtrl(ctrl: any) {
  const idMap: Record<number, string> = { 1: 'ac', 2: 'humidifier', 3: 'purifier' }
  const target = idMap[ctrl.id] || ''
  const action = ctrl.active ? 'off' : 'on'
  axios.get('/environment/api/control', { params: { target, action } }).then(() => {
    ctrl.active = !ctrl.active
    ElMessage.success(ctrl.name + ' 已' + (ctrl.active ? '开启' : '关闭'))
  }).catch((e: any) => {
    ElMessage.error('设备控制失败: ' + (e?.message || e))
  })
}
function saveThresholds() { showThresholds.value = false; ElMessage.success('阈值已保存') }

// C++ 实时数据轮询 + 事件流
let realtimeTimer: ReturnType<typeof setInterval> | null = null
const cppOnline = ref(false)
const envEvents = reactive<Array<{id:number;time:string;source:string;value:number;unit:string;online:boolean;color:string}>>([])
let envEventId = 3000

async function fetchRealtimeData() {
  try {
    const res = await realtimeApi.getAllData() as any
    const d = res.data || res
    if (!d) return
    cppOnline.value = true
    const now = new Date().toLocaleTimeString('zh-CN', { hour12: false })
    const t = d.temperature !== undefined ? Math.round(d.temperature) / 10 : envCards[0].value
    const h = d.humidity !== undefined ? Math.round(d.humidity) / 10 : envCards[1].value
    envCards[0].value = t; envCards[1].value = h
    envCards[2].value = d.pm25 !== undefined ? d.pm25 : envCards[2].value
    envCards[3].value = d.co2 !== undefined ? d.co2 : envCards[3].value
    devices[0].online = !!d.temperature_online; devices[0].value = `${t}℃`
    devices[1].online = !!d.humidity_online; devices[1].value = `${h}%`
    devices[2].online = !!d.pm25_online; devices[2].value = `${d.pm25||0}μg/m³`
    devices[3].online = !!d.co2_online; devices[3].value = `${d.co2||0}ppm`
    const items = [
      { s:'温度', v:t, u:'℃', o:!!d.temperature_online, c:'#ef4444' },
      { s:'湿度', v:h, u:'%', o:!!d.humidity_online, c:'#3b82f6' },
      { s:'PM2.5', v:d.pm25||0, u:'μg/m³', o:!!d.pm25_online, c:'#f59e0b' },
      { s:'CO₂', v:d.co2||0, u:'ppm', o:!!d.co2_online, c:'#8b5cf6' },
      { s:'光照', v:d.lux||0, u:'lux', o:!!d.light_online, c:'#f59e0b' },
    ]
    items.forEach(item => {
      envEvents.unshift({ id: envEventId++, time: now, source: item.s, value: item.v, unit: item.u, online: item.o, color: item.c })
    })
    if (envEvents.length > 60) envEvents.length = 60
  } catch { /* C++不可用 */ }
}

function addDevice() {
  if (!newDevice.name || !newDevice.model) {
    ElMessage.warning('请填写必填项')
    return
  }

  const device = {
    id: devices.length + 1,
    name: newDevice.name,
    type: newDevice.type === 'sensor' ? '传感器' : newDevice.type === 'controller' ? '控制器' : '报警器',
    icon: newDevice.type === 'sensor' ? '🌡️' : newDevice.type === 'controller' ? '🎮' : '🔔',
    value: '0',
    online: true
  }

  devices.push(device)
  showAddDevice.value = false
  ElMessage.success(`设备 ${device.name} 已添加`)

  // 重置表单
  newDevice.name = ''
  newDevice.model = ''
  newDevice.address = ''
  newDevice.unit = ''
}

onMounted(() => { initCharts(); drawFlowChart(); fetchRealtimeData(); realtimeTimer = setInterval(fetchRealtimeData, 2000) })
onUnmounted(() => { tempChart?.destroy(); humiChart?.destroy(); if (flowAnimId) cancelAnimationFrame(flowAnimId); if (realtimeTimer) clearInterval(realtimeTimer) })
</script>

<style scoped>
.env-root { min-height: 100vh; background: var(--bg-primary); position: relative; }
.env-main { position: relative; z-index: 1; padding: var(--spacing-xl); max-width: 1920px; margin: 0 auto; }

.section-header { display: flex; align-items: center; justify-content: space-between; margin-bottom: var(--spacing-lg); }
.section-title { display: flex; align-items: center; gap: var(--spacing-sm); font-size: 20px; font-weight: 600; color: var(--text-primary); margin: 0; }
.section-title svg { color: var(--accent-primary); }
.header-actions { display: flex; gap: var(--spacing-sm); }

.overview-section { margin-bottom: var(--spacing-xl); }
.cards-grid { display: grid; grid-template-columns: repeat(4, 1fr); gap: var(--spacing-lg); }

.charts-flow-section { display: grid; grid-template-columns: 1fr 1fr; gap: var(--spacing-xl); margin-bottom: var(--spacing-xl); }
.charts-area { display: flex; flex-direction: column; gap: var(--spacing-xl); }
.chart-box { height: 200px; padding: var(--spacing-md); }

.devices-control-section { display: grid; grid-template-columns: 1.2fr 1fr; gap: var(--spacing-xl); margin-bottom: var(--spacing-xl); }
.stream-section { margin-bottom: var(--spacing-xl); }

.device-list { max-height: 350px; overflow-y: auto; }
.device-row { display: flex; align-items: center; gap: var(--spacing-md); padding: var(--spacing-sm) var(--spacing-md); border-bottom: 1px solid var(--border-primary); transition: all var(--duration-fast); }
.device-row:hover { background: rgba(255,255,255,0.03); }
.device-row.offline { opacity: 0.5; }
.dev-icon { font-size: 20px; }
.dev-info { flex: 1; }
.dev-name { font-size: 14px; font-weight: 500; color: var(--text-primary); }
.dev-type { font-size: 12px; color: var(--text-secondary); }
.dev-val { font-family: var(--font-mono); font-weight: 600; color: var(--text-primary); }
.dev-actions { display: flex; gap: 4px; }
.act-btn { padding: 4px 8px; background: transparent; border: 1px solid var(--border-primary); border-radius: var(--radius-sm); color: var(--text-secondary); font-size: 12px; cursor: pointer; }
.act-btn:hover { border-color: var(--accent-primary); color: var(--accent-primary); }

.control-list { display: flex; flex-direction: column; gap: var(--spacing-sm); }
.ctrl-item { display: flex; align-items: center; gap: var(--spacing-md); padding: var(--spacing-md); background: rgba(0,0,0,0.2); border: 1px solid var(--border-primary); border-radius: var(--radius-md); transition: all var(--duration-fast); }
.ctrl-item.active { border-color: var(--status-success); }
.ctrl-icon { font-size: 24px; }
.ctrl-info { flex: 1; }
.ctrl-name { font-size: 14px; font-weight: 500; color: var(--text-primary); }
.ctrl-desc { font-size: 12px; color: var(--text-secondary); }
.ctrl-status { font-size: 12px; padding: 2px 8px; border-radius: var(--radius-full); }
.ctrl-status.on { background: rgba(34,197,94,0.2); color: var(--status-success); }
.ctrl-status.off { background: rgba(100,116,139,0.2); color: var(--text-tertiary); }
.ctrl-btn { padding: 6px 12px; border: 1px solid var(--border-primary); border-radius: var(--radius-sm); font-size: 12px; cursor: pointer; }
.btn-on { background: rgba(34,197,94,0.2); border-color: var(--status-success); color: var(--status-success); }
.btn-off { background: rgba(239,68,68,0.2); border-color: var(--status-danger); color: var(--status-danger); }

.modal-overlay { position: fixed; inset: 0; background: rgba(0,0,0,0.7); backdrop-filter: blur(8px); display: flex; align-items: center; justify-content: center; z-index: var(--z-modal); }
.modal-content { background: var(--bg-card); border: 1px solid var(--border-primary); border-radius: var(--radius-xl); width: 90%; max-width: 500px; }
.modal-header { display: flex; justify-content: space-between; align-items: center; padding: var(--spacing-lg); border-bottom: 1px solid var(--border-primary); }
.modal-header h3 { font-size: 18px; font-weight: 600; color: var(--text-primary); margin: 0; }
.modal-close { width: 32px; height: 32px; display: flex; align-items: center; justify-content: center; background: transparent; border: none; color: var(--text-secondary); font-size: 20px; cursor: pointer; }
.modal-body { padding: var(--spacing-lg); max-height: 60vh; overflow-y: auto; }
.modal-footer { display: flex; justify-content: flex-end; gap: var(--spacing-sm); padding: var(--spacing-lg); border-top: 1px solid var(--border-primary); }

.threshold-row { display: flex; align-items: center; justify-content: space-between; padding: var(--spacing-sm) 0; border-bottom: 1px solid var(--border-primary); }
.th-label { font-size: 14px; color: var(--text-primary); }
.th-inputs { display: flex; align-items: center; gap: var(--spacing-sm); }
.th-input { width: 70px; padding: 6px; background: var(--bg-primary); border: 1px solid var(--border-primary); border-radius: var(--radius-sm); color: var(--text-primary); text-align: center; font-size: 13px; }
.th-unit { font-size: 12px; color: var(--text-secondary); }

.flow-canvas { width: 100%; height: 370px; background: rgba(0,0,0,0.2); border-radius: var(--radius-md); }

@media (max-width: 1200px) {
  .cards-grid { grid-template-columns: repeat(2, 1fr); }
  .charts-flow-section { grid-template-columns: 1fr; }
  .devices-control-section { grid-template-columns: 1fr; }
}
@media (max-width: 768px) {
  .env-main { padding: var(--spacing-md); }
  .cards-grid { grid-template-columns: 1fr; }
}
.realtime-events{max-height:280px;overflow-y:auto;font-size:11px}.realtime-events::-webkit-scrollbar{width:3px}.realtime-events::-webkit-scrollbar-thumb{background:rgba(59,130,246,.15);border-radius:2px}.realtime-event-row{display:flex;align-items:center;gap:6px;padding:3px 4px;border-bottom:1px solid rgba(255,255,255,.03)}.realtime-event-row:hover{background:rgba(255,255,255,.02)}.evt-time{font-family:'JetBrains Mono',monospace;font-size:9px;color:#475569;width:55px;flex-shrink:0}.evt-src{width:70px;flex-shrink:0;font-size:10px}.evt-val{font-family:'JetBrains Mono',monospace;font-size:10px;color:#e2e8f0;width:60px;text-align:right;flex-shrink:0}.evt-status{font-size:9px;width:26px;text-align:center;flex-shrink:0}.evt-status.online{color:#22c55d}.evt-status.offline{color:#ef4444}.empty-stream{text-align:center;color:#475569;padding:16px}
</style>
