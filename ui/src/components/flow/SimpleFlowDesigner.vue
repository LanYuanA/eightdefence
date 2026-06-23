<template>
  <div class="simple-flow-designer">
    <!-- 工具栏 -->
    <div class="toolbar">
      <div class="toolbar-left">
        <CyberButton variant="ghost" size="sm" @click="clearCanvas">
          <svg width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><polyline points="3 6 5 6 21 6"/><path d="M19 6v14a2 2 0 0 1-2 2H7a2 2 0 0 1-2-2V6m3 0V4a2 2 0 0 1 2-2h4a2 2 0 0 1 2 2v2"/></svg>
          清空
        </CyberButton>
        <CyberButton variant="ghost" size="sm" @click="autoLayout">
          <svg width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><rect x="3" y="3" width="18" height="18" rx="2"/><path d="M3 9h18"/><path d="M3 15h18"/><path d="M9 3v18"/></svg>
          自动布局
        </CyberButton>
        <CyberButton variant="ghost" size="sm" @click="toggleConnectionMode">
          <svg width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M18 13v6a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2V8a2 2 0 0 1 2-2h6"/><polyline points="15 3 21 3 21 9"/><line x1="10" y1="14" x2="21" y2="3"/></svg>
          {{ connectionMode ? '退出连线' : '连线模式' }}
        </CyberButton>
      </div>

      <div class="toolbar-center">
        <input v-model="workflowName" class="workflow-name-input" placeholder="输入流程名称" />
      </div>

      <div class="toolbar-right">
        <CyberButton variant="primary" size="sm" @click="saveWorkflow">
          <svg width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M19 21H5a2 2 0 0 1-2-2V5a2 2 0 0 1 2-2h11l5 5v11a2 2 0 0 1-2 2z"/><polyline points="17 21 17 13 7 13 7 21"/><polyline points="7 3 7 8 15 8"/></svg>
          保存
        </CyberButton>
        <CyberButton variant="success" size="sm" @click="runWorkflow">
          <svg width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><polygon points="5 3 19 12 5 21 5 3"/></svg>
          运行
        </CyberButton>
      </div>
    </div>

    <!-- 主内容区 -->
    <div class="designer-content">
      <!-- 节点面板 -->
      <div class="node-palette">
        <div class="palette-header">
          <h3 class="palette-title">节点面板</h3>
        </div>
        <div class="palette-content">
          <!-- 设备节点 -->
          <div class="node-category">
            <div class="category-header" @click="toggleCategory('device')">
              <span class="category-icon">📡</span>
              <span class="category-title">设备节点</span>
            </div>
            <div v-show="expandedCategories.device" class="category-nodes">
              <div v-for="node in deviceNodes" :key="node.type" class="palette-node" draggable="true" @dragstart="handleDragStart($event, node)">
                <span class="node-icon">{{ node.icon }}</span>
                <span class="node-label">{{ node.label }}</span>
              </div>
            </div>
          </div>

          <!-- 服务节点 -->
          <div class="node-category">
            <div class="category-header" @click="toggleCategory('service')">
              <span class="category-icon">⚙️</span>
              <span class="category-title">服务节点</span>
            </div>
            <div v-show="expandedCategories.service" class="category-nodes">
              <div v-for="node in serviceNodes" :key="node.type" class="palette-node" draggable="true" @dragstart="handleDragStart($event, node)">
                <span class="node-icon">{{ node.icon }}</span>
                <span class="node-label">{{ node.label }}</span>
              </div>
            </div>
          </div>

          <!-- 逻辑节点 -->
          <div class="node-category">
            <div class="category-header" @click="toggleCategory('logic')">
              <span class="category-icon">🔀</span>
              <span class="category-title">逻辑节点</span>
            </div>
            <div v-show="expandedCategories.logic" class="category-nodes">
              <div v-for="node in logicNodes" :key="node.type" class="palette-node" draggable="true" @dragstart="handleDragStart($event, node)">
                <span class="node-icon">{{ node.icon }}</span>
                <span class="node-label">{{ node.label }}</span>
              </div>
            </div>
          </div>

          <!-- 动作节点 -->
          <div class="node-category">
            <div class="category-header" @click="toggleCategory('action')">
              <span class="category-icon">🎯</span>
              <span class="category-title">动作节点</span>
            </div>
            <div v-show="expandedCategories.action" class="category-nodes">
              <div v-for="node in actionNodes" :key="node.type" class="palette-node" draggable="true" @dragstart="handleDragStart($event, node)">
                <span class="node-icon">{{ node.icon }}</span>
                <span class="node-label">{{ node.label }}</span>
              </div>
            </div>
          </div>
        </div>
      </div>

      <!-- 画布区域 -->
      <div
        class="canvas-area"
        ref="canvasRef"
        @drop="handleDrop"
        @dragover="handleDragOver"
        @click="handleCanvasClick"
        @mousemove="handleMouseMove"
      >
        <svg class="connections-svg" width="100%" height="100%">
          <!-- 连接线 -->
          <g v-for="(conn, index) in connections" :key="index">
            <path
              :d="getConnectionPath(conn)"
              fill="none"
              :stroke="conn.active ? '#3b82f6' : '#475569'"
              :stroke-width="conn.active ? 3 : 2"
              :stroke-dasharray="conn.active ? 'none' : '5,5'"
              class="connection-line"
              @click.stop="selectConnection(conn)"
            />
            <!-- 连接箭头 -->
            <polygon
              :points="getArrowPoints(conn)"
              :fill="conn.active ? '#3b82f6' : '#475569'"
            />
          </g>

          <!-- 正在创建的连接 -->
          <line
            v-if="connectionStart"
            :x1="connectionStart.x"
            :y1="connectionStart.y"
            :x2="mousePosition.x"
            :y2="mousePosition.y"
            stroke="#3b82f6"
            stroke-width="2"
            stroke-dasharray="5,5"
          />
        </svg>

        <!-- 画布节点 -->
        <div
          v-for="node in canvasNodes"
          :key="node.id"
          class="canvas-node"
          :class="[`node-${node.type}`, { selected: selectedNode?.id === node.id, connecting: connectionStart?.id === node.id }]"
          :style="{ left: node.x + 'px', top: node.y + 'px' }"
          @mousedown.stop="startDragNode($event, node)"
          @click.stop="handleNodeClick(node)"
        >
          <div class="node-icon-wrapper" :style="{ background: node.color + '20', color: node.color }">
            {{ node.icon }}
          </div>
          <div class="node-label">{{ node.label }}</div>
          <div class="node-type">{{ node.typeLabel }}</div>
          <!-- 连接端口 -->
          <div class="node-port port-left" @click.stop="startConnection(node, 'left')"></div>
          <div class="node-port port-right" @click.stop="startConnection(node, 'right')"></div>
          <!-- 删除按钮 -->
          <button class="node-delete" @click.stop="deleteNode(node.id)">×</button>
        </div>

        <!-- 空状态提示 -->
        <div v-if="canvasNodes.length === 0" class="empty-canvas">
          <div class="empty-icon">
            <svg width="64" height="64" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1"><rect x="3" y="3" width="18" height="18" rx="2"/><path d="M12 8v8m-4-4h8"/></svg>
          </div>
          <div class="empty-text">从左侧面板拖拽节点到画布</div>
          <div class="empty-hint">或点击节点快速添加</div>
        </div>
      </div>

      <!-- 属性面板 -->
      <div class="property-panel" v-if="selectedNode || selectedConnection">
        <div class="panel-header">
          <h3 class="panel-title">{{ selectedNode ? '节点属性' : '连接属性' }}</h3>
          <button class="panel-close" @click="selectedNode = null; selectedConnection = null">×</button>
        </div>
        <div class="panel-content">
          <!-- 节点属性 -->
          <template v-if="selectedNode">
            <div class="property-item">
              <span class="property-label">节点ID</span>
              <span class="property-value">{{ selectedNode.id }}</span>
            </div>
            <div class="property-item">
              <span class="property-label">节点类型</span>
              <span class="property-value">{{ selectedNode.typeLabel }}</span>
            </div>
            <div class="property-item">
              <span class="property-label">节点名称</span>
              <input v-model="selectedNode.label" class="property-input" />
            </div>
            <CyberButton variant="danger" size="sm" @click="deleteNode(selectedNode.id)" class="mt-4">
              删除节点
            </CyberButton>
          </template>

          <!-- 连接属性 -->
          <template v-if="selectedConnection">
            <div class="property-item">
              <span class="property-label">连接ID</span>
              <span class="property-value">{{ selectedConnection.id }}</span>
            </div>
            <div class="property-item">
              <span class="property-label">源节点</span>
              <span class="property-value">{{ getNodeLabel(selectedConnection.from) }}</span>
            </div>
            <div class="property-item">
              <span class="property-label">目标节点</span>
              <span class="property-value">{{ getNodeLabel(selectedConnection.to) }}</span>
            </div>
            <div class="property-item">
              <span class="property-label">激活状态</span>
              <div class="property-switch" :class="{ on: selectedConnection.active }" @click="selectedConnection.active = !selectedConnection.active">
                <div class="switch-thumb"></div>
              </div>
            </div>
            <CyberButton variant="danger" size="sm" @click="deleteConnection(selectedConnection.id)" class="mt-4">
              删除连接
            </CyberButton>
          </template>
        </div>
      </div>
    </div>
  </div>
