<template>
  <div class="gauge-container" :style="{ width: size + 'px', height: size + 'px' }">
    <svg :width="size" :height="size" :viewBox="`0 0 ${size} ${size}`">
      <!-- 背景环 -->
      <circle
        :cx="center"
        :cy="center"
        :r="radius"
        fill="none"
        :stroke="bgColor"
        :stroke-width="strokeWidth"
        stroke-linecap="round"
      />

      <!-- 刻度线 -->
      <g v-for="i in 60" :key="i">
        <line
          :x1="center + (radius - 10) * Math.cos((i * 6 - 90) * Math.PI / 180)"
          :y1="center + (radius - 10) * Math.sin((i * 6 - 90) * Math.PI / 180)"
          :x2="center + (radius - (i % 5 === 0 ? 20 : 15)) * Math.cos((i * 6 - 90) * Math.PI / 180)"
          :y2="center + (radius - (i % 5 === 0 ? 20 : 15)) * Math.sin((i * 6 - 90) * Math.PI / 180)"
          :stroke="i % 5 === 0 ? tickColor : tickColor + '60'"
          :stroke-width="i % 5 === 0 ? 2 : 1"
        />
      </g>

      <!-- 进度弧 -->
      <path
        :d="arcPath"
        fill="none"
        :stroke="progressGradient"
        :stroke-width="strokeWidth"
        stroke-linecap="round"
        class="progress-arc"
      />

      <!-- 发光效果 -->
      <path
        :d="arcPath"
        fill="none"
        :stroke="progressColor"
        :stroke-width="strokeWidth + 4"
        stroke-linecap="round"
        stroke-opacity="0.3"
        filter="url(#blur)"
        class="progress-glow"
      />

      <!-- 指针 -->
      <g :transform="`rotate(${needleAngle}, ${center}, ${center})`">
        <line
          :x1="center"
          :y1="center"
          :x2="center"
          :y2="center - radius + 30"
          :stroke="needleColor"
          stroke-width="3"
          stroke-linecap="round"
        />
        <circle
          :cx="center"
          :cy="center"
          r="8"
          :fill="needleColor"
        />
        <circle
          :cx="center"
          :cy="center"
          r="4"
          :fill="bgColor"
        />
      </g>

      <!-- 中心数值 -->
      <text
        :x="center"
        :y="center + 30"
        text-anchor="middle"
        dominant-baseline="central"
        :fill="textColor"
        font-size="32"
        font-weight="700"
        font-family="var(--font-mono)"
        class="value-text"
      >
        {{ displayValue }}
      </text>

      <!-- 单位 -->
      <text
        :x="center"
        :y="center + 55"
        text-anchor="middle"
        dominant-baseline="central"
        :fill="subtextColor"
        font-size="14"
        font-weight="500"
      >
        {{ unit }}
      </text>

      <!-- 标签 -->
      <text
        :x="center"
        :y="center - 40"
        text-anchor="middle"
        dominant-baseline="central"
        :fill="labelColor"
        font-size="14"
        font-weight="600"
        letter-spacing="2"
      >
        {{ label }}
      </text>

      <!-- 定义渐变和滤镜 -->
      <defs>
        <linearGradient :id="`gradient-${uid}`" x1="0%" y1="0%" x2="100%" y2="0%">
          <stop offset="0%" :stop-color="progressStart" />
          <stop offset="100%" :stop-color="progressEnd" />
        </linearGradient>

        <filter id="blur">
          <feGaussianBlur stdDeviation="4" />
        </filter>
      </defs>
    </svg>

    <!-- 状态指示 -->
    <div class="gauge-status" :class="statusClass">
      <span class="status-dot" />
      <span class="status-text">{{ statusText }}</span>
    </div>
  </div>
</template>

<script setup lang="ts">
import { ref, computed, watch, onMounted } from 'vue'

const props = withDefaults(defineProps<{
  value: number
  min?: number
  max?: number
  size?: number
  strokeWidth?: number
  label?: string
  unit?: string
  progressColor?: string
  progressStart?: string
  progressEnd?: string
  bgColor?: string
  tickColor?: string
  needleColor?: string
  textColor?: string
  subtextColor?: string
  labelColor?: string
  animationDuration?: number
}>(), {
  min: 0,
  max: 100,
  size: 250,
  strokeWidth: 12,
  label: '仪表盘',
  unit: '%',
  progressColor: '#3b82f6',
  progressStart: '#3b82f6',
  progressEnd: '#8b5cf6',
  bgColor: '#1e293b',
  tickColor: '#475569',
  needleColor: '#ef4444',
  textColor: '#f1f5f9',
  subtextColor: '#94a3b8',
  labelColor: '#64748b',
  animationDuration: 1000
})

