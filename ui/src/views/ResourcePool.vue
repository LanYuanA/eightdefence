<template>
  <div class="resource-root">
    <ParticleBackground :particle-count="30" color="#06b6d4" :opacity="0.3" />
    <AppNavbar title="资源池管理" subtitle="Resource Pool Management" :menu-items="menuItems" />

    <main class="resource-main">
      <!-- 资源概览 -->
      <section class="overview-section">
        <div class="section-header">
          <h2 class="section-title">
            <svg width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M21 16V8a2 2 0 0 0-1-1.73l-7-4a2 2 0 0 0-2 0l-7 4A2 2 0 0 0 3 8v8a2 2 0 0 0 1 1.73l7 4a2 2 0 0 0 2 0l7-4A2 2 0 0 0 21 16z"/></svg>
            资源池概览
          </h2>
          <CyberButton variant="primary" size="sm" @click="showAddResource = true">
            <svg width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><line x1="12" y1="5" x2="12" y2="19"/><line x1="5" y1="12" x2="19" y2="12"/></svg>
            添加资源
          </CyberButton>
        </div>

        <div class="stats-grid">
          <div class="stat-card" :class="{ active: activeTab === 'device' }" @click="activeTab = 'device'">
            <div class="stat-icon" style="background: rgba(59,130,246,0.2); color: #3b82f6;">📡</div>
            <div class="stat-info"><div class="stat-value">{{ deviceResources.length }}</div><div class="stat-label">设备资源</div></div>
          </div>
          <div class="stat-card" :class="{ active: activeTab === 'data' }" @click="activeTab = 'data'">
            <div class="stat-icon" style="background: rgba(34,197,94,0.2); color: #22c55d;">📊</div>
            <div class="stat-info"><div class="stat-value">{{ dataResources.length }}</div><div class="stat-label">数据资源</div></div>
          </div>
          <div class="stat-card" :class="{ active: activeTab === 'service' }" @click="activeTab = 'service'">
            <div class="stat-icon" style="background: rgba(139,92,246,0.2); color: #8b5cf6;">⚡</div>
            <div class="stat-info"><div class="stat-value">{{ serviceResources.length }}</div><div class="stat-label">服务资源</div></div>
          </div>
          <div class="stat-card" :class="{ active: activeTab === 'ui' }" @click="activeTab = 'ui'">
            <div class="stat-icon" style="background: rgba(245,158,11,0.2); color: #f59e0b;">🎨</div>
            <div class="stat-info"><div class="stat-value">{{ uiResources.length }}</div><div class="stat-label">UI组件</div></div>
          </div>
        </div>
      </section>

      <!-- 资源列表 -->
      <section class="resources-section">
        <BaseCard :title="tabTitle" :subtitle="tabSubtitle" icon-color="cyan">
          <template #header>
            <div class="flex items-center justify-between w-full">
              <div class="flex items-center gap-3">
                <div class="card-icon icon-cyan">{{ tabIcon }}</div>
                <div><h3 class="card-title">{{ tabTitle }}</h3><p class="card-subtitle">{{ tabSubtitle }}</p></div>
              </div>
              <div class="search-box">
                <svg width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><circle cx="11" cy="11" r="8"/><line x1="21" y1="21" x2="16.65" y2="16.65"/></svg>
                <input v-model="searchQuery" type="text" placeholder="搜索资源..." class="search-input" />
              </div>
            </div>
          </template>

          <div class="resource-grid">
            <div v-for="resource in filteredResources" :key="resource.id" class="resource-item" :class="{ active: selectedResource?.id === resource.id }" @click="selectResource(resource)">
              <div class="resource-icon" :style="{ background: resource.color + '20', color: resource.color }">
                {{ resource.icon }}
              </div>
              <div class="resource-info">
                <div class="resource-name">{{ resource.name }}</div>
                <div class="resource-desc">{{ resource.description }}</div>
                <div class="resource-meta">
                  <span class="meta-tag">{{ resource.category }}</span>
                  <span class="meta-status" :class="resource.status">{{ resource.status === 'active' ? '启用' : '禁用' }}</span>
                </div>
              </div>
              <div class="resource-actions">
                <button class="action-btn" @click.stop="editResource(resource)">编辑</button>
                <button class="action-btn" @click.stop="deleteResource(resource)">删除</button>
              </div>
            </div>
          </div>
        </BaseCard>
      </section>
    </main>

    <!-- 添加资源弹窗 -->
    <Transition name="modal">
      <div v-if="showAddResource" class="modal-overlay" @click.self="showAddResource = false">
        <div class="modal-content">
          <div class="modal-header">
            <h3>添加资源</h3>
            <button class="modal-close" @click="showAddResource = false">×</button>
          </div>
          <div class="modal-body">
            <div class="form-grid">
              <div class="form-group"><label>资源名称 *</label><input v-model="newResource.name" placeholder="输入资源名称" /></div>
              <div class="form-group"><label>资源类型</label><select v-model="newResource.type"><option value="device">设备资源</option><option value="data">数据资源</option><option value="service">服务资源</option><option value="ui">UI组件</option></select></div>
              <div class="form-group"><label>分类</label><input v-model="newResource.category" placeholder="输入分类" /></div>
              <div class="form-group"><label>图标</label><input v-model="newResource.icon" placeholder="输入emoji图标" /></div>
              <div class="form-group full-width"><label>描述</label><textarea v-model="newResource.description" placeholder="输入资源描述" rows="3"></textarea></div>
            </div>
          </div>
          <div class="modal-footer">
            <CyberButton variant="ghost" @click="showAddResource = false">取消</CyberButton>
            <CyberButton variant="primary" @click="addResource">添加</CyberButton>
          </div>
        </div>
      </div>
    </Transition>
  </div>
