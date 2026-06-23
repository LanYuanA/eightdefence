<template>
  <div class="workflow-root">
    <AppNavbar title="流程编排设计器" subtitle="Workflow Designer" :menu-items="menuItems" />

    <main class="workflow-main">
      <!-- 流程列表 -->
      <div class="workflow-sidebar">
        <div class="sidebar-header">
          <h3 class="sidebar-title">流程列表</h3>
          <CyberButton variant="primary" size="sm" @click="createNewWorkflow">
            <svg width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><line x1="12" y1="5" x2="12" y2="19"/><line x1="5" y1="12" x2="19" y2="12"/></svg>
            新建
          </CyberButton>
        </div>

        <div class="workflow-list">
          <div
            v-for="wf in workflows"
            :key="wf.id"
            class="workflow-item"
            :class="{ active: currentWorkflow?.id === wf.id }"
            @click="selectWorkflow(wf)"
          >
            <div class="wf-icon">
              <svg width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M22 12h-4l-3 9L9 3l-3 9H2"/></svg>
            </div>
            <div class="wf-info">
              <div class="wf-name">{{ wf.name }}</div>
              <div class="wf-meta">{{ wf.nodes.length }} 个节点 · {{ wf.status }}</div>
            </div>
            <div class="wf-status" :class="wf.status">
              <span class="status-dot"></span>
            </div>
          </div>
        </div>

        <!-- 快捷操作 -->
        <div class="sidebar-section">
          <div class="section-title">快捷操作</div>
          <CyberButton variant="ghost" size="sm" @click="showResourcePoolImport = true" block>
            <svg width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M21 16V8a2 2 0 0 0-1-1.73l-7-4a2 2 0 0 0-2 0l-7 4A2 2 0 0 0 3 8v8a2 2 0 0 0 1 1.73l7 4a2 2 0 0 0 2 0l7-4A2 2 0 0 0 21 16z"/></svg>
            导入资源池配置
          </CyberButton>
          <CyberButton variant="success" size="sm" @click="exportAsApp" :disabled="!currentWorkflow || currentWorkflow.nodes.length === 0" block>
            <svg width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><rect x="3" y="3" width="18" height="18" rx="2"/><path d="M12 8v8m-4-4h8"/></svg>
            导出为应用
          </CyberButton>
        </div>
      </div>

      <!-- 设计器主区域 -->
      <div class="workflow-content">
        <SimpleFlowDesigner
          v-if="currentWorkflow"
          :initial-name="currentWorkflow.name"
          :initial-nodes="currentWorkflow.nodes"
          :initial-connections="currentWorkflow.connections"
          @save="handleSaveWorkflow"
          @run="handleRunWorkflow"
        />
        <div v-else class="empty-state">
          <div class="empty-icon">
            <svg width="80" height="80" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1"><path d="M22 12h-4l-3 9L9 3l-3 9H2"/></svg>
          </div>
          <div class="empty-text">选择或创建一个流程开始设计</div>
          <CyberButton variant="primary" @click="createNewWorkflow">创建新流程</CyberButton>
        </div>
      </div>
    </main>

    <!-- 资源池导入弹窗 -->
    <Transition name="modal">
      <div v-if="showResourcePoolImport" class="modal-overlay" @click.self="showResourcePoolImport = false">
        <div class="modal-content">
          <div class="modal-header">
            <h3>从资源池导入配置</h3>
            <button class="modal-close" @click="showResourcePoolImport = false">×</button>
          </div>
          <div class="modal-body">
            <div class="import-options">
              <div class="option-item" @click="importFromResourcePool('sensor')">
                <span class="option-icon">🌡️</span>
                <span class="option-label">传感器设备配置</span>
                <span class="option-desc">导入传感器设备及其关联服务</span>
              </div>
              <div class="option-item" @click="importFromResourcePool('security')">
                <span class="option-icon">🛡️</span>
                <span class="option-label">安防系统配置</span>
                <span class="option-desc">导入安防设备和服务配置</span>
              </div>
              <div class="option-item" @click="importFromResourcePool('fire')">
                <span class="option-icon">🔥</span>
                <span class="option-label">消防系统配置</span>
                <span class="option-desc">导入消防设备和服务配置</span>
              </div>
              <div class="option-item" @click="importFromResourcePool('environment')">
                <span class="option-icon">🌿</span>
                <span class="option-label">环境监测配置</span>
                <span class="option-desc">导入环境监测设备和服务配置</span>
              </div>
            </div>
          </div>
        </div>
      </div>
    </Transition>
  </div>