</template>

<script setup lang="ts">
import { ref, reactive } from 'vue'
import { ElMessage } from 'element-plus'
import CyberButton from '../CyberButton.vue'

interface FlowNode {
  id: string
  type: string
  typeLabel: string
  label: string
  icon: string
  color: string
  x: number
  y: number
}

interface FlowConnection {
  id: string
  from: string
  to: string
  active: boolean
}

const props = withDefaults(defineProps<{
  initialName?: string
  initialNodes?: FlowNode[]
  initialConnections?: FlowConnection[]
}>(), {
  initialName: '新建流程',
  initialNodes: () => [],
  initialConnections: () => []
})

const emit = defineEmits<{
  save: [workflow: { name: string; nodes: FlowNode[]; connections: FlowConnection[] }]
  run: [workflow: { name: string; nodes: FlowNode[]; connections: FlowConnection[] }]
}>()

const canvasRef = ref<HTMLElement | null>(null)
const workflowName = ref(props.initialName)
const canvasNodes = reactive<FlowNode[]>([...props.initialNodes])
const connections = reactive<FlowConnection[]>([...props.initialConnections])
const selectedNode = ref<FlowNode | null>(null)
const selectedConnection = ref<FlowConnection | null>(null)
const connectionMode = ref(false)
const connectionStart = ref<any>(null)
const mousePosition = reactive({ x: 0, y: 0 })

