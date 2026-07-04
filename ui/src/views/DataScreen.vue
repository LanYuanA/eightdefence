<template>
  <div class="screen-root">
    <div class="screen-bg">
      <div class="grid-lines"></div>
      <div class="radar-scan"></div>
      <canvas ref="bgCanvasRef" class="bg-canvas"></canvas>
    </div>

    <header class="screen-header">
      <div class="header-left">
        <div class="system-time">{{ currentTime }}</div>
        <div class="header-stats">
          <span class="header-stat"><span class="pulse-dot green"></span>系统正常</span>
          <span class="header-stat">刷新 2s</span>
        </div>
      </div>
      <div class="header-center">
        <h1 class="main-title">软件定义消防系统</h1>
        <div class="sub-title">Eight Defence · 数据监控大屏</div>
      </div>
      <div class="header-right">
        <div class="system-status">
          <StatusDot status="online" size="sm" /><span>系统运行正常</span>
        </div>
      </div>
    </header>

    <main class="screen-main">
      <!-- ===== 左栏 ===== -->
      <div class="left-panel">
        <!-- 环形仪表盘 -->
        <div class="panel-card">
          <div class="panel-title"><span class="title-icon">🌡️</span>环境监测<span class="live-badge">LIVE</span></div>
          <div class="gauge-grid">
            <div v-for="g in envGauges" :key="g.label" class="gauge-item">
              <canvas :ref="el => setGaugeRef(g.label, el)" class="gauge-canvas"></canvas>
              <div class="gauge-label">{{ g.label }}</div>
              <div class="gauge-value">{{ g.displayValue }}<span class="gauge-unit">{{ g.unit }}</span></div>
            </div>
          </div>
        </div>

        <!-- 设备分布 -->
        <div class="panel-card">
          <div class="panel-title"><span class="title-icon">📡</span>设备状态分布</div>
          <div class="dist-list">
            <div v-for="d in deviceDist" :key="d.label" class="dist-row">
              <span class="dist-label">{{ d.label }}</span>
              <div class="dist-bar-bg"><div class="dist-bar" :class="d.cls" :style="{ width: d.pct + '%' }"></div></div>
              <span class="dist-val">{{ d.val }}</span>
            </div>
          </div>
        </div>

        <!-- 安防系统 -->
        <div class="panel-card">
          <div class="panel-title"><span class="title-icon">🛡️</span>安防系统</div>
          <div class="security-list">
            <div v-for="s in securityItems" :key="s.label" class="sec-row" :class="{ alert: s.warning }">
              <span class="sec-icon">{{ s.icon }}</span><span class="sec-label">{{ s.label }}</span>
              <span class="sec-status" :class="s.warning ? 'text-red' : 'text-green'">{{ s.warning ? s.warnText : '正常' }}</span>
            </div>
          </div>
        </div>
      </div>

      <!-- ===== 中栏 ===== -->
      <div class="center-panel">
        <!-- 3D设备拓扑球体 -->
        <div class="panel-card topology-card">
          <div class="panel-title">
            <span class="title-icon">🌐</span>设备拓扑网络
            <span class="title-hint">拖拽旋转 | 滚轮缩放</span>
          </div>
          <div class="topo-3d-wrap" ref="topoWrapRef">
            <canvas ref="topoCanvasRef" class="topo-3d-canvas"
              @mousedown="onTopoDown" @mousemove="onTopoMove" @mouseup="onTopoUp" @mouseleave="onTopoUp" @wheel="onTopoWheel"></canvas>
            <Transition name="fade">
              <div v-if="hoveredTopoNode" class="topo-tip" :style="{ left: topoTipX + 'px', top: topoTipY + 'px' }">
                <div class="topo-tip-name">{{ hoveredTopoNode.name }}</div>
                <div class="topo-tip-info">{{ hoveredTopoNode.type }} · {{ hoveredTopoNode.status === 'online' ? '在线' : '离线' }}</div>
              </div>
            </Transition>
          </div>
        </div>

        <!-- 实时数据流向 -->
        <div class="panel-card flow-card">
          <div class="panel-title"><span class="title-icon">⚡</span>实时数据流向</div>
          <canvas ref="flowCanvasRef" class="flow-canvas"></canvas>
          <div class="flow-legend">
            <span v-for="fl in flowLegend" :key="fl.label" class="fl-item">
              <span class="fl-dot" :style="{ background: fl.color }"></span>{{ fl.label }}
            </span>
          </div>
        </div>
      </div>

      <!-- ===== 右栏 ===== -->
      <div class="right-panel">
        <!-- 消防系统 -->
        <div class="panel-card">
          <div class="panel-title"><span class="title-icon">🔥</span>消防系统</div>
          <div class="fire-grid-2">
            <div class="fire-cell" :class="{ danger: fireData.smoke > 1.0 }">
              <span class="fc-icon">🔥</span><span class="fc-label">烟雾浓度</span>
              <span class="fc-val" :class="{ 'text-red': fireData.smoke > 1.0 }">{{ fireData.smoke.toFixed(2) }}%</span>
            </div>
            <div class="fire-cell"><span class="fc-icon">🌡️</span><span class="fc-label">区域温度</span><span class="fc-val">{{ fireData.temp.toFixed(1) }}℃</span></div>
            <div class="fire-cell"><span class="fc-icon">💦</span><span class="fc-label">喷淋状态</span><span class="fc-val text-green">就绪</span></div>
            <div class="fire-cell"><span class="fc-icon">🔔</span><span class="fc-label">声光报警</span><span class="fc-val" :class="fireAlert ? 'text-red' : 'text-green'">{{ fireAlert ? '触发' : '正常' }}</span></div>
          </div>
        </div>

        <!-- 3D柱状图 - 报警统计 -->
        <div class="panel-card">
          <div class="panel-title"><span class="title-icon">📈</span>今日报警统计</div>
          <canvas ref="barCanvasRef" class="bar-canvas"></canvas>
        </div>

        <!-- 实时事件 -->
        <div class="panel-card events-panel">
          <div class="panel-title"><span class="title-icon">📋</span>实时事件</div>
          <div class="events-scroll">
            <div v-for="e in recentEvents" :key="e.id" class="event-row" :class="e.type">
              <span class="event-dot" :class="e.type"></span>
              <span class="event-text">{{ e.text }}</span>
              <span class="event-time">{{ e.time }}</span>
            </div>
          </div>
        </div>
      </div>
    </main>

    <footer class="screen-footer">
      <span class="footer-item">在线设备: {{ onlineDevices }}/{{ totalDevices }}</span>
      <span class="footer-item">数据吞吐: {{ animatedThroughput }} 条/s</span>
      <span class="footer-item">运行时间: {{ uptime }}</span>
      <span class="footer-item">版本 v2.0.0</span>
    </footer>
  </div>