</template>

<script setup lang="ts">
import { ref, reactive, onMounted } from 'vue'
import { ElMessage } from 'element-plus'
import AppNavbar from '../components/AppNavbar.vue'
import CyberButton from '../components/CyberButton.vue'
import SimpleFlowDesigner from '../components/flow/SimpleFlowDesigner.vue'

const menuItems = [
  { path: '/', label: '主控台' },
  { path: '/security', label: '安防系统' },
  { path: '/environment', label: '环境监测' },
  { path: '/fire', label: '消防系统' },
  { path: '/devices', label: '设备管理' }
]

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

interface Workflow {
  id: string
  name: string
  status: 'draft' | 'active' | 'inactive'
  nodes: FlowNode[]
  connections: FlowConnection[]
}

const STORAGE_KEY = 'workflow-designer-data'

const currentWorkflow = ref<Workflow | null>(null)
const showResourcePoolImport = ref(false)

// 默认流程数据
const defaultWorkflows: Workflow[] = [
  {
    id: 'wf-001',
    name: '环境监测流程',
    status: 'active',
    nodes: [
      { id: 'node-1', type: 'sensor', typeLabel: '传感器', label: '温度传感器', icon: '🌡️', color: '#3b82f6', x: 100, y: 100 },
      { id: 'node-2', type: 'service', typeLabel: '服务', label: '数据采集', icon: '📥', color: '#8b5cf6', x: 300, y: 100 },
      { id: 'node-3', type: 'logic', typeLabel: '逻辑', label: '条件判断', icon: '❓', color: '#f59e0b', x: 500, y: 100 },
      { id: 'node-4', type: 'action', typeLabel: '动作', label: '发送通知', icon: '📧', color: '#ef4444', x: 700, y: 50 },
      { id: 'node-5', type: 'controller', typeLabel: '控制器', label: '空调控制', icon: '❄️', color: '#22c55d', x: 700, y: 150 }
    ],
    connections: [
      { id: 'conn-1', from: 'node-1', to: 'node-2', active: true },
      { id: 'conn-2', from: 'node-2', to: 'node-3', active: true },
      { id: 'conn-3', from: 'node-3', to: 'node-4', active: true },
      { id: 'conn-4', from: 'node-3', to: 'node-5', active: true }
    ]
  },
  {
    id: 'wf-002',
    name: '安防联动流程',
    status: 'draft',
    nodes: [],
    connections: []
  },
  {
    id: 'wf-003',
    name: '消防应急流程',
    status: 'active',
    nodes: [],
    connections: []
  }
]

const workflows = reactive<Workflow[]>([])

// 从localStorage加载流程
function loadWorkflows() {
  try {
    const saved = localStorage.getItem(STORAGE_KEY)
    if (saved) {
      const parsed = JSON.parse(saved)
      workflows.splice(0, workflows.length, ...parsed)
    } else {
      // 使用默认数据
      workflows.splice(0, workflows.length, ...defaultWorkflows)
      saveWorkflows()
    }
  } catch (e) {
    console.error('加载流程数据失败:', e)
    workflows.splice(0, workflows.length, ...defaultWorkflows)
  }
}

// 保存流程到localStorage
function saveWorkflows() {
  try {
    localStorage.setItem(STORAGE_KEY, JSON.stringify(workflows))
  } catch (e) {
    console.error('保存流程数据失败:', e)
  }
}

// 选择流程
function selectWorkflow(workflow: Workflow) {
  currentWorkflow.value = workflow
}

