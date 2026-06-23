<template>
  <div class="flow-canvas" ref="canvasRef">
    <VueFlow
      v-model:nodes="nodes"
      v-model:edges="edges"
      :default-viewport="{ zoom: 1, x: 0, y: 0 }"
      :min-zoom="0.2"
      :max-zoom="4"
      :snap-to-grid="true"
      :snap-grid="[15, 15]"
      @drop="handleDrop"
      @dragover="handleDragOver"
      @node-click="handleNodeClick"
      @edge-click="handleEdgeClick"
      @pane-click="handlePaneClick"
    >
      <!-- 背景网格 -->
      <Background :gap="20" :size="1" pattern-color="rgba(59, 130, 246, 0.1)" />

      <!-- 控制面板 -->
      <Controls />

      <!-- 小地图 -->
      <MiniMap />

      <!-- 自定义节点 -->
      <template #node-device="nodeProps">
        <DeviceNode v-bind="nodeProps" />
      </template>

      <template #node-service="nodeProps">
        <ServiceNode v-bind="nodeProps" />
      </template>

      <template #node-logic="nodeProps">
        <LogicNode v-bind="nodeProps" />
      </template>

      <template #node-action="nodeProps">
        <ActionNode v-bind="nodeProps" />
      </template>
    </VueFlow>

    <!-- 空状态提示 -->
    <div v-if="nodes.length === 0" class="empty-state">
      <div class="empty-icon">
        <svg width="64" height="64" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1">
          <rect x="3" y="3" width="18" height="18" rx="2"/>
          <path d="M12 8v8m-4-4h8"/>
        </svg>
      </div>
      <div class="empty-text">从左侧面板拖拽节点到画布</div>
      <div class="empty-hint">或点击右键添加节点</div>
    </div>
  </div>
</template>

<script setup lang="ts">
import { ref, watch } from 'vue'
import { VueFlow, useVueFlow } from '@vue-flow/core'
import { Background } from '@vue-flow/background'
import { Controls } from '@vue-flow/controls'
import { MiniMap } from '@vue-flow/minimap'
import DeviceNode from './nodes/DeviceNode.vue'
import ServiceNode from './nodes/ServiceNode.vue'
import LogicNode from './nodes/LogicNode.vue'
import ActionNode from './nodes/ActionNode.vue'

interface FlowNode {
  id: string
  type: string
  position: { x: number; y: number }
  data: any
}

interface FlowEdge {
  id: string
  source: string
  target: string
  sourceHandle?: string
  targetHandle?: string
  animated?: boolean
  style?: any
}

const props = withDefaults(defineProps<{
  initialNodes?: FlowNode[]
  initialEdges?: FlowEdge[]
}>(), {
  initialNodes: () => [],
  initialEdges: () => []
})

const emit = defineEmits<{
  nodeClick: [node: FlowNode]
  edgeClick: [edge: FlowEdge]
  paneClick: []
  updateNodes: [nodes: FlowNode[]]
  updateEdges: [edges: FlowEdge[]]
}>()

const canvasRef = ref<HTMLElement | null>(null)
const nodes = ref<FlowNode[]>(props.initialNodes)
const edges = ref<FlowEdge[]>(props.initialEdges)

const { addNodes, project } = useVueFlow()

// 监听节点和边的变化
watch(nodes, (newNodes) => {
  emit('updateNodes', newNodes)
}, { deep: true })

watch(edges, (newEdges) => {
  emit('updateEdges', newEdges)
}, { deep: true })

// 拖拽进入画布
function handleDragOver(event: DragEvent) {
  event.preventDefault()
  if (event.dataTransfer) {
    event.dataTransfer.dropEffect = 'move'
  }
}

// 拖拽放下
function handleDrop(event: DragEvent) {
  event.preventDefault()

  if (!event.dataTransfer) return

  const nodeData = JSON.parse(event.dataTransfer.getData('application/json'))

  // 获取画布位置
  const canvas = canvasRef.value
  if (!canvas) return

  const rect = canvas.getBoundingClientRect()
  const position = project({
    x: event.clientX - rect.left,
    y: event.clientY - rect.top
  })

  // 创建新节点
  const newNode: FlowNode = {
    id: `${nodeData.type}_${Date.now()}`,
    type: nodeData.category,
    position,
    data: {
      label: nodeData.label,
      icon: nodeData.icon,
      color: nodeData.color,
      inputs: nodeData.inputs,
      outputs: nodeData.outputs,
      config: {}
    }
  }

  addNodes([newNode])
}

// 节点点击
function handleNodeClick(event: any) {
  emit('nodeClick', event.node)
}

// 边点击
function handleEdgeClick(event: any) {
  emit('edgeClick', event.edge)
}

// 画布点击
function handlePaneClick() {
  emit('paneClick')
}
</script>

<style scoped>
.flow-canvas {
  flex: 1;
  height: 100%;
  position: relative;
}

.empty-state {
  position: absolute;
  top: 50%;
  left: 50%;
  transform: translate(-50%, -50%);
  text-align: center;
  pointer-events: none;
}

.empty-icon {
  color: var(--text-tertiary);
  opacity: 0.5;
  margin-bottom: var(--spacing-md);
}

.empty-text {
  font-size: 16px;
  color: var(--text-secondary);
  margin-bottom: var(--spacing-sm);
}

.empty-hint {
  font-size: 13px;
  color: var(--text-tertiary);
}
</style>