// 拖拽状态
const isDragging = ref(false)
const dragNode = ref<any>(null)
const dragOffset = reactive({ x: 0, y: 0 })

// 节点分类展开状态
const expandedCategories = reactive({
  device: true,
  service: true,
  logic: true,
  action: true
})

// 节点定义
const deviceNodes = [
  { type: 'sensor', label: '传感器', icon: '🌡️', color: '#3b82f6' },
  { type: 'controller', label: '控制器', icon: '🎮', color: '#22c55d' },
  { type: 'alarm', label: '报警器', icon: '🔔', color: '#ef4444' },
  { type: 'switch', label: '开关', icon: '🔌', color: '#f59e0b' }
]

const serviceNodes = [
  { type: 'collect', label: '数据采集', icon: '📥', color: '#3b82f6' },
  { type: 'process', label: '数据处理', icon: '⚙️', color: '#8b5cf6' },
  { type: 'store', label: '数据存储', icon: '💾', color: '#f59e0b' },
  { type: 'analyze', label: '数据分析', icon: '📊', color: '#22c55d' }
]

const logicNodes = [
  { type: 'condition', label: '条件判断', icon: '❓', color: '#f59e0b' },
  { type: 'and', label: '逻辑与', icon: '∧', color: '#84cc16' },
  { type: 'or', label: '逻辑或', icon: '∨', color: '#22c55d' },
  { type: 'delay', label: '延时', icon: '⏱️', color: '#6366f1' }
]

