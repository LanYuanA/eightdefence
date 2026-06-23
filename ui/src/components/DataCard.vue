<template>
  <div
    class="data-card"
    :class="[`status-${status}`, { 'data-card-updated': isUpdated }]"
    @mouseenter="isHovered = true"
    @mouseleave="isHovered = false"
  >
    <!-- 图标区域 -->
    <div class="data-icon-wrapper" :class="`icon-bg-${iconColor}`">
      <svg
        width="24"
        height="24"
        viewBox="0 0 24 24"
        fill="none"
        stroke="currentColor"
        stroke-width="2"
        class="data-icon"
        v-html="icon"
      />
      <!-- 状态指示点 -->
      <div class="status-indicator" :class="`indicator-${status}`" />
    </div>

    <!-- 数据内容 -->
    <div class="data-content">
      <div class="data-label">{{ label }}</div>
      <div class="data-value-wrapper">
        <NumberRoll
          :value="displayValue"
          :duration="animationDuration"
          class="data-value"
        />
        <span v-if="unit" class="data-unit">{{ unit }}</span>
      </div>
      <div v-if="trend" class="data-trend" :class="`trend-${trendDirection}`">
        <svg
          width="12"
          height="12"
          viewBox="0 0 24 24"
          fill="none"
          stroke="currentColor"
          stroke-width="2"
        >
          <path v-if="trendDirection === 'up'" d="M18 15l-6-6-6 6"/>
          <path v-else-if="trendDirection === 'down'" d="M6 9l6 6 6-6"/>
          <path v-else d="M5 12h14"/>
        </svg>
        <span>{{ trend }}</span>
      </div>
    </div>

    <!-- 迷你图表（可选） -->
    <div v-if="miniChart && miniChartData.length > 0" class="mini-chart">
      <svg :viewBox="`0 0 ${miniChartWidth} ${miniChartHeight}`" class="mini-chart-svg">
        <defs>
          <linearGradient :id="`gradient-${uid}`" x1="0%" y1="0%" x2="0%" y2="100%">
            <stop offset="0%" :stop-color="chartColor" stop-opacity="0.3"/>
            <stop offset="100%" :stop-color="chartColor" stop-opacity="0"/>
          </linearGradient>
        </defs>
        <path
          :d="areaPath"
          :fill="`url(#gradient-${uid})`"
        />
        <path
          :d="linePath"
          fill="none"
          :stroke="chartColor"
          stroke-width="1.5"
          stroke-linecap="round"
          stroke-linejoin="round"
        />
      </svg>
    </div>

    <!-- 悬浮光效 -->
    <div v-if="isHovered" class="hover-glow" />
  </div>
</template>

<script setup lang="ts">
import { ref, computed, watch } from 'vue'
import NumberRoll from './NumberRoll.vue'

const props = withDefaults(defineProps<{
  label: string
  value: number
  unit?: string
  icon?: string
  iconColor?: 'blue' | 'purple' | 'green' | 'yellow' | 'red' | 'cyan'
  status?: 'normal' | 'warning' | 'danger' | 'offline'
  trend?: string
  trendDirection?: 'up' | 'down' | 'stable'
  decimals?: number
  animationDuration?: number
  miniChart?: boolean
  miniChartData?: number[]
  chartColor?: string
}>(), {
  iconColor: 'blue',
  status: 'normal',
  decimals: 1,
  animationDuration: 800,
  miniChart: false,
  miniChartData: () => [],
  chartColor: '#3b82f6'
})

const isHovered = ref(false)
const isUpdated = ref(false)
const uid = ref(Math.random().toString(36).substr(2, 9))

const miniChartWidth = 80
const miniChartHeight = 40

// 显示值
const displayValue = computed(() => {
  return Number(props.value.toFixed(props.decimals))
})

// 监听值变化，触发更新动画
watch(() => props.value, () => {
  isUpdated.value = true
  setTimeout(() => {
    isUpdated.value = false
  }, 600)
})

// 计算迷你图表路径
const linePath = computed(() => {
  if (props.miniChartData.length < 2) return ''

  const data = props.miniChartData
  const max = Math.max(...data)
  const min = Math.min(...data)
  const range = max - min || 1

  const points = data.map((val, i) => {
    const x = (i / (data.length - 1)) * miniChartWidth
    const y = miniChartHeight - ((val - min) / range) * miniChartHeight * 0.8 - miniChartHeight * 0.1
    return `${x},${y}`
  })

  return `M${points.join(' L')}`
})

const areaPath = computed(() => {
  if (!linePath.value) return ''

  const data = props.miniChartData
  const lastX = ((data.length - 1) / (data.length - 1)) * miniChartWidth

  return `${linePath.value} L${lastX},${miniChartHeight} L0,${miniChartHeight} Z`
})
</script>

<style scoped>
.data-card {
  background: var(--bg-card);
  border: 1px solid var(--border-primary);
  border-radius: var(--radius-lg);
  padding: var(--spacing-lg);
  display: flex;
  align-items: center;
  gap: var(--spacing-md);
  position: relative;
  overflow: hidden;
  transition: all var(--duration-normal) var(--ease-default);
  animation: fadeInUp var(--duration-slow) var(--ease-out) both;
}

