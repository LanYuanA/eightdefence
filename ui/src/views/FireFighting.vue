<template>
  <div class="fire-root">
    <ParticleBackground :particle-count="30" color="#ef4444" :opacity="0.3" />
    <AppNavbar title="智能消防系统" subtitle="Fire Fighting System" :menu-items="menuItems" />

    <main class="fire-main">
      <!-- 风险概览 -->
      <section class="risk-section">
        <div class="risk-card" :class="riskLevel">
          <div class="risk-icon">{{ riskIcon }}</div>
          <div class="risk-info">
            <div class="risk-label">综合风险等级</div>
            <div class="risk-value">{{ riskText }}</div>
            <div class="risk-desc">{{ riskDescription }}</div>
          </div>
          <div class="risk-bar-wrap"><div class="risk-bar" :style="{ width: riskPercent + '%' }"></div></div>
        </div>
        <div class="region-tabs">
          <button v-for="r in regions" :key="r.id" class="region-tab" :class="{ active: currentRegion === r.id }" @click="currentRegion = r.id">{{ r.name }}</button>
        </div>
      </section>

      <!-- 数据卡片: 烟雾状态 / 温度 / CO2 / 湿度 -->
      <div style="font-size:12px;color:var(--text-sub);margin-bottom:4px">当前监测: 区域 {{ currentRegion }} </div>
      <section class="cards-section">
        <DataCard v-for="(card, i) in fireCards" :key="i" v-bind="card" :delay="i * 80" />
      </section>

      <!-- 图表: 温度趋势 + 烟雾状态 -->
      <section class="charts-section">
        <BaseCard title="温度趋势" subtitle="实时温度监测" icon-color="red">
          <canvas ref="tempChartRef" class="chart-canvas"></canvas>
        </BaseCard>
        <BaseCard title="烟雾状态" subtitle="烟雾检测历史" icon-color="yellow">
          <canvas ref="smokeChartRef" class="chart-canvas"></canvas>
        </BaseCard>
      </section>

      <!-- 设备控制 + 阈值设置 -->
      <section class="control-section">
        <BaseCard icon-color="yellow">
          <template #header>
            <div class="flex items-center gap-3">
              <div class="card-icon icon-yellow">
                <svg width="20" height="20" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M12 22s8-4 8-10V5l-8-3-8 3v7c0 6 8 10 8 10z"/></svg>
              </div>
              <div><h3 class="card-title">消防设备控制</h3><p class="card-subtitle">手动/自动控制</p></div>
              <div class="card-header-actions">
                <CyberButton variant="ghost" size="sm" @click="openModal('deviceControlLog')">控制历史</CyberButton>
                <CyberButton variant="ghost" size="sm" @click="openModal('history')">监测日志</CyberButton>
                <CyberButton variant="ghost" size="sm" @click="openModal('alarmLog')">报警事件</CyberButton>
              </div>
            </div>
          </template>
          <div class="device-grid">
            <div v-for="dev in fireDevices" :key="dev.id" class="device-ctrl" :class="{ active: dev.active }">
              <div class="dev-icon">{{ dev.icon }}</div>
              <div class="dev-info">
                <div class="dev-name">{{ dev.name }}</div>
                <div class="dev-desc">{{ dev.desc }}</div>
              </div>
              <div class="dev-switch" :class="{ on: dev.active }" @click="toggleDevice(dev)">
                <div class="switch-thumb"></div>
              </div>
            </div>
          </div>
          <div class="ctrl-btns">
            <CyberButton variant="danger" size="sm" @click="emergencyAction">🔥 火灾模拟</CyberButton>
            <CyberButton variant="secondary" size="sm" @click="stopSimulation">🛑 停止模拟</CyberButton>
          </div>
        </BaseCard>
        <BaseCard title="预警阈值设置" subtitle="报警触发条件" icon-color="yellow">
          <div v-for="t in thresholds" :key="t.key" class="threshold-row">
            <div class="th-label">
              <span class="th-icon">{{ t.key === 'temp' ? '🌡' : '☠' }}</span>
              <span>{{ t.label }}</span>
            </div>
            <div class="th-control">
              <input type="range" v-model.number="t.value" :min="t.min" :max="t.max" class="th-slider" />
              <span class="th-val">{{ t.value }}{{ t.unit }}</span>
              <CyberButton variant="ghost" size="sm" @click="saveThreshold(t)">保存</CyberButton>
            </div>
          </div>
        </BaseCard>
      </section>

      <!-- 消防数据流向 + 事件流 并排 -->
      <section class="monitor-section">
        <div class="flow-panel">
          <BaseCard title="消防数据流向" subtitle="实时数据传输" icon-color="blue">
            <template #header>
              <div class="flex items-center gap-3">
                <div class="card-icon icon-blue">
                  <svg width="20" height="20" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M22 12h-4l-3 9L9 3l-3 9H2"/></svg>
                </div>
                <div><h3 class="card-title">消防数据流向</h3><p class="card-subtitle">实时数据传输</p></div>
              </div>
            </template>
            <canvas ref="flowCanvasRef" class="flow-canvas"></canvas>
          </BaseCard>
        </div>
        <div class="stream-panel">
          <BaseCard icon-color="red">
            <template #header>
              <div class="flex items-center gap-3">
                <div class="card-icon icon-red">
                  <svg width="20" height="20" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M10.29 3.86L1.82 18a2 2 0 0 0 1.71 3h16.94a2 2 0 0 0 1.71-3L13.71 3.86a2 2 0 0 0-3.42 0z"/><line x1="12" y1="9" x2="12" y2="13"/><line x1="12" y1="17" x2="12.01" y2="17"/></svg>
                </div>
                <div><h3 class="card-title">消防事件流</h3><p class="card-subtitle">实时告警监控</p></div>
                <div class="card-header-actions">
                  <CyberButton variant="ghost" size="sm" @click="exportDataStore">数据存储</CyberButton>
                </div>
              </div>
            </template>

            <div class="realtime-events">
              <div v-if="fireEvents.length === 0" class="empty-stream">等待数据流...</div>
              <div v-for="evt in fireEvents.slice(0, 15)" :key="evt.id" class="realtime-event-row">
                <span class="evt-time">{{ evt.time }}</span>
                <span class="evt-src" :style="{color: evt.color}">{{ evt.source }}</span>
                <span class="evt-val">{{ evt.value }}{{ evt.unit }}</span>
                <span class="evt-status" :class="evt.online ? 'online' : 'offline'">{{ evt.online ? '在线' : '离线' }}</span>
              </div>
            </div>
          </BaseCard>
        </div>
      </section>
    </main>

    <!-- 火情确认弹窗 -->
    <div v-if="modals.fireConfirm" class="modal-overlay" @click.self="closeModal('fireConfirm')">
      <div class="modal alarm-modal">
        <div class="modal-header danger">
          <span>⚠️ 火灾告警</span>
          <span class="modal-close" @click="closeModal('fireConfirm')">&times;</span>
        </div>
        <div class="modal-body">
          <p>检测到<span class="text-danger fw-bold">{{ alarmTypeText }}</span>超出阈值！</p>
          <p>当前值：<span class="text-danger">{{ confirmValueText }}</span></p>
          <p>阈值设置：<span class="text-sub">{{ confirmThresholdText }}</span></p>
        </div>
        <div class="modal-footer">
          <CyberButton variant="danger" @click="confirmFire">确认火情</CyberButton>
          <CyberButton variant="ghost" @click="dismissFireAlarm">火情误报</CyberButton>
        </div>
      </div>
    </div>

    <!-- 设备控制历史弹窗 -->
    <div v-if="modals.deviceControlLog" class="modal-overlay" @click.self="closeModal('deviceControlLog')">
      <div class="modal" style="width:550px">
        <div class="modal-header"><span>设备控制历史记录</span><span class="modal-close" @click="closeModal('deviceControlLog')">&times;</span></div>
        <div class="modal-body" style="max-height:350px;overflow-y:auto">
          <div v-if="deviceControlLogList.length===0" style="color:var(--text-sub);text-align:center">暂无记录</div>
          <div v-for="(item,i) in deviceControlLogList" :key="i" class="log-row">
            <span>{{ item.time }}</span><span>{{ item.device }}</span><span :class="item.action==='开启'?'text-success':'text-danger'">{{ item.action }}</span>
          </div>
        </div>
      </div>
    </div>

    <!-- 历史监测日志弹窗 -->
    <div v-if="modals.history" class="modal-overlay" @click.self="closeModal('history')">
      <div class="modal" style="width:600px">
        <div class="modal-header"><span>历史监测日志 (近30天)</span><span class="modal-close" @click="closeModal('history')">&times;</span></div>
        <div class="modal-body" style="max-height:400px;overflow-y:auto">
          <div v-if="historyLogs.length===0" style="color:var(--text-sub);text-align:center">加载中...</div>
          <div v-for="(log,i) in historyLogs" :key="i" class="log-row">
            <span class="log-time">{{ log.timestamp }}</span>
            <span :class="log.level==='alarm'?'text-danger':log.level==='warning'?'text-warning':'text-sub'">{{ log.event }}</span>
            <span class="text-sub">{{ log.details }}</span>
          </div>
        </div>
      </div>
    </div>

    <!-- 报警事件库弹窗 -->
    <div v-if="modals.alarmLog" class="modal-overlay" @click.self="closeModal('alarmLog')">
      <div class="modal" style="width:550px">
        <div class="modal-header"><span>报警事件库</span><span class="modal-close" @click="closeModal('alarmLog')">&times;</span></div>
        <div class="modal-body" style="max-height:350px;overflow-y:auto">
          <div v-if="alarmEvents.length===0" style="color:var(--text-sub);text-align:center">暂无报警事件</div>
          <div v-for="(evt,i) in alarmEvents" :key="i" class="log-row">
            <span class="log-time">{{ evt.timestamp }}</span>
            <span :class="evt.action==='confirmed'?'text-warning':'text-sub'">{{ evt.action==='confirmed' ? '火情确认' : '火情误报' }}</span>
            <span class="text-sub">{{ evt.operator }}</span>
          </div>
        </div>
      </div>
    </div>
  </div>
