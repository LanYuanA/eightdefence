<template>
  <div
    class="device-card"
    :class="[
      `status-${status}`,
      { 'card-draggable': draggable, 'card-selected': selected }
    ]"
    :draggable="draggable"
    @mouseenter="isHovered = true"
    @mouseleave="isHovered = false"
    @click="$emit('click')"
    @dragstart="$emit('dragstart', $event)"
  >
    <!-- 背景光效 -->
    <div class="card-glow" :style="{ background: glowColor }" />

    <!-- 顶部状态条 -->
    <div class="status-bar" :style="{ background: statusColor }" />

    <!-- 设备图标 -->
    <div class="device-icon-wrapper">
      <div class="icon-ring" :style="{ borderColor: statusColor }">
        <span class="device-icon">{{ icon }}</span>
      </div>
      <StatusDot
        :status="status"
        :size="'sm'"
        :pulse="status === 'online'"
        class="status-indicator"
      />
    </div>

    <!-- 设备信息 -->
    <div class="device-info">
      <h4 class="device-name">{{ name }}</h4>
      <p class="device-type">{{ type }}</p>
    </div>

    <!-- 数据展示 -->
    <div class="device-data">
      <div class="data-value">
        <NumberRoll
          :value="value"
          :decimals="decimals"
          :duration="500"
          class="value-number"
        />
        <span class="value-unit">{{ unit }}</span>
      </div>
      <div v-if="trend" class="data-trend" :class="trendClass">
        <svg width="12" height="12" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
          <path v-if="trend === 'up'" d="M18 15l-6-6-6 6"/>
          <path v-else-if="trend === 'down'" d="M6 9l6 6 6-6"/>
          <path v-else d="M5 12h14"/>
        </svg>
        <span>{{ trendText }}</span>
      </div>
    </div>

    <!-- 迷你图表 -->
    <div v-if="showChart && chartData.length > 0" class="mini-chart">
      <svg :viewBox="`0 0 ${chartWidth} ${chartHeight}`" class="chart-svg">
        <defs>
          <linearGradient :id="`chartGrad-${uid}`" x1="0%" y1="0%" x2="0%" y2="100%">
            <stop offset="0%" :stop-color="statusColor" stop-opacity="0.3" />
            <stop offset="100%" :stop-color="statusColor" stop-opacity="0" />
          </linearGradient>
        </defs>
        <path :d="areaPath" :fill="`url(#chartGrad-${uid})`" />
        <path :d="linePath" fill="none" :stroke="statusColor" stroke-width="1.5" />
      </svg>
    </div>

    <!-- 操作按钮 -->
    <div v-if="showActions" class="device-actions">
      <button
        v-for="action in actions"
        :key="action.id"
        class="action-btn"
        :class="`action-${action.type}`"
        @click.stop="$emit('action', action.id)"
        :title="action.label"
      >
        <svg width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" v-html="action.icon" />
      </button>
    </div>

    <!-- 选中边框 -->
    <div v-if="selected" class="selected-border" />
  </div>
</template>

<script setup lang="ts">
import { ref, computed } from 'vue'
import StatusDot from './StatusDot.vue'
import NumberRoll from './NumberRoll.vue'

interface DeviceAction {
  id: string
  label: string
  icon: string
  type: 'primary' | 'secondary' | 'danger'
}

const props = withDefaults(defineProps<{
  name: string
  type?: string
  icon?: string
  value?: number
  unit?: string
  decimals?: number
  status?: 'online' | 'offline' | 'warning' | 'danger'
  trend?: 'up' | 'down' | 'stable'
  trendText?: string
  showChart?: boolean
  chartData?: number[]
  showActions?: boolean
  actions?: DeviceAction[]
  draggable?: boolean
  selected?: boolean
}>(), {
  type: '传感器',
  icon: '📡',
  value: 0,
  unit: '',
  decimals: 1,
  status: 'online',
  trendText: '',
  showChart: false,
  chartData: () => [],
  showActions: false,
  actions: () => [],
  draggable: false,
  selected: false
})

defineEmits<{
  click: []
  dragstart: [event: DragEvent]
  action: [actionId: string]
}>()

const isHovered = ref(false)
const uid = ref(Math.random().toString(36).substr(2, 9))

const chartWidth = 80
const chartHeight = 30

// 状态颜色
const statusColor = computed(() => {
  const colors = {
    online: 'var(--status-success)',
    offline: 'var(--text-tertiary)',
    warning: 'var(--status-warning)',
    danger: 'var(--status-danger)'
  }
  return colors[props.status]
})

// 发光颜色
const glowColor = computed(() => {
  if (!isHovered.value) return 'transparent'
  const colors = {
    online: 'radial-gradient(circle, rgba(34, 197, 94, 0.1), transparent 70%)',
    offline: 'transparent',
    warning: 'radial-gradient(circle, rgba(245, 158, 11, 0.1), transparent 70%)',
    danger: 'radial-gradient(circle, rgba(239, 68, 68, 0.1), transparent 70%)'
  }
  return colors[props.status]
})

// 趋势样式
const trendClass = computed(() => {
  if (!props.trend) return ''
  return `trend-${props.trend}`
})

