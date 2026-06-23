<template>
  <div class="live-stream">
    <!-- 流式数据展示 -->
    <div class="stream-container" ref="streamRef">
      <div
        v-for="(item, index) in streamItems"
        :key="item.id"
        class="stream-item"
        :class="[`item-${item.type}`, { 'item-entering': item.entering }]"
        :style="{ animationDelay: `${index * 50}ms` }"
      >
        <!-- 时间戳 -->
        <span class="item-time">{{ item.time }}</span>

        <!-- 类型标签 -->
        <span class="item-type" :class="`type-${item.type}`">
          {{ item.typeLabel }}
        </span>

        <!-- 数据源 -->
        <span class="item-source">{{ item.source }}</span>

        <!-- 数据内容 -->
        <span class="item-content">{{ item.content }}</span>

        <!-- 数值（如果有） -->
        <span v-if="item.value !== undefined" class="item-value" :class="valueClass(item)">
          {{ item.value }}{{ item.unit }}
        </span>

        <!-- 状态指示 -->
        <span class="item-status" :class="`status-${item.status}`">
          <span class="status-dot" />
          {{ item.statusLabel }}
        </span>
      </div>

      <!-- 空状态 -->
      <div v-if="streamItems.length === 0" class="empty-stream">
        <div class="empty-icon">
          <svg width="48" height="48" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.5">
            <path d="M13 2L3 14h9l-1 8 10-12h-9l1-8z"/>
          </svg>
        </div>
        <p class="empty-text">等待数据流...</p>
      </div>
    </div>

    <!-- 流控制 -->
    <div class="stream-controls">
      <div class="control-left">
        <button class="ctrl-btn" :class="{ active: isPaused }" @click="togglePause">
          <svg v-if="!isPaused" width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
            <rect x="6" y="4" width="4" height="16"/>
            <rect x="14" y="4" width="4" height="16"/>
          </svg>
          <svg v-else width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
            <polygon points="5 3 19 12 5 21 5 3"/>
          </svg>
          {{ isPaused ? '继续' : '暂停' }}
        </button>

        <button class="ctrl-btn" @click="clearStream">
          <svg width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
            <polyline points="3 6 5 6 21 6"/>
            <path d="M19 6v14a2 2 0 0 1-2 2H7a2 2 0 0 1-2-2V6m3 0V4a2 2 0 0 1 2-2h4a2 2 0 0 1 2 2v2"/>
          </svg>
          清空
        </button>
      </div>

      <div class="control-right">
        <div class="filter-group">
          <button
            v-for="filter in filters"
            :key="filter.id"
            class="filter-btn"
            :class="{ active: activeFilter === filter.id }"
            @click="setFilter(filter.id)"
          >
            <span class="filter-dot" :style="{ background: filter.color }" />
            {{ filter.label }}
          </button>
        </div>

        <div class="stream-stats">
          <span class="stat-item">
            <span class="stat-label">速率</span>
            <span class="stat-value">{{ streamRate }} 条/s</span>
          </span>
          <span class="stat-item">
            <span class="stat-label">总计</span>
            <span class="stat-value">{{ totalCount }}</span>
          </span>
        </div>
      </div>
    </div>
  </div>
</template>

<script setup lang="ts">
import { ref, onMounted, onUnmounted } from 'vue'

interface StreamItem {
  id: string
  time: string
  type: 'sensor' | 'alarm' | 'control' | 'system'
  typeLabel: string
  source: string
  content: string
  value?: number
  unit?: string
  status: 'normal' | 'warning' | 'danger'
  statusLabel: string
  entering?: boolean
}

const props = withDefaults(defineProps<{
  maxItems?: number
  autoScroll?: boolean
}>(), {
  maxItems: 100,
  autoScroll: true
})

const streamRef = ref<HTMLElement | null>(null)
const streamItems = ref<StreamItem[]>([])
const isPaused = ref(false)
const activeFilter = ref<string | null>(null)
const streamRate = ref(0)
const totalCount = ref(0)

// 过滤器
const filters = [
  { id: 'sensor', label: '传感器', color: '#3b82f6' },
  { id: 'alarm', label: '告警', color: '#ef4444' },
  { id: 'control', label: '控制', color: '#22c55d' },
  { id: 'system', label: '系统', color: '#f59e0b' }
]

// 模拟数据流
let simulationInterval: ReturnType<typeof setInterval> | null = null
const lastCount = ref(0)