</template>

<script setup lang="ts">
import { ref, reactive, computed, onMounted, onUnmounted } from 'vue'
import axios from "axios"
import { ElMessage } from 'element-plus'
import { Chart, registerables } from 'chart.js'
import ParticleBackground from '../components/ParticleBackground.vue'
import AppNavbar from '../components/AppNavbar.vue'
import DataCard from '../components/DataCard.vue'
import BaseCard from '../components/BaseCard.vue'
import CyberButton from '../components/CyberButton.vue'
Chart.register(...registerables)

const menuItems = [
  { path: '/', label: '主控台' }, { path: '/environment', label: '环境监测' },
  { path: '/fire', label: '消防系统' }, { path: '/security', label: '安防系统' },
  { path: '/devices', label: '设备管理' }, { path: '/logs', label: '系统日志' }
]

// === 风险等级 ===
const riskLevel = ref('low')
const riskText = computed(() => ({ critical: '极高', high: '高', medium: '中', low: '低' }[riskLevel.value] || '低'))
const riskIcon = computed(() => ({ critical: '🔴', high: '🟠', medium: '🟡', low: '🟢' }[riskLevel.value] || '🟢'))
const riskDescription = computed(() => ({ critical: '立即疏散！', high: '高度警戒', medium: '加强监控', low: '正常运行' }[riskLevel.value] || ''))
const riskPercent = computed(() => ({ critical: 95, high: 70, medium: 40, low: 15 }[riskLevel.value] || 15))
const currentRegion = ref('A')
const regions = [
  { id: 'A', name: '区域 A' },
  { id: 'B', name: '区域 B' },
  { id: 'C', name: '区域 C' },
]

