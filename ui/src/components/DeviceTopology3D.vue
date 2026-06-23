<template>
  <div class="topology-container" ref="containerRef">
    <canvas ref="canvasRef" class="topology-canvas" />

    <!-- 悬浮信息卡 -->
    <Transition name="fade">
      <div
        v-if="hoveredNode"
        class="node-tooltip"
        :style="{ left: tooltipPos.x + 'px', top: tooltipPos.y + 'px' }"
      >
        <div class="tooltip-header" :style="{ borderColor: hoveredNode.color }">
          <span class="tooltip-icon">{{ hoveredNode.icon }}</span>
          <span class="tooltip-title">{{ hoveredNode.label }}</span>
        </div>
        <div class="tooltip-body">
          <div class="tooltip-stat">
            <span class="stat-label">状态</span>
            <span class="stat-value" :class="{ 'status-online': hoveredNode.active }">
              {{ hoveredNode.active ? '在线' : '离线' }}
            </span>
          </div>
          <div class="tooltip-stat">
            <span class="stat-label">数据量</span>
            <span class="stat-value">{{ hoveredNode.dataCount || 0 }} 条/s</span>
          </div>
          <div class="tooltip-stat">
            <span class="stat-label">类型</span>
            <span class="stat-value">{{ hoveredNode.type || '传感器' }}</span>
          </div>
        </div>
      </div>
    </Transition>

    <!-- 控制按钮 -->
    <div class="topology-controls">
      <button class="ctrl-btn" @click="resetView" title="重置视图">
        <svg width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
          <path d="M3 12a9 9 0 1 0 9-9 9.75 9.75 0 0 0-6.74 2.74L3 8"/>
          <path d="M3 3v5h5"/>
        </svg>
      </button>
      <button class="ctrl-btn" @click="toggleAutoRotate" :class="{ active: autoRotate }" title="自动旋转">
        <svg width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
          <path d="M21.5 2v6h-6M2.5 22v-6h6M2 11.5a10 10 0 0 1 18.8-4.3M22 12.5a10 10 0 0 1-18.8 4.3"/>
        </svg>
      </button>
      <button class="ctrl-btn" @click="toggleLabels" :class="{ active: showLabels }" title="显示标签">
        <svg width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
          <path d="M4 7V4h16v3"/>
          <path d="M9 20h6"/>
          <path d="M12 4v16"/>
        </svg>
      </button>
    </div>
  </div>
</template>

<script setup lang="ts">
import { ref, reactive, onMounted, onUnmounted } from 'vue'

interface TopologyNode {
  id: string
  x: number
  y: number
  z: number
  size: number
  color: string
  icon: string
  label: string
  type?: string
  active?: boolean
  dataCount?: number
}

interface TopologyEdge {
  from: string
  to: string
  active?: boolean
  color?: string
}

const props = withDefaults(defineProps<{
  nodes?: TopologyNode[]
  edges?: TopologyEdge[]
  autoRotate?: boolean
  showLabels?: boolean
}>(), {
  nodes: () => [
    { id: 'cloud', x: 0, y: 0, z: 0, size: 30, color: '#3b82f6', icon: '☁️', label: '云测仪', type: '传感器', active: true, dataCount: 128 },
    { id: 'smoke', x: -150, y: -100, z: 50, size: 20, color: '#ef4444', icon: '🔥', label: '烟雾报警', type: '报警器', active: true, dataCount: 64 },
    { id: 'water', x: 150, y: -100, z: -50, size: 20, color: '#06b6d4', icon: '💧', label: '水浸传感器', type: '传感器', active: true, dataCount: 32 },
    { id: 'infrared', x: -100, y: 100, z: 80, size: 22, color: '#8b5cf6', icon: '👤', label: '红外探测', type: '探测器', active: true, dataCount: 48 },
    { id: 'light', x: 100, y: 100, z: -80, size: 18, color: '#f59e0b', icon: '💡', label: '光感传感器', type: '传感器', active: false, dataCount: 0 },
    { id: 'aircon', x: 0, y: -150, z: 100, size: 24, color: '#22c55d', icon: '❄️', label: '空调控制', type: '控制器', active: true, dataCount: 16 },
    { id: 'gateway', x: 0, y: 0, z: 150, size: 35, color: '#ec4899', icon: '🔌', label: '网关', type: '网关', active: true, dataCount: 256 }
  ],
  edges: () => [
    { from: 'cloud', to: 'gateway', active: true },
    { from: 'smoke', to: 'gateway', active: true },
    { from: 'water', to: 'gateway', active: true },
    { from: 'infrared', to: 'gateway', active: true },
    { from: 'light', to: 'gateway', active: false },
    { from: 'aircon', to: 'gateway', active: true }
  ],
  autoRotate: true,
  showLabels: true
})