// 创建新流程
function createNewWorkflow() {
  const newWorkflow: Workflow = {
    id: `wf-${Date.now()}`,
    name: '新建流程',
    status: 'draft',
    nodes: [],
    connections: []
  }
  workflows.push(newWorkflow)
  currentWorkflow.value = newWorkflow
  saveWorkflows() // 保存到localStorage
  ElMessage.success('新流程已创建')
}

// 保存流程
function handleSaveWorkflow(workflow: { name: string; nodes: FlowNode[]; connections: FlowConnection[] }) {
  if (currentWorkflow.value) {
    currentWorkflow.value.name = workflow.name
    currentWorkflow.value.nodes = workflow.nodes
    currentWorkflow.value.connections = workflow.connections
    saveWorkflows() // 保存到localStorage
    ElMessage.success('流程已保存')
  }
}

// 运行流程
function handleRunWorkflow(workflow: { name: string; nodes: FlowNode[]; connections: FlowConnection[] }) {
  ElMessage.success(`流程 "${workflow.name}" 开始运行`)
}

// 导出流程为自定义应用
function exportAsApp() {
  if (!currentWorkflow.value || currentWorkflow.value.nodes.length === 0) {
    ElMessage.warning('请先设计流程，至少需要一个节点')
    return
  }

  const wf = currentWorkflow.value
  const deviceNodes = wf.nodes.filter(n => n.type === 'device' || n.type === 'sensor' || n.type === 'controller' || n.type === 'alarm')
  const serviceNodes = wf.nodes.filter(n => n.type === 'service' || n.type === 'collect' || n.type === 'process')

  const app = {
    id: `app-${Date.now()}`,
    name: wf.name,
    description: `由流程编排"${wf.name}"生成的自定义应用`,
    type: 'custom',
    icon: '🔀',
    devices: deviceNodes.map(d => d.id),
    services: serviceNodes.map(s => s.id),
    nodes: JSON.parse(JSON.stringify(wf.nodes)),
    connections: JSON.parse(JSON.stringify(wf.connections)),
    createdAt: new Date().toLocaleString('zh-CN')
  }

  // 保存到localStorage
  const APP_STORAGE_KEY = 'custom-apps'
  let apps = []
  try {
    const saved = localStorage.getItem(APP_STORAGE_KEY)
    if (saved) apps = JSON.parse(saved)
  } catch { /* ignore */ }

  apps.push(app)
  localStorage.setItem(APP_STORAGE_KEY, JSON.stringify(apps))

  ElMessage.success(`流程 "${wf.name}" 已导出为应用！可在主控台查看和运行`)
}

// 组件挂载时加载数据
onMounted(() => {
  loadWorkflows()
})

