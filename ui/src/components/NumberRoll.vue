<template>
  <span class="number-roll" :class="{ 'number-roll-animate': isAnimating }">
    <span
      v-for="(digit, index) in digits"
      :key="index"
      class="digit-wrapper"
      :style="{ animationDelay: `${index * 50}ms` }"
    >
      <span class="digit" :class="{ 'digit-animate': digit.changed }">
        {{ digit.value }}
      </span>
    </span>
  </span>
</template>

<script setup lang="ts">
import { ref, computed, watch, onMounted } from 'vue'

const props = withDefaults(defineProps<{
  value: number
  duration?: number
  decimals?: number
}>(), {
  duration: 800,
  decimals: 1
})

const isAnimating = ref(false)
const currentValue = ref(0)
let animationFrame: number | null = null

interface DigitInfo {
  value: string
  changed: boolean
}

// 格式化数字
function formatNumber(num: number): string {
  return num.toFixed(props.decimals)
}

// 计算每一位数字
const digits = computed<DigitInfo[]>(() => {
  const formatted = formatNumber(currentValue.value)
  return formatted.split('').map(char => ({
    value: char,
    changed: false
  }))
})

// 动画函数
function animateValue(start: number, end: number, duration: number) {
  if (animationFrame) {
    cancelAnimationFrame(animationFrame)
  }

  isAnimating.value = true
  const startTime = performance.now()
  const diff = end - start

  function update(currentTime: number) {
    const elapsed = currentTime - startTime
    const progress = Math.min(elapsed / duration, 1)

    // 使用缓动函数
    const easedProgress = easeOutCubic(progress)
    currentValue.value = start + diff * easedProgress

    if (progress < 1) {
      animationFrame = requestAnimationFrame(update)
    } else {
      currentValue.value = end
      isAnimating.value = false
      animationFrame = null
    }
  }

  animationFrame = requestAnimationFrame(update)
}

// 缓动函数
function easeOutCubic(t: number): number {
  return 1 - Math.pow(1 - t, 3)
}

// 监听值变化
watch(() => props.value, (newVal, oldVal) => {
  if (newVal !== oldVal) {
    animateValue(oldVal || 0, newVal, props.duration)
  }
}, { immediate: true })

onMounted(() => {
  if (props.value !== 0) {
    animateValue(0, props.value, props.duration)
  }
})
</script>

<style scoped>
.number-roll {
  display: inline-flex;
  font-variant-numeric: tabular-nums;
  overflow: hidden;
}

.digit-wrapper {
  display: inline-block;
}

.digit {
  display: inline-block;
  transition: transform 0.3s ease-out, opacity 0.3s ease-out;
}

.digit-animate {
  animation: digit-change 0.4s ease-out;
}

@keyframes digit-change {
  0% {
    transform: translateY(100%);
    opacity: 0;
  }
  50% {
    transform: translateY(-10%);
    opacity: 0.8;
  }
  100% {
    transform: translateY(0);
    opacity: 1;
  }
}

/* 小数点样式 */
.digit-wrapper:last-child .digit {
  color: var(--text-tertiary);
}
</style>