</template>

<script setup lang="ts">
import { ref, reactive, computed } from 'vue'
import { ElMessage } from 'element-plus'
import ParticleBackground from '../components/ParticleBackground.vue'
import AppNavbar from '../components/AppNavbar.vue'
import BaseCard from '../components/BaseCard.vue'
import CyberButton from '../components/CyberButton.vue'

const menuItems = [
  { path: '/', label: '主控台' },
  { path: '/workflow', label: '流程编排' },
  { path: '/atomic-services', label: '原子服务' },
  { path: '/devices', label: '设备管理' }
]

const activeTab = ref('device')
const searchQuery = ref('')
const showAddResource = ref(false)
const selectedResource = ref<any>(null)

interface Resource {
  id: string
  name: string
  type: 'device' | 'data' | 'service' | 'ui'
  category: string
  description: string
  icon: string
  color: string
  status: 'active' | 'inactive'
}

const resources = reactive<Resource[]>([
  { id: 'res-001', name: '温度传感器', type: 'device', category: '传感器', description: '云测仪SD123温度传感器', icon: '🌡️', color: '#3b82f6', status: 'active' },
  { id: 'res-002', name: '湿度传感器', type: 'device', category: '传感器', description: '云测仪SD123湿度传感器', icon: '💧', color: '#06b6d4', status: 'active' },
  { id: 'res-003', name: '烟雾报警器', type: 'device', category: '报警器', description: '烟雾检测报警设备', icon: '🔥', color: '#ef4444', status: 'active' },
  { id: 'res-004', name: '温度数据', type: 'data', category: '环境数据', description: '温度实时数据流', icon: '📊', color: '#22c55d', status: 'active' },
  { id: 'res-005', name: '湿度数据', type: 'data', category: '环境数据', description: '湿度实时数据流', icon: '📈', color: '#8b5cf6', status: 'active' },
  { id: 'res-006', name: '读取设备数据', type: 'service', category: '基础服务', description: '从设备读取数据的原子服务', icon: '📖', color: '#f59e0b', status: 'active' },
  { id: 'res-007', name: '数据图表组件', type: 'ui', category: '图表组件', description: '实时数据折线图组件', icon: '📉', color: '#ec4899', status: 'active' },
  { id: 'res-008', name: '仪表盘组件', type: 'ui', category: '仪表组件', description: '数据仪表盘组件', icon: '🎯', color: '#14b8a6', status: 'active' }
])