</template>

<script setup lang="ts">
import { ref, reactive, onMounted, onUnmounted, computed, nextTick } from 'vue'
import { useTime } from '../composables/useTime'
import StatusDot from '../components/StatusDot.vue'

const { currentTime } = useTime()

// ==================== 环境仪表盘 ====================
const envGauges = reactive([
  { label: '温度', value: 24.5, displayValue: 24.5, unit: '℃', min: 0, max: 50, color: '#3b82f6', ref: null as any },
  { label: '湿度', value: 65, displayValue: 65, unit: '%', min: 0, max: 100, color: '#06b6d4', ref: null as any },
  { label: 'PM2.5', value: 42, displayValue: 42, unit: 'μg', min: 0, max: 200, color: '#f59e0b', ref: null as any },
  { label: 'CO₂', value: 520, displayValue: 520, unit: 'ppm', min: 0, max: 2000, color: '#8b5cf6', ref: null as any },
])

function setGaugeRef(label: string, el: any) {
  const g = envGauges.find(g => g.label === label)
  if (g && el) g.ref = el
}

// ==================== 设备统计 ====================
const onlineDevices = ref(24)
const offlineDevices = ref(3)
const alertDevices = ref(2)
const totalDevices = computed(() => onlineDevices.value + offlineDevices.value + alertDevices.value)
const deviceDist = computed(() => {
  const t = totalDevices.value || 1
  return [
    { label: '在线', val: onlineDevices.value, pct: (onlineDevices.value / t) * 100, cls: 'online' },
    { label: '离线', val: offlineDevices.value, pct: (offlineDevices.value / t) * 100, cls: 'offline' },
    { label: '告警', val: alertDevices.value, pct: (alertDevices.value / t) * 100, cls: 'alert' },
  ]
})

// ==================== 安防 ====================
const securityItems = reactive([
  { icon: '💧', label: '水浸检测', warning: false, warnText: '异常' },
  { icon: '👤', label: '入侵检测', warning: false, warnText: '检测到' },
  { icon: '☁️', label: '气体检测', warning: false, warnText: '泄漏' },
  { icon: '🔌', label: '门禁系统', warning: false, warnText: '异常' },
])

// ==================== 消防 ====================
const fireAlert = ref(false)
const fireData = reactive({ temp: 28.5, smoke: 0.32 })

// ==================== 实时事件 ====================
const recentEvents = reactive<Array<{ id: number; text: string; time: string; type: string }>>([
  { id: 1, text: '云测仪温度数据更新: 24.5℃', time: '10:30:22', type: 'info' },
  { id: 2, text: 'CO₂浓度偏高提醒: 520ppm', time: '10:30:18', type: 'warning' },
  { id: 3, text: '烟雾报警器状态正常', time: '10:30:15', type: 'info' },
  { id: 4, text: '空调控制器自动调温至24℃', time: '10:30:10', type: 'info' },
  { id: 5, text: '红外探测器数据上报', time: '10:30:05', type: 'info' },
  { id: 6, text: '水浸传感器状态正常', time: '10:29:58', type: 'info' },
  { id: 7, text: 'PM2.5浓度上升至42μg/m³', time: '10:29:42', type: 'warning' },
])
let eventCounter = 8

// ==================== 数据吞吐量 ====================
const animatedThroughput = ref(1284)
const throughputTarget = ref(1284)

