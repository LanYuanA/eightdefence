<template>
  <button
    class="cyber-btn"
    :class="[
      `btn-${variant}`,
      `btn-${size}`,
      {
        'btn-block': block,
        'btn-loading': loading,
        'btn-disabled': disabled
      }
    ]"
    :disabled="disabled || loading"
    @click="handleClick"
    @mousedown="isPressed = true"
    @mouseup="isPressed = false"
    @mouseleave="isPressed = false"
  >
    <!-- 背景动画层 -->
    <div class="btn-bg" />

    <!-- 边框动画 -->
    <div class="btn-border" />

    <!-- 加载状态 -->
    <div v-if="loading" class="btn-spinner">
      <svg class="spinner-icon" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
        <path d="M12 2v4m0 12v4M4.93 4.93l2.83 2.83m8.48 8.48l2.83 2.83M2 12h4m12 0h4M4.93 19.07l2.83-2.83m8.48-8.48l2.83-2.83"/>
      </svg>
    </div>

    <!-- 图标（左侧） -->
    <svg
      v-if="icon && !loading"
      class="btn-icon"
      :class="{ 'icon-only': !$slots.default }"
      width="16"
      height="16"
      viewBox="0 0 24 24"
      fill="none"
      stroke="currentColor"
      stroke-width="2"
      v-html="icon"
    />

    <!-- 内容 -->
    <span v-if="$slots.default" class="btn-content">
      <slot />
    </span>

    <!-- 图标（右侧） -->
    <svg
      v-if="suffixIcon && !loading"
      class="btn-icon btn-icon-right"
      width="16"
      height="16"
      viewBox="0 0 24 24"
      fill="none"
      stroke="currentColor"
      stroke-width="2"
      v-html="suffixIcon"
    />

    <!-- 波纹效果容器 -->
    <div class="ripple-container" ref="rippleContainer" />

    <!-- 悬浮光效 -->
    <div class="btn-glow" />
  </button>
</template>

<script setup lang="ts">
import { ref } from 'vue'

const props = withDefaults(defineProps<{
  variant?: 'primary' | 'secondary' | 'ghost' | 'danger' | 'success' | 'warning'
  size?: 'sm' | 'md' | 'lg'
  icon?: string
  suffixIcon?: string
  block?: boolean
  loading?: boolean
  disabled?: boolean
}>(), {
  variant: 'primary',
  size: 'md',
  block: false,
  loading: false,
  disabled: false
})

const emit = defineEmits<{
  click: [event: MouseEvent]
}>()

const isPressed = ref(false)
const rippleContainer = ref<HTMLElement | null>(null)

function handleClick(event: MouseEvent) {
  if (props.disabled || props.loading) return

  // 创建波纹效果
  createRipple(event)

  emit('click', event)
}

function createRipple(event: MouseEvent) {
  if (!rippleContainer.value) return

  const button = rippleContainer.value.parentElement
  if (!button) return

  const rect = button.getBoundingClientRect()
  const x = event.clientX - rect.left
  const y = event.clientY - rect.top

  const ripple = document.createElement('span')
  ripple.className = 'ripple'
  ripple.style.left = `${x}px`
  ripple.style.top = `${y}px`

  rippleContainer.value.appendChild(ripple)

  setTimeout(() => {
    ripple.remove()
  }, 600)
}
</script>

<style scoped>
.cyber-btn {
  position: relative;
  display: inline-flex;
  align-items: center;
  justify-content: center;
  gap: 8px;
  font-family: var(--font-sans);
  font-weight: 600;
  cursor: pointer;
  border: none;
  outline: none;
  overflow: hidden;
  transition: all var(--duration-normal) var(--ease-default);
  isolation: isolate;
}

/* 尺寸 */
.btn-sm {
  padding: 6px 12px;
  font-size: 12px;
  border-radius: var(--radius-sm);
}

.btn-md {
  padding: 10px 20px;
  font-size: 14px;
  border-radius: var(--radius-md);
}

.btn-lg {
  padding: 14px 28px;
  font-size: 16px;
  border-radius: var(--radius-lg);
}

/* 块级按钮 */
.btn-block {
  width: 100%;
}

/* 背景层 */
.btn-bg {
  position: absolute;
  top: 0;
  left: 0;
  width: 100%;
  height: 100%;
  z-index: -2;
  transition: all var(--duration-normal) var(--ease-default);
}

/* 边框层 */
.btn-border {
  position: absolute;
  top: 0;
  left: 0;
  width: 100%;
  height: 100%;
  z-index: -1;
  border-radius: inherit;
  transition: all var(--duration-normal) var(--ease-default);
}

/* 图标 */
.btn-icon {
  flex-shrink: 0;
  transition: transform var(--duration-normal) var(--ease-default);
}

.btn-icon.icon-only {
  margin: 0;
}