// === 传感器数据卡片 ===
const fireCards = reactive([
  { label: '烟雾状态', value: 0, unit: '', icon: '💨', iconColor: 'yellow' as const, status: 'normal' as const, trend: '', trendDirection: 'stable' as const, miniChartData: [0,0,0,0,0,0,0,0] },
  { label: '温度', value: 0, unit: '°C', icon: '🌡', iconColor: 'red' as const, status: 'normal' as const, trend: '', trendDirection: 'stable' as const, miniChartData: [25,25,25,25,25,25,25,25] },
  { label: 'CO₂浓度', value: 0, unit: 'ppm', icon: '☁', iconColor: 'purple' as const, status: 'normal' as const, trend: '', trendDirection: 'stable' as const, miniChartData: [400,400,400,400,400,400,400,400] },
  { label: '湿度', value: 0, unit: '%', icon: '💧', iconColor: 'cyan' as const, status: 'normal' as const, trend: '', trendDirection: 'stable' as const, miniChartData: [50,50,50,50,50,50,50,50] },
])

// === 图表 ===
const tempChartRef = ref<HTMLCanvasElement | null>(null)
const smokeChartRef = ref<HTMLCanvasElement | null>(null)
let tempChart: Chart | null = null
let smokeChart: Chart | null = null
const tempHistory: number[] = []
const smokeHistory: number[] = []