// ==================== 3D拓扑球体 ====================
interface TopoNode {
  id: string; name: string; type: string; status: string; color: string; icon: string
  x: number; y: number; z: number; baseR: number
}
const topoNodes: TopoNode[] = []
const topoEdges: Array<{ from: number; to: number; color: string; active: boolean }> = []
const nodeDefs = [
  { id: 'gw', name: '核心网关', type: '网关', color: '#ec4899', icon: '🔌' },
  { id: 'cloud', name: '云测仪', type: '传感器', color: '#3b82f6', icon: '☁️' },
  { id: 'smoke', name: '烟雾报警', type: '报警器', color: '#ef4444', icon: '🔥' },
  { id: 'water', name: '水浸传感', type: '传感器', color: '#06b6d4', icon: '💧' },
  { id: 'infrared', name: '红外探测', type: '探测器', color: '#8b5cf6', icon: '👤' },
  { id: 'light', name: '光感传感', type: '传感器', color: '#f59e0b', icon: '💡' },
  { id: 'aircon', name: '空调控制', type: '控制器', color: '#22c55d', icon: '❄️' },
  { id: 'humi', name: '恒湿净化', type: '控制器', color: '#06b6d4', icon: '🌀' },
  { id: 'tvoc', name: 'TVOC传感', type: '传感器', color: '#ec4899', icon: '🧪' },
  { id: 'ch2o', name: '甲醛传感', type: '传感器', color: '#14b8a6', icon: '⚗️' },
  { id: 'pm10', name: 'PM10传感', type: '传感器', color: '#f59e0b', icon: '💨' },
  { id: 'srv1', name: '数据服务器', type: '服务器', color: '#3b82f6', icon: '🖥️' },
  { id: 'srv2', name: '备份服务器', type: '服务器', color: '#8b5cf6', icon: '💾' },
  { id: 'alarm', name: '报警控制', type: '控制器', color: '#ef4444', icon: '🔔' },
  { id: 'spray', name: '喷淋控制', type: '控制器', color: '#06b6d4', icon: '💦' },
  { id: 'cam', name: '摄像头A', type: '摄像头', color: '#f59e0b', icon: '📷' },
]

function initTopo() {
  topoNodes.length = 0; topoEdges.length = 0
  const n = nodeDefs.length; const phi = Math.PI * (3 - Math.sqrt(5))
  nodeDefs.forEach((d, i) => {
    const y = 1 - (i / (n - 1)) * 2
    const rad = Math.sqrt(1 - y * y); const theta = phi * i
    const r = 110 + Math.sin(i * 1.7) * 25
    topoNodes.push({
      ...d, baseR: r,
      status: i % 5 === 0 ? 'offline' : 'online',
      x: Math.cos(theta) * rad * r,
      y: y * r,
      z: Math.sin(theta) * rad * r,
    })
  })
  const gi = topoNodes.findIndex(n => n.id === 'gw')
  topoNodes.forEach((_, i) => {
    if (i !== gi) topoEdges.push({ from: i, to: gi, color: topoNodes[i].color, active: topoNodes[i].status === 'online' })
  })
  for (let i = 0; i < n; i++)
    for (let j = i + 1; j < n; j++)
      if (topoNodes[i].type === topoNodes[j].type && Math.random() < 0.22)
        topoEdges.push({ from: i, to: j, color: '#475569', active: true })
}
initTopo()

const topoWrapRef = ref<HTMLElement | null>(null)
const topoCanvasRef = ref<HTMLCanvasElement | null>(null)
const hoveredTopoNode = ref<TopoNode | null>(null)
const topoTipX = ref(0); const topoTipY = ref(0)
let isTopoDrag = false, topoDragX = 0, topoDragY = 0
let rotX = -15, rotY = 25, startRotX = 0, startRotY = 0
let zoom = 1.0

function project3D(x: number, y: number, z: number, cx: number, cy: number) {
  const rx = rotX * Math.PI / 180, ry = rotY * Math.PI / 180
  const x1 = x * Math.cos(ry) - z * Math.sin(ry)
  const z1 = x * Math.sin(ry) + z * Math.cos(ry)
  const y1 = y * Math.cos(rx) - z1 * Math.sin(rx)
  const z2 = y * Math.sin(rx) + z1 * Math.cos(rx)
  const s = 500 / (500 + z2) * zoom
  return { x: cx + x1 * s, y: cy + y1 * s, s }
}

function onTopoDown(e: MouseEvent) { isTopoDrag = true; topoDragX = e.clientX; topoDragY = e.clientY; startRotX = rotX; startRotY = rotY }
function onTopoMove(e: MouseEvent) {
  if (isTopoDrag) {
    rotY = startRotY + (e.clientX - topoDragX) * 0.3
    rotX = startRotX + (e.clientY - topoDragY) * 0.3
  }
  if (topoCanvasRef.value && topoWrapRef.value) {
    const r = topoCanvasRef.value.getBoundingClientRect()
    checkTopoHover(e.clientX - r.left, e.clientY - r.top, r.width, r.height)
  }
}
function onTopoUp() { isTopoDrag = false }
function onTopoWheel(e: WheelEvent) { e.preventDefault(); zoom = Math.max(0.4, Math.min(2.5, zoom - e.deltaY * 0.001)) }

