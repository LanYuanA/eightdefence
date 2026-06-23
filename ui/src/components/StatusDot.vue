<template>
  <div class="status-dot-wrapper" :class="`size-${size}`">
    <!-- 主状态点 -->
    <div
      class="status-dot"
      :class="[`status-${status}`, { 'dot-pulse': pulse }]"
      :style="{ '--dot-color': dotColor }"
    />

    <!-- 脉冲环（可选） -->
    <div
      v-if="pulse && status !== 'offline'"
      class="pulse-ring"
      :class="`ring-${status}`"
    />

    <!-- 标签（可选） -->
    <span v-if="label" class="status-label" :class="`label-${status}`">
      {{ label }}
    </span>
  </div>
</template>

<script setup lang="ts">
import { computed } from 'vue'

const props = withDefaults(defineProps<{
  status: 'online' | 'offline' | 'warning' | 'danger' | 'info'
  label?: string
  size?: 'sm' | 'md' | 'lg'
  pulse?: boolean
  color?: string
}>(), {
  size: 'md',
  pulse: true
})

const dotColor = computed(() => {
  if (props.color) return props.color

  const colors = {
    online: 'var(--status-success)',
    offline: 'var(--text-tertiary)',
    warning: 'var(--status-warning)',
    danger: 'var(--status-danger)',
    info: 'var(--status-info)'
  }
  return colors[props.status]
})
</script>

<style scoped>
.status-dot-wrapper {
  display: inline-flex;
  align-items: center;
  gap: 8px;
  position: relative;
}

/* 尺寸 */
.size-sm .status-dot {
  width: 8px;
  height: 8px;
}

.size-md .status-dot {
  width: 12px;
  height: 12px;
}

.size-lg .status-dot {
  width: 16px;
  height: 16px;
}

/* 状态点 */
.status-dot {
  border-radius: 50%;
  position: relative;
  z-index: 1;
  transition: all var(--duration-normal) var(--ease-default);
}

/* 状态颜色 */
.status-online {
  background: var(--status-success);
  box-shadow: var(--status-success-glow);
}

.status-offline {
  background: var(--text-tertiary);
  box-shadow: none;
}

.status-warning {
  background: var(--status-warning);
  box-shadow: var(--status-warning-glow);
}

.status-danger {
  background: var(--status-danger);
  box-shadow: var(--status-danger-glow);
}

.status-info {
  background: var(--status-info);
  box-shadow: var(--status-info-glow);
}

/* 脉冲动画 */
.dot-pulse.status-online {
  animation: breathe 2s ease-in-out infinite;
}

.dot-pulse.status-warning {
  animation: pulse 1.5s ease-in-out infinite;
}

.dot-pulse.status-danger {
  animation: blink-fast 0.8s ease-in-out infinite;
}

.dot-pulse.status-info {
  animation: breathe 2.5s ease-in-out infinite;
}

/* 脉冲环 */
.pulse-ring {
  position: absolute;
  top: 50%;
  left: 50%;
  transform: translate(-50%, -50%);
  border-radius: 50%;
  animation: pulse-expand 2s ease-out infinite;
  z-index: 0;
}

.size-sm .pulse-ring {
  width: 8px;
  height: 8px;
}

.size-md .pulse-ring {
  width: 12px;
  height: 12px;
}

.size-lg .pulse-ring {
  width: 16px;
  height: 16px;
}

.ring-online {
  border: 2px solid var(--status-success);
}

.ring-warning {
  border: 2px solid var(--status-warning);
}

.ring-danger {
  border: 2px solid var(--status-danger);
}

.ring-info {
  border: 2px solid var(--status-info);
}

/* 标签 */
.status-label {
  font-size: 13px;
  font-weight: 500;
  transition: color var(--duration-normal) var(--ease-default);
}

.label-online {
  color: var(--status-success);
}

.label-offline {
  color: var(--text-tertiary);
}

.label-warning {
  color: var(--status-warning);
}

.label-danger {
  color: var(--status-danger);
}

.label-info {
  color: var(--status-info);
}

/* 悬浮效果 */
.status-dot-wrapper:hover .status-dot {
  transform: scale(1.2);
}

.status-dot-wrapper:hover .pulse-ring {
  animation-duration: 1s;
}
</style>
