<template>
  <div class="graph-root">
    <ParticleBackground :particle-count="30" color="#06b6d4" :opacity="0.3" />
    <AppNavbar title="设备关系图谱" subtitle="Device Relationship Graph — 3D" :menu-items="menuItems" />

    <main class="graph-main">
      <section class="control-section">
        <div class="section-header">
          <h2 class="section-title">
            <svg width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><circle cx="12" cy="12" r="10"/><line x1="2" y1="12" x2="22" y2="12"/><path d="M12 2a15.3 15.3 0 0 1 4 10 15.3 15.3 0 0 1-4 10 15.3 15.3 0 0 1-4-10 15.3 15.3 0 0 1 4-10z"/></svg>
            设备关系图谱 · 3D
          </h2>
          <div class="header-actions">
            <CyberButton variant="ghost" size="sm" @click="resetView">重置视角</CyberButton>
            <CyberButton variant="ghost" size="sm" @click="toggleAutoRotate">{{ autoRotate ? '暂停旋转' : '自动旋转' }}</CyberButton>
            <CyberButton variant="ghost" size="sm" @click="toggleLabels">{{ showLabels ? '隐藏标签' : '显示标签' }}</CyberButton>
            <CyberButton variant="primary" size="sm" @click="exportGraph">导出图谱</CyberButton>
          </div>
        </div>

        <div class="filter-bar">
          <div class="filter-group">
            <span class="filter-label">设备类型：</span>
            <button v-for="type in deviceTypes" :key="type.value" class="filter-btn" :class="{ active: activeTypes.includes(type.value) }" @click="toggleType(type.value)">
              <span class="filter-dot" :style="{ background: type.color }"></span>{{ type.label }}
            </button>
          </div>
          <div class="filter-group">
            <span class="filter-label">连接类型：</span>
            <button v-for="conn in connectionTypes" :key="conn.value" class="filter-btn" :class="{ active: activeConnections.includes(conn.value) }" @click="toggleConnection(conn.value)">{{ conn.label }}</button>
          </div>
        </div>
      </section>

      <section class="graph-canvas-section">
        <div class="graph-3d-wrap" ref="wrapRef"
          @mousedown="onMouseDown" @mousemove="onMouseMove" @mouseup="onMouseUp" @mouseleave="onMouseUp" @wheel="onWheel">
          <canvas ref="canvasRef"></canvas>

          <Transition name="fade">
            <div v-if="hoveredNode" class="node-tooltip" :style="{ left: tipX + 'px', top: tipY + 'px' }">
              <div class="tooltip-header" :style="{ borderColor: hoveredNode.color }">
                <span class="tooltip-icon">{{ hoveredNode.icon }}</span>
                <span class="tooltip-title">{{ hoveredNode.name }}</span>
              </div>
              <div class="tooltip-body">
                <div class="tooltip-stat"><span class="stat-label">类型</span><span class="stat-value">{{ hoveredNode.type }}</span></div>
                <div class="tooltip-stat"><span class="stat-label">状态</span><span class="stat-value" :class="hoveredNode.status === 'online' ? 'online' : 'offline'">{{ hoveredNode.status === 'online' ? '在线' : '离线' }}</span></div>
                <div class="tooltip-stat"><span class="stat-label">层级</span><span class="stat-value">{{ hoveredNode.layer }}</span></div>
                <div class="tooltip-stat"><span class="stat-label">连接数</span><span class="stat-value">{{ hoveredNode.connections }}</span></div>
              </div>
            </div>
          </Transition>
        </div>

        <div class="graph-legend">
          <div class="legend-title">设备类型</div>
          <div v-for="type in deviceTypes" :key="type.value" class="legend-item">
            <span class="legend-dot" :style="{ background: type.color }"></span>{{ type.label }}
          </div>
          <div class="legend-title" style="margin-top:14px">连接类型</div>
          <div v-for="conn in connectionTypes" :key="conn.value" class="legend-item">
            <span class="legend-line" :style="{ borderColor: conn.color }"></span>{{ conn.label }}
          </div>
        </div>
      </section>
    </main>
  </div>
</template>

