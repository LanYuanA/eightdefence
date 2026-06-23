<template>
  <div class="flow-designer">
    <!-- 工具栏 -->
    <div class="toolbar">
      <div class="toolbar-left">
        <CyberButton variant="ghost" size="sm" @click="saveWorkflow">
          <svg width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M19 21H5a2 2 0 0 1-2-2V5a2 2 0 0 1 2-2h11l5 5v11a2 2 0 0 1-2 2z"/><polyline points="17 21 17 13 7 13 7 21"/><polyline points="7 3 7 8 15 8"/></svg>
          保存
        </CyberButton>
        <CyberButton variant="ghost" size="sm" @click="loadWorkflow">
          <svg width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M21 15v4a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2v-4"/><polyline points="7 10 12 15 17 10"/><line x1="12" y1="15" x2="12" y2="3"/></svg>
          加载
        </CyberButton>
        <CyberButton variant="ghost" size="sm" @click="clearCanvas">
          <svg width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><polyline points="3 6 5 6 21 6"/><path d="M19 6v14a2 2 0 0 1-2 2H7a2 2 0 0 1-2-2V6m3 0V4a2 2 0 0 1 2-2h4a2 2 0 0 1 2 2v2"/></svg>
          清空
        </CyberButton>
      </div>

      <div class="toolbar-center">
        <span class="workflow-name">{{ workflowName }}</span>
      </div>

      <div class="toolbar-right">
        <CyberButton variant="primary" size="sm" @click="runWorkflow">
          <svg width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><polygon points="5 3 19 12 5 21 5 3"/></svg>
          运行
        </CyberButton>
        <CyberButton variant="ghost" size="sm" @click="exportWorkflow">
          <svg width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M21 15v4a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2v-4"/><polyline points="17 8 12 3 7 8"/><line x1="12" y1="3" x2="12" y2="15"/></svg>
          导出
        </CyberButton>
      </div>
    </div>

    <!-- 主内容区 -->
    <div class="designer-content">
      <!-- 节点面板 -->
      <NodePalette />

      <!-- 流程画布 -->
      <FlowCanvas
        :initial-nodes="nodes"
        :initial-edges="edges"
        @node-click="handleNodeClick"
        @edge-click="handleEdgeClick"
        @pane-click="handlePaneClick"
        @update-nodes="handleUpdateNodes"
        @update-edges="handleUpdateEdges"
      />

      <!-- 节点配置面板 -->
      <Transition name="slide-right">
        <NodeConfig
          v-if="selectedNode"
          :node="selectedNode"
          @close="selectedNode = null"
          @save="handleSaveNode"
          @delete="handleDeleteNode"
        />
      </Transition>
    </div>
  </div>
</template>

<script setup lang="ts">
import { ref } from 'vue'
import { ElMessage } from 'element-plus'
import CyberButton from '../CyberButton.vue'
import NodePalette from './NodePalette.vue'
import FlowCanvas from './FlowCanvas.vue'
import NodeConfig from './NodeConfig.vue'

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
  workflowId?: string
  initialName?: string
  initialNodes?: FlowNode[]
  initialEdges?: FlowEdge[]
}>(), {
  workflowId: '',
  initialName: '新建流程',
  initialNodes: () => [],
  initialEdges: () => []
})

const emit = defineEmits<{
  save: [workflow: { id: string; name: string; nodes: FlowNode[]; edges: FlowEdge[] }]
  run: [workflowId: string]
}>()

const workflowName = ref(props.initialName)
const nodes = ref<FlowNode[]>(props.initialNodes)
const edges = ref<FlowEdge[]>(props.initialEdges)
const selectedNode = ref<FlowNode | null>(null)

// 节点点击
function handleNodeClick(node: FlowNode) {
  selectedNode.value = node
}

// 边点击
function handleEdgeClick(edge: FlowEdge) {
  console.log('Edge clicked:', edge)
}

// 画布点击
function handlePaneClick() {
  selectedNode.value = null
}

// 更新节点
function handleUpdateNodes(updatedNodes: FlowNode[]) {
  nodes.value = updatedNodes
}

// 更新边
function handleUpdateEdges(updatedEdges: FlowEdge[]) {
  edges.value = updatedEdges
}

// 保存节点配置
function handleSaveNode(node: FlowNode) {
  const index = nodes.value.findIndex(n => n.id === node.id)
  if (index !== -1) {
    nodes.value[index] = { ...node }
  }
  ElMessage.success('节点配置已保存')
}

// 删除节点
function handleDeleteNode(nodeId: string) {
  nodes.value = nodes.value.filter(n => n.id !== nodeId)
  edges.value = edges.value.filter(e => e.source !== nodeId && e.target !== nodeId)
  selectedNode.value = null
  ElMessage.success('节点已删除')
}

// 保存流程
function saveWorkflow() {
  const workflow = {
    id: props.workflowId || `workflow_${Date.now()}`,
    name: workflowName.value,
    nodes: nodes.value,
    edges: edges.value
  }
  emit('save', workflow)
  ElMessage.success('流程已保存')
}

// 加载流程
function loadWorkflow() {
  ElMessage.info('加载流程功能开发中...')
}

// 清空画布
function clearCanvas() {
  nodes.value = []
  edges.value = []
  selectedNode.value = null
  ElMessage.success('画布已清空')
}

// 运行流程
function runWorkflow() {
  if (props.workflowId) {
    emit('run', props.workflowId)
  } else {
    ElMessage.warning('请先保存流程')
  }
}

// 导出流程
function exportWorkflow() {
  const workflow = {
    name: workflowName.value,
    nodes: nodes.value,
    edges: edges.value
  }
  const blob = new Blob([JSON.stringify(workflow, null, 2)], { type: 'application/json' })
  const url = URL.createObjectURL(blob)
  const a = document.createElement('a')
  a.href = url
  a.download = `${workflowName.value}.json`
  a.click()
  URL.revokeObjectURL(url)
  ElMessage.success('流程已导出')
}
</script>

<style scoped>
.flow-designer {
  display: flex;
  flex-direction: column;
  height: 100%;
  background: var(--bg-primary);
}

.toolbar {
  display: flex;
  align-items: center;
  justify-content: space-between;
  padding: var(--spacing-sm) var(--spacing-md);
  background: var(--bg-card);
  border-bottom: 1px solid var(--border-primary);
}

.toolbar-left,
.toolbar-right {
  display: flex;
  gap: var(--spacing-xs);
}

.toolbar-center {
  flex: 1;
  text-align: center;
}

.workflow-name {
  font-size: 14px;
  font-weight: 600;
  color: var(--text-primary);
}

.designer-content {
  flex: 1;
  display: flex;
  overflow: hidden;
}
</style>
