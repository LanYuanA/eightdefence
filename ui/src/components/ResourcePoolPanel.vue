<template>
  <div class="resource-pool-panel">
    <!-- 面板头部 -->
    <div class="panel-header">
      <div class="flex items-center gap-3">
        <div class="panel-icon">
          <svg width="20" height="20" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
            <path d="M21 16V8a2 2 0 0 0-1-1.73l-7-4a2 2 0 0 0-2 0l-7 4A2 2 0 0 0 3 8v8a2 2 0 0 0 1 1.73l7 4a2 2 0 0 0 2 0l7-4A2 2 0 0 0 21 16z"/>
            <polyline points="3.27 6.96 12 12.01 20.73 6.96"/>
            <line x1="12" y1="22.08" x2="12" y2="12"/>
          </svg>
        </div>
        <div>
          <h3 class="panel-title">数据资源池</h3>
          <p class="panel-subtitle">管理所有可用数据源</p>
        </div>
      </div>
      <button class="panel-close" @click="$emit('close')">
        <svg width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
          <line x1="18" y1="6" x2="6" y2="18"/>
          <line x1="6" y1="6" x2="18" y2="18"/>
        </svg>
      </button>
    </div>

    <!-- 搜索框 -->
    <div class="search-box">
      <svg width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" class="search-icon">
        <circle cx="11" cy="11" r="8"/>
        <line x1="21" y1="21" x2="16.65" y2="16.65"/>
      </svg>
      <input
        v-model="searchQuery"
        type="text"
        placeholder="搜索数据源..."
        class="search-input"
      />
    </div>

    <!-- 数据源分类 -->
    <div class="category-tabs">
      <button
        v-for="category in categories"
        :key="category.id"
        class="category-tab"
        :class="{ active: activeCategory === category.id }"
        @click="activeCategory = category.id"
      >
        <span class="category-count">{{ category.count }}</span>
        {{ category.name }}
      </button>
    </div>

    <!-- 数据源列表 -->
    <div class="data-source-list custom-scrollbar">
      <div
        v-for="source in filteredSources"
        :key="source.id"
        class="data-source-item"
        :class="{ 'is-selected': selectedSources.includes(source.id) }"
        draggable="true"
        @dragstart="handleDragStart($event, source)"
        @click="toggleSource(source)"
      >
        <div class="source-icon" :class="`icon-${source.color}`">
          <svg width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" v-html="source.icon" />
        </div>
        <div class="source-info">
          <div class="source-name">{{ source.name }}</div>
          <div class="source-meta">
            <span class="source-device">{{ source.deviceName }}</span>
            <span class="source-unit">{{ source.unit }}</span>
          </div>
        </div>
        <div class="source-value">
          <span class="value-number">{{ source.currentValue }}</span>
          <span class="value-unit">{{ source.unit }}</span>
        </div>
        <div class="source-status">
          <StatusDot :status="source.status" :size="'sm'" :pulse="false" />
        </div>
      </div>

      <!-- 空状态 -->
      <div v-if="filteredSources.length === 0" class="empty-state">
        <svg width="48" height="48" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.5" class="empty-icon">
          <circle cx="12" cy="12" r="10"/>
          <line x1="8" y1="15" x2="16" y2="15"/>
          <line x1="9" y1="9" x2="9.01" y2="9"/>
          <line x1="15" y1="9" x2="15.01" y2="9"/>
        </svg>
        <p class="empty-text">未找到匹配的数据源</p>
      </div>
    </div>

    <!-- 已选数据源 -->
    <div v-if="selectedSources.length > 0" class="selected-section">
      <div class="selected-header">
        <span class="selected-title">已选数据源</span>
        <span class="selected-count">{{ selectedSources.length }}</span>
      </div>
      <div class="selected-list">
        <div
          v-for="sourceId in selectedSources"
          :key="sourceId"
          class="selected-tag"
        >
          <span>{{ getSourceName(sourceId) }}</span>
          <button class="tag-remove" @click="removeSource(sourceId)">
            <svg width="12" height="12" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
              <line x1="18" y1="6" x2="6" y2="18"/>
              <line x1="6" y1="6" x2="18" y2="18"/>
            </svg>
          </button>
        </div>
      </div>
    </div>

    <!-- 操作按钮 -->
    <div class="panel-actions">
      <CyberButton variant="ghost" size="sm" @click="clearSelection">
        清空选择
      </CyberButton>
      <CyberButton variant="primary" size="sm" @click="applySelection">
        应用到面板
      </CyberButton>
    </div>
  </div>