function getTimeLabels() {
  const now = new Date()
  const labels = []
  for (let i = 60; i >= 0; i -= 15) {
    const t = new Date(now.getTime() - i * 60000)
    labels.push(t.getHours().toString().padStart(2,'0') + ':' + t.getMinutes().toString().padStart(2,'0'))
  }
  return labels
}
function initCharts() {
  const timeLabels = getTimeLabels()
  if (tempChartRef.value) {
    tempChart = new Chart(tempChartRef.value, {
      type: 'line', data: { labels: timeLabels, datasets: [{ label: '温度 °C', data: Array(5).fill(25), borderColor: '#ef4444', backgroundColor: 'rgba(239,68,68,0.1)', fill: true, tension: 0.3, pointRadius: 0 }] },
      options: { responsive: true, maintainAspectRatio: false, plugins: { legend: { display: false } }, scales: { x: { ticks: { color: '#666', font: { size: 10 } }, grid: { color: 'rgba(255,255,255,0.05)' } }, y: { min: 0, max: 100, ticks: { color: '#888' }, grid: { color: 'rgba(255,255,255,0.05)' } } } }
    })
  }
  if (smokeChartRef.value) {
    smokeChart = new Chart(smokeChartRef.value, {
      type: 'line', data: { labels: timeLabels, datasets: [{ label: '烟雾', data: Array(5).fill(0), borderColor: '#eab308', backgroundColor: 'rgba(234,179,8,0.1)', fill: true, tension: 0.3, pointRadius: 0, stepped: true }] },
      options: { responsive: true, maintainAspectRatio: false, plugins: { legend: { display: false } }, scales: { x: { ticks: { color: '#666', font: { size: 10 } }, grid: { color: 'rgba(255,255,255,0.05)' } }, y: { min: 0, max: 1.5, ticks: { stepSize: 1, color: '#888', callback: (v: any) => v === 0 ? '正常' : v === 1 ? '异常' : '' }, grid: { color: 'rgba(255,255,255,0.05)' } } } }
    })
  }
}

// === 设备控制 ===
const fireDevices = reactive([
  { id: 1, name: '舱门控制', desc: '火情联动紧急开启', icon: '🚪', active: false },
  { id: 2, name: '排烟风机', desc: '火情确认后自动开启', icon: '🌀', active: false },
  { id: 3, name: '喷淋系统', desc: '温度过高自动启动', icon: '💦', active: false },
  { id: 4, name: '声光报警', desc: '检测到火情立即报警', icon: '🔔', active: false },
])
const deviceControlLogList = ref<{time:string;device:string;action:string}[]>([])

function addDeviceControlLog(device: string, action: string) {
  deviceControlLogList.value.unshift({ time: new Date().toLocaleTimeString('zh-CN', { hour12: false }), device, action })
  if (deviceControlLogList.value.length > 100) deviceControlLogList.value.length = 100
}

function toggleDevice(dev: any) {
  const idMap: Record<number, string> = { 1: 'cabin', 2: 'fan', 3: 'sprinkler', 4: 'horn' }
  const target = idMap[dev.id] || ''
  const action = dev.active ? 'off' : 'on'
  axios.get('/fire/api/control', { params: { target, action, speed: 200 } }).then(() => {
    dev.active = !dev.active
    addDeviceControlLog(dev.name, dev.active ? '开启' : '关闭')
    ElMessage.success(dev.name + ' 已' + (dev.active ? '开启' : '关闭'))
  }).catch((e: any) => {
    ElMessage.error('设备控制失败: ' + (e?.message || e))
  })
}

function emergencyAction() {
  axios.get('/fire/api/control', { params: { target: 'fire', action: 'simulate' } }).then(() => {
    ElMessage.warning('火灾模拟已触发')
  }).catch((e: any) => {
    ElMessage.error('模拟失败: ' + (e?.message || e))
  })
}

function stopSimulation() {
  axios.get("/fire/api/control", { params: { target: "fire", action: "reset" } }).then(() => {
    alarmDismissed.value = false
    if (alarmDismissTimer) { clearTimeout(alarmDismissTimer); alarmDismissTimer = null }
    riskLevel.value = "low"
    fireCards[0].value = 0; fireCards[0].trend = "正常"
    fireCards[1].value = 25; fireCards[2].value = 400; fireCards[3].value = 50
    fireDevices.forEach(d => { d.active = false })
    tempHistory.length = 0; smokeHistory.length = 0
    for (let i = 0; i < 5; i++) { tempHistory.push(25); smokeHistory.push(0) }
    ElMessage.success("模拟已停止，状态已恢复")
  }).catch((e: any) => {
    ElMessage.error("停止失败: " + (e?.message || e))
  })
}

// === 阈值 ===
const thresholds = reactive([
  { key: 'temp', label: '温度报警阈值', value: 55, min: 30, max: 80, unit: '°C' },
  { key: 'co2', label: 'CO₂报警阈值', value: 1000, min: 400, max: 2000, unit: 'ppm' },
])
function saveThreshold(t: any) { ElMessage.success(`${t.label} 已保存为 ${t.value}${t.unit}`); addDeviceControlLog(t.label, `设置为${t.value}${t.unit}`) }

// === 弹窗管理 ===
const modals = reactive({
  fireConfirm: false, deviceControlLog: false, history: false, alarmLog: false
})
function openModal(name: string) { (modals as any)[name] = true; if (name === 'history') fetchHistoryLogs(); if (name === 'alarmLog') fetchAlarmEvents() }
function closeModal(name: string) { (modals as any)[name] = false }