const actionNodes = [
  { type: 'control', label: '设备控制', icon: '🎮', color: '#3b82f6' },
  { type: 'notify', label: '发送通知', icon: '📧', color: '#ef4444' },
  { type: 'log', label: '记录日志', icon: '📝', color: '#22c55d' },
  { type: 'trigger', label: '触发流程', icon: '▶️', color: '#f59e0b' }
]

// 切换分类展开
function toggleCategory(category: string) {
  expandedCategories[category as keyof typeof expandedCategories] = !expandedCategories[category as keyof typeof expandedCategories]
}

// 拖拽开始
function handleDragStart(event: DragEvent, node: any) {
  if (event.dataTransfer) {
    event.dataTransfer.setData('application/json', JSON.stringify(node))
    event.dataTransfer.effectAllowed = 'copy'
  }
}

// 拖拽进入画布
function handleDragOver(event: DragEvent) {
  event.preventDefault()
  if (event.dataTransfer) {
    event.dataTransfer.dropEffect = 'copy'
  }
}

// 拖拽放下
function handleDrop(event: DragEvent) {
  event.preventDefault()
  if (!event.dataTransfer || !canvasRef.value) return

  const data = JSON.parse(event.dataTransfer.getData('application/json'))
  const rect = canvasRef.value.getBoundingClientRect()
  const x = event.clientX - rect.left
  const y = event.clientY - rect.top

  addNodeToCanvas(data, x, y)
}

// 添加节点到画布
function addNodeToCanvas(nodeData: any, x: number, y: number) {
  const node: FlowNode = {
    id: `${nodeData.type}-${Date.now()}`,
    type: nodeData.type,
    typeLabel: nodeData.label,
    label: nodeData.label,
    icon: nodeData.icon,
    color: nodeData.color,
    x: x - 50,
    y: y - 30
  }

  canvasNodes.push(node)
  ElMessage.success(`已添加 ${nodeData.label}`)
}

// 节点拖拽
function startDragNode(event: MouseEvent, node: FlowNode) {
  if (connectionMode.value) return

  isDragging.value = true
  dragNode.value = node
  dragOffset.x = event.clientX - node.x
  dragOffset.y = event.clientY - node.y

  document.addEventListener('mousemove', handleDragNode)
  document.addEventListener('mouseup', stopDragNode)
}

function handleDragNode(event: MouseEvent) {
  if (!isDragging.value || !dragNode.value || !canvasRef.value) return

  const rect = canvasRef.value.getBoundingClientRect()
  dragNode.value.x = Math.max(0, Math.min(event.clientX - dragOffset.x, rect.width - 100))
  dragNode.value.y = Math.max(0, Math.min(event.clientY - dragOffset.y, rect.height - 60))
}

function stopDragNode() {
  isDragging.value = false
  dragNode.value = null
  document.removeEventListener('mousemove', handleDragNode)
  document.removeEventListener('mouseup', stopDragNode)
}

// 鼠标移动
function handleMouseMove(event: MouseEvent) {
  if (!canvasRef.value) return
  const rect = canvasRef.value.getBoundingClientRect()
  mousePosition.x = event.clientX - rect.left
  mousePosition.y = event.clientY - rect.top
}

// 节点点击
function handleNodeClick(node: FlowNode) {
  if (connectionMode.value) {
    if (!connectionStart.value) {
      connectionStart.value = { id: node.id, x: node.x + 50, y: node.y + 30 }
    } else if (connectionStart.value.id !== node.id) {
      createConnection(connectionStart.value.id, node.id)
      connectionStart.value = null
    }
  } else {
    selectedNode.value = node
    selectedConnection.value = null
  }
}

// 画布点击
function handleCanvasClick(event: MouseEvent) {
  if (event.target === canvasRef.value) {
    selectedNode.value = null
    selectedConnection.value = null
    connectionStart.value = null
  }
}

// 选择连接
function selectConnection(conn: FlowConnection) {
  selectedConnection.value = conn
  selectedNode.value = null
}