function checkTopoHover(mx: number, my: number, w: number, h: number) {
  const cx = w / 2, cy = h / 2
  let found: TopoNode | null = null
  for (const node of topoNodes) {
    const p = project3D(node.x, node.y, node.z, cx, cy)
    if (Math.hypot(mx - p.x, my - p.y) < 16 * p.s) { found = node; break }
  }
  hoveredTopoNode.value = found
  if (found) { topoTipX.value = mx + 12; topoTipY.value = my - 8 }
}

// ==================== Canvas Refs ====================
const bgCanvasRef = ref<HTMLCanvasElement | null>(null)
const flowCanvasRef = ref<HTMLCanvasElement | null>(null)
const barCanvasRef = ref<HTMLCanvasElement | null>(null)

const flowLegend = [
  { label: '传感器', color: '#3b82f6' }, { label: '告警', color: '#ef4444' },
  { label: '控制', color: '#22c55d' }, { label: '日志', color: '#f59e0b' },
]

// ==================== 绘制函数 ====================
function drawBg(ctx: CanvasRenderingContext2D, w: number, h: number, t: number) {
  ctx.clearRect(0, 0, w, h)
  for (let i = 0; i < 50; i++) {
    const s = i * 137.5
    ctx.beginPath(); ctx.arc(((s * 7 + t * 18) % w + w) % w, ((s * 13 + Math.sin(t + i) * 50) % h + h) % h, 1.3, 0, Math.PI * 2)
    ctx.fillStyle = `rgba(59,130,246,${0.12 + Math.sin(t * 2 + i * 0.6) * 0.08})`; ctx.fill()
  }
}

function drawTopo(ctx: CanvasRenderingContext2D, w: number, h: number, t: number) {
  ctx.clearRect(0, 0, w, h)
  if (!isTopoDrag) rotY += 0.15
  const cx = w / 2, cy = h / 2

  const proj = topoNodes.map((n, i) => {
    const p = project3D(n.x, n.y, n.z, cx, cy)
    return { ...p, i, n, depth: n.x * Math.sin(rotY * Math.PI / 180) + n.z * Math.cos(rotY * Math.PI / 180) }
  })
  proj.sort((a, b) => a.depth - b.depth)

  // 边
  for (const p of proj) {
    for (const e of topoEdges) {
      if (e.from !== p.i && e.to !== p.i) continue
      const oi = e.from === p.i ? e.to : e.from
      const o = proj.find(pp => pp.i === oi)
      if (!o) continue
      ctx.beginPath(); ctx.moveTo(p.x, p.y); ctx.lineTo(o.x, o.y)
      ctx.strokeStyle = e.active ? e.color + '35' : '#33415520'
      ctx.lineWidth = e.active ? 1 : 0.4; ctx.stroke()
    }
  }

  // 节点
  for (const p of proj) {
    const sz = 13 * p.s
    if (p.n.status === 'online') {
      const g = ctx.createRadialGradient(p.x, p.y, sz * 0.5, p.x, p.y, sz * 2.2)
      g.addColorStop(0, p.n.color + '45'); g.addColorStop(1, 'transparent')
      ctx.beginPath(); ctx.arc(p.x, p.y, sz * 2.2, 0, Math.PI * 2); ctx.fillStyle = g; ctx.fill()
    }
    ctx.beginPath(); ctx.arc(p.x, p.y, sz, 0, Math.PI * 2)
    ctx.fillStyle = p.n.status === 'online' ? p.n.color + '35' : '#1e293b'
    ctx.strokeStyle = p.n.status === 'online' ? p.n.color : '#475569'
    ctx.lineWidth = 1.3; ctx.fill(); ctx.stroke()
    const isz = Math.max(9, sz * 0.75)
    ctx.font = `${isz}px serif`; ctx.textAlign = 'center'; ctx.textBaseline = 'middle'
    ctx.fillStyle = '#fff'; ctx.fillText(p.n.icon, p.x, p.y)
    if (p.s > 0.45) {
      ctx.font = `${Math.max(8, 9 * p.s)}px sans-serif`
      ctx.fillStyle = p.n.color; ctx.fillText(p.n.name, p.x, p.y + sz + 11 * p.s)
    }
  }
}

