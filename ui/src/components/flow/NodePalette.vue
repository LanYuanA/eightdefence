<template>
  <div class="node-palette">
    <div class="palette-header">
      <h3 class="palette-title">节点面板</h3>
      <div class="palette-search">
        <svg width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><circle cx="11" cy="11" r="8"/><line x1="21" y1="21" x2="16.65" y2="16.65"/></svg>
        <input v-model="searchQuery" type="text" placeholder="搜索节点..." class="search-input" />
      </div>
    </div>

    <div class="palette-content">
      <!-- 设备节点 -->
      <div class="node-category">
        <div class="category-header" @click="toggleCategory('device')">
          <span class="category-icon">📡</span>
          <span class="category-title">设备节点</span>
          <span class="category-arrow" :class="{ expanded: expandedCategories.device }">▼</span>
        </div>
        <div v-show="expandedCategories.device" class="category-nodes">
          <div
            v-for="node in filteredDeviceNodes"
            :key="node.type"
            class="node-item"
            draggable="true"
            @dragstart="handleDragStart($event, node)"
          >
            <span class="node-icon">{{ node.icon }}</span>
            <span class="node-label">{{ node.label }}</span>
          </div>
        </div>
      </div>

      <!-- 原子服务节点 -->
      <div class="node-category">
        <div class="category-header" @click="toggleCategory('service')">
          <span class="category-icon">⚡</span>
          <span class="category-title">原子服务</span>
          <span class="category-arrow" :class="{ expanded: expandedCategories.service }">▼</span>
        </div>
        <div v-show="expandedCategories.service" class="category-nodes">
          <div
            v-for="node in filteredServiceNodes"
            :key="node.type"
            class="node-item"
            draggable="true"
            @dragstart="handleDragStart($event, node)"
          >
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
          <span class="category-arrow" :class="{ expanded: expandedCategories.logic }">▼</span>
        </div>
        <div v-show="expandedCategories.logic" class="category-nodes">
          <div
            v-for="node in filteredLogicNodes"
            :key="node.type"
            class="node-item"
            draggable="true"
            @dragstart="handleDragStart($event, node)"
          >
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
          <span class="category-arrow" :class="{ expanded: expandedCategories.action }">▼</span>
        </div>
        <div v-show="expandedCategories.action" class="category-nodes">
          <div
            v-for="node in filteredActionNodes"
            :key="node.type"
            class="node-item"
            draggable="true"
            @dragstart="handleDragStart($event, node)"
          >
            <span class="node-icon">{{ node.icon }}</span>
            <span class="node-label">{{ node.label }}</span>
          </div>
        </div>
      </div>
    </div>
  </div>
</template>

<script setup lang="ts">
import { ref, computed } from 'vue'

interface NodeType {
  type: string
  label: string
  icon: string
  category: string
  color: string
  inputs: string[]
  outputs: string[]
}

const searchQuery = ref('')
const expandedCategories = ref({
  device: true,
  service: true,
  logic: true,
  action: true
})

// 设备节点
const deviceNodes: NodeType[] = [
  { type: 'sensor', label: '传感器', icon: '🌡️', category: 'device', color: '#3b82f6', inputs: [], outputs: ['value'] },
  { type: 'controller', label: '控制器', icon: '🎮', category: 'device', color: '#22c55d', inputs: ['command'], outputs: ['status'] },
  { type: 'alarm', label: '报警器', icon: '🔔', category: 'device', color: '#ef4444', inputs: ['trigger'], outputs: ['status'] },
  { type: 'switch', label: '开关', icon: '🔌', category: 'device', color: '#f59e0b', inputs: ['on/off'], outputs: ['state'] }
]

// 原子服务节点
const serviceNodes: NodeType[] = [
  { type: 'atomic_lower', label: '下层服务', icon: '⚙️', category: 'service', color: '#8b5cf6', inputs: ['input'], outputs: ['output'] },
  { type: 'atomic_upper', label: '上层服务', icon: '🔧', category: 'service', color: '#6366f1', inputs: ['input'], outputs: ['output'] },
  { type: 'data_read', label: '数据读取', icon: '📖', category: 'service', color: '#06b6d4', inputs: ['device'], outputs: ['data'] },
  { type: 'data_write', label: '数据写入', icon: '✏️', category: 'service', color: '#14b8a6', inputs: ['data', 'device'], outputs: ['result'] }
]

// 逻辑节点
const logicNodes: NodeType[] = [
  { type: 'condition', label: '条件判断', icon: '❓', category: 'logic', color: '#f59e0b', inputs: ['value'], outputs: ['true', 'false'] },
  { type: 'and', label: '逻辑与', icon: '∧', category: 'logic', color: '#84cc16', inputs: ['a', 'b'], outputs: ['result'] },
  { type: 'or', label: '逻辑或', icon: '∨', category: 'logic', color: '#22c55d', inputs: ['a', 'b'], outputs: ['result'] },
  { type: 'not', label: '逻辑非', icon: '¬', category: 'logic', color: '#ef4444', inputs: ['input'], outputs: ['output'] },
  { type: 'delay', label: '延时', icon: '⏱️', category: 'logic', color: '#6366f1', inputs: ['input'], outputs: ['output'] },
  { type: 'switch_case', label: '多条件分支', icon: '㉈', category: 'logic', color: '#8b5cf6', inputs: ['value'], outputs: ['case1', 'case2', 'case3'] }
]