.btn-icon-right {
  order: 1;
}

/* 内容 */
.btn-content {
  position: relative;
  z-index: 1;
}

/* ===== 主要变体 ===== */
.btn-primary .btn-bg {
  background: var(--accent-gradient);
}

.btn-primary {
  color: white;
  box-shadow: 0 4px 15px rgba(59, 130, 246, 0.4);
}

.btn-primary:hover {
  box-shadow: 0 6px 20px rgba(59, 130, 246, 0.5);
  transform: translateY(-2px);
}

.btn-primary:hover .btn-icon {
  transform: scale(1.1);
}

.btn-primary:active {
  transform: translateY(0);
  box-shadow: 0 2px 10px rgba(59, 130, 246, 0.3);
}

/* ===== 次要变体 ===== */
.btn-secondary .btn-bg {
  background: transparent;
}

.btn-secondary .btn-border {
  border: 2px solid var(--border-primary);
}

.btn-secondary {
  color: var(--text-primary);
}

.btn-secondary:hover .btn-border {
  border-color: var(--accent-primary);
  box-shadow: 0 0 15px rgba(59, 130, 246, 0.2);
}

.btn-secondary:hover {
  background: rgba(59, 130, 246, 0.1);
}

/* ===== 幽灵变体 ===== */
.btn-ghost .btn-bg {
  background: transparent;
}

.btn-ghost {
  color: var(--text-secondary);
}

.btn-ghost:hover {
  background: rgba(255, 255, 255, 0.05);
  color: var(--text-primary);
}

/* ===== 危险变体 ===== */
.btn-danger .btn-bg {
  background: linear-gradient(135deg, #ef4444, #dc2626);
}

.btn-danger {
  color: white;
  box-shadow: 0 4px 15px rgba(239, 68, 68, 0.4);
}

.btn-danger:hover {
  box-shadow: 0 6px 20px rgba(239, 68, 68, 0.5);
  transform: translateY(-2px);
}

/* ===== 成功变体 ===== */
.btn-success .btn-bg {
  background: linear-gradient(135deg, #22c55d, #16a34a);
}

.btn-success {
  color: white;
  box-shadow: 0 4px 15px rgba(34, 197, 94, 0.4);
}

.btn-success:hover {
  box-shadow: 0 6px 20px rgba(34, 197, 94, 0.5);
  transform: translateY(-2px);
}

/* ===== 警告变体 ===== */
.btn-warning .btn-bg {
  background: linear-gradient(135deg, #f59e0b, #d97706);
}

.btn-warning {
  color: white;
  box-shadow: 0 4px 15px rgba(245, 158, 11, 0.4);
}

.btn-warning:hover {
  box-shadow: 0 6px 20px rgba(245, 158, 11, 0.5);
  transform: translateY(-2px);
}

/* ===== 加载状态 ===== */
.btn-loading {
  pointer-events: none;
  opacity: 0.8;
}

.btn-spinner {
  animation: spin 1s linear infinite;
}

.spinner-icon {
  width: 16px;
  height: 16px;
}

/* ===== 禁用状态 ===== */
.btn-disabled {
  pointer-events: none;
  opacity: 0.5;
  cursor: not-allowed;
}

/* ===== 波纹效果 ===== */
.ripple-container {
  position: absolute;
  top: 0;
  left: 0;
  width: 100%;
  height: 100%;
  overflow: hidden;
  z-index: 0;
  pointer-events: none;
}

:deep(.ripple) {
  position: absolute;
  width: 0;
  height: 0;
  border-radius: 50%;
  background: rgba(255, 255, 255, 0.3);
  transform: translate(-50%, -50%);
  animation: ripple-effect 0.6s ease-out;
}

/* ===== 悬浮光效 ===== */
.btn-glow {
  position: absolute;
  top: -50%;
  left: -50%;
  width: 200%;
  height: 200%;
  background: radial-gradient(
    circle,
    rgba(255, 255, 255, 0.1) 0%,
    transparent 50%
  );
  opacity: 0;
  transition: opacity var(--duration-normal) var(--ease-default);
  pointer-events: none;
  z-index: -1;
}

.cyber-btn:hover .btn-glow {
  opacity: 1;
}

/* ===== 按下效果 ===== */
.cyber-btn:active {
  transform: scale(0.98);
}

/* ===== 霓虹边框效果（悬浮时） ===== */
.btn-primary:hover::after,
.btn-secondary:hover::after,
.btn-danger:hover::after,
.btn-success:hover::after,
.btn-warning:hover::after {
  content: '';
  position: absolute;
  top: -2px;
  left: -2px;
  right: -2px;
  bottom: -2px;
  border-radius: inherit;
  background: var(--accent-gradient);
  z-index: -3;
  opacity: 0.3;
  filter: blur(8px);
  animation: pulse 2s ease-in-out infinite;
}
</style>