function drawFlow(ctx: CanvasRenderingContext2D, w: number, h: number, t: number) {
  ctx.clearRect(0, 0, w, h)
  const layers = [
    { y: h * 0.82, nodes: [{ x: w * 0.22, c: '#3b82f6' }, { x: w * 0.5, c: '#22c55d' }, { x: w * 0.78, c: '#ef4444' }], name: '设备层' },
    { y: h * 0.58, nodes: [{ x: w * 0.35, c: '#3b82f6' }, { x: w * 0.65, c: '#8b5cf6' }], name: '原子服务下层' },
    { y: h * 0.34, nodes: [{ x: w * 0.5, c: '#ec4899' }], name: '原子服务上层' },
    { y: h * 0.1, nodes: [{ x: w * 0.2, c: '#8b5cf6' }, { x: w * 0.5, c: '#3b82f6' }, { x: w * 0.8, c: '#ef4444' }], name: '应用层' },
  ]
  const fcs = ['#3b82f6', '#8b5cf6', '#22c55d', '#ef4444']
  for (let li = 0; li < layers.length - 1; li++) {
    for (const fn of layers[li].nodes) {
      for (const tn of layers[li + 1].nodes) {
        const ci = (layers[li].nodes.indexOf(fn) + layers[li + 1].nodes.indexOf(tn)) % fcs.length
        ctx.beginPath(); ctx.moveTo(fn.x, layers[li].y); ctx.lineTo(tn.x, layers[li + 1].y)
        ctx.strokeStyle = fcs[ci] + '18'; ctx.lineWidth = 1; ctx.stroke()
        const prog = ((t * 0.4 + fn.x * 0.01 + tn.x * 0.01) % 1 + 1) % 1
        ctx.beginPath(); ctx.arc(fn.x + (tn.x - fn.x) * prog, layers[li].y + (layers[li + 1].y - layers[li].y) * prog, 2.2, 0, Math.PI * 2)
        ctx.fillStyle = fcs[ci]; ctx.fill()
      }
    }
  }
  for (const layer of layers) {
    ctx.font = '10px sans-serif'; ctx.fillStyle = '#475569'; ctx.textAlign = 'left'
    ctx.fillText(layer.name, 6, layer.y - 6)
    for (const node of layer.nodes) {
      ctx.beginPath(); ctx.arc(node.x, layer.y, 9, 0, Math.PI * 2)
      ctx.fillStyle = node.c + '25'; ctx.strokeStyle = node.c; ctx.lineWidth = 1.3; ctx.fill(); ctx.stroke()
    }
  }
}

function drawBar3D(ctx: CanvasRenderingContext2D, w: number, h: number, t: number) {
  ctx.clearRect(0, 0, w, h)
  const data = [
    { label: '紧急', value: 3 + Math.sin(t * 3) * 0.4, color: '#ef4444' },
    { label: '警告', value: 7 + Math.sin(t * 2.5 + 1) * 0.8, color: '#f59e0b' },
    { label: '提示', value: 12 + Math.sin(t * 2 + 2) * 1.2, color: '#06b6d4' },
    { label: '已解决', value: 25 + Math.cos(t * 1.5) * 1.5, color: '#22c55d' },
  ]
  const bw = (w - 60) / data.length - 14, baseY = h - 25, maxV = 30, d = 10
  data.forEach((item, i) => {
    const bh = (item.value / maxV) * (h - 55)
    const x = 30 + i * (bw + 14), y = baseY - bh
    ctx.fillStyle = item.color + '55'; ctx.fillRect(x + d, y - d, bw, bh)
    const g = ctx.createLinearGradient(x, y, x, baseY)
    g.addColorStop(0, item.color); g.addColorStop(1, item.color + '35')
    ctx.fillStyle = g; ctx.fillRect(x, y, bw, bh)
    ctx.fillStyle = item.color + '70'
    ctx.beginPath(); ctx.moveTo(x, y); ctx.lineTo(x + d, y - d); ctx.lineTo(x + d + bw, y - d); ctx.lineTo(x + bw, y); ctx.closePath(); ctx.fill()
    ctx.strokeStyle = item.color; ctx.lineWidth = 1; ctx.strokeRect(x, y, bw, bh)
    ctx.font = 'bold 13px "JetBrains Mono", monospace'; ctx.textAlign = 'center'; ctx.fillStyle = item.color
    ctx.fillText(Math.round(item.value).toString(), x + bw / 2, y - 10)
    ctx.font = '10px sans-serif'; ctx.fillStyle = '#94a3b8'; ctx.fillText(item.label, x + bw / 2, baseY + 16)
  })
}

function drawGauge(ctx: CanvasRenderingContext2D, w: number, h: number, val: number, min: number, max: number, color: string) {
  ctx.clearRect(0, 0, w, h)
  const cx = w / 2, cy = h * 0.62, r = Math.min(w, h) * 0.35
  const sa = Math.PI * 0.72, ea = Math.PI * 2.28
  const ratio = Math.max(0, Math.min(1, (val - min) / (max - min)))
  const na = sa + (ea - sa) * ratio
  ctx.beginPath(); ctx.arc(cx, cy, r, sa, ea); ctx.strokeStyle = '#1e293b'; ctx.lineWidth = 9; ctx.lineCap = 'round'; ctx.stroke()
  ctx.beginPath(); ctx.arc(cx, cy, r, sa, na); ctx.strokeStyle = color; ctx.lineWidth = 9; ctx.lineCap = 'round'; ctx.stroke()
  ctx.beginPath(); ctx.arc(cx, cy, r, sa, na); ctx.strokeStyle = color + '35'; ctx.lineWidth = 16; ctx.lineCap = 'round'; ctx.stroke()
  for (let i = 0; i <= 20; i++) {
    const a = sa + (ea - sa) * (i / 20)
    ctx.beginPath(); ctx.moveTo(cx + Math.cos(a) * (r - 14), cy + Math.sin(a) * (r - 14))
    ctx.lineTo(cx + Math.cos(a) * (r - 5), cy + Math.sin(a) * (r - 5))
    ctx.strokeStyle = i <= ratio * 20 ? color : '#334155'; ctx.lineWidth = 1.3; ctx.stroke()
  }
  const nl = r - 18
  ctx.beginPath(); ctx.moveTo(cx, cy); ctx.lineTo(cx + Math.cos(na) * nl, cy + Math.sin(na) * nl)
  ctx.strokeStyle = '#fff'; ctx.lineWidth = 2.2; ctx.lineCap = 'round'; ctx.stroke()
  ctx.beginPath(); ctx.arc(cx, cy, 4.5, 0, Math.PI * 2); ctx.fillStyle = '#fff'; ctx.fill()
  ctx.beginPath(); ctx.arc(cx, cy, 2.8, 0, Math.PI * 2); ctx.fillStyle = color; ctx.fill()
}