</template>

<script setup lang="ts">
import { ref, computed } from 'vue'
import StatusDot from './StatusDot.vue'
import CyberButton from './CyberButton.vue'

interface DataSource {
  id: string
  name: string
  deviceName: string
  unit: string
  currentValue: number
  status: 'online' | 'offline' | 'warning' | 'danger'
  category: string
  color: string
  icon: string
}

const props = withDefaults(defineProps<{
  dataSources?: DataSource[]
}>(), {
  dataSources: () => []
})

const emit = defineEmits<{
  close: []
  apply: [sources: DataSource[]]
}>()

const searchQuery = ref('')
const activeCategory = ref('all')
const selectedSources = ref<string[]>([])

// 分类列表
const categories = computed(() => {
  const cats = [
    { id: 'all', name: '全部', count: props.dataSources.length },
    { id: 'sensor', name: '传感器', count: props.dataSources.filter(s => s.category === 'sensor').length },
    { id: 'environment', name: '环境', count: props.dataSources.filter(s => s.category === 'environment').length },
    { id: 'security', name: '安防', count: props.dataSources.filter(s => s.category === 'security').length },
    { id: 'fire', name: '消防', count: props.dataSources.filter(s => s.category === 'fire').length }
  ]
  return cats.filter(c => c.count > 0 || c.id === 'all')
})

// 过滤后的数据源
const filteredSources = computed(() => {
  let sources = props.dataSources

  // 按分类过滤
  if (activeCategory.value !== 'all') {
    sources = sources.filter(s => s.category === activeCategory.value)
  }

  // 按搜索词过滤
  if (searchQuery.value) {
    const query = searchQuery.value.toLowerCase()
    sources = sources.filter(s =>
      s.name.toLowerCase().includes(query) ||
      s.deviceName.toLowerCase().includes(query)
    )
  }

  return sources
})

// 获取数据源名称
function getSourceName(id: string): string {
  const source = props.dataSources.find(s => s.id === id)
  return source?.name || ''
}

// 切换数据源选择状态
function toggleSource(source: DataSource) {
  const index = selectedSources.value.indexOf(source.id)
  if (index === -1) {
    selectedSources.value.push(source.id)
  } else {
    selectedSources.value.splice(index, 1)
  }
}

// 移除已选数据源
function removeSource(id: string) {
  const index = selectedSources.value.indexOf(id)
  if (index !== -1) {
    selectedSources.value.splice(index, 1)
  }
}

// 清空选择
function clearSelection() {
  selectedSources.value = []
}

// 应用选择
function applySelection() {
  const selected = props.dataSources.filter(s => selectedSources.value.includes(s.id))
  emit('apply', selected)
}

// 拖拽开始
function handleDragStart(event: DragEvent, source: DataSource) {
  if (event.dataTransfer) {
    event.dataTransfer.setData('application/json', JSON.stringify(source))
    event.dataTransfer.effectAllowed = 'copy'
  }
}
</script>

<style scoped>
.resource-pool-panel {
  background: var(--bg-card);
  border: 1px solid var(--border-primary);
  border-radius: var(--radius-lg);
  display: flex;
  flex-direction: column;
  height: 100%;
  max-height: 600px;
  animation: slideInRight var(--duration-normal) var(--ease-out);
}

/* 面板头部 */
.panel-header {
  display: flex;
  align-items: center;
  justify-content: space-between;
  padding: var(--spacing-lg);
  border-bottom: 1px solid var(--border-primary);
}