<script setup lang="ts">
import { ref, reactive, onMounted, onUnmounted } from 'vue'
import { ElMessage } from 'element-plus'
import ParticleBackground from '../components/ParticleBackground.vue'
import AppNavbar from '../components/AppNavbar.vue'
import CyberButton from '../components/CyberButton.vue'

const menuItems = [
  { path: '/', label: '主控台' },
  { path: '/devices', label: '设备管理' },
  { path: '/ai-analysis', label: 'AI分析' },
  { path: '/settings', label: '系统设置' }
]

const wrapRef = ref<HTMLElement | null>(null)
const canvasRef = ref<HTMLCanvasElement | null>(null)
const showLabels = ref(true)
const autoRotate = ref(true)
const hoveredNode = ref<any>(null)
const tipX = ref(0); const tipY = ref(0)

const deviceTypes = [
  { value: 'sensor', label: '传感器', color: '#3b82f6' },
  { value: 'controller', label: '控制器', color: '#22c55d' },
  { value: 'alarm', label: '报警器', color: '#ef4444' },
  { value: 'gateway', label: '网关', color: '#8b5cf6' },
]
const connectionTypes = [
  { value: 'data', label: '数据', color: '#3b82f6' },
  { value: 'control', label: '控制', color: '#22c55d' },
  { value: 'alarm', label: '报警', color: '#ef4444' },
]
const activeTypes = ref(['sensor', 'controller', 'alarm', 'gateway'])
const activeConnections = ref(['data', 'control', 'alarm'])

// ==================== 3D多层图谱数据 ====================
interface GraphNode3D {
  id: string; name: string; type: string; icon: string; color: string
  status: string; connections: number; layer: string; x: number; y: number; z: number
}
interface GraphEdge3D { source: string; target: string; type: string; active: boolean }

