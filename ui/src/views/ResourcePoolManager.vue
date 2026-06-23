<template>
  <div class="resource-pool-root">
    <ParticleBackground :particle-count="40" color="#8b5cf6" :opacity="0.3" />
    <AppNavbar title="资源池管理" subtitle="Resource Pool Manager" :menu-items="menuItems" />

    <main class="pool-main">
      <!-- 左侧面板：资源选择 -->
      <div class="left-panel">
        <!-- 设备资源 -->
        <div class="resource-section">
          <div class="section-header">
            <h3 class="section-title">
              <span class="section-icon">📡</span>
              设备资源
            </h3>
            <span class="device-count">{{ selectedDevices.length }}/{{ deviceResources.length }}</span>
          </div>
          <div class="resource-list">
            <div
              v-for="device in deviceResources"
              :key="device.id"
              class="resource-item"
              :class="{ selected: selectedDevices.includes(device.id) }"
              @click="toggleDevice(device.id)"
              draggable="true"
              @dragstart="handleDragStart($event, 'device', device)"
            >
              <div class="resource-icon" :style="{ background: device.color + '20', color: device.color }">
                {{ device.icon }}
              </div>
              <div class="resource-info">
                <div class="resource-name">{{ device.name }}</div>
                <div class="resource-type">{{ device.type }}</div>
              </div>
              <div class="resource-check" v-if="selectedDevices.includes(device.id)">✓</div>
            </div>
          </div>
        </div>

        <!-- 原子服务资源 -->
        <div class="resource-section">
          <div class="section-header">
            <h3 class="section-title">
              <span class="section-icon">⚙️</span>
              原子服务
            </h3>
            <span class="device-count">{{ selectedServices.length }}/{{ serviceResources.length }}</span>
          </div>
          <div class="resource-list">
            <div
              v-for="service in serviceResources"
              :key="service.id"
              class="resource-item"
              :class="{ selected: selectedServices.includes(service.id) }"
              @click="toggleService(service.id)"
              draggable="true"
              @dragstart="handleDragStart($event, 'service', service)"
            >
              <div class="resource-icon" :style="{ background: service.color + '20', color: service.color }">
                {{ service.icon }}
              </div>
              <div class="resource-info">
                <div class="resource-name">{{ service.name }}</div>
                <div class="resource-type">{{ service.type }}</div>
              </div>
              <div class="resource-check" v-if="selectedServices.includes(service.id)">✓</div>
            </div>
          </div>
        </div>

        <!-- 应用资源 -->
        <div class="resource-section">
          <div class="section-header">
            <h3 class="section-title">
              <span class="section-icon">📱</span>
              应用
            </h3>
            <span class="device-count">{{ selectedApps.length }}/{{ appResources.length }}</span>
          </div>
          <div class="resource-list">
            <div
              v-for="app in appResources"
              :key="app.id"
              class="resource-item"
              :class="{ selected: selectedApps.includes(app.id) }"
              @click="toggleApp(app.id)"
              draggable="true"
              @dragstart="handleDragStart($event, 'app', app)"
            >
              <div class="resource-icon" :style="{ background: app.color + '20', color: app.color }">
                {{ app.icon }}
              </div>
              <div class="resource-info">
                <div class="resource-name">{{ app.name }}</div>
                <div class="resource-type">{{ app.type }}</div>
              </div>
              <div class="resource-check" v-if="selectedApps.includes(app.id)">✓</div>
            </div>
          </div>
        </div>
      </div>

      <!-- 中间面板：可视化画布 -->
      <div class="center-panel">
        <!-- 工具栏 -->
        <div class="toolbar">
          <div class="toolbar-left">
            <CyberButton variant="ghost" size="sm" @click="clearCanvas">
              <svg width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><polyline points="3 6 5 6 21 6"/><path d="M19 6v14a2 2 0 0 1-2 2H7a2 2 0 0 1-2-2V6m3 0V4a2 2 0 0 1 2-2h4a2 2 0 0 1 2 2v2"/></svg>
              清空画布
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
            <span class="canvas-title">资源关系画布</span>
            <span class="canvas-hint" v-if="connectionMode">点击源节点，再点击目标节点创建连接</span>
          </div>
          <div class="toolbar-right">
            <CyberButton variant="success" size="sm" @click="generateApp" :disabled="canvasNodes.length === 0">
              <svg width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><rect x="3" y="3" width="18" height="18" rx="2"/><path d="M12 8v8m-4-4h8"/></svg>
              生成应用
            </CyberButton>
            <CyberButton variant="primary" size="sm" @click="saveConfiguration">
              <svg width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M19 21H5a2 2 0 0 1-2-2V5a2 2 0 0 1 2-2h11l5 5v11a2 2 0 0 1-2 2z"/><polyline points="17 21 17 13 7 13 7 21"/><polyline points="7 3 7 8 15 8"/></svg>
              保存配置
            </CyberButton>
            <CyberButton variant="ghost" size="sm" @click="exportConfiguration">
              <svg width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M21 15v4a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2v-4"/><polyline points="17 8 12 3 7 8"/><line x1="12" y1="3" x2="12" y2="15"/></svg>
              导出JSON
            </CyberButton>
          </div>
        </div>

        <!-- 画布区域 -->
        <div
          class="canvas-area"
          ref="canvasRef"
          @drop="handleDrop"
          @dragover="handleDragOver"
          @click="handleCanvasClick"
        >
          <svg class="connections-svg" :width="canvasWidth" :height="canvasHeight">
            <!-- 连接线 -->
            <g v-for="(conn, index) in connections" :key="index">
              <path
                :d="getConnectionPath(conn)"
                fill="none"
                :stroke="conn.active ? '#3b82f6' : '#475569'"
                :stroke-width="conn.active ? 3 : 2"
                :stroke-dasharray="conn.active ? 'none' : '5,5'"
                class="connection-line"
              />
              <!-- 连接箭头 -->
              <polygon
                :points="getArrowPoints(conn)"
                :fill="conn.active ? '#3b82f6' : '#475569'"
              />
              <!-- 连接标签 -->
              <text
                :x="getConnectionLabelX(conn)"
                :y="getConnectionLabelY(conn)"
                text-anchor="middle"
                fill="#94a3b8"
                font-size="10"
              >
                {{ conn.label }}
              </text>
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
            @mousedown="startDragNode($event, node)"
            @click="handleNodeClick(node)"
          >
            <div class="node-icon" :style="{ background: node.color + '20', color: node.color }">
              {{ node.icon }}
            </div>
            <div class="node-label">{{ node.label }}</div>
            <div class="node-type">{{ node.typeLabel }}</div>
            <!-- 连接端口 -->
            <div class="node-port port-left" @click.stop="startConnection(node, 'left')"></div>
            <div class="node-port port-right" @click.stop="startConnection(node, 'right')"></div>
            <div class="node-port port-top" @click.stop="startConnection(node, 'top')"></div>
            <div class="node-port port-bottom" @click.stop="startConnection(node, 'bottom')"></div>
            <!-- 删除按钮 -->
            <button class="node-delete" @click.stop="deleteNode(node.id)">×</button>
          </div>

          <!-- 空状态提示 -->
          <div v-if="canvasNodes.length === 0" class="empty-canvas">
            <div class="empty-icon">
              <svg width="64" height="64" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1"><rect x="3" y="3" width="18" height="18" rx="2"/><path d="M12 8v8m-4-4h8"/></svg>
            </div>
            <div class="empty-text">从左侧面板拖拽资源到画布</div>
            <div class="empty-hint">或点击资源快速添加</div>
          </div>
        </div>
      </div>

      <!-- 右侧面板：属性配置 -->
      <div class="right-panel">
        <!-- 节点属性 -->
        <div v-if="selectedNode" class="property-section">
          <div class="section-header">
            <h3 class="section-title">
              <span class="section-icon">⚙️</span>
              节点属性
            </h3>
          </div>
          <div class="property-content">
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
              <input v-model="selectedNode.label" class="property-input" @change="updateNodeLabel" />
            </div>
            <div class="property-item">
              <span class="property-label">X坐标</span>
              <input v-model.number="selectedNode.x" type="number" class="property-input" @change="updateNodePosition" />
            </div>
            <div class="property-item">
              <span class="property-label">Y坐标</span>
              <input v-model.number="selectedNode.y" type="number" class="property-input" @change="updateNodePosition" />
            </div>
          </div>
        </div>

        <!-- 连接属性 -->
        <div v-if="selectedConnection" class="property-section">
          <div class="section-header">
            <h3 class="section-title">
              <span class="section-icon">🔗</span>
              连接属性
            </h3>
          </div>
          <div class="property-content">
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
              <span class="property-label">连接标签</span>
              <input v-model="selectedConnection.label" class="property-input" />
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
          </div>
        </div>

        <!-- 统计信息 -->
        <div class="property-section">
          <div class="section-header">
            <h3 class="section-title">
              <span class="section-icon">📊</span>
              统计信息
            </h3>
          </div>
          <div class="property-content">
            <div class="stat-item">
              <span class="stat-label">设备节点</span>
              <span class="stat-value">{{ canvasNodes.filter(n => n.type === 'device').length }}</span>
            </div>
            <div class="stat-item">
              <span class="stat-label">服务节点</span>
              <span class="stat-value">{{ canvasNodes.filter(n => n.type === 'service').length }}</span>
            </div>
            <div class="stat-item">
              <span class="stat-label">应用节点</span>
              <span class="stat-value">{{ canvasNodes.filter(n => n.type === 'app').length }}</span>
            </div>
            <div class="stat-item">
              <span class="stat-label">连接数量</span>
              <span class="stat-value">{{ connections.length }}</span>
            </div>
          </div>
        </div>

        <!-- 快速操作 -->
        <div class="property-section">
          <div class="section-header">
            <h3 class="section-title">
              <span class="section-icon">⚡</span>
              快速操作
            </h3>
          </div>
          <div class="quick-actions">
            <CyberButton variant="ghost" size="sm" @click="addAllSelected">添加所有选中</CyberButton>
            <CyberButton variant="ghost" size="sm" @click="clearSelection">清空选择</CyberButton>
            <CyberButton variant="ghost" size="sm" @click="createSampleConfig">创建示例</CyberButton>
          </div>
        </div>
      </div>
    </main>

    <!-- 生成应用弹窗 -->
    <Transition name="modal">
      <div v-if="showGenerateApp" class="modal-overlay" @click.self="showGenerateApp = false">
        <div class="modal-content">
          <div class="modal-header">
            <h3>生成自定义应用</h3>
            <button class="modal-close" @click="showGenerateApp = false">×</button>
          </div>
          <div class="modal-body">
            <div class="app-preview">
              <div class="preview-label">应用预览</div>
              <div class="preview-stats">
                <span>设备节点: {{ canvasNodes.filter(n => n.type === 'device').length }}</span>
                <span>服务节点: {{ canvasNodes.filter(n => n.type === 'service').length }}</span>
                <span>连接数: {{ connections.length }}</span>
              </div>
            </div>
            <div class="form-grid">
              <div class="form-group full-width"><label>应用名称 *</label><input v-model="appForm.name" placeholder="输入应用名称" /></div>
              <div class="form-group full-width"><label>应用描述</label><textarea v-model="appForm.description" placeholder="输入应用描述" rows="2"></textarea></div>
              <div class="form-group"><label>应用图标</label><input v-model="appForm.icon" placeholder="输入emoji图标" /></div>
              <div class="form-group"><label>应用类型</label><select v-model="appForm.type"><option value="monitoring">监测应用</option><option value="control">控制应用</option><option value="analysis">分析应用</option><option value="custom">自定义</option></select></div>
            </div>
          </div>
          <div class="modal-footer">
            <CyberButton variant="ghost" @click="showGenerateApp = false">取消</CyberButton>
            <CyberButton variant="success" @click="confirmGenerateApp">生成应用</CyberButton>
          </div>
        </div>
      </div>
    </Transition>
  </div>
