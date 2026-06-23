<template>
  <div class="data-flow-container" ref="containerRef">
    <svg class="flow-svg" :width="width" :height="height">
      <!-- 定义渐变和滤镜 -->
      <defs>
        <linearGradient :id="`flowGradient-${uid}`" x1="0%" y1="0%" x2="100%" y2="0%">
          <stop offset="0%" :stop-color="startColor" stop-opacity="0" />
          <stop offset="50%" :stop-color="flowColor" stop-opacity="1" />
          <stop offset="100%" :stop-color="endColor" stop-opacity="0" />
        </linearGradient>

        <filter :id="`glow-${uid}`">
          <feGaussianBlur stdDeviation="3" result="blur" />
          <feComposite in="SourceGraphic" in2="blur" operator="over" />
        </filter>
      </defs>

      <!-- 连接线 -->
      <g v-for="(flow, index) in computedFlows" :key="index">
        <!-- 背景线 -->
        <path
          v-if="flow.path"
          :d="flow.path"
          fill="none"
          :stroke="flowColor"
          stroke-width="1"
          stroke-opacity="0.1"
        />

        <!-- 流动线 -->
        <path
          v-if="flow.path"
          :d="flow.path"
          fill="none"
          :stroke="`url(#flowGradient-${uid})`"
          :stroke-width="flow.active ? 2 : 1"
          :stroke-opacity="flow.active ? 0.8 : 0.3"
          :filter="flow.active ? `url(#glow-${uid})` : ''"
          class="flow-line"
          :style="{ animationDuration: `${flow.speed}s` }"
        />

        <!-- 数据包 -->
        <circle
          v-for="(packet, pIdx) in flow.packets"
          :key="pIdx"
          :cx="packet.x"
          :cy="packet.y"
          :r="packet.size"
          :fill="flowColor"
          :opacity="packet.opacity"
          class="data-packet"
        />
      </g>

      <!-- 节点 -->
      <g v-for="(node, index) in nodes" :key="`node-${index}`">
        <!-- 节点外圈 -->
        <circle
          :cx="node.x"
          :cy="node.y"
          :r="node.size + 8"
          fill="none"
          :stroke="node.color"
          stroke-width="2"
          stroke-opacity="0.3"
          class="node-ring"
          :class="{ 'node-active': node.active }"
        />

        <!-- 节点内圈 -->
        <circle
          :cx="node.x"
          :cy="node.y"
          :r="node.size"
          :fill="node.color"
          fill-opacity="0.2"
          :stroke="node.color"
          stroke-width="2"
        />

        <!-- 节点图标 -->
        <text
          :x="node.x"
          :y="node.y"
          text-anchor="middle"
          dominant-baseline="central"
          :fill="node.color"
          font-size="16"
          v-html="node.icon"
        />

        <!-- 节点标签 -->
        <text
          :x="node.x"
          :y="node.y + node.size + 20"
          text-anchor="middle"
          :fill="node.color"
          font-size="12"
          font-weight="500"
        >
          {{ node.label }}
        </text>

        <!-- 数据量指示器 -->
        <text
          v-if="node.dataCount !== undefined"
          :x="node.x"
          :y="node.y - node.size - 10"
          text-anchor="middle"
          :fill="node.color"
          font-size="10"
          opacity="0.8"
        >
          {{ node.dataCount }} 条/s
        </text>
      </g>
    </svg>

    <!-- 控制面板 -->
    <div class="flow-controls" v-if="showControls">
      <button
        v-for="control in controlButtons"
        :key="control.id"
        class="flow-control-btn"
        :class="{ active: activeFlow === control.id }"
        @click="toggleFlow(control.id)"
      >
        <span class="control-dot" :style="{ background: control.color }" />
        {{ control.label }}
      </button>
    </div>
  </div>
</template>

<script setup lang="ts">
import { ref, onMounted, onUnmounted, computed } from 'vue'

interface FlowNode {
  id: string
  x: number
  y: number
  size: number
  color: string
  icon: string
  label: string
  active?: boolean
  dataCount?: number
}

interface DataFlow {
  from: string
  to: string
  path?: string
  active: boolean
  speed: number
  packets?: Array<{
    x: number
    y: number
    size: number
    opacity: number
    progress: number
  }>
}

const props = withDefaults(defineProps<{
  nodes?: FlowNode[]
  flows?: DataFlow[]
  flowColor?: string
  startColor?: string
  endColor?: string
  showControls?: boolean
  autoAnimate?: boolean
}>(), {
  nodes: () => [
    { id: 'device', x: 100, y: 200, size: 24, color: '#3b82f6', icon: '📡', label: '传感器', active: true, dataCount: 128 },
    { id: 'gateway', x: 350, y: 150, size: 28, color: '#8b5cf6', icon: '🔌', label: '网关', active: true, dataCount: 64 },
    { id: 'server', x: 600, y: 200, size: 32, color: '#22c55d', icon: '🖥️', label: '服务器', active: true, dataCount: 32 },
    { id: 'database', x: 850, y: 150, size: 24, color: '#f59e0b', icon: '💾', label: '数据库', active: true, dataCount: 16 },
    { id: 'ui', x: 600, y: 350, size: 28, color: '#ec4899', icon: '📊', label: '界面', active: true, dataCount: 8 }
  ],
  flows: () => [],
  flowColor: '#3b82f6',
  startColor: '#1e40af',
  endColor: '#60a5fa',
  showControls: true,
  autoAnimate: true
})

