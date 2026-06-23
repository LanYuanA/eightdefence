<template>
  <div class="graph-root">
    <ParticleBackground :particle-count="30" color="#06b6d4" :opacity="0.3" />
    <AppNavbar title="设备关系图谱" subtitle="Device Relationship Graph" :menu-items="menuItems" />

    <main class="graph-main">
      <!-- 图谱控制 -->
      <section class="control-section">
        <div class="section-header">
          <h2 class="section-title">
            <svg width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><circle cx="12" cy="12" r="10"/><line x1="2" y1="12" x2="22" y2="12"/><path d="M12 2a15.3 15.3 0 0 1 4 10 15.3 15.3 0 0 1-4 10 15.3 15.3 0 0 1-4-10 15.3 15.3 0 0 1 4-10z"/></svg>
            设备关系图谱
          </h2>
          <div class="header-actions">
            <CyberButton variant="ghost" size="sm" @click="resetGraph">
              <svg width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M21.5 2v6h-6M2.5 22v-6h6M2 11.5a10 10 0 0 1 18.8-4.3M22 12.5a10 10 0 0 1-18.8 4.3"/></svg>
              重置布局
            </CyberButton>
            <CyberButton variant="ghost" size="sm" @click="toggleLabels">
              <svg width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M4 7V4h16v3"/><path d="M9 20h6"/><path d="M12 4v16"/></svg>
              {{ showLabels ? '隐藏标签' : '显示标签' }}
            </CyberButton>
            <CyberButton variant="primary" size="sm" @click="exportGraph">
              <svg width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M21 15v4a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2v-4"/><polyline points="17 8 12 3 7 8"/><line x1="12" y1="3" x2="12" y2="15"/></svg>
              导出图谱
            </CyberButton>
          </div>
        </div>

        <!-- 筛选器 -->
        <div class="filter-bar">
          <div class="filter-group">
            <span class="filter-label">设备类型：</span>
            <button v-for="type in deviceTypes" :key="type.value" class="filter-btn" :class="{ active: activeTypes.includes(type.value) }" @click="toggleType(type.value)">
              <span class="filter-dot" :style="{ background: type.color }"></span>
              {{ type.label }}
            </button>
          </div>
          <div class="filter-group">
            <span class="filter-label">连接类型：</span>
            <button v-for="conn in connectionTypes" :key="conn.value" class="filter-btn" :class="{ active: activeConnections.includes(conn.value) }" @click="toggleConnection(conn.value)">
              {{ conn.label }}
            </button>
          </div>
        </div>
      </section>

      <!-- 图谱画布 -->
      <section class="graph-canvas-section">
        <div class="graph-canvas" ref="canvasRef" @mousemove="handleMouseMove" @mouseleave="hoveredNode = null">
          <canvas ref="graphCanvasRef"></canvas>

          <!-- 节点信息卡 -->
          <Transition name="fade">
            <div v-if="hoveredNode" class="node-tooltip" :style="{ left: tooltipPos.x + 'px', top: tooltipPos.y + 'px' }">
              <div class="tooltip-header" :style="{ borderColor: hoveredNode.color }">
                <span class="tooltip-icon">{{ hoveredNode.icon }}</span>
                <span class="tooltip-title">{{ hoveredNode.name }}</span>
              </div>
              <div class="tooltip-body">
                <div class="tooltip-stat"><span class="stat-label">类型</span><span class="stat-value">{{ hoveredNode.type }}</span></div>
                <div class="tooltip-stat"><span class="stat-label">状态</span><span class="stat-value" :class="hoveredNode.status">{{ hoveredNode.status === 'online' ? '在线' : '离线' }}</span></div>
                <div class="tooltip-stat"><span class="stat-label">连接数</span><span class="stat-value">{{ hoveredNode.connections }}</span></div>
              </div>
            </div>
          </Transition>
        </div>

        <!-- 图例 -->
        <div class="graph-legend">
          <div class="legend-title">图例</div>
          <div class="legend-items">
            <div v-for="type in deviceTypes" :key="type.value" class="legend-item">
              <span class="legend-dot" :style="{ background: type.color }"></span>
              <span class="legend-label">{{ type.label }}</span>
            </div>
          </div>
          <div class="legend-connections">
            <div class="legend-title">连接类型</div>
            <div v-for="conn in connectionTypes" :key="conn.value" class="legend-item">
              <span class="legend-line" :style="{ borderColor: conn.color }"></span>
              <span class="legend-label">{{ conn.label }}</span>
            </div>
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