const containerRef = ref<HTMLElement | null>(null)
const canvasRef = ref<HTMLCanvasElement | null>(null)
const hoveredNode = ref<TopologyNode | null>(null)
const tooltipPos = reactive({ x: 0, y: 0 })
const autoRotate = ref(props.autoRotate)
const showLabels = ref(props.showLabels)

let animationId: number | null = null
let rotationX = 0
let rotationY = 0
let isDragging = false
let lastMouseX = 0
let lastMouseY = 0

// 3D投影
function project3D(x: number, y: number, z: number, cx: number, cy: number) {
  const perspective = 800
  const rotX = rotationX * Math.PI / 180
  const rotY = rotationY * Math.PI / 180

  // 绕Y轴旋转
  const x1 = x * Math.cos(rotY) - z * Math.sin(rotY)
  const z1 = x * Math.sin(rotY) + z * Math.cos(rotY)

  // 绕X轴旋转
  const y1 = y * Math.cos(rotX) - z1 * Math.sin(rotX)
  const z2 = y * Math.sin(rotX) + z1 * Math.cos(rotX)

  // 透视投影
  const scale = perspective / (perspective + z2)
  return {
    x: cx + x1 * scale,
    y: cy + y1 * scale,
    scale
  }
}

onMounted(() => {
  if (!canvasRef.value || !containerRef.value) return

  const canvas = canvasRef.value
  const ctx = canvas.getContext('2d')
  if (!ctx) return

  // 设置画布大小
  function resize() {
    if (!containerRef.value) return
    const rect = containerRef.value.getBoundingClientRect()
    canvas.width = rect.width
    canvas.height = rect.height
  }
  resize()
  window.addEventListener('resize', resize)

  // 鼠标事件
  canvas.addEventListener('mousedown', (e) => {
    isDragging = true
    lastMouseX = e.clientX
    lastMouseY = e.clientY
  })

  canvas.addEventListener('mousemove', (e) => {
    if (isDragging) {
      rotationY += (e.clientX - lastMouseX) * 0.5
      rotationX += (e.clientY - lastMouseY) * 0.5
      lastMouseX = e.clientX
      lastMouseY = e.clientY
    }

    // 检测悬浮节点
    const rect = canvas.getBoundingClientRect()
    const mouseX = e.clientX - rect.left
    const mouseY = e.clientY - rect.top
    const cx = canvas.width / 2
    const cy = canvas.height / 2

    let found = false
    for (const node of props.nodes) {
      const pos = project3D(node.x, node.y, node.z, cx, cy)
      const dist = Math.sqrt((mouseX - pos.x) ** 2 + (mouseY - pos.y) ** 2)
      if (dist < node.size * pos.scale + 10) {
        hoveredNode.value = node
        tooltipPos.x = e.clientX - rect.left + 15
        tooltipPos.y = e.clientY - rect.top - 10
        found = true
        break
      }
    }
    if (!found) {
      hoveredNode.value = null
    }
  })

  canvas.addEventListener('mouseup', () => {
    isDragging = false
  })

  canvas.addEventListener('mouseleave', () => {
    isDragging = false
    hoveredNode.value = null
  })

  // 动画循环
  function animate() {
    if (!ctx) return

    ctx.clearRect(0, 0, canvas.width, canvas.height)

    const cx = canvas.width / 2
    const cy = canvas.height / 2

    if (autoRotate.value && !isDragging) {
      rotationY += 0.2
    }

    // 绘制边
    props.edges.forEach(edge => {
      const fromNode = props.nodes.find(n => n.id === edge.from)
      const toNode = props.nodes.find(n => n.id === edge.to)
      if (!fromNode || !toNode) return

      const from = project3D(fromNode.x, fromNode.y, fromNode.z, cx, cy)
      const to = project3D(toNode.x, toNode.y, toNode.z, cx, cy)

      ctx.beginPath()
      ctx.moveTo(from.x, from.y)
      ctx.lineTo(to.x, to.y)
      ctx.strokeStyle = edge.active ? (edge.color || '#3b82f6') : '#334155'
      ctx.lineWidth = edge.active ? 2 : 1
      ctx.globalAlpha = edge.active ? 0.6 : 0.3
      ctx.stroke()

      // 流动粒子
      if (edge.active) {
        const t = (Date.now() % 2000) / 2000
        const px = from.x + (to.x - from.x) * t
        const py = from.y + (to.y - from.y) * t

        ctx.beginPath()
        ctx.arc(px, py, 3, 0, Math.PI * 2)
        ctx.fillStyle = edge.color || '#3b82f6'
        ctx.globalAlpha = 0.8
        ctx.fill()
      }
    })

    ctx.globalAlpha = 1

    // 绘制节点
    props.nodes.forEach(node => {
      const pos = project3D(node.x, node.y, node.z, cx, cy)
      const size = node.size * pos.scale

      // 外圈发光
      if (node.active) {
        const gradient = ctx.createRadialGradient(pos.x, pos.y, size, pos.x, pos.y, size * 2)
        gradient.addColorStop(0, node.color + '40')
        gradient.addColorStop(1, 'transparent')
        ctx.beginPath()
        ctx.arc(pos.x, pos.y, size * 2, 0, Math.PI * 2)
        ctx.fillStyle = gradient
        ctx.fill()
      }

      // 节点主体
      ctx.beginPath()
      ctx.arc(pos.x, pos.y, size, 0, Math.PI * 2)
      ctx.fillStyle = node.active ? node.color + '30' : '#1e293b'
      ctx.strokeStyle = node.active ? node.color : '#475569'
      ctx.lineWidth = 2
      ctx.fill()
      ctx.stroke()

      // 节点图标
      ctx.font = `${size}px serif`
      ctx.textAlign = 'center'
      ctx.textBaseline = 'middle'
      ctx.fillStyle = node.active ? '#fff' : '#64748b'
      ctx.fillText(node.icon, pos.x, pos.y)

      // 标签
      if (showLabels.value) {
        ctx.font = '12px sans-serif'
        ctx.fillStyle = node.active ? node.color : '#64748b'
        ctx.fillText(node.label, pos.x, pos.y + size + 15)
      }
    })

    animationId = requestAnimationFrame(animate)
  }

  animate()
})