</template>

<script setup lang="ts">
import { ref, reactive, onMounted } from 'vue'
import { ElMessage } from 'element-plus'
import ParticleBackground from '../components/ParticleBackground.vue'
import AppNavbar from '../components/AppNavbar.vue'
import CyberButton from '../components/CyberButton.vue'

const APP_STORAGE_KEY = 'custom-apps'

const menuItems = [
  { path: '/', label: '主控台' },
  { path: '/devices', label: '设备管理' },
  { path: '/workflow', label: '流程编排' },
  { path: '/settings', label: '系统设置' }
]

// 生成应用相关
const showGenerateApp = ref(false)
const appForm = reactive({
  name: '',
  description: '',
  icon: '📱',
  type: 'custom'
})

// 画布相关
const canvasRef = ref<HTMLElement | null>(null)
const canvasWidth = ref(800)
const canvasHeight = ref(600)
const connectionMode = ref(false)
const connectionStart = ref<any>(null)
const mousePosition = reactive({ x: 0, y: 0 })

// 选择状态
const selectedDevices = ref<string[]>([])
const selectedServices = ref<string[]>([])
const selectedApps = ref<string[]>([])
const selectedNode = ref<any>(null)
const selectedConnection = ref<any>(null)

// 拖拽状态
const isDragging = ref(false)
const dragNode = ref<any>(null)
const dragOffset = reactive({ x: 0, y: 0 })