function generateStreamItem(): StreamItem {
  const types: StreamItem['type'][] = ['sensor', 'alarm', 'control', 'system']
  const type = types[Math.floor(Math.random() * types.length)]

  const typeLabels: Record<string, string> = {
    sensor: '传感器',
    alarm: '告警',
    control: '控制',
    system: '系统'
  }

  const sources: Record<string, string[]> = {
    sensor: ['云测仪', '温湿度传感器', 'PM2.5传感器', 'CO2传感器'],
    alarm: ['烟雾报警', '水浸报警', '入侵报警', '气体报警'],
    control: ['空调控制', '通风控制', '喷淋控制', '声光报警'],
    system: ['网关', '服务器', '数据库', '消息队列']
  }

  const contents: Record<string, string[]> = {
    sensor: ['数据更新', '状态上报', '数值变化', '阈值检测'],
    alarm: ['触发报警', '报警解除', '预警提示', '风险升级'],
    control: ['指令下发', '状态反馈', '联动触发', '手动干预'],
    system: ['连接建立', '数据同步', '心跳检测', '配置更新']
  }

  const statusOptions: Array<{ status: StreamItem['status']; label: string }> = [
    { status: 'normal', label: '正常' },
    { status: 'warning', label: '警告' },
    { status: 'danger', label: '危险' }
  ]

  const statusIdx = type === 'alarm' ? (Math.random() > 0.3 ? 2 : 1) :
                    type === 'sensor' ? (Math.random() > 0.8 ? 1 : 0) : 0

  const now = new Date()
  const time = now.toLocaleTimeString('zh-CN', { hour12: false })

  return {
    id: `item_${Date.now()}_${Math.random().toString(36).substr(2, 6)}`,
    time,
    type,
    typeLabel: typeLabels[type],
    source: sources[type][Math.floor(Math.random() * sources[type].length)],
    content: contents[type][Math.floor(Math.random() * contents[type].length)],
    value: type === 'sensor' ? Math.round(Math.random() * 100 * 10) / 10 : undefined,
    unit: type === 'sensor' ? ['℃', '%', 'ppm', 'μg/m³'][Math.floor(Math.random() * 4)] : undefined,
    status: statusOptions[statusIdx].status,
    statusLabel: statusOptions[statusIdx].label,
    entering: true
  }
}

function addStreamItem(item: StreamItem) {
  if (isPaused.value) return

  streamItems.value.unshift(item)

  // 移除进入动画
  setTimeout(() => {
    item.entering = false
  }, 500)

  // 限制数量
  if (streamItems.value.length > props.maxItems) {
    streamItems.value.pop()
  }

  totalCount.value++

  // 自动滚动
  if (props.autoScroll && streamRef.value) {
    streamRef.value.scrollTop = 0
  }
}

// 计算流速率
function updateRate() {
  streamRate.value = totalCount.value - lastCount.value
  lastCount.value = totalCount.value
}

// 切换暂停
function togglePause() {
  isPaused.value = !isPaused.value
}

// 清空流
function clearStream() {
  streamItems.value = []
  totalCount.value = 0
}

// 设置过滤器
function setFilter(filterId: string) {
  activeFilter.value = activeFilter.value === filterId ? null : filterId
}

// 数值样式
function valueClass(item: StreamItem) {
  if (item.status === 'danger') return 'value-danger'
  if (item.status === 'warning') return 'value-warning'
  return 'value-normal'
}

onMounted(() => {
  // 模拟数据流
  simulationInterval = setInterval(() => {
    if (!isPaused.value) {
      addStreamItem(generateStreamItem())
    }
  }, 800)

  // 每秒更新速率
  setInterval(updateRate, 1000)
})

onUnmounted(() => {
  if (simulationInterval) {
    clearInterval(simulationInterval)
  }
})
</script>

<style scoped>
.live-stream {
  display: flex;
  flex-direction: column;
  height: 100%;
  background: var(--bg-card);
  border: 1px solid var(--border-primary);
  border-radius: var(--radius-lg);
  overflow: hidden;
}

/* 流容器 */
.stream-container {
  flex: 1;
  overflow-y: auto;
  padding: var(--spacing-md);
  min-height: 300px;
}

/* 流项 */
.stream-item {
  display: flex;
  align-items: center;
  gap: var(--spacing-md);
  padding: var(--spacing-sm) var(--spacing-md);
  margin-bottom: var(--spacing-xs);
  background: rgba(0, 0, 0, 0.2);
  border-radius: var(--radius-sm);
  font-size: 13px;
  transition: all var(--duration-fast) var(--ease-default);
  animation: slideInLeft var(--duration-normal) var(--ease-out);
}

.stream-item:hover {
  background: rgba(0, 0, 0, 0.3);
}

.item-entering {
  animation: slideInLeft var(--duration-normal) var(--ease-out);
}

/* 时间 */
.item-time {
  font-family: var(--font-mono);
  font-size: 11px;
  color: var(--text-tertiary);
  min-width: 70px;
}

/* 类型标签 */
.item-type {
  display: inline-flex;
  padding: 2px 8px;
  border-radius: var(--radius-full);
  font-size: 11px;
  font-weight: 500;
  min-width: 50px;
  justify-content: center;
}

.type-sensor {
  background: rgba(59, 130, 246, 0.2);
  color: var(--accent-primary);
  border: 1px solid rgba(59, 130, 246, 0.3);
}