function resetView() {
  rotationX = 0
  rotationY = 0
}

function toggleAutoRotate() {
  autoRotate.value = !autoRotate.value
}

function toggleLabels() {
  showLabels.value = !showLabels.value
}

onUnmounted(() => {
  if (animationId) {
    cancelAnimationFrame(animationId)
  }
})
</script>

<style scoped>
.topology-container {
  position: relative;
  width: 100%;
  height: 100%;
  min-height: 400px;
  background: radial-gradient(circle at center, rgba(139, 92, 246, 0.05), transparent 70%);
  border-radius: var(--radius-lg);
  overflow: hidden;
}

.topology-canvas {
  width: 100%;
  height: 100%;
  cursor: grab;
}

.topology-canvas:active {
  cursor: grabbing;
}

/* 悬浮提示 */
.node-tooltip {
  position: absolute;
  background: var(--bg-card);
  border: 1px solid var(--border-primary);
  border-radius: var(--radius-md);
  padding: var(--spacing-md);
  min-width: 180px;
  box-shadow: var(--shadow-lg);
  pointer-events: none;
  z-index: 10;
}

.tooltip-header {
  display: flex;
  align-items: center;
  gap: var(--spacing-sm);
  padding-bottom: var(--spacing-sm);
  border-bottom: 1px solid var(--border-primary);
  margin-bottom: var(--spacing-sm);
}

.tooltip-icon {
  font-size: 20px;
}

.tooltip-title {
  font-size: 14px;
  font-weight: 600;
  color: var(--text-primary);
}

.tooltip-stat {
  display: flex;
  justify-content: space-between;
  align-items: center;
  padding: 4px 0;
}

.stat-label {
  font-size: 12px;
  color: var(--text-secondary);
}

.stat-value {
  font-size: 13px;
  font-weight: 500;
  color: var(--text-primary);
}

.stat-value.status-online {
  color: var(--status-success);
}

/* 控制按钮 */
.topology-controls {
  position: absolute;
  top: var(--spacing-md);
  right: var(--spacing-md);
  display: flex;
  flex-direction: column;
  gap: var(--spacing-xs);
}

.ctrl-btn {
  width: 36px;
  height: 36px;
  display: flex;
  align-items: center;
  justify-content: center;
  background: rgba(15, 23, 42, 0.8);
  backdrop-filter: blur(8px);
  border: 1px solid var(--border-primary);
  border-radius: var(--radius-md);
  color: var(--text-secondary);
  cursor: pointer;
  transition: all var(--duration-fast) var(--ease-default);
}

.ctrl-btn:hover {
  background: var(--bg-card);
  color: var(--text-primary);
  border-color: var(--accent-primary);
}

.ctrl-btn.active {
  background: rgba(59, 130, 246, 0.2);
  border-color: var(--accent-primary);
  color: var(--accent-primary);
}
</style>