.panel-icon {
  width: 40px;
  height: 40px;
  display: flex;
  align-items: center;
  justify-content: center;
  background: rgba(59, 130, 246, 0.2);
  border-radius: var(--radius-md);
  color: var(--accent-primary);
}

.panel-title {
  font-size: 16px;
  font-weight: 600;
  color: var(--text-primary);
  margin: 0;
}

.panel-subtitle {
  font-size: 12px;
  color: var(--text-secondary);
  margin: 2px 0 0;
}

.panel-close {
  width: 32px;
  height: 32px;
  display: flex;
  align-items: center;
  justify-content: center;
  background: transparent;
  border: none;
  border-radius: var(--radius-sm);
  color: var(--text-secondary);
  cursor: pointer;
  transition: all var(--duration-fast) var(--ease-default);
}

.panel-close:hover {
  background: rgba(255, 255, 255, 0.05);
  color: var(--text-primary);
}

/* 搜索框 */
.search-box {
  position: relative;
  padding: var(--spacing-md) var(--spacing-lg);
}

.search-icon {
  position: absolute;
  left: 28px;
  top: 50%;
  transform: translateY(-50%);
  color: var(--text-tertiary);
}

.search-input {
  width: 100%;
  padding: 10px 12px 10px 36px;
  background: var(--bg-primary);
  border: 1px solid var(--border-primary);
  border-radius: var(--radius-md);
  color: var(--text-primary);
  font-size: 14px;
  outline: none;
  transition: all var(--duration-normal) var(--ease-default);
}

.search-input:focus {
  border-color: var(--accent-primary);
  box-shadow: 0 0 0 3px rgba(59, 130, 246, 0.2);
}

.search-input::placeholder {
  color: var(--text-tertiary);
}

/* 分类标签 */
.category-tabs {
  display: flex;
  gap: var(--spacing-xs);
  padding: 0 var(--spacing-lg) var(--spacing-md);
  overflow-x: auto;
}

.category-tab {
  display: flex;
  align-items: center;
  gap: 6px;
  padding: 6px 12px;
  background: transparent;
  border: 1px solid var(--border-primary);
  border-radius: var(--radius-full);
  color: var(--text-secondary);
  font-size: 12px;
  font-weight: 500;
  cursor: pointer;
  white-space: nowrap;
  transition: all var(--duration-fast) var(--ease-default);
}

.category-tab:hover {
  border-color: var(--accent-primary);
  color: var(--text-primary);
}

.category-tab.active {
  background: rgba(59, 130, 246, 0.2);
  border-color: var(--accent-primary);
  color: var(--accent-primary);
}

.category-count {
  display: inline-flex;
  align-items: center;
  justify-content: center;
  min-width: 18px;
  height: 18px;
  padding: 0 4px;
  background: rgba(255, 255, 255, 0.1);
  border-radius: var(--radius-full);
  font-size: 10px;
}

/* 数据源列表 */
.data-source-list {
  flex: 1;
  overflow-y: auto;
  padding: 0 var(--spacing-lg);
}

.data-source-item {
  display: flex;
  align-items: center;
  gap: var(--spacing-md);
  padding: var(--spacing-md);
  margin-bottom: var(--spacing-sm);
  background: transparent;
  border: 1px solid transparent;
  border-radius: var(--radius-md);
  cursor: pointer;
  transition: all var(--duration-fast) var(--ease-default);
}

.data-source-item:hover {
  background: rgba(255, 255, 255, 0.03);
  border-color: var(--border-primary);
}

.data-source-item.is-selected {
  background: rgba(59, 130, 246, 0.1);
  border-color: var(--accent-primary);
}

.source-icon {
  width: 32px;
  height: 32px;
  display: flex;
  align-items: center;
  justify-content: center;
  border-radius: var(--radius-sm);
  flex-shrink: 0;
}

.icon-blue {
  background: rgba(59, 130, 246, 0.2);
  color: var(--accent-primary);
}