.type-alarm {
  background: rgba(239, 68, 68, 0.2);
  color: var(--status-danger);
  border: 1px solid rgba(239, 68, 68, 0.3);
}

.type-control {
  background: rgba(34, 197, 94, 0.2);
  color: var(--status-success);
  border: 1px solid rgba(34, 197, 94, 0.3);
}

.type-system {
  background: rgba(245, 158, 11, 0.2);
  color: var(--status-warning);
  border: 1px solid rgba(245, 158, 11, 0.3);
}

/* 来源 */
.item-source {
  color: var(--text-secondary);
  min-width: 100px;
}

/* 内容 */
.item-content {
  flex: 1;
  color: var(--text-primary);
}

/* 数值 */
.item-value {
  font-family: var(--font-mono);
  font-weight: 600;
  min-width: 80px;
  text-align: right;
}

.value-normal {
  color: var(--status-success);
}

.value-warning {
  color: var(--status-warning);
}

.value-danger {
  color: var(--status-danger);
}

/* 状态 */
.item-status {
  display: flex;
  align-items: center;
  gap: 6px;
  min-width: 70px;
  font-size: 12px;
}

.status-dot {
  width: 6px;
  height: 6px;
  border-radius: 50%;
}

.status-normal {
  color: var(--status-success);
}

.status-normal .status-dot {
  background: var(--status-success);
  box-shadow: 0 0 6px var(--status-success);
}

.status-warning {
  color: var(--status-warning);
}

.status-warning .status-dot {
  background: var(--status-warning);
  box-shadow: 0 0 6px var(--status-warning);
  animation: pulse 1.5s ease-in-out infinite;
}

.status-danger {
  color: var(--status-danger);
}

.status-danger .status-dot {
  background: var(--status-danger);
  box-shadow: 0 0 6px var(--status-danger);
  animation: blink-fast 0.8s ease-in-out infinite;
}

/* 空状态 */
.empty-stream {
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: center;
  height: 100%;
  min-height: 200px;
  color: var(--text-tertiary);
}

.empty-icon {
  margin-bottom: var(--spacing-md);
  opacity: 0.5;
  animation: pulse 2s ease-in-out infinite;
}

.empty-text {
  font-size: 14px;
}

/* 控制栏 */
.stream-controls {
  display: flex;
  align-items: center;
  justify-content: space-between;
  padding: var(--spacing-md);
  border-top: 1px solid var(--border-primary);
  background: rgba(0, 0, 0, 0.2);
}

.control-left,
.control-right {
  display: flex;
  align-items: center;
  gap: var(--spacing-md);
}

/* 控制按钮 */
.ctrl-btn {
  display: flex;
  align-items: center;
  gap: 6px;
  padding: 6px 12px;
  background: transparent;
  border: 1px solid var(--border-primary);
  border-radius: var(--radius-md);
  color: var(--text-secondary);
  font-size: 12px;
  cursor: pointer;
  transition: all var(--duration-fast) var(--ease-default);
}

.ctrl-btn:hover {
  background: var(--bg-card-hover);
  border-color: var(--accent-primary);
  color: var(--text-primary);
}

.ctrl-btn.active {
  background: rgba(59, 130, 246, 0.2);
  border-color: var(--accent-primary);
  color: var(--accent-primary);
}

/* 过滤器 */
.filter-group {
  display: flex;
  gap: var(--spacing-xs);
}

.filter-btn {
  display: flex;
  align-items: center;
  gap: 4px;
  padding: 4px 10px;
  background: transparent;
  border: 1px solid var(--border-primary);
  border-radius: var(--radius-full);
  color: var(--text-secondary);
  font-size: 11px;
  cursor: pointer;
  transition: all var(--duration-fast) var(--ease-default);
}

.filter-btn:hover {
  border-color: var(--text-secondary);
  color: var(--text-primary);
}

.filter-btn.active {
  background: rgba(255, 255, 255, 0.1);
  border-color: var(--text-primary);
  color: var(--text-primary);
}

.filter-dot {
  width: 6px;
  height: 6px;
  border-radius: 50%;
}

/* 统计 */
.stream-stats {
  display: flex;
  gap: var(--spacing-lg);
}

.stat-item {
  display: flex;
  flex-direction: column;
  align-items: center;
  gap: 2px;
}

.stat-label {
  font-size: 10px;
  color: var(--text-tertiary);
  text-transform: uppercase;
  letter-spacing: 0.5px;
}

.stat-value {
  font-size: 14px;
  font-weight: 600;
  color: var(--text-primary);
  font-family: var(--font-mono);
}

/* 响应式 */
@media (max-width: 768px) {
  .stream-item {
    flex-wrap: wrap;
    gap: var(--spacing-sm);
  }

  .item-time,
  .item-source {
    min-width: auto;
  }

  .item-content {
    width: 100%;
  }

  .stream-controls {
    flex-direction: column;
    gap: var(--spacing-md);
  }

  .control-left,
  .control-right {
    width: 100%;
    justify-content: center;
  }
}
</style>