// 动作节点
const actionNodes: NodeType[] = [
  { type: 'control', label: '设备控制', icon: '🎮', category: 'action', color: '#3b82f6', inputs: ['command'], outputs: ['result'] },
  { type: 'notify', label: '发送通知', icon: '📧', category: 'action', color: '#ef4444', inputs: ['message'], outputs: ['sent'] },
  { type: 'log', label: '记录日志', icon: '📝', category: 'action', color: '#22c55d', inputs: ['data'], outputs: ['logged'] },
  { type: 'trigger', label: '触发流程', icon: '▶️', category: 'action', color: '#f59e0b', inputs: ['trigger'], outputs: ['result'] }
]

// 过滤节点
const filteredDeviceNodes = computed(() => {
  if (!searchQuery.value) return deviceNodes
  return deviceNodes.filter(n => n.label.includes(searchQuery.value))
})

const filteredServiceNodes = computed(() => {
  if (!searchQuery.value) return serviceNodes
  return serviceNodes.filter(n => n.label.includes(searchQuery.value))
})

const filteredLogicNodes = computed(() => {
  if (!searchQuery.value) return logicNodes
  return logicNodes.filter(n => n.label.includes(searchQuery.value))
})

const filteredActionNodes = computed(() => {
  if (!searchQuery.value) return actionNodes
  return actionNodes.filter(n => n.label.includes(searchQuery.value))
})

// 展开/折叠分类
function toggleCategory(category: 'device' | 'service' | 'logic' | 'action') {
  expandedCategories.value[category] = !expandedCategories.value[category]
}

// 拖拽开始
function handleDragStart(event: DragEvent, node: NodeType) {
  if (event.dataTransfer) {
    event.dataTransfer.setData('application/json', JSON.stringify(node))
    event.dataTransfer.effectAllowed = 'copy'
  }
}
</script>

<style scoped>
.node-palette {
  width: 240px;
  background: var(--bg-card);
  border-right: 1px solid var(--border-primary);
  display: flex;
  flex-direction: column;
  height: 100%;
}

.palette-header {
  padding: var(--spacing-md);
  border-bottom: 1px solid var(--border-primary);
}

.palette-title {
  font-size: 14px;
  font-weight: 600;
  color: var(--text-primary);
  margin: 0 0 var(--spacing-sm);
}

.palette-search {
  display: flex;
  align-items: center;
  gap: 8px;
  padding: 6px 10px;
  background: var(--bg-primary);
  border: 1px solid var(--border-primary);
  border-radius: var(--radius-sm);
}

.palette-search svg {
  color: var(--text-tertiary);
  flex-shrink: 0;
}

.search-input {
  flex: 1;
  background: transparent;
  border: none;
  outline: none;
  color: var(--text-primary);
  font-size: 12px;
}

.palette-content {
  flex: 1;
  overflow-y: auto;
  padding: var(--spacing-sm);
}

.node-category {
  margin-bottom: var(--spacing-sm);
}

.category-header {
  display: flex;
  align-items: center;
  gap: 8px;
  padding: 8px;
  cursor: pointer;
  border-radius: var(--radius-sm);
  transition: background var(--duration-fast);
}

.category-header:hover {
  background: rgba(255, 255, 255, 0.05);
}

.category-icon {
  font-size: 16px;
}

.category-title {
  flex: 1;
  font-size: 12px;
  font-weight: 600;
  color: var(--text-secondary);
  text-transform: uppercase;
  letter-spacing: 0.5px;
}

.category-arrow {
  font-size: 10px;
  color: var(--text-tertiary);
  transition: transform var(--duration-fast);
}

.category-arrow.expanded {
  transform: rotate(0deg);
}

.category-arrow:not(.expanded) {
  transform: rotate(-90deg);
}

.category-nodes {
  padding-left: var(--spacing-sm);
}

.node-item {
  display: flex;
  align-items: center;
  gap: 8px;
  padding: 8px;
  margin: 2px 0;
  background: rgba(0, 0, 0, 0.2);
  border: 1px solid var(--border-primary);
  border-radius: var(--radius-sm);
  cursor: grab;
  transition: all var(--duration-fast);
}

.node-item:hover {
  background: rgba(59, 130, 246, 0.1);
  border-color: var(--accent-primary);
  transform: translateX(4px);
}

.node-item:active {
  cursor: grabbing;
  transform: scale(0.95);
}

.node-icon {
  font-size: 16px;
}

.node-label {
  font-size: 12px;
  color: var(--text-primary);
}
</style>