const canvasRef = ref<HTMLElement | null>(null)
const graphCanvasRef = ref<HTMLCanvasElement | null>(null)
const showLabels = ref(true)
const hoveredNode = ref<any>(null)
const tooltipPos = reactive({ x: 0, y: 0 })

const deviceTypes = [
  { value: 'sensor', label: '传感器', color: '#3b82f6' },
  { value: 'controller', label: '控制器', color: '#22c55d' },
  { value: 'alarm', label: '报警器', color: '#ef4444' },
  { value: 'gateway', label: '网关', color: '#8b5cf6' }
]

const connectionTypes = [
  { value: 'data', label: '数据连接', color: '#3b82f6' },
  { value: 'control', label: '控制连接', color: '#22c55d' },
  { value: 'alarm', label: '报警连接', color: '#ef4444' }
]

const activeTypes = ref(['sensor', 'controller', 'alarm', 'gateway'])
const activeConnections = ref(['data', 'control', 'alarm'])

// 图谱数据 - 完整设备拓扑
const graphData = reactive({
  nodes: [
    // 传感器设备
    { id: 'cloud-temp', name: '云测仪-温度', type: 'sensor', icon: '🌡️', color: '#3b82f6', x: 300, y: 80, status: 'online', connections: 4 },
    { id: 'cloud-humi', name: '云测仪-湿度', type: 'sensor', icon: '💧', color: '#06b6d4', x: 400, y: 80, status: 'online', connections: 3 },
    { id: 'cloud-pm25', name: '云测仪-PM2.5', type: 'sensor', icon: '💨', color: '#f59e0b', x: 500, y: 80, status: 'online', connections: 3 },
    { id: 'cloud-co2', name: '云测仪-CO2', type: 'sensor', icon: '☁️', color: '#8b5cf6', x: 600, y: 80, status: 'online', connections: 3 },
    { id: 'smoke', name: '烟雾报警器', type: 'alarm', icon: '🔥', color: '#ef4444', x: 150, y: 200, status: 'online', connections: 4 },
    { id: 'water', name: '水浸传感器', type: 'sensor', icon: '💧', color: '#06b6d4', x: 150, y: 320, status: 'online', connections: 3 },
    { id: 'infrared', name: '红外探测器', type: 'sensor', icon: '👤', color: '#8b5cf6', x: 750, y: 200, status: 'online', connections: 4 },
    { id: 'light', name: '弱光传感器', type: 'sensor', icon: '💡', color: '#f59e0b', x: 750, y: 320, status: 'online', connections: 2 },
    { id: 'tvoc', name: 'TVOC传感器', type: 'sensor', icon: '🧪', color: '#ec4899', x: 300, y: 450, status: 'online', connections: 2 },
    { id: 'ch2o', name: '甲醛传感器', type: 'sensor', icon: '⚗️', color: '#14b8a6', x: 450, y: 450, status: 'online', connections: 2 },
    // 控制设备
    { id: 'aircon', name: '空调控制器', type: 'controller', icon: '❄️', color: '#22c55d', x: 600, y: 450, status: 'online', connections: 3 },
    { id: 'humidifier', name: '恒湿净化机', type: 'controller', icon: '🌀', color: '#22c55d', x: 750, y: 450, status: 'offline', connections: 2 },
    { id: 'alarm', name: '声光报警器', type: 'alarm', icon: '🔔', color: '#ef4444', x: 450, y: 300, status: 'online', connections: 5 },
    { id: 'spray', name: '喷淋控制器', type: 'controller', icon: '💦', color: '#06b6d4', x: 600, y: 300, status: 'online', connections: 2 },
    // 网关
    { id: 'gateway', name: '网关', type: 'gateway', icon: '🔌', color: '#f59e0b', x: 450, y: 180, status: 'online', connections: 12 }
  ],
  edges: [
    // 传感器 → 网关
    { source: 'cloud-temp', target: 'gateway', type: 'data', active: true },
    { source: 'cloud-humi', target: 'gateway', type: 'data', active: true },
    { source: 'cloud-pm25', target: 'gateway', type: 'data', active: true },
    { source: 'cloud-co2', target: 'gateway', type: 'data', active: true },
    { source: 'smoke', target: 'gateway', type: 'alarm', active: true },
    { source: 'water', target: 'gateway', type: 'data', active: true },
    { source: 'infrared', target: 'gateway', type: 'alarm', active: true },
    { source: 'light', target: 'gateway', type: 'data', active: true },
    { source: 'tvoc', target: 'gateway', type: 'data', active: true },
    { source: 'ch2o', target: 'gateway', type: 'data', active: true },
    // 网关 → 控制设备
    { source: 'gateway', target: 'aircon', type: 'control', active: true },
    { source: 'gateway', target: 'humidifier', type: 'control', active: true },
    { source: 'gateway', target: 'alarm', type: 'alarm', active: true },
    { source: 'gateway', target: 'spray', type: 'control', active: true },
    // 报警联动
    { source: 'smoke', target: 'alarm', type: 'alarm', active: true },
    { source: 'smoke', target: 'spray', type: 'control', active: true },
    { source: 'infrared', target: 'alarm', type: 'alarm', active: true }
  ]
})