.data-card:hover {
  transform: translateY(-2px);
  box-shadow: var(--shadow-md);
  border-color: var(--border-accent);
}

/* 状态样式 */
.status-normal {
  border-left: 3px solid var(--status-success);
}

.status-warning {
  border-left: 3px solid var(--status-warning);
}

.status-danger {
  border-left: 3px solid var(--status-danger);
  animation: alarm-border 2s ease-in-out infinite;
}

.status-offline {
  border-left: 3px solid var(--text-tertiary);
  opacity: 0.7;
}

/* 更新动画 */
.data-card-updated {
  animation: data-update 0.6s ease-out;
}

/* 图标 */
.data-icon-wrapper {
  width: 52px;
  height: 52px;
  display: flex;
  align-items: center;
  justify-content: center;
  border-radius: var(--radius-md);
  position: relative;
  flex-shrink: 0;
  transition: transform var(--duration-normal) var(--ease-default);
}

.data-card:hover .data-icon-wrapper {
  transform: scale(1.1);
}

.icon-bg-blue {
  background: linear-gradient(135deg, rgba(59, 130, 246, 0.2), rgba(59, 130, 246, 0.1));
  color: var(--accent-primary);
}

.icon-bg-purple {
  background: linear-gradient(135deg, rgba(139, 92, 246, 0.2), rgba(139, 92, 246, 0.1));
  color: var(--accent-secondary);
}

.icon-bg-green {
  background: linear-gradient(135deg, rgba(34, 197, 94, 0.2), rgba(34, 197, 94, 0.1));
  color: var(--status-success);
}

.icon-bg-yellow {
  background: linear-gradient(135deg, rgba(245, 158, 11, 0.2), rgba(245, 158, 11, 0.1));
  color: var(--status-warning);
}

.icon-bg-red {
  background: linear-gradient(135deg, rgba(239, 68, 68, 0.2), rgba(239, 68, 68, 0.1));
  color: var(--status-danger);
}

.icon-bg-cyan {
  background: linear-gradient(135deg, rgba(6, 182, 212, 0.2), rgba(6, 182, 212, 0.1));
  color: var(--status-info);
}

.data-icon {
  transition: transform var(--duration-normal) var(--ease-default);
}

.data-card:hover .data-icon {
  transform: rotate(-5deg);
}

/* 状态指示点 */
.status-indicator {
  position: absolute;
  top: -2px;
  right: -2px;
  width: 12px;
  height: 12px;
  border-radius: 50%;
  border: 2px solid var(--bg-card);
}

.indicator-normal {
  background: var(--status-success);
  box-shadow: var(--status-success-glow);
  animation: breathe 2s ease-in-out infinite;
}

.indicator-warning {
  background: var(--status-warning);
  box-shadow: var(--status-warning-glow);
  animation: pulse 1.5s ease-in-out infinite;
}

.indicator-danger {
  background: var(--status-danger);
  box-shadow: var(--status-danger-glow);
  animation: blink-fast 0.8s ease-in-out infinite;
}

.indicator-offline {
  background: var(--text-tertiary);
}

/* 数据内容 */
.data-content {
  flex: 1;
  min-width: 0;
}

.data-label {
  font-size: 13px;
  color: var(--text-secondary);
  margin-bottom: 4px;
  text-transform: uppercase;
  letter-spacing: 0.5px;
}

.data-value-wrapper {
  display: flex;
  align-items: baseline;
  gap: 6px;
}

.data-value {
  font-size: 28px;
  font-weight: 700;
  color: var(--text-primary);
  font-family: var(--font-mono);
  line-height: 1.2;
}

.data-unit {
  font-size: 14px;
  color: var(--text-tertiary);
  font-weight: 500;
}

/* 趋势 */
.data-trend {
  display: flex;
  align-items: center;
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
  width: 80px;
  height: 40px;
  flex-shrink: 0;
  opacity: 0.8;
  transition: opacity var(--duration-normal) var(--ease-default);
}

.data-card:hover .mini-chart {
  opacity: 1;
}

.mini-chart-svg {
  width: 100%;
  height: 100%;
}

/* 悬浮光效 */
.hover-glow {
  position: absolute;
  top: 0;
  left: 0;
  width: 100%;
  height: 100%;
  background: radial-gradient(
    circle at var(--mouse-x, 50%) var(--mouse-y, 50%),
    rgba(59, 130, 246, 0.1) 0%,
    transparent 50%
  );
  pointer-events: none;
  animation: fadeIn var(--duration-fast) var(--ease-out);
}

/* 响应式 */
@media (max-width: 768px) {
  .data-card {
    flex-direction: column;
    align-items: flex-start;
    text-align: left;
  }

  .mini-chart {
    width: 100%;
    margin-top: var(--spacing-sm);
  }
}
</style>