// 设备资源
const deviceResources = reactive([
  { id: 'dev-001', name: '温度传感器', type: '云测仪 SD123', icon: '🌡️', color: '#3b82f6' },
  { id: 'dev-002', name: '湿度传感器', type: '云测仪 SD123', icon: '💧', color: '#06b6d4' },
  { id: 'dev-003', name: 'PM2.5传感器', type: '云测仪 SD123', icon: '💨', color: '#f59e0b' },
  { id: 'dev-004', name: 'CO2传感器', type: '云测仪 SD123', icon: '☁️', color: '#8b5cf6' },
  { id: 'dev-005', name: '烟雾报警器', type: 'SMK-200', icon: '🔥', color: '#ef4444' },
  { id: 'dev-006', name: '水浸传感器', type: 'WTR-100', icon: '💧', color: '#06b6d4' },
  { id: 'dev-007', name: '红外探测器', type: 'IRD-300', icon: '👤', color: '#8b5cf6' },
  { id: 'dev-008', name: '弱光传感器', type: 'LS-100', icon: '💡', color: '#f59e0b' },
  { id: 'dev-009', name: '空调控制器', type: 'ACT-500', icon: '❄️', color: '#22c55d' },
  { id: 'dev-010', name: '恒湿净化机', type: 'HUM-400', icon: '🌀', color: '#22c55d' }
])