let animationId: number | null = null

function toggleType(type: string) {
  const index = activeTypes.value.indexOf(type)
  if (index === -1) {
    activeTypes.value.push(type)
  } else {
    activeTypes.value.splice(index, 1)
  }
}

function toggleConnection(conn: string) {
  const index = activeConnections.value.indexOf(conn)
  if (index === -1) {
    activeConnections.value.push(conn)
  } else {
    activeConnections.value.splice(index, 1)
  }
}

function handleMouseMove(event: MouseEvent) {
  if (!canvasRef.value) return

  const rect = canvasRef.value.getBoundingClientRect()
  const mouseX = event.clientX - rect.left
  const mouseY = event.clientY - rect.top

  // 查找鼠标位置的节点
  let foundNode = null
  for (const node of graphData.nodes) {
    const dx = mouseX - node.x
    const dy = mouseY - node.y
    const distance = Math.sqrt(dx * dx + dy * dy)

    if (distance < 30) {
      foundNode = node
      break
    }
  }

  if (foundNode) {
    hoveredNode.value = foundNode
    tooltipPos.x = event.clientX - rect.left + 15
    tooltipPos.y = event.clientY - rect.top - 10
  } else {
    hoveredNode.value = null
  }
}

function resetGraph() {
  // 重置节点位置到初始值
  const initialPositions: Record<string, { x: number; y: number }> = {
    'cloud': { x: 400, y: 200 },
    'smoke': { x: 200, y: 100 },
    'water': { x: 600, y: 100 },
    'infrared': { x: 200, y: 300 },
    'aircon': { x: 600, y: 300 },
    'gateway': { x: 400, y: 400 }
  }

  graphData.nodes.forEach(node => {
    const pos = initialPositions[node.id]
    if (pos) {
      node.x = pos.x
      node.y = pos.y
    }
  })

  ElMessage.success('图谱布局已重置')
}

function toggleLabels() {
  showLabels.value = !showLabels.value
}

function exportGraph() {
  // 导出图谱为图片
  if (!graphCanvasRef.value) return

  const canvas = graphCanvasRef.value
  const link = document.createElement('a')
  link.download = 'device-graph.png'
  link.href = canvas.toDataURL('image/png')
  link.click()

  ElMessage.success('图谱已导出')
}