// 从资源池导入
function importFromResourcePool(type: string) {
  const configs: Record<string, any> = {
    sensor: {
      name: '传感器监测流程',
      nodes: [
        { id: 'sensor-1', type: 'sensor', typeLabel: '传感器', label: '温度传感器', icon: '🌡️', color: '#3b82f6', x: 100, y: 100 },
        { id: 'sensor-2', type: 'sensor', typeLabel: '传感器', label: '湿度传感器', icon: '💧', color: '#06b6d4', x: 100, y: 200 },
        { id: 'service-1', type: 'service', typeLabel: '服务', label: '数据采集', icon: '📥', color: '#8b5cf6', x: 300, y: 150 },
        { id: 'service-2', type: 'service', typeLabel: '服务', label: '数据处理', icon: '⚙️', color: '#22c55d', x: 500, y: 150 }
      ],
      connections: [
        { id: 'conn-1', from: 'sensor-1', to: 'service-1', active: true },
        { id: 'conn-2', from: 'sensor-2', to: 'service-1', active: true },
        { id: 'conn-3', from: 'service-1', to: 'service-2', active: true }
      ]
    },
    security: {
      name: '安防监控流程',
      nodes: [
        { id: 'sensor-1', type: 'sensor', typeLabel: '传感器', label: '红外探测', icon: '👤', color: '#8b5cf6', x: 100, y: 100 },
        { id: 'sensor-2', type: 'sensor', typeLabel: '传感器', label: '水浸传感', icon: '💧', color: '#06b6d4', x: 100, y: 200 },
        { id: 'service-1', type: 'service', typeLabel: '服务', label: '报警判断', icon: '🔔', color: '#ef4444', x: 300, y: 150 },
        { id: 'action-1', type: 'action', typeLabel: '动作', label: '声光报警', icon: '📢', color: '#f59e0b', x: 500, y: 100 },
        { id: 'action-2', type: 'action', typeLabel: '动作', label: '通知中心', icon: '📧', color: '#8b5cf6', x: 500, y: 200 }
      ],
      connections: [
        { id: 'conn-1', from: 'sensor-1', to: 'service-1', active: true },
        { id: 'conn-2', from: 'sensor-2', to: 'service-1', active: true },
        { id: 'conn-3', from: 'service-1', to: 'action-1', active: true },
        { id: 'conn-4', from: 'service-1', to: 'action-2', active: true }
      ]
    },
    fire: {
      name: '消防应急流程',
      nodes: [
        { id: 'sensor-1', type: 'sensor', typeLabel: '传感器', label: '烟雾报警', icon: '🔥', color: '#ef4444', x: 100, y: 100 },
        { id: 'sensor-2', type: 'sensor', typeLabel: '传感器', label: '温度传感', icon: '🌡️', color: '#f59e0b', x: 100, y: 200 },
        { id: 'service-1', type: 'service', typeLabel: '服务', label: '火灾判断', icon: '🔔', color: '#ef4444', x: 300, y: 150 },
        { id: 'action-1', type: 'action', typeLabel: '动作', label: '喷淋控制', icon: '💦', color: '#06b6d4', x: 500, y: 100 },
        { id: 'action-2', type: 'action', typeLabel: '动作', label: '排烟风机', icon: '🌀', color: '#22c55d', x: 500, y: 200 }
      ],
      connections: [
        { id: 'conn-1', from: 'sensor-1', to: 'service-1', active: true },
        { id: 'conn-2', from: 'sensor-2', to: 'service-1', active: true },
        { id: 'conn-3', from: 'service-1', to: 'action-1', active: true },
        { id: 'conn-4', from: 'service-1', to: 'action-2', active: true }
      ]
    },
    environment: {
      name: '环境监测流程',
      nodes: [
        { id: 'sensor-1', type: 'sensor', typeLabel: '传感器', label: 'PM2.5传感', icon: '💨', color: '#f59e0b', x: 100, y: 100 },
        { id: 'sensor-2', type: 'sensor', typeLabel: '传感器', label: 'CO2传感', icon: '☁️', color: '#8b5cf6', x: 100, y: 200 },
        { id: 'service-1', type: 'service', typeLabel: '服务', label: '数据融合', icon: '🔀', color: '#22c55d', x: 300, y: 150 },
        { id: 'service-2', type: 'service', typeLabel: '服务', label: '趋势分析', icon: '📈', color: '#3b82f6', x: 500, y: 150 }
      ],
      connections: [
        { id: 'conn-1', from: 'sensor-1', to: 'service-1', active: true },
        { id: 'conn-2', from: 'sensor-2', to: 'service-1', active: true },
        { id: 'conn-3', from: 'service-1', to: 'service-2', active: true }
      ]
    }
  }

  const config = configs[type]
  if (config) {
    const newWorkflow: Workflow = {
      id: `wf-${Date.now()}`,
      name: config.name,
      status: 'draft',
      nodes: config.nodes,
      connections: config.connections
    }
    workflows.push(newWorkflow)
    currentWorkflow.value = newWorkflow
    showResourcePoolImport.value = false
    saveWorkflows() // 保存到localStorage
    ElMessage.success(`已导入 ${config.name} 配置`)
  }
}
</script>

<style scoped>
.workflow-root { min-height: 100vh; background: var(--bg-primary); }
.workflow-main { display: flex; height: calc(100vh - 60px); }

.workflow-sidebar {
  width: 280px;
  background: var(--bg-card);
  border-right: 1px solid var(--border-primary);
  display: flex;
  flex-direction: column;
}

.sidebar-header {
  display: flex;
  align-items: center;
  justify-content: space-between;
  padding: var(--spacing-md);
  border-bottom: 1px solid var(--border-primary);
}