// 原子服务资源
const serviceResources = reactive([
  // 基础下层服务
  { id: 'svc-001', name: '数据采集服务', type: '下层服务', icon: '📥', color: '#3b82f6' },
  { id: 'svc-002', name: '数据处理服务', type: '下层服务', icon: '⚙️', color: '#8b5cf6' },
  { id: 'svc-003', name: '报警判断服务', type: '下层服务', icon: '🔔', color: '#ef4444' },
  { id: 'svc-004', name: '数据存储服务', type: '下层服务', icon: '💾', color: '#f59e0b' },
  { id: 'svc-005', name: '设备控制服务', type: '下层服务', icon: '🎮', color: '#22c55d' },
  // 数据分析类
  { id: 'svc-006', name: '数据融合服务', type: '下层服务', icon: '🔀', color: '#8b5cf6' },
  { id: 'svc-007', name: '统计分析服务', type: '下层服务', icon: '📊', color: '#3b82f6' },
  { id: 'svc-008', name: '趋势预测服务', type: '下层服务', icon: '📈', color: '#22c55d' },
  { id: 'svc-009', name: '设备健康管理', type: '上层服务', icon: '🏥', color: '#06b6d4' },
  // 自动化控制类
  { id: 'svc-010', name: '定时任务服务', type: '下层服务', icon: '⏰', color: '#f59e0b' },
  { id: 'svc-011', name: '条件联动服务', type: '下层服务', icon: '⚡', color: '#ef4444' },
  { id: 'svc-012', name: '设备冗余切换', type: '下层服务', icon: '🔄', color: '#8b5cf6' },
  { id: 'svc-013', name: '场景模式切换', type: '上层服务', icon: '🎬', color: '#ec4899' },
  // 报警管理类
  { id: 'svc-014', name: '报警升级服务', type: '下层服务', icon: '⬆️', color: '#ef4444' },
  { id: 'svc-015', name: '报警确认服务', type: '下层服务', icon: '✅', color: '#22c55d' },
  { id: 'svc-016', name: '报警统计分析', type: '上层服务', icon: '📊', color: '#f59e0b' },
  // 组合上层服务
  { id: 'svc-017', name: '环境监测服务', type: '上层服务', icon: '🌡️', color: '#22c55d' },
  { id: 'svc-018', name: '安防监控服务', type: '上层服务', icon: '🛡️', color: '#8b5cf6' },
  { id: 'svc-019', name: '消防预警服务', type: '上层服务', icon: '🔥', color: '#ef4444' },
  { id: 'svc-020', name: '设备联动服务', type: '上层服务', icon: '🔗', color: '#ec4899' },
  { id: 'svc-021', name: '发送通知服务', type: '下层服务', icon: '📧', color: '#06b6d4' }
])

// 应用资源
const appResources = reactive([
  { id: 'app-001', name: '环境监测应用', type: '监测应用', icon: '🌡️', color: '#22c55d' },
  { id: 'app-002', name: '安防系统应用', type: '安防应用', icon: '🛡️', color: '#8b5cf6' },
  { id: 'app-003', name: '消防系统应用', type: '消防应用', icon: '🔥', color: '#ef4444' },
  { id: 'app-004', name: '自定义应用', type: '自定义', icon: '📱', color: '#3b82f6' }
])

// 画布节点
const canvasNodes = reactive<any[]>([])

// 连接
const connections = reactive<any[]>([])

// 切换选择
function toggleDevice(id: string) {
  const index = selectedDevices.value.indexOf(id)
  if (index === -1) {
    selectedDevices.value.push(id)
  } else {
    selectedDevices.value.splice(index, 1)
  }
}