// 绘制图谱
function drawGraph() {
  if (!graphCanvasRef.value || !canvasRef.value) return

  const canvas = graphCanvasRef.value
  const ctx = canvas.getContext('2d')
  if (!ctx) return

  const rect = canvasRef.value.getBoundingClientRect()
  canvas.width = rect.width
  canvas.height = rect.height

  function animate() {
    if (!ctx) return

    ctx.clearRect(0, 0, canvas.width, canvas.height)

    // 绘制边
    graphData.edges.forEach(edge => {
      if (!activeConnections.value.includes(edge.type)) return

      const source = graphData.nodes.find(n => n.id === edge.source)
      const target = graphData.nodes.find(n => n.id === edge.target)
      if (!source || !target) return

      const connType = connectionTypes.find(c => c.value === edge.type)

      ctx.beginPath()
      ctx.moveTo(source.x, source.y)
      ctx.lineTo(target.x, target.y)
      ctx.strokeStyle = connType?.color || '#475569'
      ctx.lineWidth = edge.active ? 2 : 1
      ctx.globalAlpha = edge.active ? 0.6 : 0.3
      ctx.stroke()

      // 流动粒子
      if (edge.active) {
        const t = (Date.now() % 2000) / 2000
        const px = source.x + (target.x - source.x) * t
        const py = source.y + (target.y - source.y) * t

        ctx.beginPath()
        ctx.arc(px, py, 4, 0, Math.PI * 2)
        ctx.fillStyle = connType?.color || '#3b82f6'
        ctx.globalAlpha = 0.8
        ctx.fill()
      }
    })

    ctx.globalAlpha = 1

    // 绘制节点
    graphData.nodes.forEach(node => {
      if (!activeTypes.value.includes(node.type)) return

      // 外圈发光
      if (node.status === 'online') {
        const gradient = ctx.createRadialGradient(node.x, node.y, 20, node.x, node.y, 40)
        gradient.addColorStop(0, node.color + '40')
        gradient.addColorStop(1, 'transparent')
        ctx.beginPath()
        ctx.arc(node.x, node.y, 40, 0, Math.PI * 2)
        ctx.fillStyle = gradient
        ctx.fill()
      }

      // 节点主体
      ctx.beginPath()
      ctx.arc(node.x, node.y, 24, 0, Math.PI * 2)
      ctx.fillStyle = node.status === 'online' ? node.color + '30' : '#1e293b'
      ctx.strokeStyle = node.status === 'online' ? node.color : '#475569'
      ctx.lineWidth = 2
      ctx.fill()
      ctx.stroke()

      // 节点图标
      ctx.font = '20px serif'
      ctx.textAlign = 'center'
      ctx.textBaseline = 'middle'
      ctx.fillStyle = node.status === 'online' ? '#fff' : '#64748b'
      ctx.fillText(node.icon, node.x, node.y)

      // 标签
      if (showLabels.value) {
        ctx.font = '12px sans-serif'
        ctx.fillStyle = node.status === 'online' ? node.color : '#64748b'
        ctx.fillText(node.name, node.x, node.y + 35)
      }
    })

    animationId = requestAnimationFrame(animate)
  }

  animate()
}

onMounted(() => {
  drawGraph()
})

onUnmounted(() => {
  if (animationId) {
    cancelAnimationFrame(animationId)
  }
})
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
.graph-canvas { flex: 1; height: 600px; background: var(--bg-card); border: 1px solid var(--border-primary); border-radius: var(--radius-lg); position: relative; overflow: hidden; }
.graph-canvas canvas { width: 100%; height: 100%; }

.node-tooltip { position: absolute; background: var(--bg-card); border: 1px solid var(--border-primary); border-radius: var(--radius-md); padding: var(--spacing-md); min-width: 180px; box-shadow: var(--shadow-lg); pointer-events: none; z-index: 10; }
.tooltip-header { display: flex; align-items: center; gap: var(--spacing-sm); padding-bottom: var(--spacing-sm); border-bottom: 1px solid var(--border-primary); margin-bottom: var(--spacing-sm); }
.tooltip-icon { font-size: 20px; }
.tooltip-title { font-size: 14px; font-weight: 600; color: var(--text-primary); }
.tooltip-stat { display: flex; justify-content: space-between; align-items: center; padding: 4px 0; }
.stat-label { font-size: 12px; color: var(--text-secondary); }
.stat-value { font-size: 13px; font-weight: 500; color: var(--text-primary); }
.stat-value.online { color: #22c55d; }
.stat-value.offline { color: #ef4444; }

.graph-legend { width: 200px; background: var(--bg-card); border: 1px solid var(--border-primary); border-radius: var(--radius-lg); padding: var(--spacing-md); }
.legend-title { font-size: 12px; font-weight: 600; color: var(--text-secondary); text-transform: uppercase; letter-spacing: 0.5px; margin-bottom: var(--spacing-sm); }
.legend-items { display: flex; flex-direction: column; gap: var(--spacing-sm); margin-bottom: var(--spacing-lg); }
.legend-item { display: flex; align-items: center; gap: 8px; }
.legend-dot { width: 10px; height: 10px; border-radius: 50%; }
.legend-line { width: 20px; height: 0; border-top: 2px solid; }
.legend-label { font-size: 12px; color: var(--text-secondary); }
.legend-connections { display: flex; flex-direction: column; gap: var(--spacing-sm); }

@media (max-width: 1200px) { .graph-canvas-section { flex-direction: column; } .graph-legend { width: 100%; } }
@media (max-width: 768px) { .graph-main { padding: var(--spacing-md); } .filter-bar { flex-direction: column; gap: var(--spacing-sm); } }
</style>