const graphData = reactive({
  nodes: [
    // Z=80: 应用层
    { id: 'app-security', name: '安防系统', type: 'gateway', icon: '🛡️', color: '#8b5cf6', status: 'online', connections: 6, layer: '应用层', x: -80, y: -140, z: 80 },
    { id: 'app-env', name: '环境监测', type: 'gateway', icon: '🌡️', color: '#3b82f6', status: 'online', connections: 5, layer: '应用层', x: 80, y: -140, z: 80 },
    { id: 'app-fire', name: '消防系统', type: 'gateway', icon: '🔥', color: '#ef4444', status: 'online', connections: 5, layer: '应用层', x: 0, y: -160, z: 80 },
    // Z=0: 原子服务层
    { id: 'svc-collect', name: '数据采集', type: 'controller', icon: '📥', color: '#3b82f6', status: 'online', connections: 10, layer: '原子服务', x: -120, y: -40, z: 0 },
    { id: 'svc-process', name: '数据处理', type: 'controller', icon: '⚙️', color: '#8b5cf6', status: 'online', connections: 8, layer: '原子服务', x: 0, y: -20, z: 0 },
    { id: 'svc-alarm', name: '报警判断', type: 'controller', icon: '🔔', color: '#ef4444', status: 'online', connections: 7, layer: '原子服务', x: 120, y: -40, z: 0 },
    { id: 'svc-store', name: '数据存储', type: 'controller', icon: '💾', color: '#f59e0b', status: 'online', connections: 5, layer: '原子服务', x: 0, y: 60, z: 0 },
    // Z=-80: 设备抽象层（云测仪→7个虚拟传感器 + 独立设备）
    { id: 'abs-temp', name: '虚拟温度', type: 'sensor', icon: '🌡️', color: '#3b82f6', status: 'online', connections: 3, layer: '抽象层', x: -200, y: 80, z: -80 },
    { id: 'abs-humi', name: '虚拟湿度', type: 'sensor', icon: '💧', color: '#06b6d4', status: 'online', connections: 3, layer: '抽象层', x: -140, y: 95, z: -80 },
    { id: 'abs-pm25', name: '虚拟PM2.5', type: 'sensor', icon: '💨', color: '#f59e0b', status: 'online', connections: 3, layer: '抽象层', x: -70, y: 108, z: -80 },
    { id: 'abs-co2', name: '虚拟CO₂', type: 'sensor', icon: '☁️', color: '#8b5cf6', status: 'online', connections: 3, layer: '抽象层', x: 0, y: 115, z: -80 },
    { id: 'abs-tvoc', name: '虚拟TVOC', type: 'sensor', icon: '🧪', color: '#ec4899', status: 'online', connections: 3, layer: '抽象层', x: 70, y: 108, z: -80 },
    { id: 'abs-ch2o', name: '虚拟甲醛', type: 'sensor', icon: '⚗️', color: '#14b8a6', status: 'online', connections: 3, layer: '抽象层', x: 140, y: 95, z: -80 },
    { id: 'abs-pm10', name: '虚拟PM10', type: 'sensor', icon: '💨', color: '#f59e0b', status: 'online', connections: 3, layer: '抽象层', x: 200, y: 80, z: -80 },
    { id: 'abs-smoke', name: '虚拟烟雾', type: 'alarm', icon: '🔥', color: '#ef4444', status: 'online', connections: 3, layer: '抽象层', x: -200, y: 160, z: -80 },
    { id: 'abs-water', name: '虚拟水浸', type: 'sensor', icon: '💧', color: '#06b6d4', status: 'online', connections: 2, layer: '抽象层', x: 200, y: 160, z: -80 },
    // Z=-160: 设备层
    { id: 'dev-cloud', name: '云测仪', type: 'sensor', icon: '☁️', color: '#3b82f6', status: 'online', connections: 7, layer: '设备层', x: -100, y: 200, z: -160 },
    { id: 'dev-smoke', name: '烟雾报警器', type: 'alarm', icon: '🔥', color: '#ef4444', status: 'online', connections: 3, layer: '设备层', x: 100, y: 190, z: -160 },
    { id: 'dev-water', name: '水浸传感器', type: 'sensor', icon: '💧', color: '#06b6d4', status: 'online', connections: 2, layer: '设备层', x: 180, y: 170, z: -160 },
    { id: 'dev-infrared', name: '红外探测器', type: 'sensor', icon: '👤', color: '#8b5cf6', status: 'online', connections: 3, layer: '设备层', x: -180, y: 180, z: -160 },
    { id: 'dev-light', name: '弱光传感器', type: 'sensor', icon: '💡', color: '#f59e0b', status: 'offline', connections: 2, layer: '设备层', x: 0, y: 220, z: -160 },
    { id: 'dev-aircon', name: '空调控制器', type: 'controller', icon: '❄️', color: '#22c55d', status: 'online', connections: 2, layer: '设备层', x: -60, y: 240, z: -160 },
    { id: 'dev-humi', name: '恒湿净化机', type: 'controller', icon: '🌀', color: '#22c55d', status: 'offline', connections: 2, layer: '设备层', x: 80, y: 235, z: -160 },
  ] as GraphNode3D[],
  edges: [
    // 设备层 → 抽象层 (云测仪解耦为7个虚拟传感器)
    { source: 'dev-cloud', target: 'abs-temp', type: 'data', active: true },
    { source: 'dev-cloud', target: 'abs-humi', type: 'data', active: true },
    { source: 'dev-cloud', target: 'abs-pm25', type: 'data', active: true },
    { source: 'dev-cloud', target: 'abs-co2', type: 'data', active: true },
    { source: 'dev-cloud', target: 'abs-tvoc', type: 'data', active: true },
    { source: 'dev-cloud', target: 'abs-ch2o', type: 'data', active: true },
    { source: 'dev-cloud', target: 'abs-pm10', type: 'data', active: true },
    { source: 'dev-smoke', target: 'abs-smoke', type: 'alarm', active: true },
    { source: 'dev-water', target: 'abs-water', type: 'data', active: true },
    { source: 'dev-infrared', target: 'abs-smoke', type: 'data', active: true },
    { source: 'dev-light', target: 'abs-temp', type: 'data', active: false },
    { source: 'dev-aircon', target: 'abs-temp', type: 'control', active: true },
    { source: 'dev-humi', target: 'abs-humi', type: 'control', active: false },
    // 抽象层 → 原子服务
    { source: 'abs-temp', target: 'svc-collect', type: 'data', active: true },
    { source: 'abs-humi', target: 'svc-collect', type: 'data', active: true },
    { source: 'abs-pm25', target: 'svc-collect', type: 'data', active: true },
    { source: 'abs-co2', target: 'svc-collect', type: 'data', active: true },
    { source: 'abs-tvoc', target: 'svc-collect', type: 'data', active: true },
    { source: 'abs-ch2o', target: 'svc-collect', type: 'data', active: true },
    { source: 'abs-pm10', target: 'svc-collect', type: 'data', active: true },
    { source: 'abs-smoke', target: 'svc-alarm', type: 'alarm', active: true },
    { source: 'abs-water', target: 'svc-alarm', type: 'data', active: true },
    { source: 'abs-smoke', target: 'svc-collect', type: 'data', active: true },
    { source: 'abs-water', target: 'svc-collect', type: 'data', active: true },
    // 原子服务间
    { source: 'svc-collect', target: 'svc-process', type: 'data', active: true },
    { source: 'svc-collect', target: 'svc-alarm', type: 'data', active: true },
    { source: 'svc-process', target: 'svc-store', type: 'data', active: true },
    { source: 'svc-alarm', target: 'svc-process', type: 'control', active: true },
    // 原子服务 → 应用层
    { source: 'svc-collect', target: 'app-security', type: 'data', active: true },
    { source: 'svc-collect', target: 'app-env', type: 'data', active: true },
    { source: 'svc-collect', target: 'app-fire', type: 'data', active: true },
    { source: 'svc-process', target: 'app-env', type: 'data', active: true },
    { source: 'svc-alarm', target: 'app-security', type: 'alarm', active: true },
    { source: 'svc-alarm', target: 'app-fire', type: 'alarm', active: true },
    { source: 'svc-store', target: 'app-security', type: 'data', active: true },
    { source: 'svc-store', target: 'app-env', type: 'data', active: true },
    { source: 'svc-store', target: 'app-fire', type: 'data', active: true },
    // 跨层级直接连接 (紧急联动)
    { source: 'dev-smoke', target: 'svc-alarm', type: 'alarm', active: true },
    { source: 'dev-infrared', target: 'app-security', type: 'alarm', active: true },
    { source: 'dev-water', target: 'app-fire', type: 'alarm', active: true },
    { source: 'abs-temp', target: 'app-env', type: 'data', active: true },
    { source: 'abs-co2', target: 'app-env', type: 'data', active: true },
  ] as GraphEdge3D[],
})