function toggleService(id: string) {
  const index = selectedServices.value.indexOf(id)
  if (index === -1) {
    selectedServices.value.push(id)
  } else {
    selectedServices.value.splice(index, 1)
  }
}

function toggleApp(id: string) {
  const index = selectedApps.value.indexOf(id)
  if (index === -1) {
    selectedApps.value.push(id)
  } else {
    selectedApps.value.splice(index, 1)
  }
}

// 拖拽处理
function handleDragStart(event: DragEvent, type: string, resource: any) {
  if (event.dataTransfer) {
    event.dataTransfer.setData('application/json', JSON.stringify({ type, resource }))
    event.dataTransfer.effectAllowed = 'copy'
  }
}

function handleDragOver(event: DragEvent) {
  event.preventDefault()
  if (event.dataTransfer) {
    event.dataTransfer.dropEffect = 'copy'
  }
}

function handleDrop(event: DragEvent) {
  event.preventDefault()
  if (!event.dataTransfer || !canvasRef.value) return

  const data = JSON.parse(event.dataTransfer.getData('application/json'))
  const rect = canvasRef.value.getBoundingClientRect()
  const x = event.clientX - rect.left
  const y = event.clientY - rect.top

  addNodeToCanvas(data.type, data.resource, x, y)
}

// 添加节点到画布
function addNodeToCanvas(type: string, resource: any, x: number, y: number) {
  const node = {
    id: `${type}-${resource.id}-${Date.now()}`,
    type,
    typeLabel: type === 'device' ? '设备' : type === 'service' ? '服务' : '应用',
    label: resource.name,
    icon: resource.icon,
    color: resource.color,
    x: x - 50,
    y: y - 30,
    resourceId: resource.id
  }

  canvasNodes.push(node)
  ElMessage.success(`已添加 ${resource.name}`)
}