const newResource = reactive({
  name: '',
  type: 'device' as 'device' | 'data' | 'service' | 'ui',
  category: '',
  description: '',
  icon: '',
  color: '#3b82f6'
})

const deviceResources = computed(() => resources.filter(r => r.type === 'device'))
const dataResources = computed(() => resources.filter(r => r.type === 'data'))
const serviceResources = computed(() => resources.filter(r => r.type === 'service'))
const uiResources = computed(() => resources.filter(r => r.type === 'ui'))

const tabTitle = computed(() => ({
  device: '设备资源',
  data: '数据资源',
  service: '服务资源',
  ui: 'UI组件资源'
}[activeTab.value]))

const tabSubtitle = computed(() => ({
  device: '已注册的设备资源',
  data: '可绑定的数据流',
  service: '可用的原子服务',
  ui: '可拖拽的UI组件'
}[activeTab.value]))

const tabIcon = computed(() => ({
  device: '📡',
  data: '📊',
  service: '⚡',
  ui: '🎨'
}[activeTab.value]))

const filteredResources = computed(() => {
  let result = resources.filter(r => r.type === activeTab.value)
  if (searchQuery.value) {
    const q = searchQuery.value.toLowerCase()
    result = result.filter(r => r.name.toLowerCase().includes(q) || r.description.toLowerCase().includes(q))
  }
  return result
})

function selectResource(resource: Resource) {
  selectedResource.value = resource
}

function editResource(resource: Resource) {
  // 打开编辑弹窗（简化为修改名称）
  const newName = prompt('请输入新的资源名称:', resource.name)
  if (newName && newName !== resource.name) {
    resource.name = newName
    ElMessage.success(`资源名称已更新为 "${newName}"`)
  }
}

function deleteResource(resource: Resource) {
  const index = resources.findIndex(r => r.id === resource.id)
  if (index !== -1) {
    resources.splice(index, 1)
    if (selectedResource.value?.id === resource.id) {
      selectedResource.value = null
    }
    ElMessage.success(`资源 "${resource.name}" 已删除`)
  }
}

function addResource() {
  if (!newResource.name) {
    ElMessage.warning('请输入资源名称')
    return
  }
  const resource: Resource = {
    id: `res-${Date.now()}`,
    ...newResource,
    status: 'active'
  }
  resources.push(resource)
  showAddResource.value = false
  ElMessage.success('资源已添加')
  newResource.name = ''
  newResource.category = ''
  newResource.description = ''
  newResource.icon = ''
}
</script>

<style scoped>
.resource-root { min-height: 100vh; background: var(--bg-primary); position: relative; }
.resource-main { position: relative; z-index: 1; padding: var(--spacing-xl); max-width: 1920px; margin: 0 auto; }

.section-header { display: flex; align-items: center; justify-content: space-between; margin-bottom: var(--spacing-lg); }
.section-title { display: flex; align-items: center; gap: var(--spacing-sm); font-size: 20px; font-weight: 600; color: var(--text-primary); margin: 0; }
.section-title svg { color: var(--accent-primary); }

.stats-grid { display: grid; grid-template-columns: repeat(4, 1fr); gap: var(--spacing-lg); margin-bottom: var(--spacing-xl); }
.stat-card { display: flex; align-items: center; gap: var(--spacing-md); padding: var(--spacing-lg); background: var(--bg-card); border: 1px solid var(--border-primary); border-radius: var(--radius-lg); cursor: pointer; transition: all var(--duration-fast); }
.stat-card:hover { border-color: var(--border-accent); }
.stat-card.active { border-color: var(--accent-primary); background: rgba(59,130,246,0.1); }
.stat-icon { width: 48px; height: 48px; display: flex; align-items: center; justify-content: center; border-radius: var(--radius-md); font-size: 24px; }
.stat-value { font-size: 28px; font-weight: 700; color: var(--text-primary); font-family: var(--font-mono); }
.stat-label { font-size: 13px; color: var(--text-secondary); }

.search-box { display: flex; align-items: center; gap: 8px; padding: 8px 12px; background: var(--bg-primary); border: 1px solid var(--border-primary); border-radius: var(--radius-md); }
.search-box svg { color: var(--text-tertiary); }
.search-input { background: transparent; border: none; outline: none; color: var(--text-primary); font-size: 14px; width: 200px; }