// ==================== 统一动画循环 ====================
let animId = 0
function animate(ts: number) {
  const t = ts * 0.001
  if (bgCanvasRef.value) {
    const c = bgCanvasRef.value; const ctx = c.getContext('2d')
    if (ctx) { c.width = c.parentElement?.clientWidth || window.innerWidth; c.height = c.parentElement?.clientHeight || window.innerHeight; drawBg(ctx, c.width, c.height, t) }
  }
  if (topoCanvasRef.value && topoWrapRef.value) {
    const c = topoCanvasRef.value; const ctx = c.getContext('2d'); const r = topoWrapRef.value.getBoundingClientRect()
    if (ctx && r.width > 0) { c.width = r.width; c.height = r.height; drawTopo(ctx, c.width, c.height, t) }
  }
  if (flowCanvasRef.value) {
    const c = flowCanvasRef.value; const ctx = c.getContext('2d'); const p = c.parentElement
    if (ctx && p) { c.width = p.clientWidth; c.height = 170; drawFlow(ctx, c.width, c.height, t) }
  }
  if (barCanvasRef.value) {
    const c = barCanvasRef.value; const ctx = c.getContext('2d'); const p = c.parentElement
    if (ctx && p) { c.width = p.clientWidth; c.height = 160; drawBar3D(ctx, c.width, c.height, t) }
  }
  for (const g of envGauges) {
    if (g.ref) {
      const c = g.ref as HTMLCanvasElement; const ctx = c.getContext('2d'); const p = c.parentElement
      if (ctx && p) { const s = p.clientWidth; c.width = s; c.height = s; drawGauge(ctx, s, s, g.value, g.min, g.max, g.color) }
    }
  }
  animId = requestAnimationFrame(animate)
}

// ==================== 数据更新 ====================
let dataTimer: ReturnType<typeof setInterval> | null = null
function updateData() {
  envGauges[0].value = Math.round((24 + Math.sin(Date.now() * 0.001) * 1.5) * 10) / 10
  envGauges[1].value = Math.round((60 + Math.cos(Date.now() * 0.0008) * 8) * 10) / 10
  envGauges[2].value = Math.round(40 + Math.sin(Date.now() * 0.0012) * 8)
  envGauges[3].value = Math.round(500 + Math.cos(Date.now() * 0.0006) * 80)
  for (const g of envGauges) g.displayValue = Math.round(g.value * 10) / 10

  fireData.temp = Math.round((28 + Math.random() * 1.5) * 10) / 10
  fireData.smoke = Math.round((0.2 + Math.random() * 0.25) * 100) / 100
  fireAlert.value = fireData.smoke > 0.5

  onlineDevices.value = Math.max(20, Math.min(28, onlineDevices.value + Math.floor(Math.random() * 3) - 1))
  offlineDevices.value = Math.max(1, Math.min(5, 30 - onlineDevices.value - alertDevices.value))
  throughputTarget.value = Math.floor(1200 + Math.sin(Date.now() * 0.0005) * 200 + Math.random() * 50)

  if (Math.random() < 0.04) {
    const s = securityItems[Math.floor(Math.random() * securityItems.length)]
    s.warning = !s.warning
  }
  if (Math.random() < 0.35) {
    const msgs = [
      { text: `温度传感器: ${envGauges[0].displayValue}℃`, type: 'info' },
      { text: `湿度传感器: ${envGauges[1].displayValue}%`, type: 'info' },
      { text: '设备心跳检测完成', type: 'info' },
      { text: '红外探测器触发', type: 'warning' },
      { text: 'CO₂浓度接近上限', type: 'warning' },
      { text: '网络延迟短暂升高', type: 'warning' },
    ]
    const m = msgs[Math.floor(Math.random() * msgs.length)]
    recentEvents.unshift({ id: eventCounter++, text: m.text, time: new Date().toLocaleTimeString('zh-CN', { hour12: false }), type: m.type })
    if (recentEvents.length > 40) recentEvents.length = 40
  }
}

let throughputAnimId = 0
function animThroughput() {
  const d = throughputTarget.value - animatedThroughput.value
  animatedThroughput.value = Math.round(animatedThroughput.value + d * 0.12)
  if (Math.abs(d) < 0.5) animatedThroughput.value = throughputTarget.value
  throughputAnimId = requestAnimationFrame(animThroughput)
}

// ==================== 生命周期 ====================
const uptime = ref('15天 8小时 32分钟')

onMounted(() => {
  nextTick(() => {
    animId = requestAnimationFrame(animate)
    throughputAnimId = requestAnimationFrame(animThroughput)
  })
  dataTimer = setInterval(updateData, 2000)
})