// 节点拖拽
function startDragNode(event: MouseEvent, node: any) {
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

// 节点点击
function handleNodeClick(node: any) {
  if (connectionMode.value) {
    if (!connectionStart.value) {
      connectionStart.value = { id: node.id, x: node.x + 50, y: node.y + 30 }
    } else if (connectionStart.value.id !== node.id) {
      // 创建连接
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
  if (event.target === canvasRef.value || (event.target as HTMLElement).classList.contains('canvas-area')) {
    selectedNode.value = null
    selectedConnection.value = null
    connectionStart.value = null
  }
}

// 创建连接
function createConnection(fromId: string, toId: string) {
  const existing = connections.find(c => c.from === fromId && c.to === toId)
  if (existing) {
    ElMessage.warning('连接已存在')
    return
  }

  const fromNode = canvasNodes.find(n => n.id === fromId)
  const toNode = canvasNodes.find(n => n.id === toId)

  connections.push({
    id: `conn-${Date.now()}`,
    from: fromId,
    to: toId,
    label: `${fromNode?.label} → ${toNode?.label}`,
    active: true
  })

  ElMessage.success('连接已创建')
}

// 开始连线
function startConnection(node: any, port: string) {
  if (!connectionMode.value) return

  connectionStart.value = {
    id: node.id,
    x: node.x + (port === 'left' ? 0 : port === 'right' ? 100 : 50),
    y: node.y + (port === 'top' ? 0 : port === 'bottom' ? 60 : 30)
  }
}

// 切换连线模式
function toggleConnectionMode() {
  connectionMode.value = !connectionMode.value
  connectionStart.value = null
  ElMessage.info(connectionMode.value ? '连线模式已开启' : '连线模式已关闭')
}

// 获取连接路径
function getConnectionPath(conn: any): string {
  const fromNode = canvasNodes.find(n => n.id === conn.from)
  const toNode = canvasNodes.find(n => n.id === conn.to)
  if (!fromNode || !toNode) return ''

  const x1 = fromNode.x + 50
  const y1 = fromNode.y + 30
  const x2 = toNode.x + 50
  const y2 = toNode.y + 30

  const midX = (x1 + x2) / 2

  return `M${x1},${y1} C${midX},${y1} ${midX},${y2} ${x2},${y2}`
}

// 获取箭头点
function getArrowPoints(conn: any): string {
  const fromNode = canvasNodes.find(n => n.id === conn.from)
  const toNode = canvasNodes.find(n => n.id === conn.to)
  if (!fromNode || !toNode) return ''

  const x2 = toNode.x + 50
  const y2 = toNode.y + 30

  return `${x2 - 8},${y2 - 4} ${x2},${y2} ${x2 - 8},${y2 + 4}`
}

// 获取连接标签位置
function getConnectionLabelX(conn: any): number {
  const fromNode = canvasNodes.find(n => n.id === conn.from)
  const toNode = canvasNodes.find(n => n.id === conn.to)
  if (!fromNode || !toNode) return 0
  return (fromNode.x + toNode.x + 100) / 2
}

function getConnectionLabelY(conn: any): number {
  const fromNode = canvasNodes.find(n => n.id === conn.from)
  const toNode = canvasNodes.find(n => n.id === conn.to)
  if (!fromNode || !toNode) return 0
  return (fromNode.y + toNode.y + 60) / 2 - 10
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
    ElMessage.success('节点已删除')
  }
}

// 删除连接
function deleteConnection(connId: string) {
  const index = connections.findIndex(c => c.id === connId)
  if (index !== -1) {
    connections.splice(index, 1)
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
  const levels: Record<string, any[]> = { device: [], service: [], app: [] }
  canvasNodes.forEach(node => {
    if (levels[node.type]) {
      levels[node.type].push(node)
    }
  })

  let y = 50
  Object.keys(levels).forEach(type => {
    levels[type].forEach((node: any, index: number) => {
      node.x = 100 + index * 150
      node.y = y
    })
    y += 150
  })

  ElMessage.success('自动布局完成')
}

// 添加所有选中
function addAllSelected() {
  let x = 50
  let y = 50

  selectedDevices.value.forEach(id => {
    const device = deviceResources.find(d => d.id === id)
    if (device) {
      addNodeToCanvas('device', device, x, y)
      x += 150
      if (x > 600) { x = 50; y += 120 }
    }
  })

  selectedServices.value.forEach(id => {
    const service = serviceResources.find(s => s.id === id)
    if (service) {
      addNodeToCanvas('service', service, x, y)
      x += 150
      if (x > 600) { x = 50; y += 120 }
    }
  })

  selectedApps.value.forEach(id => {
    const app = appResources.find(a => a.id === id)
    if (app) {
      addNodeToCanvas('app', app, x, y)
      x += 150
      if (x > 600) { x = 50; y += 120 }
    }
  })
}

// 清空选择
function clearSelection() {
  selectedDevices.value = []
  selectedServices.value = []
  selectedApps.value = []
}

// 创建示例配置
function createSampleConfig() {
  clearCanvas()

  // 添加设备
  addNodeToCanvas('device', deviceResources[0], 50, 50)
  addNodeToCanvas('device', deviceResources[1], 50, 150)
  addNodeToCanvas('device', deviceResources[4], 50, 250)

  // 添加服务
  addNodeToCanvas('service', serviceResources[0], 250, 50)
  addNodeToCanvas('service', serviceResources[1], 250, 150)
  addNodeToCanvas('service', serviceResources[2], 250, 250)

  // 添加应用
  addNodeToCanvas('app', appResources[0], 450, 100)
  addNodeToCanvas('app', appResources[1], 450, 200)

  // 创建连接
  setTimeout(() => {
    if (canvasNodes.length >= 8) {
      createConnection(canvasNodes[0].id, canvasNodes[3].id)
      createConnection(canvasNodes[1].id, canvasNodes[3].id)
      createConnection(canvasNodes[2].id, canvasNodes[5].id)
      createConnection(canvasNodes[3].id, canvasNodes[4].id)
      createConnection(canvasNodes[4].id, canvasNodes[6].id)
      createConnection(canvasNodes[5].id, canvasNodes[7].id)
    }
  }, 100)

  ElMessage.success('示例配置已创建')
}

// 更新节点标签
function updateNodeLabel() {
  if (selectedNode.value) {
    ElMessage.success('节点标签已更新')
  }
}

// 更新节点位置
function updateNodePosition() {
  if (selectedNode.value) {
    ElMessage.success('节点位置已更新')
  }
}

// 保存配置
function saveConfiguration() {
  const config = {
    nodes: canvasNodes,
    connections: connections,
    timestamp: new Date().toISOString()
  }
  localStorage.setItem('resource-pool-config', JSON.stringify(config))
  ElMessage.success('配置已保存')
}

// 导出配置
function exportConfiguration() {
  const config = {
    nodes: canvasNodes,
    connections: connections,
    timestamp: new Date().toISOString()
  }

  const blob = new Blob([JSON.stringify(config, null, 2)], { type: 'application/json' })
  const url = URL.createObjectURL(blob)
  const a = document.createElement('a')
  a.href = url
  a.download = 'resource-pool-config.json'
  a.click()
  URL.revokeObjectURL(url)
  ElMessage.success('配置已导出')
}

// 打开生成应用弹窗
function generateApp() {
  if (canvasNodes.length === 0) {
    ElMessage.warning('请先在画布上添加节点')
    return
  }
  appForm.name = ''
  appForm.description = ''
  appForm.icon = '📱'
  appForm.type = 'custom'
  showGenerateApp.value = true
}

// 确认生成应用
function confirmGenerateApp() {
  if (!appForm.name) {
    ElMessage.warning('请输入应用名称')
    return
  }

  // 构建应用数据
  const deviceNodes = canvasNodes.filter(n => n.type === 'device')
  const serviceNodes = canvasNodes.filter(n => n.type === 'service')

  const app = {
    id: `app-${Date.now()}`,
    name: appForm.name,
    description: appForm.description || '用户自定义应用',
    type: appForm.type,
    icon: appForm.icon,
    devices: deviceNodes.map(d => d.resourceId || d.id),
    services: serviceNodes.map(s => s.resourceId || s.id),
    nodes: JSON.parse(JSON.stringify(canvasNodes)),
    connections: JSON.parse(JSON.stringify(connections)),
    createdAt: new Date().toLocaleString('zh-CN')
  }

  // 保存到localStorage
  let apps = []
  try {
    const saved = localStorage.getItem(APP_STORAGE_KEY)
    if (saved) apps = JSON.parse(saved)
  } catch { /* ignore */ }

  apps.push(app)
  localStorage.setItem(APP_STORAGE_KEY, JSON.stringify(apps))

  showGenerateApp.value = false
  ElMessage.success(`应用 "${app.name}" 已生成！可在主控台查看和运行`)

  // 重置表单
  appForm.name = ''
  appForm.description = ''
}

// 加载配置
function loadConfiguration() {
  const saved = localStorage.getItem('resource-pool-config')
  if (saved) {
    try {
      const config = JSON.parse(saved)
      canvasNodes.splice(0, canvasNodes.length, ...config.nodes)
      connections.splice(0, connections.length, ...config.connections)
      ElMessage.success('配置已加载')
    } catch (e) {
      ElMessage.error('配置加载失败')
    }
  }
}

onMounted(() => {
  if (canvasRef.value) {
    const rect = canvasRef.value.getBoundingClientRect()
    canvasWidth.value = rect.width
    canvasHeight.value = rect.height
  }

  // 加载保存的配置
  loadConfiguration()
})
</script>

<style scoped>
.resource-pool-root { min-height: 100vh; background: var(--bg-primary); position: relative; }
.pool-main { display: flex; height: calc(100vh - 60px); position: relative; z-index: 1; }

/* 左侧面板 */
.left-panel {
  width: 280px;
  background: var(--bg-card);
  border-right: 1px solid var(--border-primary);
  display: flex;
  flex-direction: column;
  overflow-y: auto;
}

.resource-section {
  padding: var(--spacing-md);
  border-bottom: 1px solid var(--border-primary);
}

.section-header {
  display: flex;
  align-items: center;
  justify-content: space-between;
  margin-bottom: var(--spacing-sm);
}

.section-title {
  display: flex;
  align-items: center;
  gap: var(--spacing-xs);
  font-size: 13px;
  font-weight: 600;
  color: var(--text-secondary);
  text-transform: uppercase;
  letter-spacing: 0.5px;
  margin: 0;
}

.section-icon { font-size: 16px; }
.device-count { font-size: 11px; color: var(--text-tertiary); background: rgba(255,255,255,0.1); padding: 2px 6px; border-radius: var(--radius-full); }

.resource-list {
  display: flex;
  flex-direction: column;
  gap: var(--spacing-xs);
  max-height: 200px;
  overflow-y: auto;
}

.resource-item {
  display: flex;
  align-items: center;
  gap: var(--spacing-sm);
  padding: var(--spacing-sm);
  background: rgba(0,0,0,0.2);
  border: 1px solid var(--border-primary);
  border-radius: var(--radius-sm);
  cursor: pointer;
  transition: all var(--duration-fast);
}

.resource-item:hover {
  border-color: var(--accent-primary);
  background: rgba(59,130,246,0.1);
}

.resource-item.selected {
  border-color: var(--accent-primary);
  background: rgba(59,130,246,0.2);
}

.resource-icon {
  width: 32px;
  height: 32px;
  display: flex;
  align-items: center;
  justify-content: center;
  border-radius: var(--radius-sm);
  font-size: 16px;
}

.resource-info { flex: 1; }
.resource-name { font-size: 12px; font-weight: 500; color: var(--text-primary); }
.resource-type { font-size: 10px; color: var(--text-tertiary); }
.resource-check {
  width: 20px;
  height: 20px;
  display: flex;
  align-items: center;
  justify-content: center;
  background: var(--accent-primary);
  border-radius: 50%;
  color: white;
  font-size: 12px;
}

/* 中间面板 */
.center-panel {
  flex: 1;
  display: flex;
  flex-direction: column;
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
.toolbar-center { display: flex; flex-direction: column; align-items: center; }
.canvas-title { font-size: 14px; font-weight: 600; color: var(--text-primary); }
.canvas-hint { font-size: 11px; color: var(--text-tertiary); }

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
  pointer-events: none;
}

.connection-line {
  transition: stroke var(--duration-fast);
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
.node-app { border-left: 4px solid #22c55d; }

.node-icon {
  width: 32px;
  height: 32px;
  display: flex;
  align-items: center;
  justify-content: center;
  border-radius: var(--radius-sm);
  font-size: 18px;
  margin: 0 auto var(--spacing-xs);
}

.node-label {
  font-size: 11px;
  font-weight: 600;
  color: var(--text-primary);
  text-align: center;
  white-space: nowrap;
  overflow: hidden;
  text-overflow: ellipsis;
}

.node-type {
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
.port-top { top: -6px; left: 50%; transform: translateX(-50%); }
.port-bottom { bottom: -6px; left: 50%; transform: translateX(-50%); }

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

/* 右侧面板 */
.right-panel {
  width: 280px;
  background: var(--bg-card);
  border-left: 1px solid var(--border-primary);
  display: flex;
  flex-direction: column;
  overflow-y: auto;
}

.property-section {
  padding: var(--spacing-md);
  border-bottom: 1px solid var(--border-primary);
}

.property-content {
  display: flex;
  flex-direction: column;
  gap: var(--spacing-sm);
}

.property-item {
  display: flex;
  align-items: center;
  justify-content: space-between;
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

.stat-item {
  display: flex;
  justify-content: space-between;
  padding: var(--spacing-xs) 0;
}

.stat-label { font-size: 12px; color: var(--text-secondary); }
.stat-value { font-size: 12px; font-weight: 600; color: var(--text-primary); font-family: var(--font-mono); }

.quick-actions {
  display: flex;
  flex-direction: column;
  gap: var(--spacing-xs);
}

.mt-4 { margin-top: var(--spacing-md); }

/* 模态框 */
.modal-overlay { position: fixed; inset: 0; background: rgba(0,0,0,0.7); backdrop-filter: blur(8px); display: flex; align-items: center; justify-content: center; z-index: var(--z-modal); }
.modal-content { background: var(--bg-card); border: 1px solid var(--border-primary); border-radius: var(--radius-xl); width: 90%; max-width: 550px; }
.modal-header { display: flex; justify-content: space-between; align-items: center; padding: var(--spacing-lg); border-bottom: 1px solid var(--border-primary); }
.modal-header h3 { font-size: 18px; font-weight: 600; color: var(--text-primary); margin: 0; }
.modal-close { width: 32px; height: 32px; display: flex; align-items: center; justify-content: center; background: transparent; border: none; color: var(--text-secondary); font-size: 20px; cursor: pointer; }
.modal-body { padding: var(--spacing-lg); }
.modal-footer { display: flex; justify-content: flex-end; gap: var(--spacing-sm); padding: var(--spacing-lg); border-top: 1px solid var(--border-primary); }

.app-preview { background: rgba(59,130,246,0.1); border: 1px solid rgba(59,130,246,0.2); border-radius: var(--radius-md); padding: var(--spacing-md); margin-bottom: var(--spacing-lg); }
.preview-label { font-size: 12px; color: var(--text-secondary); margin-bottom: var(--spacing-sm); }
.preview-stats { display: flex; gap: var(--spacing-lg); font-size: 13px; color: var(--text-primary); }

.form-grid { display: grid; grid-template-columns: repeat(2, 1fr); gap: var(--spacing-md); }
.form-group { display: flex; flex-direction: column; gap: 6px; }
.form-group.full-width { grid-column: span 2; }
.form-group label { font-size: 13px; color: var(--text-secondary); }
.form-group input, .form-group select, .form-group textarea { padding: 10px 12px; background: var(--bg-primary); border: 1px solid var(--border-primary); border-radius: var(--radius-md); color: var(--text-primary); font-size: 14px; outline: none; }
.form-group input:focus, .form-group select:focus, .form-group textarea:focus { border-color: var(--accent-primary); }

@media (max-width: 1200px) {
  .left-panel { width: 240px; }
  .right-panel { width: 240px; }
}
</style>