// === 火情告警逻辑 ===
const alarmDismissed = ref(false)
let alarmDismissTimer: ReturnType<typeof setTimeout> | null = null
const alarmTypeText = ref('')
const confirmValueText = ref('')
const confirmThresholdText = ref('')

const justConfirmed = ref(false)
function confirmFire() {
  closeModal("fireConfirm")
  const operator = localStorage.getItem("loginUser") || "管理员"
  axios.get("/fire/api/control", { params: { action: "confirm", operator } }).then(() => {
    fireDevices.forEach(d => { if (!d.active) { d.active = true; addDeviceControlLog(d.name, "开启") } })
    justConfirmed.value = true
    setTimeout(() => { justConfirmed.value = false }, 3000)
  }).catch(() => {
    fireDevices.forEach(d => { if (!d.active) { d.active = true; addDeviceControlLog(d.name, "开启") } })
  })
  alarmDismissed.value = true
  if (alarmDismissTimer) clearTimeout(alarmDismissTimer)
  alarmDismissTimer = setTimeout(() => { alarmDismissed.value = false }, 60000)
  ElMessage.success("已确认火情，60秒后可再次告警")
}

function dismissFireAlarm() {
  closeModal('fireConfirm')
  const operator = localStorage.getItem('loginUser') || '管理员'
  axios.get('/fire/api/control', { params: { action: 'dismiss', operator } }).catch(() => {})
  alarmDismissed.value = true
  if (alarmDismissTimer) clearTimeout(alarmDismissTimer)
  alarmDismissTimer = setTimeout(() => { alarmDismissed.value = false }, 60000)
  ElMessage.info('已标记为火情误报，60秒后可再次告警')
}

// === 历史日志 ===
const historyLogs = ref<any[]>([])
async function fetchHistoryLogs() {
  try {
    const res = await axios.get('/fire/api/logs', { params: { limit: 200 } })
    historyLogs.value = res.data || []
  } catch { historyLogs.value = [] }
}
// === 数据存储导出 ===
async function exportDataStore() {
  try {
    const logsRes = await axios.get('/fire/api/logs', { params: { limit: 500 } })
    const actionsRes = await axios.get('/fire/api/fire_actions')
    const tempPeaks: {time:string;value:string}[] = []; const co2Peaks: {time:string;value:string}[] = []
    if (logsRes.data) {
      const now = Date.now(); const thirtyDays = 30 * 24 * 3600 * 1000
      logsRes.data.forEach((log: any) => {
        const t = new Date(log.timestamp).getTime()
        if (now - t <= thirtyDays) {
          if (log.event && log.event.includes('温度')) tempPeaks.push({ time: log.timestamp, value: log.details })
          if (log.event && log.event.includes('CO')) co2Peaks.push({ time: log.timestamp, value: log.details })
        }
      })
    }
    const data = {
      exportTime: new Date().toISOString(),
      thresholds: { temp: thresholds[0].value, co2: thresholds[1].value },
      temp30DayPeaks: tempPeaks.slice(-100),
      co230DayPeaks: co2Peaks.slice(-100),
      logs: logsRes.data || [],
      fireActions: actionsRes.data || []
    }
    const blob = new Blob([JSON.stringify(data, null, 2)], { type: 'application/json' })
    const url = URL.createObjectURL(blob)
    const a = document.createElement('a'); a.href = url; a.download = 'fire_data_30day_' + new Date().toISOString().slice(0,10) + '.json'; a.click()
    URL.revokeObjectURL(url)
    ElMessage.success('数据已导出为JSON文件(含30天峰值)')
  } catch { ElMessage.error('数据导出失败') }
}


// === 消防事件流 ===
const fireEvents = reactive<{id:number;time:string;source:string;value:string;unit:string;online:boolean;color:string}[]>([])
let fireEventId = 0
const alarmEvents = ref<any[]>([])
async function fetchAlarmEvents() {
  try {
    const res = await axios.get('/fire/api/fire_actions')
    alarmEvents.value = res.data || []
  } catch { alarmEvents.value = [] }
}