const uid = ref(Math.random().toString(36).substr(2, 9))
const displayValue = ref(0)
const animatedValue = ref(0)

const center = computed(() => props.size / 2)
const radius = computed(() => (props.size - props.strokeWidth * 2) / 2 - 20)

// 计算指针角度
const needleAngle = computed(() => {
  const range = props.max - props.min
  const normalized = (animatedValue.value - props.min) / range
  return normalized * 270 - 135
})

// 计算弧形路径
const arcPath = computed(() => {
  const range = props.max - props.min
  const normalized = (animatedValue.value - props.min) / range
  const angle = normalized * 270
  const startAngle = -225
  const endAngle = startAngle + angle

  const startRad = startAngle * Math.PI / 180
  const endRad = endAngle * Math.PI / 180

  const x1 = center.value + radius.value * Math.cos(startRad)
  const y1 = center.value + radius.value * Math.sin(startRad)
  const x2 = center.value + radius.value * Math.cos(endRad)
  const y2 = center.value + radius.value * Math.sin(endRad)

  const largeArc = angle > 180 ? 1 : 0

  return `M ${x1} ${y1} A ${radius.value} ${radius.value} 0 ${largeArc} 1 ${x2} ${y2}`
})

// 进度渐变
const progressGradient = computed(() => `url(#gradient-${uid.value})`)

// 状态
const statusClass = computed(() => {
  const range = props.max - props.min
  const normalized = (props.value - props.min) / range
  if (normalized > 0.8) return 'status-danger'
  if (normalized > 0.6) return 'status-warning'
  return 'status-normal'
})

const statusText = computed(() => {
  const range = props.max - props.min
  const normalized = (props.value - props.min) / range
  if (normalized > 0.8) return '危险'
  if (normalized > 0.6) return '警告'
  return '正常'
})

// 动画
function animateValue(from: number, to: number, duration: number) {
  const startTime = performance.now()
  const diff = to - from

  function update(currentTime: number) {
    const elapsed = currentTime - startTime
    const progress = Math.min(elapsed / duration, 1)

    // 缓动函数
    const eased = 1 - Math.pow(1 - progress, 3)
    animatedValue.value = from + diff * eased
    displayValue.value = Math.round(animatedValue.value)

    if (progress < 1) {
      requestAnimationFrame(update)
    }
  }

  requestAnimationFrame(update)
}

watch(() => props.value, (newVal, oldVal) => {
  animateValue(oldVal || 0, newVal, props.animationDuration)
})

onMounted(() => {
  animateValue(0, props.value, props.animationDuration)
})
</script>

<style scoped>
.gauge-container {
  position: relative;
  display: inline-block;
}

.progress-arc {
  transition: d 0.3s ease-out;
}

.progress-glow {
  transition: d 0.3s ease-out;
}

.value-text {
  transition: all 0.3s ease-out;
}

/* 状态指示 */
.gauge-status {
  position: absolute;
  bottom: 20%;
  left: 50%;
  transform: translateX(-50%);
  display: flex;
  align-items: center;
  gap: 6px;
  padding: 4px 12px;
  border-radius: var(--radius-full);
  font-size: 12px;
  font-weight: 500;
}

.status-dot {
  width: 8px;
  height: 8px;
  border-radius: 50%;
  animation: breathe 2s ease-in-out infinite;
}

.status-normal {
  background: rgba(34, 197, 94, 0.2);
  color: var(--status-success);
}

.status-normal .status-dot {
  background: var(--status-success);
  box-shadow: 0 0 8px var(--status-success);
}

.status-warning {
  background: rgba(245, 158, 11, 0.2);
  color: var(--status-warning);
}

.status-warning .status-dot {
  background: var(--status-warning);
  box-shadow: 0 0 8px var(--status-warning);
  animation: pulse 1.5s ease-in-out infinite;
}

.status-danger {
  background: rgba(239, 68, 68, 0.2);
  color: var(--status-danger);
}

.status-danger .status-dot {
  background: var(--status-danger);
  box-shadow: 0 0 8px var(--status-danger);
  animation: blink-fast 0.8s ease-in-out infinite;
}
</style>