// 图表路径
const linePath = computed(() => {
  if (props.chartData.length < 2) return ''
  const data = props.chartData
  const max = Math.max(...data)
  const min = Math.min(...data)
  const range = max - min || 1
  const points = data.map((val, i) => {
    const x = (i / (data.length - 1)) * chartWidth
    const y = chartHeight - ((val - min) / range) * chartHeight * 0.8 - chartHeight * 0.1
    return `${x},${y}`
  })
  return `M${points.join(' L')}`
})

const areaPath = computed(() => {
  if (!linePath.value) return ''
  return `${linePath.value} L${chartWidth},${chartHeight} L0,${chartHeight} Z`
})
</script>

<style scoped>
.device-card {
  position: relative;
  background: var(--bg-card);
  border: 1px solid var(--border-primary);
  border-radius: var(--radius-lg);
  padding: var(--spacing-lg);
  overflow: hidden;
  transition: all var(--duration-normal) var(--ease-default);
  cursor: pointer;
}

.device-card:hover {
  transform: translateY(-4px) scale(1.02);
  box-shadow: var(--shadow-lg);
  border-color: var(--border-accent);
}

/* 拖拽状态 */
.card-draggable {
  cursor: grab;
}

.card-draggable:active {
  cursor: grabbing;
  transform: scale(0.98);
}

/* 选中状态 */
.card-selected {
  border-color: var(--accent-primary);
  box-shadow: 0 0 20px rgba(59, 130, 246, 0.3);
}

/* 背景光效 */
.card-glow {
  position: absolute;
  top: 0;
  left: 0;
  width: 100%;
  height: 100%;
  pointer-events: none;
  transition: background var(--duration-normal) var(--ease-default);
}

/* 状态条 */
.status-bar {
  position: absolute;
  top: 0;
  left: 0;
  width: 100%;
  height: 3px;
}

/* 设备图标 */
.device-icon-wrapper {
  position: relative;
  display: flex;
  justify-content: center;
  margin-bottom: var(--spacing-md);
}

.icon-ring {
  width: 64px;
  height: 64px;
  display: flex;
  align-items: center;
  justify-content: center;
  border: 2px solid;
  border-radius: 50%;
  background: rgba(0, 0, 0, 0.3);
  transition: all var(--duration-normal) var(--ease-default);
}

.device-card:hover .icon-ring {
  transform: scale(1.1);
}

.device-icon {
  font-size: 28px;
}

.status-indicator {
  position: absolute;
  top: 0;
  right: calc(50% - 40px);
}

/* 设备信息 */
.device-info {
  text-align: center;
  margin-bottom: var(--spacing-md);
}

.device-name {
  font-size: 16px;
  font-weight: 600;
  color: var(--text-primary);
  margin: 0 0 4px;
}

.device-type {
  font-size: 12px;
  color: var(--text-secondary);
  margin: 0;
}

/* 数据展示 */
.device-data {
  text-align: center;
  margin-bottom: var(--spacing-md);
}

.data-value {
  display: flex;
  align-items: baseline;
  justify-content: center;
  gap: 6px;
}

.value-number {
  font-size: 32px;
  font-weight: 700;
  color: var(--text-primary);
  font-family: var(--font-mono);
}

.value-unit {
  font-size: 14px;
  color: var(--text-secondary);
}

.data-trend {
  display: flex;
  align-items: center;
  justify-content: center;
  gap: 4px;
  font-size: 12px;
  font-weight: 500;
  margin-top: 6px;
}

.trend-up {
  color: var(--status-success);
}

.trend-down {
  color: var(--status-danger);
}

.trend-stable {
  color: var(--text-secondary);
}

/* 迷你图表 */
.mini-chart {
  height: 30px;
  margin-bottom: var(--spacing-md);
  opacity: 0.8;
}

.chart-svg {
  width: 100%;
  height: 100%;
}

/* 操作按钮 */
.device-actions {
  display: flex;
  justify-content: center;
  gap: var(--spacing-sm);
  padding-top: var(--spacing-md);
  border-top: 1px solid var(--border-primary);
}

.action-btn {
  width: 32px;
  height: 32px;
  display: flex;
  align-items: center;
  justify-content: center;
  background: transparent;
  border: 1px solid var(--border-primary);
  border-radius: var(--radius-md);
  color: var(--text-secondary);
  cursor: pointer;
  transition: all var(--duration-fast) var(--ease-default);
}

.action-btn:hover {
  background: var(--bg-card-hover);
  border-color: var(--accent-primary);
  color: var(--accent-primary);
}

.action-primary:hover {
  border-color: var(--accent-primary);
  color: var(--accent-primary);
}

.action-danger:hover {
  border-color: var(--status-danger);
  color: var(--status-danger);
}

/* 选中边框 */
.selected-border {
  position: absolute;
  top: -2px;
  left: -2px;
  right: -2px;
  bottom: -2px;
  border: 2px solid var(--accent-primary);
  border-radius: var(--radius-lg);
  pointer-events: none;
  animation: pulse 2s ease-in-out infinite;
}

/* 离线状态 */
.status-offline {
  opacity: 0.6;
}

.status-offline .device-icon {
  filter: grayscale(100%);
}

/* 报警状态 */
.status-danger {
  animation: alarm-border 2s ease-in-out infinite;
}

.status-warning {
  border-color: var(--status-warning);
}
</style>