// ==================== 3D渲染引擎 ====================
let animId = 0
let rotX = -20, rotY = 30, startRX = 0, startRY = 0
let isDrag = false, dragSX = 0, dragSY = 0
let viewZoom = 1.0

function project(x: number, y: number, z: number, cx: number, cy: number) {
  const rx = rotX * Math.PI / 180, ry = rotY * Math.PI / 180
  const x1 = x * Math.cos(ry) - z * Math.sin(ry)
  const z1 = x * Math.sin(ry) + z * Math.cos(ry)
  const y1 = y * Math.cos(rx) - z1 * Math.sin(rx)
  const z2 = y * Math.sin(rx) + z1 * Math.cos(rx)
  const s = 600 / (600 + z2) * viewZoom
  return { x: cx + x1 * s, y: cy + y1 * s, s, depth: z2 }
}

function onMouseDown(e: MouseEvent) { isDrag = true; dragSX = e.clientX; dragSY = e.clientY; startRX = rotX; startRY = rotY }
function onMouseMove(e: MouseEvent) {
  if (isDrag) { rotY = startRY + (e.clientX - dragSX) * 0.4; rotX = startRX + (e.clientY - dragSY) * 0.4 }
  if (canvasRef.value && wrapRef.value) {
    const r = canvasRef.value.getBoundingClientRect()
    checkHover(e.clientX - r.left, e.clientY - r.top, r.width, r.height)
  }
}
function onMouseUp() { isDrag = false }
function onWheel(e: WheelEvent) { e.preventDefault(); viewZoom = Math.max(0.3, Math.min(3, viewZoom - e.deltaY * 0.0008)) }

function checkHover(mx: number, my: number, w: number, h: number) {
  const cx = w / 2, cy = h / 2
  let found: GraphNode3D | null = null
  for (const node of graphData.nodes) {
    if (!activeTypes.value.includes(node.type)) continue
    const p = project(node.x, node.y, node.z, cx, cy)
    if (Math.hypot(mx - p.x, my - p.y) < 16 * p.s) { found = node; break }
  }
  hoveredNode.value = found
  if (found) { tipX.value = mx + 14; tipY.value = my - 8 }
}