.sidebar-title {
  font-size: 14px;
  font-weight: 600;
  color: var(--text-primary);
  margin: 0;
}

.workflow-list {
  flex: 1;
  overflow-y: auto;
  padding: var(--spacing-sm);
}

.workflow-item {
  display: flex;
  align-items: center;
  gap: var(--spacing-sm);
  padding: var(--spacing-md);
  margin-bottom: var(--spacing-xs);
  background: transparent;
  border: 1px solid transparent;
  border-radius: var(--radius-md);
  cursor: pointer;
  transition: all var(--duration-fast);
}

.workflow-item:hover {
  background: rgba(255, 255, 255, 0.03);
  border-color: var(--border-primary);
}

.workflow-item.active {
  background: rgba(59, 130, 246, 0.1);
  border-color: var(--accent-primary);
}

.wf-icon {
  width: 36px;
  height: 36px;
  display: flex;
  align-items: center;
  justify-content: center;
  background: rgba(59, 130, 246, 0.2);
  border-radius: var(--radius-md);
  color: var(--accent-primary);
}

.wf-info { flex: 1; }
.wf-name { font-size: 13px; font-weight: 500; color: var(--text-primary); }
.wf-meta { font-size: 11px; color: var(--text-tertiary); margin-top: 2px; }
.wf-status { display: flex; align-items: center; }
.status-dot { width: 8px; height: 8px; border-radius: 50%; }
.wf-status.active .status-dot { background: var(--status-success); box-shadow: 0 0 8px var(--status-success); }
.wf-status.draft .status-dot { background: var(--text-tertiary); }
.wf-status.inactive .status-dot { background: var(--status-warning); }

.sidebar-section {
  padding: var(--spacing-md);
  border-top: 1px solid var(--border-primary);
}

.section-title {
  font-size: 12px;
  font-weight: 600;
  color: var(--text-secondary);
  text-transform: uppercase;
  letter-spacing: 0.5px;
  margin-bottom: var(--spacing-sm);
}

.workflow-content { flex: 1; overflow: hidden; }

.empty-state {
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: center;
  height: 100%;
  color: var(--text-tertiary);
}

.empty-icon { margin-bottom: var(--spacing-lg); opacity: 0.5; }
.empty-text { font-size: 16px; margin-bottom: var(--spacing-lg); }

/* 模态框 */
.modal-overlay { position: fixed; inset: 0; background: rgba(0,0,0,0.7); backdrop-filter: blur(8px); display: flex; align-items: center; justify-content: center; z-index: var(--z-modal); }
.modal-content { background: var(--bg-card); border: 1px solid var(--border-primary); border-radius: var(--radius-xl); width: 90%; max-width: 500px; }
.modal-header { display: flex; justify-content: space-between; align-items: center; padding: var(--spacing-lg); border-bottom: 1px solid var(--border-primary); }
.modal-header h3 { font-size: 18px; font-weight: 600; color: var(--text-primary); margin: 0; }
.modal-close { width: 32px; height: 32px; display: flex; align-items: center; justify-content: center; background: transparent; border: none; color: var(--text-secondary); font-size: 20px; cursor: pointer; }
.modal-body { padding: var(--spacing-lg); }

.import-options {
  display: flex;
  flex-direction: column;
  gap: var(--spacing-sm);
}

.option-item {
  display: flex;
  align-items: center;
  gap: var(--spacing-md);
  padding: var(--spacing-md);
  background: rgba(0,0,0,0.2);
  border: 1px solid var(--border-primary);
  border-radius: var(--radius-md);
  cursor: pointer;
  transition: all var(--duration-fast);
}

.option-item:hover {
  border-color: var(--accent-primary);
  background: rgba(59,130,246,0.1);
}

.option-icon { font-size: 24px; }
.option-label { font-size: 14px; font-weight: 500; color: var(--text-primary); flex: 1; }
.option-desc { font-size: 11px; color: var(--text-tertiary); }

@media (max-width: 768px) { .workflow-sidebar { width: 200px; } }
</style>