onUnmounted(() => {
  cancelAnimationFrame(animId)
  cancelAnimationFrame(throughputAnimId)
  if (dataTimer) clearInterval(dataTimer)
})
</script>

<style scoped>
.screen-root { min-height: 100vh; background: #060b18; color: #e2e8f0; overflow: hidden; position: relative; font-family: 'PingFang SC','Microsoft YaHei',sans-serif; }
.screen-bg { position: fixed; inset: 0; pointer-events: none; z-index: 0; }
.bg-canvas { position: absolute; inset: 0; width: 100%; height: 100%; }
.grid-lines { position: absolute; inset: 0; background-image: linear-gradient(rgba(59,130,246,0.035) 1px,transparent 1px), linear-gradient(90deg, rgba(59,130,246,0.035) 1px,transparent 1px); background-size: 50px 50px; }
.radar-scan { position: absolute; top: 50%; left: 50%; width: 500px; height: 500px; transform: translate(-50%,-50%); border-radius: 50%; background: radial-gradient(circle, rgba(59,130,246,0.05) 0%, transparent 70%); animation: radar 4s ease-in-out infinite; }
@keyframes radar { 0%,100% { transform: translate(-50%,-50%) scale(1); opacity: 0.5; } 50% { transform: translate(-50%,-50%) scale(1.35); opacity: 0.15; } }

.screen-header { display: flex; align-items: center; justify-content: space-between; padding: 10px 18px; background: rgba(10,14,28,0.9); border-bottom: 1px solid rgba(59,130,246,0.18); position: relative; z-index: 2; }
.header-left,.header-right { min-width: 150px; }
.system-time { font-family: 'JetBrains Mono',monospace; font-size: 15px; color: #e2e8f0; font-weight: 600; }
.header-stats { display: flex; gap: 12px; margin-top: 2px; }
.header-stat { font-size: 10px; color: #64748b; display: flex; align-items: center; gap: 4px; }
.pulse-dot { width: 6px; height: 6px; border-radius: 50%; }
.pulse-dot.green { background: #22c55d; box-shadow: 0 0 6px #22c55d; animation: dotPulse 2s infinite; }
@keyframes dotPulse { 0%,100% { opacity: 1; } 50% { opacity: 0.35; } }
.header-center { text-align: center; }
.main-title { font-size: 24px; font-weight: 700; margin: 0; letter-spacing: 5px; background: linear-gradient(135deg, #3b82f6 20%, #8b5cf6 50%, #06b6d4 80%); -webkit-background-clip: text; -webkit-text-fill-color: transparent; }
.sub-title { font-size: 10px; color: #475569; margin-top: 1px; letter-spacing: 3px; }
.system-status { display: flex; align-items: center; gap: 6px; font-size: 12px; color: #22c55d; justify-content: flex-end; }

.screen-main { display: grid; grid-template-columns: 240px 1fr 240px; gap: 10px; padding: 8px 10px; height: calc(100vh - 96px); position: relative; z-index: 1; }
.left-panel,.right-panel { display: flex; flex-direction: column; gap: 8px; overflow-y: auto; }
.left-panel::-webkit-scrollbar,.right-panel::-webkit-scrollbar { width: 2px; }
.left-panel::-webkit-scrollbar-thumb,.right-panel::-webkit-scrollbar-thumb { background: rgba(59,130,246,0.12); border-radius: 2px; }
.center-panel { display: flex; flex-direction: column; gap: 8px; }

.panel-card { background: rgba(15,23,42,0.72); border: 1px solid rgba(59,130,246,0.1); border-radius: 9px; padding: 10px 12px; backdrop-filter: blur(6px); transition: border-color 0.3s; }
.panel-card:hover { border-color: rgba(59,130,246,0.25); }
.panel-title { display: flex; align-items: center; gap: 7px; font-size: 12px; font-weight: 600; color: #94a3b8; margin-bottom: 8px; text-transform: uppercase; letter-spacing: 0.8px; }
.title-icon { font-size: 15px; }
.live-badge { margin-left: auto; font-size: 8px; padding: 1px 6px; background: rgba(34,197,94,0.2); color: #22c55d; border-radius: 8px; letter-spacing: 0.5px; animation: dotPulse 1.5s infinite; }
.title-hint { margin-left: auto; font-size: 9px; color: #475569; text-transform: none; letter-spacing: 0; }

/* 仪表盘 */
.gauge-grid { display: grid; grid-template-columns: 1fr 1fr; gap: 4px; }
.gauge-item { text-align: center; }
.gauge-canvas { width: 100%; aspect-ratio: 1; }
.gauge-label { font-size: 10px; color: #64748b; margin-top: -6px; }
.gauge-value { font-size: 16px; font-weight: 700; color: #e2e8f0; font-family: 'JetBrains Mono',monospace; }
.gauge-unit { font-size: 9px; color: #64748b; margin-left: 1px; }

/* 设备分布 */
.dist-list { display: flex; flex-direction: column; gap: 6px; }
.dist-row { display: flex; align-items: center; gap: 8px; }
.dist-label { width: 28px; font-size: 10px; color: #94a3b8; }
.dist-bar-bg { flex: 1; height: 5px; background: rgba(255,255,255,0.05); border-radius: 3px; overflow: hidden; }
.dist-bar { height: 100%; border-radius: 3px; transition: width 0.6s; }
.dist-bar.online { background: linear-gradient(90deg, #22c55d, #4ade80); }
.dist-bar.offline { background: linear-gradient(90deg, #64748b, #94a3b8); }
.dist-bar.alert { background: linear-gradient(90deg, #ef4444, #f87171); }
.dist-val { width: 22px; text-align: right; font-size: 13px; font-weight: 600; font-family: 'JetBrains Mono',monospace; }

/* 安防 */
.security-list { display: flex; flex-direction: column; gap: 5px; }
.sec-row { display: flex; align-items: center; gap: 8px; padding: 6px 8px; background: rgba(0,0,0,0.2); border-radius: 5px; border-left: 2px solid transparent; transition: all 0.3s; }
.sec-row.alert { border-left-color: #ef4444; background: rgba(239,68,68,0.07); }
.sec-icon { font-size: 16px; }
.sec-label { flex: 1; font-size: 11px; color: #94a3b8; }
.sec-status { font-size: 10px; font-weight: 600; }
.text-red { color: #ef4444; }
.text-green { color: #22c55d; }

/* 3D拓扑 */
.topology-card { flex: 1; display: flex; flex-direction: column; }
.topo-3d-wrap { flex: 1; position: relative; overflow: hidden; border-radius: 7px; min-height: 250px; }
.topo-3d-canvas { width: 100%; height: 100%; cursor: grab; }
.topo-3d-canvas:active { cursor: grabbing; }
.topo-tip { position: absolute; background: rgba(10,14,28,0.95); border: 1px solid rgba(59,130,246,0.35); border-radius: 7px; padding: 8px 12px; pointer-events: none; z-index: 10; box-shadow: 0 4px 16px rgba(0,0,0,0.5); }
.topo-tip-name { font-size: 13px; font-weight: 600; color: #e2e8f0; }
.topo-tip-info { font-size: 10px; color: #94a3b8; margin-top: 2px; }

/* 数据流向 */
.flow-card { }
.flow-canvas { width: 100%; height: 170px; }
.flow-legend { display: flex; justify-content: center; gap: 14px; margin-top: 4px; }
.fl-item { display: flex; align-items: center; gap: 4px; font-size: 9px; color: #64748b; }
.fl-dot { width: 7px; height: 7px; border-radius: 50%; }

/* 消防 */
.fire-grid-2 { display: grid; grid-template-columns: 1fr 1fr; gap: 6px; }
.fire-cell { display: flex; flex-direction: column; align-items: center; gap: 3px; padding: 8px 4px; background: rgba(0,0,0,0.22); border-radius: 7px; border: 1px solid transparent; transition: all 0.3s; }
.fire-cell.danger { border-color: #ef4444; background: rgba(239,68,68,0.08); animation: dangerFlash 2s infinite; }
@keyframes dangerFlash { 0%,100% { box-shadow: 0 0 0 rgba(239,68,68,0); } 50% { box-shadow: 0 0 12px rgba(239,68,68,0.25); } }
.fc-icon { font-size: 20px; }
.fc-label { font-size: 9px; color: #64748b; }
.fc-val { font-size: 13px; font-weight: 600; font-family: 'JetBrains Mono',monospace; }

/* 3D柱状图 */
.bar-canvas { width: 100%; height: 160px; }

/* 事件 */
.events-panel { }
.events-scroll { max-height: 160px; overflow-y: auto; display: flex; flex-direction: column; gap: 4px; }
.events-scroll::-webkit-scrollbar { width: 2px; }
.events-scroll::-webkit-scrollbar-thumb { background: rgba(59,130,246,0.15); border-radius: 2px; }
.event-row { display: flex; align-items: center; gap: 7px; padding: 4px 6px; border-radius: 4px; font-size: 10px; transition: background 0.3s; }
.event-row:hover { background: rgba(255,255,255,0.02); }
.event-dot { width: 5px; height: 5px; border-radius: 50%; flex-shrink: 0; }
.event-dot.info { background: #3b82f6; }
.event-dot.warning { background: #f59e0b; }
.event-text { flex: 1; color: #94a3b8; white-space: nowrap; overflow: hidden; text-overflow: ellipsis; }
.event-time { font-family: 'JetBrains Mono',monospace; font-size: 9px; color: #475569; flex-shrink: 0; }

/* 底部 */
.screen-footer { display: flex; justify-content: center; gap: 28px; padding: 6px 16px; background: rgba(10,14,28,0.9); border-top: 1px solid rgba(59,130,246,0.12); position: relative; z-index: 2; }
.footer-item { font-size: 10px; color: #475569; }

.fade-enter-active,.fade-leave-active { transition: opacity 0.15s; }
.fade-enter-from,.fade-leave-to { opacity: 0; }

@media (max-width: 1400px) { .screen-main { grid-template-columns: 220px 1fr 220px; } }
@media (max-width: 1100px) { .screen-main { grid-template-columns: 1fr; } .left-panel,.right-panel { display: none; } }
</style>