// 开始连线
function startConnection(node: FlowNode, port: string) {
  if (!connectionMode.value) return

  connectionStart.value = {
    id: node.id,
    x: node.x + (port === 'left' ? 0 : 100),
    y: node.y + 30
  }
}

// 切换连线模式
function toggleConnectionMode() {
  connectionMode.value = !connectionMode.value
  connectionStart.value = null
  ElMessage.info(connectionMode.value ? '连线模式已开启' : '连线模式已关闭')
}

// 创建连接
function createConnection(fromId: string, toId: string) {
  const existing = connections.find(c => c.from === fromId && c.to === toId)
  if (existing) {
    ElMessage.warning('连接已存在')
    return
  }

  connections.push({
    id: `conn-${Date.now()}`,
    from: fromId,
    to: toId,
    active: true
  })

  ElMessage.success('连接已创建')
}

// 获取连接路径
function getConnectionPath(conn: FlowConnection): string {
  const fromNode = canvasNodes.find(n => n.id === conn.from)
  const toNode = canvasNodes.find(n => n.id === conn.to)
  if (!fromNode || !toNode) return ''

  const x1 = fromNode.x + 100
  const y1 = fromNode.y + 30
  const x2 = toNode.x
  const y2 = toNode.y + 30

  const midX = (x1 + x2) / 2

  return `M${x1},${y1} C${midX},${y1} ${midX},${y2} ${x2},${y2}`
}

// 获取箭头点
function getArrowPoints(conn: FlowConnection): string {
  const fromNode = canvasNodes.find(n => n.id === conn.from)
  const toNode = canvasNodes.find(n => n.id === conn.to)
  if (!fromNode || !toNode) return ''

  const x2 = toNode.x
  const y2 = toNode.y + 30

  return `${x2 - 8},${y2 - 4} ${x2},${y2} ${x2 - 8},${y2 + 4}`
}

// 获取节点标签
function getNodeLabel(nodeId: string): string {
  const node = canvasNodes.find(n => n.id === nodeId)
  return node?.label || '未知'
}

// 删除节点
function deleteNode(nodeId: string) {
  const index = canvasNodes.findIndex(n => n.id === nodeId)
  if (index !== -1) {
    canvasNodes.splice(index, 1)
    // 删除相关连接
    const relatedConns = connections.filter(c => c.from === nodeId || c.to === nodeId)
    relatedConns.forEach(c => deleteConnection(c.id))
    selectedNode.value = null
    ElMessage.success('节点已删除')
  }
}

// 删除连接
function deleteConnection(connId: string) {
  const index = connections.findIndex(c => c.id === connId)
  if (index !== -1) {
    connections.splice(index, 1)
    selectedConnection.value = null
    ElMessage.success('连接已删除')
  }
}

// 清空画布
function clearCanvas() {
  canvasNodes.splice(0, canvasNodes.length)
  connections.splice(0, connections.length)
  selectedNode.value = null
  selectedConnection.value = null
  ElMessage.success('画布已清空')
}

// 自动布局
function autoLayout() {
  const levels: Record<string, FlowNode[]> = { device: [], service: [], logic: [], action: [] }
  canvasNodes.forEach(node => {
    if (levels[node.type]) {
      levels[node.type].push(node)
    }
  })

  let y = 50
  Object.keys(levels).forEach(type => {
    levels[type].forEach((node, index) => {
      node.x = 100 + index * 150
      node.y = y
    })
    y += 120
  })

  ElMessage.success('自动布局完成')
}

// 保存流程
function saveWorkflow() {
  const workflow = {
    name: workflowName.value,
    nodes: [...canvasNodes],
    connections: [...connections]
  }
  emit('save', workflow)
  ElMessage.success('流程已保存')
}

// 运行流程
function runWorkflow() {
  const workflow = {
    name: workflowName.value,
    nodes: [...canvasNodes],
    connections: [...connections]
  }
  emit('run', workflow)
  ElMessage.success('流程已运行')
}
</script>

