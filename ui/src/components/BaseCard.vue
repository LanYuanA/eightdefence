<template>
  <div
    class="base-card"
    :class="[
      `card-${variant}`,
      {
        'card-hoverable': hoverable,
        'card-no-padding': noPadding,
        'card-animate-in': animateIn
      }
    ]"
    :style="{ animationDelay: `${delay}ms` }"
  >
    <!-- 卡片头部 -->
    <div v-if="$slots.header || title" class="card-header-wrapper">
      <slot name="header">
        <div class="card-header-content">
          <div class="flex items-center gap-3">
            <div v-if="icon" class="card-icon" :class="`icon-${iconColor}`">
              <svg
                width="20"
                height="20"
                viewBox="0 0 24 24"
                fill="none"
                stroke="currentColor"
                stroke-width="2"
                v-html="icon"
              />
            </div>
            <div>
              <h3 class="card-title">{{ title }}</h3>
              <p v-if="subtitle" class="card-subtitle">{{ subtitle }}</p>
            </div>
          </div>
          <div v-if="$slots.actions" class="card-actions">
            <slot name="actions" />
          </div>
        </div>
      </slot>
    </div>

    <!-- 卡片内容 -->
    <div class="card-body" :class="{ 'no-padding': noPadding }">
      <slot />
    </div>

    <!-- 卡片底部 -->
    <div v-if="$slots.footer" class="card-footer">
      <slot name="footer" />
    </div>

    <!-- 装饰性光线效果 -->
    <div v-if="shine" class="card-shine" />
  </div>
</template>

<script setup lang="ts">
withDefaults(defineProps<{
  title?: string
  subtitle?: string
  icon?: string
  iconColor?: 'blue' | 'purple' | 'green' | 'yellow' | 'red' | 'cyan'
  variant?: 'default' | 'elevated' | 'outlined' | 'glass'
  hoverable?: boolean
  noPadding?: boolean
  animateIn?: boolean
  delay?: number
  shine?: boolean
}>(), {
  iconColor: 'blue',
  variant: 'default',
  hoverable: false,
  noPadding: false,
  animateIn: true,
  delay: 0,
  shine: false
})
</script>

<style scoped>
.base-card {
  background: var(--bg-card);
  border: 1px solid var(--border-primary);
  border-radius: var(--radius-lg);
  overflow: hidden;
  position: relative;
  transition: all var(--duration-normal) var(--ease-default);
}

/* 变体 */
.card-default {
  background: var(--bg-card);
}

.card-elevated {
  background: var(--bg-card);
  box-shadow: var(--shadow-md);
}

.card-outlined {
  background: transparent;
  border: 2px solid var(--border-primary);
}

.card-glass {
  background: var(--glass-bg);
  backdrop-filter: var(--glass-blur) var(--glass-saturate);
  -webkit-backdrop-filter: var(--glass-blur) var(--glass-saturate);
  border: 1px solid var(--glass-border);
}

/* 悬浮效果 */
.card-hoverable {
  cursor: pointer;
}

.card-hoverable:hover {
  transform: translateY(-4px);
  box-shadow: var(--shadow-lg);
  border-color: var(--border-accent);
}

.card-hoverable:hover .card-shine {
  animation: shine-sweep 0.8s ease-in-out;
}

/* 无内边距 */
.card-no-padding .card-body {
  padding: 0;
}

/* 入场动画 */
.card-animate-in {
  animation: fadeInUp var(--duration-slow) var(--ease-out) both;
}

/* 头部 */
.card-header-wrapper {
  padding: var(--spacing-lg);
  border-bottom: 1px solid var(--border-primary);
  background: linear-gradient(
    180deg,
    rgba(255, 255, 255, 0.03) 0%,
    transparent 100%
  );
}

.card-header-content {
  display: flex;
  align-items: center;
  justify-content: space-between;
}

.card-icon {
  width: 40px;
  height: 40px;
  display: flex;
  align-items: center;
  justify-content: center;
  border-radius: var(--radius-md);
  transition: all var(--duration-normal) var(--ease-default);
}

.icon-blue {
  background: rgba(59, 130, 246, 0.2);
  color: var(--accent-primary);
}

.icon-purple {
  background: rgba(139, 92, 246, 0.2);
  color: var(--accent-secondary);
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

.icon-cyan {
  background: rgba(6, 182, 212, 0.2);
  color: var(--status-info);
}

.card-title {
  font-size: 16px;
  font-weight: 600;
  color: var(--text-primary);
  margin: 0;
}

.card-subtitle {
  font-size: 13px;
  color: var(--text-secondary);
  margin-top: 4px;
}

.card-actions {
  display: flex;
  align-items: center;
  gap: var(--spacing-sm);
}

/* 内容 */
.card-body {
  padding: var(--spacing-lg);
}

/* 底部 */
.card-footer {
  padding: var(--spacing-md) var(--spacing-lg);
  border-top: 1px solid var(--border-primary);
  background: linear-gradient(
    0deg,
    rgba(255, 255, 255, 0.02) 0%,
    transparent 100%
  );
}

/* 光线效果 */
.card-shine {
  position: absolute;
  top: 0;
  left: 0;
  width: 100%;
  height: 100%;
  background: linear-gradient(
    90deg,
    transparent 0%,
    rgba(255, 255, 255, 0.05) 50%,
    transparent 100%
  );
  transform: translateX(-100%) skewX(-15deg);
  pointer-events: none;
}

/* 响应式 */
@media (max-width: 768px) {
  .card-header-content {
    flex-direction: column;
    align-items: flex-start;
    gap: var(--spacing-md);
  }

  .card-actions {
    width: 100%;
    justify-content: flex-end;
  }
}
</style>