.resource-grid { display: grid; grid-template-columns: repeat(auto-fill, minmax(300px, 1fr)); gap: var(--spacing-md); max-height: 500px; overflow-y: auto; }
.resource-item { display: flex; align-items: center; gap: var(--spacing-md); padding: var(--spacing-md); background: rgba(0,0,0,0.2); border: 1px solid var(--border-primary); border-radius: var(--radius-md); cursor: pointer; transition: all var(--duration-fast); }
.resource-item:hover { background: rgba(0,0,0,0.3); border-color: var(--border-accent); }
.resource-item.active { border-color: var(--accent-primary); background: rgba(59,130,246,0.1); }
.resource-icon { width: 48px; height: 48px; display: flex; align-items: center; justify-content: center; border-radius: var(--radius-md); font-size: 24px; }
.resource-info { flex: 1; }
.resource-name { font-size: 14px; font-weight: 500; color: var(--text-primary); }
.resource-desc { font-size: 12px; color: var(--text-secondary); margin-top: 2px; }
.resource-meta { display: flex; gap: 8px; margin-top: 6px; }
.meta-tag { display: inline-flex; padding: 2px 8px; background: rgba(59,130,246,0.2); border: 1px solid rgba(59,130,246,0.3); border-radius: var(--radius-full); font-size: 11px; color: var(--accent-primary); }
.meta-status { display: inline-flex; padding: 2px 8px; border-radius: var(--radius-full); font-size: 11px; }
.meta-status.active { background: rgba(34,197,94,0.2); color: var(--status-success); }
.meta-status.inactive { background: rgba(100,116,139,0.2); color: var(--text-tertiary); }
.resource-actions { display: flex; flex-direction: column; gap: 4px; }
.action-btn { padding: 4px 8px; background: transparent; border: 1px solid var(--border-primary); border-radius: var(--radius-sm); color: var(--text-secondary); font-size: 12px; cursor: pointer; transition: all var(--duration-fast); }
.action-btn:hover { border-color: var(--accent-primary); color: var(--accent-primary); }

.modal-overlay { position: fixed; inset: 0; background: rgba(0,0,0,0.7); backdrop-filter: blur(8px); display: flex; align-items: center; justify-content: center; z-index: var(--z-modal); }
.modal-content { background: var(--bg-card); border: 1px solid var(--border-primary); border-radius: var(--radius-xl); width: 90%; max-width: 600px; }
.modal-header { display: flex; justify-content: space-between; align-items: center; padding: var(--spacing-lg); border-bottom: 1px solid var(--border-primary); }
.modal-header h3 { font-size: 18px; font-weight: 600; color: var(--text-primary); margin: 0; }
.modal-close { width: 32px; height: 32px; display: flex; align-items: center; justify-content: center; background: transparent; border: none; color: var(--text-secondary); font-size: 20px; cursor: pointer; }
.modal-body { padding: var(--spacing-lg); }
.modal-footer { display: flex; justify-content: flex-end; gap: var(--spacing-sm); padding: var(--spacing-lg); border-top: 1px solid var(--border-primary); }

.form-grid { display: grid; grid-template-columns: repeat(2, 1fr); gap: var(--spacing-md); }
.form-group { display: flex; flex-direction: column; gap: 6px; }
.form-group.full-width { grid-column: span 2; }
.form-group label { font-size: 13px; color: var(--text-secondary); }
.form-group input, .form-group select, .form-group textarea { padding: 10px 12px; background: var(--bg-primary); border: 1px solid var(--border-primary); border-radius: var(--radius-md); color: var(--text-primary); font-size: 14px; outline: none; }
.form-group input:focus, .form-group select:focus, .form-group textarea:focus { border-color: var(--accent-primary); }

@media (max-width: 1200px) { .stats-grid { grid-template-columns: repeat(2, 1fr); } }
@media (max-width: 768px) { .resource-main { padding: var(--spacing-md); } .stats-grid { grid-template-columns: 1fr; } .form-grid { grid-template-columns: 1fr; } .form-group.full-width { grid-column: span 1; } }
</style>