<style scoped>
.simple-flow-designer {
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

.toolbar-left, .toolbar-right { display: flex; gap: var(--spacing-xs); }
.toolbar-center { flex: 1; display: flex; justify-content: center; }

.workflow-name-input {
  width: 200px;
  padding: 6px 12px;
  background: var(--bg-primary);
  border: 1px solid var(--border-primary);
  border-radius: var(--radius-sm);
  color: var(--text-primary);
  font-size: 14px;
  text-align: center;
  outline: none;
}

.workflow-name-input:focus { border-color: var(--accent-primary); }

.designer-content {
  flex: 1;
  display: flex;
  overflow: hidden;
}

/* 节点面板 */
.node-palette {
  width: 200px;
  background: var(--bg-card);
  border-right: 1px solid var(--border-primary);
  overflow-y: auto;
}

.palette-header {
  padding: var(--spacing-md);
  border-bottom: 1px solid var(--border-primary);
}

.palette-title {
  font-size: 14px;
  font-weight: 600;
  color: var(--text-primary);
  margin: 0;
}

.palette-content {
  padding: var(--spacing-sm);
}

.node-category {
  margin-bottom: var(--spacing-sm);
}

.category-header {
  display: flex;
  align-items: center;
  gap: var(--spacing-xs);
  padding: var(--spacing-sm);
  cursor: pointer;
  border-radius: var(--radius-sm);
  transition: background var(--duration-fast);
}

.category-header:hover { background: rgba(255,255,255,0.05); }

.category-icon { font-size: 14px; }
.category-title { font-size: 12px; font-weight: 600; color: var(--text-secondary); }

.category-nodes {
  padding-left: var(--spacing-sm);
}

.palette-node {
  display: flex;
  align-items: center;
  gap: var(--spacing-xs);
  padding: var(--spacing-sm);
  margin: 2px 0;
  background: rgba(0,0,0,0.2);
  border: 1px solid var(--border-primary);
  border-radius: var(--radius-sm);
  cursor: grab;
  transition: all var(--duration-fast);
}

.palette-node:hover {
  background: rgba(59,130,246,0.1);
  border-color: var(--accent-primary);
  transform: translateX(4px);
}

.palette-node:active { cursor: grabbing; transform: scale(0.95); }

.palette-node .node-icon { font-size: 14px; }
.palette-node .node-label { font-size: 11px; color: var(--text-primary); }

/* 画布区域 */
.canvas-area {
  flex: 1;
  position: relative;
  overflow: hidden;
  background:
    linear-gradient(rgba(59,130,246,0.03) 1px, transparent 1px),
    linear-gradient(90deg, rgba(59,130,246,0.03) 1px, transparent 1px);
  background-size: 20px 20px;
}

.connections-svg {
  position: absolute;
  top: 0;
  left: 0;
  width: 100%;
  height: 100%;
  pointer-events: none;
}

.connection-line {
  pointer-events: stroke;
  cursor: pointer;
}

.connection-line:hover {
  stroke: var(--accent-primary) !important;
  stroke-width: 4 !important;
}

.canvas-node {
  position: absolute;
  width: 100px;
  background: var(--bg-card);
  border: 2px solid var(--border-primary);
  border-radius: var(--radius-md);
  padding: var(--spacing-sm);
  cursor: move;
  transition: all var(--duration-fast);
  z-index: 10;
}

.canvas-node:hover {
  border-color: var(--accent-primary);
  box-shadow: var(--shadow-md);
  z-index: 20;
}

.canvas-node.selected {
  border-color: var(--accent-primary);
  box-shadow: 0 0 20px rgba(59,130,246,0.3);
}

.canvas-node.connecting {
  border-color: #22c55d;
  box-shadow: 0 0 20px rgba(34,197,94,0.3);
}

.node-device { border-left: 4px solid #3b82f6; }
.node-service { border-left: 4px solid #8b5cf6; }
.node-logic { border-left: 4px solid #f59e0b; }
.node-action { border-left: 4px solid #22c55d; }

.node-icon-wrapper {
  width: 32px;
  height: 32px;
  display: flex;
  align-items: center;
  justify-content: center;
  border-radius: var(--radius-sm);
  font-size: 16px;
  margin: 0 auto var(--spacing-xs);
}

.canvas-node .node-label {
  font-size: 11px;
  font-weight: 600;
  color: var(--text-primary);
  text-align: center;
  white-space: nowrap;
  overflow: hidden;
  text-overflow: ellipsis;
}

.canvas-node .node-type {
  font-size: 9px;
  color: var(--text-tertiary);
  text-align: center;
  margin-top: 2px;
}

.node-port {
  position: absolute;
  width: 12px;
  height: 12px;
  background: var(--accent-primary);
  border: 2px solid var(--bg-card);
  border-radius: 50%;
  cursor: crosshair;
  opacity: 0;
  transition: opacity var(--duration-fast);
  z-index: 15;
}

.canvas-node:hover .node-port { opacity: 1; }

.port-left { left: -6px; top: 50%; transform: translateY(-50%); }
.port-right { right: -6px; top: 50%; transform: translateY(-50%); }

.node-delete {
  position: absolute;
  top: -8px;
  right: -8px;
  width: 20px;
  height: 20px;
  display: flex;
  align-items: center;
  justify-content: center;
  background: var(--status-danger);
  border: none;
  border-radius: 50%;
  color: white;
  font-size: 12px;
  cursor: pointer;
  opacity: 0;
  transition: opacity var(--duration-fast);
  z-index: 20;
}

.canvas-node:hover .node-delete { opacity: 1; }

.empty-canvas {
  position: absolute;
  top: 50%;
  left: 50%;
  transform: translate(-50%, -50%);
  text-align: center;
  pointer-events: none;
}

.empty-icon { color: var(--text-tertiary); opacity: 0.5; margin-bottom: var(--spacing-md); }
.empty-text { font-size: 16px; color: var(--text-secondary); margin-bottom: var(--spacing-sm); }
.empty-hint { font-size: 13px; color: var(--text-tertiary); }

/* 属性面板 */
.property-panel {
  width: 250px;
  background: var(--bg-card);
  border-left: 1px solid var(--border-primary);
  overflow-y: auto;
}

.panel-header {
  display: flex;
  align-items: center;
  justify-content: space-between;
  padding: var(--spacing-md);
  border-bottom: 1px solid var(--border-primary);
}

.panel-title {
  font-size: 14px;
  font-weight: 600;
  color: var(--text-primary);
  margin: 0;
}

.panel-close {
  width: 28px;
  height: 28px;
  display: flex;
  align-items: center;
  justify-content: center;
  background: transparent;
  border: none;
  color: var(--text-secondary);
  font-size: 18px;
  cursor: pointer;
  border-radius: var(--radius-sm);
}

.panel-close:hover { background: rgba(255,255,255,0.1); color: var(--text-primary); }

.panel-content {
  padding: var(--spacing-md);
}

.property-item {
  display: flex;
  align-items: center;
  justify-content: space-between;
  margin-bottom: var(--spacing-sm);
}

.property-label { font-size: 12px; color: var(--text-secondary); }
.property-value { font-size: 12px; color: var(--text-primary); font-family: var(--font-mono); }
.property-input {
  width: 120px;
  padding: 4px 8px;
  background: var(--bg-primary);
  border: 1px solid var(--border-primary);
  border-radius: var(--radius-sm);
  color: var(--text-primary);
  font-size: 12px;
  outline: none;
}

.property-input:focus { border-color: var(--accent-primary); }

.property-switch {
  width: 40px;
  height: 20px;
  background: var(--bg-primary);
  border: 1px solid var(--border-primary);
  border-radius: 10px;
  cursor: pointer;
  position: relative;
  transition: all var(--duration-fast);
}

.property-switch.on { background: var(--status-success); border-color: var(--status-success); }

.switch-thumb {
  position: absolute;
  top: 2px;
  left: 2px;
  width: 14px;
  height: 14px;
  background: white;
  border-radius: 50%;
  transition: transform var(--duration-fast);
}

.property-switch.on .switch-thumb { transform: translateX(20px); }

.mt-4 { margin-top: var(--spacing-md); }
</style>