.icon-green {
  background: rgba(34, 197, 94, 0.2);
  color: var(--status-success);
}

.icon-yellow {
  background: rgba(245, 158, 11, 0.2);
  color: var(--status-warning);
}

.icon-red {
  background: rgba(239, 68, 68, 0.2);
  color: var(--status-danger);
}

.icon-purple {
  background: rgba(139, 92, 246, 0.2);
  color: var(--accent-secondary);
}

.icon-cyan {
  background: rgba(6, 182, 212, 0.2);
  color: var(--status-info);
}

.source-info {
  flex: 1;
  min-width: 0;
}

.source-name {
  font-size: 14px;
  font-weight: 500;
  color: var(--text-primary);
  white-space: nowrap;
  overflow: hidden;
  text-overflow: ellipsis;
}

.source-meta {
  display: flex;
  align-items: center;
  gap: 8px;
  margin-top: 2px;
}

.source-device {
  font-size: 11px;
  color: var(--text-secondary);
}

.source-unit {
  font-size: 11px;
  color: var(--text-tertiary);
}

.source-value {
  display: flex;
  align-items: baseline;
  gap: 4px;
  flex-shrink: 0;
}

.value-number {
  font-size: 16px;
  font-weight: 600;
  color: var(--text-primary);
  font-family: var(--font-mono);
}

.value-unit {
  font-size: 11px;
  color: var(--text-tertiary);
}

.source-status {
  flex-shrink: 0;
}

/* 空状态 */
.empty-state {
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: center;
  padding: var(--spacing-2xl);
  color: var(--text-tertiary);
}

.empty-icon {
  margin-bottom: var(--spacing-md);
  opacity: 0.5;
}

.empty-text {
  font-size: 14px;
}

/* 已选数据源 */
.selected-section {
  padding: var(--spacing-md) var(--spacing-lg);
  border-top: 1px solid var(--border-primary);
}

.selected-header {
  display: flex;
  align-items: center;
  justify-content: space-between;
  margin-bottom: var(--spacing-sm);
}

.selected-title {
  font-size: 12px;
  font-weight: 600;
  color: var(--text-secondary);
  text-transform: uppercase;
  letter-spacing: 0.5px;
}

.selected-count {
  display: inline-flex;
  align-items: center;
  justify-content: center;
  min-width: 20px;
  height: 20px;
  padding: 0 6px;
  background: var(--accent-primary);
  border-radius: var(--radius-full);
  font-size: 11px;
  font-weight: 600;
  color: white;
}

.selected-list {
  display: flex;
  flex-wrap: wrap;
  gap: var(--spacing-xs);
}

.selected-tag {
  display: inline-flex;
  align-items: center;
  gap: 6px;
  padding: 4px 10px;
  background: rgba(59, 130, 246, 0.2);
  border: 1px solid rgba(59, 130, 246, 0.3);
  border-radius: var(--radius-full);
  font-size: 12px;
  color: var(--accent-primary);
}

.tag-remove {
  display: flex;
  align-items: center;
  justify-content: center;
  width: 16px;
  height: 16px;
  background: transparent;
  border: none;
  border-radius: 50%;
  color: var(--accent-primary);
  cursor: pointer;
  transition: all var(--duration-fast) var(--ease-default);
}

.tag-remove:hover {
  background: rgba(59, 130, 246, 0.3);
}

/* 操作按钮 */
.panel-actions {
  display: flex;
  justify-content: flex-end;
  gap: var(--spacing-sm);
  padding: var(--spacing-md) var(--spacing-lg);
  border-top: 1px solid var(--border-primary);
}

/* 响应式 */
@media (max-width: 768px) {
  .resource-pool-panel {
    max-height: 80vh;
  }

  .category-tabs {
    flex-wrap: wrap;
  }

  .data-source-item {
    flex-wrap: wrap;
  }

  .source-value {
    width: 100%;
    justify-content: flex-end;
    margin-top: var(--spacing-xs);
  }
}
</style>