// === 数据流向图 ===
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
  canvas.height = 400

  const ox = 160
  const layers = [
    { name: "应用层", y: 12, nodes: [
      { id: "app-fire", x: 380 + ox, icon: "🔥", label: "消防应用", color: "#ef4444" }
    ]},
    { name: "消防原子服务", y: 70, nodes: [
      { id: "svc-fire", x: 380 + ox, icon: "🛡", label: "消防原子服务", color: "#ef4444" }
    ]},
    { name: "原子服务下层", y: 140, nodes: [
      { id: "svc-collect", x: 100 + ox, icon: "📥", label: "数据采集服务", color: "#3b82f6" },
      { id: "svc-judge", x: 240 + ox, icon: "⚖", label: "火情判断服务", color: "#eab308" },
      { id: "svc-control", x: 380 + ox, icon: "🎮", label: "设备控制服务", color: "#22c55d" },
      { id: "svc-alarm", x: 520 + ox, icon: "🔔", label: "报警服务", color: "#ef4444" },
      { id: "svc-store", x: 650 + ox, icon: "💾", label: "数据存储服务", color: "#a855f7" },
    ]},
    { name: "设备抽象层", y: 220, nodes: [
      { id: "abs-fire", x: 380 + ox, icon: "🔗", label: "消防抽象", color: "#f59e0b" },
    ]},
    { name: "设备层", y: 310, nodes: [
      { id: "dev-smoke", x: 60 + ox, icon: "💨", label: "烟雾传感器", color: "#94a3b8" },
      { id: "dev-cloud", x: 160 + ox, icon: "☁", label: "云测仪", color: "#94a3b8" },
      { id: "dev-alarm", x: 270 + ox, icon: "🔔", label: "声光报警器", color: "#94a3b8" },
      { id: "dev-cabin", x: 390 + ox, icon: "🚪", label: "舱门电机", color: "#94a3b8" },
      { id: "dev-sprinkler", x: 500 + ox, icon: "💦", label: "水淋电机", color: "#94a3b8" },
      { id: "dev-exhaust", x: 610 + ox, icon: "🌀", label: "排烟风机", color: "#94a3b8" },
    ]},
  ]
  const connections = [
    { from: "dev-smoke", to: "abs-fire" }, { from: "dev-cloud", to: "abs-fire" },
    { from: "dev-alarm", to: "abs-fire" }, { from: "dev-cabin", to: "abs-fire" },
    { from: "dev-sprinkler", to: "abs-fire" }, { from: "dev-exhaust", to: "abs-fire" },
    { from: "abs-fire", to: "svc-collect" },
    { from: "svc-collect", to: "svc-judge" },
    { from: "svc-judge", to: "svc-alarm" }, { from: "svc-judge", to: "svc-control" },
    { from: "svc-judge", to: "svc-store" },
    { from: "svc-alarm", to: "svc-fire" }, { from: "svc-store", to: "svc-fire" },
    { from: "svc-collect", to: "svc-fire" },
    { from: "svc-fire", to: "app-fire" },
    { from: "app-fire", to: "svc-control" },
    { from: "svc-control", to: "abs-fire" },
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

// === 数据轮询 ===
let pollTimer: ReturnType<typeof setInterval> | null = null

async function pollStatus() {
  try {
    const res = await axios.get('/fire/api/status')
    const d = res.data

    // Risk level
    const riskMap: Record<string, string> = { '安全': 'low', '预警': 'medium', '火灾': 'high' }
    riskLevel.value = riskMap[d.system?.overallRisk] || 'low'

    // Cards
    const smokeState = d.smoke?.state || 0
    fireCards[0].value = smokeState; fireCards[0].trend = smokeState === 0 ? '正常' : '检测到烟雾'
    // status handled via value
    fireCards[1].value = d.temperature?.value ?? 0
    fireCards[2].value = d.co2?.value ?? 0
    fireCards[3].value = d.humidity?.value ?? 0

    // Charts
    tempHistory.push(d.temperature?.value ?? 25)
    smokeHistory.push(d.smoke?.state ?? 0)
    if (tempHistory.length > 5) { tempHistory.shift(); smokeHistory.shift() }
    if (tempChart) { tempChart.data.labels = getTimeLabels(); tempChart.data.datasets[0].data = [...tempHistory]; tempChart.update('none') }
    if (smokeChart) { smokeChart.data.labels = getTimeLabels(); smokeChart.data.datasets[0].data = [...smokeHistory]; smokeChart.update('none') }

    // Device status from backend (only update if not manually controlled)
    // Backend device running state is authoritative when fire is simulated
    if (d.system?.fireSimulated === 'true' && !justConfirmed.value) {
      fireDevices[0].active = d.devices?.cabin?.running === 'true'
      fireDevices[1].active = d.devices?.exhaustFan?.running === 'true'
      fireDevices[2].active = d.devices?.sprinkler?.running === 'true'
      fireDevices[3].active = d.services?.alarmActive === 'true'
    }

    // Event stream
    const now = new Date().toLocaleTimeString('zh-CN', { hour12: false })
    const items = [
      { s: '烟雾', v: smokeState === 0 ? '正常' : '异常', u: '', o: d.smoke?.online === 'true', c: '#eab308' },
      { s: '温度', v: (d.temperature?.value ?? 0).toFixed(1), u: '°C', o: d.temperature?.online === 'true', c: '#ef4444' },
      { s: 'CO₂', v: (d.co2?.value ?? 0).toString(), u: 'ppm', o: d.co2?.online === 'true', c: '#a855f7' },
      { s: '湿度', v: (d.humidity?.value ?? 0).toFixed(1), u: '%', o: d.humidity?.online === 'true', c: '#06b6d4' },
      { s: '声光报警', v: d.services?.alarmActive === 'true' ? '激活' : '待机', u: '', o: d.devices?.alarm?.online === 'true', c: '#ef4444' },
    ]
    items.forEach(item => {
      fireEvents.unshift({ id: fireEventId++, time: now, source: item.s, value: item.v, unit: item.u, online: item.o, color: item.c })
    })
    if (fireEvents.length > 60) fireEvents.length = 60

    // Alarm check
    if (!alarmDismissed.value && !modals.fireConfirm) {
      const tempVal = d.temperature?.value ?? 0
      const smokeVal = d.smoke?.state ?? 0
      if (smokeVal === 1) {
        alarmTypeText.value = '烟雾'; confirmValueText.value = '检测到烟雾'; confirmThresholdText.value = '烟雾报警'
        modals.fireConfirm = true
      } else if (tempVal > (thresholds[0].value)) {
        alarmTypeText.value = '温度'; confirmValueText.value = tempVal.toFixed(1) + '°C'; confirmThresholdText.value = thresholds[0].value + '°C'
        modals.fireConfirm = true
      }
    }

    // Auto-reset dismissed when data normal
    if (alarmDismissed.value && d.smoke?.state === 0 && (d.temperature?.value ?? 0) < thresholds[0].value) {
      alarmDismissed.value = false
      if (alarmDismissTimer) { clearTimeout(alarmDismissTimer); alarmDismissTimer = null }
    }
  } catch { /* ignore poll errors */ }
}

onMounted(() => {
  initCharts()
  drawFlowChart()
  pollStatus()
  pollTimer = setInterval(pollStatus, 2000)
})

onUnmounted(() => {
  tempChart?.destroy(); smokeChart?.destroy()
  cancelAnimationFrame(flowAnimId)
  if (pollTimer) clearInterval(pollTimer)
  if (alarmDismissTimer) clearTimeout(alarmDismissTimer)
})
</script>

<style scoped>
.fire-root { min-height: 100vh; background: var(--bg-primary); position: relative; }
.fire-main { position: relative; z-index: 1; padding: var(--spacing-xl); max-width: 1920px; margin: 0 auto; display: flex; flex-direction: column; gap: var(--spacing-lg); }

.risk-section { }
.risk-card { display: flex; align-items: center; gap: var(--spacing-lg); padding: var(--spacing-lg); background: var(--bg-card); border-radius: var(--radius-lg); border: 1px solid var(--border-primary); }
.risk-card.critical { border-left: 4px solid var(--status-danger); }
.risk-card.high { border-left: 4px solid var(--status-warning); }
.risk-card.medium { border-left: 4px solid #eab308; }
.risk-card.low { border-left: 4px solid var(--status-success); }
.risk-icon { font-size: 36px; }
.risk-label { font-size: 13px; color: var(--text-sub); }
.risk-value { font-size: 24px; font-weight: 700; color: var(--text-primary); }
.risk-desc { font-size: 12px; color: var(--text-sub); }
.region-tabs { display: flex; gap: 4px; margin-top: var(--spacing-sm); }
.region-tab { padding: 6px 18px; background: transparent; border: 1px solid var(--border-primary); border-radius: 6px; color: var(--text-sub); cursor: pointer; font-size: 13px; transition: all var(--duration-fast); }
.region-tab:hover { border-color: var(--accent-primary); color: var(--text-primary); }
.region-tab.active { background: rgba(59,130,246,0.2); border-color: var(--accent-primary); color: var(--accent-primary); }

.risk-bar-wrap { flex: 1; height: 8px; background: var(--bg-primary); border-radius: 4px; overflow: hidden; }
.risk-bar { height: 100%; background: var(--accent-primary); border-radius: 4px; transition: width 0.5s; }
.risk-card.critical .risk-bar { background: var(--status-danger); animation: alarm-bar 1s infinite; }
@keyframes alarm-bar { 0%,100%{opacity:1} 50%{opacity:0.4} }

.cards-section { display: grid; grid-template-columns: repeat(4, 1fr); gap: var(--spacing-lg); }

.charts-section { display: grid; grid-template-columns: 1fr 1fr; gap: var(--spacing-lg); }
.chart-canvas { width: 100%; height: 200px; }

.control-section { display: grid; grid-template-columns: 1.3fr 0.7fr; gap: var(--spacing-lg); }
.device-grid { display: flex; flex-direction: column; gap: 6px; }
.device-ctrl { display: flex; align-items: center; gap: var(--spacing-md); padding: 10px 12px; background: var(--bg-primary); border-radius: 8px; transition: all var(--duration-fast); }
.device-ctrl.active { background: rgba(34,197,94,0.08); }
.dev-icon { font-size: 20px; }
.dev-info { flex: 1; }
.dev-name { font-size: 13px; color: var(--text-primary); font-weight: 500; }
.dev-desc { font-size: 11px; color: var(--text-sub); }
.dev-switch { width: 44px; height: 24px; background: var(--bg-card); border: 1px solid var(--border-primary); border-radius: 12px; cursor: pointer; position: relative; transition: all var(--duration-fast); flex-shrink: 0; }
.dev-switch.on { background: var(--status-success); border-color: var(--status-success); }
.switch-thumb { width: 18px; height: 18px; background: white; border-radius: 50%; position: absolute; top: 2px; left: 2px; transition: transform 0.2s; }
.dev-switch.on .switch-thumb { transform: translateX(20px); }
.ctrl-btns { display: flex; gap: 8px; margin-top: 12px; }

.threshold-row { display: flex; align-items: center; justify-content: space-between; padding: 10px 0; border-bottom: 1px solid var(--border-primary); }
.th-label { display: flex; align-items: center; gap: 8px; font-size: 13px; color: var(--text-primary); }
.th-icon { font-size: 18px; }
.th-control { display: flex; align-items: center; gap: 8px; }
.th-slider { width: 120px; accent-color: var(--accent-primary); }
.th-val { font-size: 13px; color: var(--text-sub); min-width: 55px; text-align: right; }


.monitor-section { display: grid; grid-template-columns: 1.2fr 1fr; gap: var(--spacing-lg); }
.flow-canvas { width: 100%; height: 400px; background: rgba(0,0,0,0.2); border-radius: var(--radius-md); }


.realtime-events{max-height:280px;overflow-y:auto;font-size:11px}.realtime-events::-webkit-scrollbar{width:3px}.realtime-events::-webkit-scrollbar-thumb{background:rgba(239,68,68,.15);border-radius:2px}.realtime-event-row{display:flex;align-items:center;gap:6px;padding:3px 4px;border-bottom:1px solid rgba(255,255,255,.03)}.realtime-event-row:hover{background:rgba(255,255,255,.02)}.evt-time{font-family:JetBrains Mono,monospace;font-size:9px;color:#475569;width:55px;flex-shrink:0}.evt-src{width:70px;flex-shrink:0;font-size:10px}.evt-val{font-family:JetBrains Mono,monospace;font-size:10px;color:#e2e8f0;width:60px;text-align:right;flex-shrink:0}.evt-status{font-size:9px;width:26px;text-align:center;flex-shrink:0}.evt-status.online{color:#22c55d}.evt-status.offline{color:#ef4444}.empty-stream{text-align:center;color:#475569;padding:16px}

.card-header-actions { display: flex; gap: 4px; margin-left: auto; }

/* Modals */
.modal-overlay { position: fixed; inset: 0; background: rgba(0,0,0,0.6); display: flex; align-items: center; justify-content: center; z-index: 1000; }
.modal { background: var(--bg-card); border: 1px solid var(--border-primary); border-radius: var(--radius-lg); padding: 0; min-width: 400px; }
.modal.alarm-modal { border: 2px solid var(--status-danger); animation: alarm-border 1s infinite; }
@keyframes alarm-border { 0%,100%{border-color:var(--status-danger)} 50%{border-color:transparent} }
.modal-header { display: flex; justify-content: space-between; align-items: center; padding: 14px 18px; border-bottom: 1px solid var(--border-primary); font-size: 16px; font-weight: 600; color: var(--text-primary); }
.modal-header.danger { color: var(--status-danger); }
.modal-close { cursor: pointer; font-size: 22px; color: var(--text-sub); }
.modal-body { padding: 18px; line-height: 1.8; color: var(--text-primary); }
.modal-footer { display: flex; justify-content: center; gap: 16px; padding: 14px 18px; border-top: 1px solid var(--border-primary); }

.text-danger { color: var(--status-danger); }
.text-warning { color: var(--status-warning); }
.text-success { color: var(--status-success); }
.text-sub { color: var(--text-sub); }
.fw-bold { font-weight: 700; }

.log-row { display: flex; gap: 16px; padding: 6px 0; border-bottom: 1px solid var(--border-primary); font-size: 12px; }
.log-time { color: var(--text-sub); min-width: 70px; }

@media (max-width: 900px) {
  .cards-section { grid-template-columns: repeat(2, 1fr); }
  .charts-section { grid-template-columns: 1fr; }
  .control-section { grid-template-columns: 1fr; }
}
</style>