function toggleType(t: string) { const i = activeTypes.value.indexOf(t); i === -1 ? activeTypes.value.push(t) : activeTypes.value.splice(i, 1) }
function toggleConnection(c: string) { const i = activeConnections.value.indexOf(c); i === -1 ? activeConnections.value.push(c) : activeConnections.value.splice(i, 1) }
function resetView() { rotX = -20; rotY = 30; viewZoom = 1.0; ElMessage.success('视角已重置') }
function toggleAutoRotate() { autoRotate.value = !autoRotate.value }
function toggleLabels() { showLabels.value = !showLabels.value }
function exportGraph() {
  if (!canvasRef.value) return
  const a = document.createElement('a'); a.download = 'device-graph-3d.png'
  a.href = canvasRef.value.toDataURL('image/png'); a.click()
  ElMessage.success('图谱已导出')
}

function draw(ts: number) {
  if (!canvasRef.value || !wrapRef.value) return
  const canvas = canvasRef.value; const ctx = canvas.getContext('2d')
  if (!ctx) return
  const rect = wrapRef.value.getBoundingClientRect()
  canvas.width = rect.width; canvas.height = rect.height
  const w = canvas.width, h = canvas.height, cx = w / 2, cy = h / 2
  if (autoRotate.value && !isDrag) rotY += 0.25

  // 深空背景
  ctx.clearRect(0, 0, w, h)
  const bgGrad = ctx.createRadialGradient(cx, cy, 0, cx, cy, Math.max(w, h) * 0.7)
  bgGrad.addColorStop(0, 'rgba(30,64,175,0.08)'); bgGrad.addColorStop(0.5, 'rgba(15,23,42,0.03)'); bgGrad.addColorStop(1, 'rgba(0,0,0,0)')
  ctx.fillStyle = bgGrad; ctx.fillRect(0, 0, w, h)

  // 层轨道环
  const orbitRings = [100, 200, 300, 400]
  orbitRings.forEach((r, i) => {
    ctx.beginPath(); ctx.arc(cx, cy, r, 0, Math.PI * 2)
    ctx.strokeStyle = ['rgba(139,92,246,0.08)','rgba(59,130,246,0.06)','rgba(6,182,212,0.06)','rgba(245,158,11,0.05)'][i]
    ctx.lineWidth = 1; ctx.stroke()
    // 虚线轨道
    ctx.setLineDash([4, 20]); ctx.lineDashOffset = ts * 0.02
    ctx.strokeStyle = ['rgba(139,92,246,0.04)','rgba(59,130,246,0.03)','rgba(6,182,212,0.03)','rgba(245,158,11,0.02)'][i]
    ctx.stroke(); ctx.setLineDash([])
  })

  // 投影并排序
  const proj = graphData.nodes
    .filter(n => activeTypes.value.includes(n.type))
    .map(n => ({ ...project(n.x, n.y, n.z, cx, cy), node: n }))
    .sort((a, b) => a.depth - b.depth)

  // 层标签
  const layers = [
    { z: 80, label: '应用层', y: -145, color: '#8b5cf6' },
    { z: 0, label: '原子服务层', y: 10, color: '#3b82f6' },
    { z: -80, label: '设备抽象层', y: 110, color: '#06b6d4' },
    { z: -160, label: '设备层', y: 210, color: '#f59e0b' },
  ]
  ctx.font = 'bold 11px sans-serif'; ctx.textAlign = 'left'
  for (const l of layers) {
    const lp = project(0, l.y, l.z, cx, cy)
    ctx.fillStyle = l.color + '60'; ctx.fillText(l.label, 12, Math.max(18, lp.y - 5))
  }

  // 画边+粒子流
  const edgeSet = new Set<string>()
  for (const p of proj) {
    for (const edge of graphData.edges) {
      if (!activeConnections.value.includes(edge.type)) continue
      if (edge.source !== p.node.id && edge.target !== p.node.id) continue
      const oid = edge.source === p.node.id ? edge.target : edge.source
      const key = [p.node.id, oid].sort().join('-')
      if (edgeSet.has(key)) continue; edgeSet.add(key)
      const other = proj.find(pp => pp.node.id === oid)
      if (!other) continue
      const ct = connectionTypes.find(c => c.value === edge.type)
      const col = ct?.color || '#475569'
      // 主连线
      ctx.beginPath(); ctx.moveTo(p.x, p.y); ctx.lineTo(other.x, other.y)
      ctx.strokeStyle = edge.active ? col + '35' : '#33415512'
      ctx.lineWidth = edge.active ? 1.8 : 0.5; ctx.stroke()
      // 发光连线
      if (edge.active) {
        ctx.beginPath(); ctx.moveTo(p.x, p.y); ctx.lineTo(other.x, other.y)
        ctx.strokeStyle = col + '15'; ctx.lineWidth = 4; ctx.stroke()
      }
      // 多个流动粒子
      if (edge.active) {
        for (let pi = 0; pi < 4; pi++) {
          const prog = ((ts * 0.0003 + pi * 0.25 + p.node.id.charCodeAt(0) * 0.005) % 1 + 1) % 1
          const px = p.x + (other.x - p.x) * prog, py = p.y + (other.y - p.y) * prog
          // 拖尾
          const trailProg = Math.max(0, prog - 0.04)
          const tx = p.x + (other.x - p.x) * trailProg, ty = p.y + (other.y - p.y) * trailProg
          ctx.beginPath(); ctx.moveTo(tx, ty); ctx.lineTo(px, py)
          ctx.strokeStyle = col + '50'; ctx.lineWidth = 3; ctx.stroke()
          // 粒子主体
          ctx.beginPath(); ctx.arc(px, py, 2.5, 0, Math.PI * 2)
          ctx.fillStyle = '#fff'; ctx.fill()
          ctx.beginPath(); ctx.arc(px, py, 5, 0, Math.PI * 2)
          ctx.fillStyle = col + '60'; ctx.fill()
        }
      }
    }
  }

  // 画节点
  const pulse = Math.sin(ts * 0.003) * 0.3 + 0.7 // 0.4~1.0 呼吸
  for (const p of proj) {
    const sz = 16 * p.s; const node = p.node
    // 外圈脉动光晕
    if (node.status === 'online') {
      const pr = sz * 2.5 * pulse
      const g = ctx.createRadialGradient(p.x, p.y, sz * 0.4, p.x, p.y, pr)
      g.addColorStop(0, node.color + '60'); g.addColorStop(0.5, node.color + '15'); g.addColorStop(1, 'transparent')
      ctx.beginPath(); ctx.arc(p.x, p.y, pr, 0, Math.PI * 2); ctx.fillStyle = g; ctx.fill()
      // 第二层光晕
      const g2 = ctx.createRadialGradient(p.x, p.y, sz * 0.2, p.x, p.y, sz * 4)
      g2.addColorStop(0, node.color + '20'); g2.addColorStop(1, 'transparent')
      ctx.beginPath(); ctx.arc(p.x, p.y, sz * 4, 0, Math.PI * 2); ctx.fillStyle = g2; ctx.fill()
    }
    // 节点主体
    const grad = ctx.createRadialGradient(p.x - sz*0.3, p.y - sz*0.3, sz*0.1, p.x, p.y, sz)
    const base = node.status === 'online' ? node.color : '#334155'
    grad.addColorStop(0, '#ffffff'); grad.addColorStop(0.3, base); grad.addColorStop(1, base + '60')
    ctx.beginPath(); ctx.arc(p.x, p.y, sz, 0, Math.PI * 2)
    ctx.fillStyle = grad; ctx.fill()
    ctx.strokeStyle = node.status === 'online' ? '#ffffff40' : '#475569'; ctx.lineWidth = 1.5; ctx.stroke()
    // 图标
    const isz = Math.max(11, sz * 0.75)
    ctx.font = `${isz}px serif`; ctx.textAlign = 'center'; ctx.textBaseline = 'middle'
    ctx.fillStyle = '#fff'; ctx.fillText(node.icon, p.x, p.y)
    // 标签
    if (showLabels.value && p.s > 0.35) {
      ctx.font = `bold ${Math.max(9, 11 * p.s)}px sans-serif`
      ctx.fillStyle = node.status === 'online' ? node.color : '#64748b'
      ctx.fillText(node.name, p.x, p.y + sz + 14 * p.s)
    }
  }
}