const width = ref(1000)
const height = ref(500)
const containerRef = ref<HTMLElement | null>(null)
const activeFlow = ref<string | null>(null)
const uid = ref(Math.random().toString(36).substr(2, 9))

// 控制按钮
const controlButtons = computed(() => [
  { id: 'sensor-data', label: '传感器数据流', color: '#3b82f6' },
  { id: 'alarm-data', label: '告警数据流', color: '#ef4444' },
  { id: 'control-data', label: '控制指令流', color: '#22c55d' },
  { id: 'log-data', label: '日志数据流', color: '#f59e0b' }
])

// 切换数据流显示
function toggleFlow(flowId: string) {
  activeFlow.value = activeFlow.value === flowId ? null : flowId
}

// 计算两个节点之间的路径
function computePath(fromId: string, toId: string): string {
  const fromNode = props.nodes.find(n => n.id === fromId)
  const toNode = props.nodes.find(n => n.id === toId)
  if (!fromNode || !toNode) return ''

  const x1 = fromNode.x
  const y1 = fromNode.y
  const x2 = toNode.x
  const y2 = toNode.y

  // 贝塞尔曲线
  const midX = (x1 + x2) / 2
  return `M${x1},${y1} C${midX},${y1} ${midX},${y2} ${x2},${y2}`
}

// 获取带路径的flows
const computedFlows = computed(() => {
  return props.flows.map(flow => ({
    ...flow,
    path: flow.path || computePath(flow.from, flow.to)
  }))
})

// 动画帧
let animationId: number | null = null
let packets: Array<{
  flowIndex: number
  progress: number
  x: number
  y: number
  size: number
  opacity: number
}> = []

onMounted(() => {
  if (!containerRef.value) return

  const rect = containerRef.value.getBoundingClientRect()
  width.value = rect.width || 1000
  height.value = rect.height || 500

  if (props.autoAnimate) {
    startAnimation()
  }
})

function startAnimation() {
  function animate() {
    // 更新数据包位置
    packets.forEach(packet => {
      packet.progress += 0.01
      if (packet.progress > 1) {
        packet.progress = 0
        packet.opacity = 1
      }

      // 计算位置（贝塞尔曲线插值）
      const flow = computedFlows.value[packet.flowIndex]
      if (flow) {
        // 简单线性插值
        const fromNode = props.nodes.find(n => n.id === flow.from)
        const toNode = props.nodes.find(n => n.id === flow.to)
        if (fromNode && toNode) {
          packet.x = fromNode.x + (toNode.x - fromNode.x) * packet.progress
          packet.y = fromNode.y + (toNode.y - fromNode.y) * packet.progress
          packet.opacity = Math.sin(packet.progress * Math.PI)
        }
      }
    })

    animationId = requestAnimationFrame(animate)
  }

  animate()
}

onUnmounted(() => {
  if (animationId) {
    cancelAnimationFrame(animationId)
  }
})
</script>

<style scoped>
.data-flow-container {
  position: relative;
  width: 100%;
  height: 100%;
  min-height: 400px;
  background: radial-gradient(circle at center, rgba(59, 130, 246, 0.05), transparent 70%);
  border-radius: var(--radius-lg);
  overflow: hidden;
}

.flow-svg {
  width: 100%;
  height: 100%;
}

.flow-line {
  animation: flow-dash linear infinite;
  stroke-dasharray: 10 5;
}

.data-packet {
  animation: pulse 1s ease-in-out infinite;
}

.node-ring {
  animation: breathe 3s ease-in-out infinite;
}

.node-active .node-ring {
  animation: pulse-ring 2s ease-in-out infinite;
}

/* 控制面板 */
.flow-controls {
  position: absolute;
  bottom: var(--spacing-md);
  left: 50%;
  transform: translateX(-50%);
  display: flex;
  gap: var(--spacing-sm);
  padding: var(--spacing-sm);
  background: rgba(15, 23, 42, 0.8);
  backdrop-filter: blur(8px);
  border: 1px solid var(--border-primary);
  border-radius: var(--radius-full);
}

.flow-control-btn {
  display: flex;
  align-items: center;
  gap: 6px;
  padding: 6px 12px;
  background: transparent;
  border: 1px solid transparent;
  border-radius: var(--radius-full);
  color: var(--text-secondary);
  font-size: 12px;
  cursor: pointer;
  transition: all var(--duration-fast) var(--ease-default);
}

.flow-control-btn:hover {
  background: rgba(255, 255, 255, 0.05);
  color: var(--text-primary);
}

.flow-control-btn.active {
  background: rgba(59, 130, 246, 0.2);
  border-color: var(--accent-primary);
  color: var(--accent-primary);
}

.control-dot {
  width: 8px;
  height: 8px;
  border-radius: 50%;
}

@keyframes flow-dash {
  from {
    stroke-dashoffset: 30;
  }
  to {
    stroke-dashoffset: 0;
  }
}
</style>