function animate(ts: number) { draw(ts); animId = requestAnimationFrame(animate) }

onMounted(() => { animId = requestAnimationFrame(animate) })
onUnmounted(() => { cancelAnimationFrame(animId) })
</script>

<style scoped>
.graph-root { min-height: 100vh; background: var(--bg-primary); position: relative; }
.graph-main { position: relative; z-index: 1; padding: var(--spacing-xl); max-width: 1920px; margin: 0 auto; }

.section-header { display: flex; align-items: center; justify-content: space-between; margin-bottom: var(--spacing-lg); }
.section-title { display: flex; align-items: center; gap: var(--spacing-sm); font-size: 20px; font-weight: 600; color: var(--text-primary); margin: 0; }
.section-title svg { color: #06b6d4; }
.header-actions { display: flex; gap: var(--spacing-sm); }

.filter-bar { display: flex; gap: var(--spacing-xl); margin-bottom: var(--spacing-lg); }
.filter-group { display: flex; align-items: center; gap: var(--spacing-sm); }
.filter-label { font-size: 13px; color: var(--text-secondary); }
.filter-btn { display: flex; align-items: center; gap: 6px; padding: 6px 12px; background: transparent; border: 1px solid var(--border-primary); border-radius: var(--radius-full); color: var(--text-secondary); font-size: 12px; cursor: pointer; transition: all var(--duration-fast); }
.filter-btn:hover { border-color: var(--accent-primary); color: var(--text-primary); }
.filter-btn.active { background: rgba(59,130,246,0.2); border-color: var(--accent-primary); color: var(--accent-primary); }
.filter-dot { width: 8px; height: 8px; border-radius: 50%; }

.graph-canvas-section { display: flex; gap: var(--spacing-xl); }
.graph-3d-wrap { flex: 1; height: 620px; background: var(--bg-card); border: 1px solid var(--border-primary); border-radius: var(--radius-lg); position: relative; overflow: hidden; cursor: grab; }
.graph-3d-wrap:active { cursor: grabbing; }
.graph-3d-wrap canvas { width: 100%; height: 100%; }

.node-tooltip { position: absolute; background: var(--bg-card); border: 1px solid var(--border-primary); border-radius: var(--radius-md); padding: var(--spacing-md); min-width: 180px; box-shadow: var(--shadow-lg); pointer-events: none; z-index: 10; }
.tooltip-header { display: flex; align-items: center; gap: var(--spacing-sm); padding-bottom: var(--spacing-sm); border-bottom: 1px solid var(--border-primary); margin-bottom: var(--spacing-sm); }
.tooltip-icon { font-size: 20px; }
.tooltip-title { font-size: 14px; font-weight: 600; color: var(--text-primary); }
.tooltip-stat { display: flex; justify-content: space-between; align-items: center; padding: 4px 0; }
.stat-label { font-size: 12px; color: var(--text-secondary); }
.stat-value { font-size: 13px; font-weight: 500; color: var(--text-primary); }
.stat-value.online { color: #22c55d; }
.stat-value.offline { color: #ef4444; }

.graph-legend { width: 180px; background: var(--bg-card); border: 1px solid var(--border-primary); border-radius: var(--radius-lg); padding: var(--spacing-md); flex-shrink: 0; }
.legend-title { font-size: 11px; font-weight: 600; color: var(--text-secondary); text-transform: uppercase; letter-spacing: 0.5px; margin-bottom: var(--spacing-sm); }
.legend-item { display: flex; align-items: center; gap: 8px; margin-bottom: 6px; font-size: 12px; color: var(--text-secondary); }
.legend-dot { width: 10px; height: 10px; border-radius: 50%; }
.legend-line { width: 20px; height: 0; border-top: 2px solid; }

.fade-enter-active,.fade-leave-active { transition: opacity 0.15s; }
.fade-enter-from,.fade-leave-to { opacity: 0; }

@media (max-width: 1200px) { .graph-canvas-section { flex-direction: column; } .graph-legend { width: 100%; display: flex; flex-wrap: wrap; gap: 16px; } }
@media (max-width: 768px) { .graph-main { padding: var(--spacing-md); } .filter-bar { flex-direction: column; gap: var(--spacing-sm); } }
</style>
