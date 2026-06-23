<template>
  <div class="skeleton-loader" :class="`skeleton-${variant}`">
    <!-- 卡片骨架 -->
    <template v-if="variant === 'card'">
      <div class="skeleton-card">
        <div class="skeleton-header">
          <div class="skeleton-avatar" />
          <div class="skeleton-title-group">
            <div class="skeleton-line skeleton-title" />
            <div class="skeleton-line skeleton-subtitle" />
          </div>
        </div>
        <div class="skeleton-body">
          <div class="skeleton-line" v-for="i in lines" :key="i" :style="{ width: getLineWidth(i) }" />
        </div>
        <div v-if="hasFooter" class="skeleton-footer">
          <div class="skeleton-button" />
          <div class="skeleton-button skeleton-button-short" />
        </div>
      </div>
    </template>

    <!-- 数据卡片骨架 -->
    <template v-else-if="variant === 'data-card'">
      <div class="skeleton-data-card">
        <div class="skeleton-icon" />
        <div class="skeleton-data-content">
          <div class="skeleton-line skeleton-label" />
          <div class="skeleton-line skeleton-value" />
          <div class="skeleton-line skeleton-trend" />
        </div>
      </div>
    </template>

    <!-- 列表项骨架 -->
    <template v-else-if="variant === 'list-item'">
      <div class="skeleton-list-item" v-for="i in count" :key="i">
        <div class="skeleton-avatar skeleton-avatar-sm" />
        <div class="skeleton-list-content">
          <div class="skeleton-line skeleton-list-title" />
          <div class="skeleton-line skeleton-list-desc" />
        </div>
        <div class="skeleton-badge" />
      </div>
    </template>

    <!-- 表格行骨架 -->
    <template v-else-if="variant === 'table-row'">
      <div class="skeleton-table-row" v-for="i in count" :key="i">
        <div class="skeleton-cell" v-for="j in columns" :key="j" :style="{ width: `${100 / columns}%` }">
          <div class="skeleton-line" />
        </div>
      </div>
    </template>

    <!-- 图表骨架 -->
    <template v-else-if="variant === 'chart'">
      <div class="skeleton-chart">
        <div class="skeleton-chart-header">
          <div class="skeleton-line skeleton-chart-title" />
          <div class="skeleton-line skeleton-chart-subtitle" />
        </div>
        <div class="skeleton-chart-body">
          <div class="skeleton-chart-bar" v-for="i in 6" :key="i" :style="{ height: getBarHeight(i) }" />
        </div>
      </div>
    </template>

    <!-- 自定义线条 -->
    <template v-else>
      <div class="skeleton-line" v-for="i in lines" :key="i" :style="{ width: getLineWidth(i) }" />
    </template>
  </div>
</template>

<script setup lang="ts">
// SkeletonLoader component

const props = withDefaults(defineProps<{
  variant?: 'text' | 'card' | 'data-card' | 'list-item' | 'table-row' | 'chart'
  lines?: number
  count?: number
  columns?: number
  hasFooter?: boolean
}>(), {
  variant: 'text',
  lines: 3,
  count: 3,
  columns: 4,
  hasFooter: true
})

// 计算线条宽度
function getLineWidth(index: number): string {
  if (index === props.lines) return '60%'
  if (index === props.lines - 1) return '80%'
  return '100%'
}

// 计算柱状图高度
function getBarHeight(index: number): string {
  const heights = ['40%', '65%', '85%', '55%', '75%', '90%']
  return heights[index % heights.length]
}
</script>

<style scoped>
.skeleton-loader {
  width: 100%;
}

/* 基础线条 */
.skeleton-line {
  height: 14px;
  background: linear-gradient(
    90deg,
    var(--bg-card) 0%,
    var(--border-primary) 50%,
    var(--bg-card) 100%
  );
  background-size: 200px 100%;
  animation: skeleton-loading 1.5s ease-in-out infinite;
  border-radius: var(--radius-sm);
  margin-bottom: 12px;
}

.skeleton-line:last-child {
  margin-bottom: 0;
}

/* 卡片骨架 */
.skeleton-card {
  background: var(--bg-card);
  border: 1px solid var(--border-primary);
  border-radius: var(--radius-lg);
  padding: var(--spacing-lg);
}

.skeleton-header {
  display: flex;
  align-items: center;
  gap: var(--spacing-md);
  margin-bottom: var(--spacing-lg);
}

.skeleton-avatar {
  width: 48px;
  height: 48px;
  border-radius: var(--radius-md);
  background: linear-gradient(
    90deg,
    var(--bg-primary) 0%,
    var(--border-primary) 50%,
    var(--bg-primary) 100%
  );
  background-size: 200px 100%;
  animation: skeleton-loading 1.5s ease-in-out infinite;
  flex-shrink: 0;
}

.skeleton-avatar-sm {
  width: 36px;
  height: 36px;
}

.skeleton-title-group {
  flex: 1;
}

.skeleton-title {
  height: 18px;
  width: 60%;
  margin-bottom: 8px;
}

.skeleton-subtitle {
  height: 12px;
  width: 40%;
}

.skeleton-body {
  margin-bottom: var(--spacing-lg);
}

.skeleton-footer {
  display: flex;
  gap: var(--spacing-sm);
  padding-top: var(--spacing-md);
  border-top: 1px solid var(--border-primary);
}

.skeleton-button {
  height: 36px;
  width: 80px;
  border-radius: var(--radius-md);
  background: linear-gradient(
    90deg,
    var(--bg-primary) 0%,
    var(--border-primary) 50%,
    var(--bg-primary) 100%
  );
  background-size: 200px 100%;
  animation: skeleton-loading 1.5s ease-in-out infinite;
}

.skeleton-button-short {
  width: 60px;
}

/* 数据卡片骨架 */
.skeleton-data-card {
  display: flex;
  align-items: center;
  gap: var(--spacing-md);
  background: var(--bg-card);
  border: 1px solid var(--border-primary);
  border-radius: var(--radius-lg);
  padding: var(--spacing-lg);
}

.skeleton-icon {
  width: 52px;
  height: 52px;
  border-radius: var(--radius-md);
  background: linear-gradient(
    90deg,
    var(--bg-primary) 0%,
    var(--border-primary) 50%,
    var(--bg-primary) 100%
  );
  background-size: 200px 100%;
  animation: skeleton-loading 1.5s ease-in-out infinite;
  flex-shrink: 0;
}

.skeleton-data-content {
  flex: 1;
}

.skeleton-label {
  height: 12px;
  width: 50%;
  margin-bottom: 8px;
}

.skeleton-value {
  height: 28px;
  width: 70%;
  margin-bottom: 6px;
}

.skeleton-trend {
  height: 10px;
  width: 30%;
}

/* 列表项骨架 */
.skeleton-list-item {
  display: flex;
  align-items: center;
  gap: var(--spacing-md);
  padding: var(--spacing-md);
  border-bottom: 1px solid var(--border-primary);
}

.skeleton-list-item:last-child {
  border-bottom: none;
}

.skeleton-list-content {
  flex: 1;
}

.skeleton-list-title {
  height: 14px;
  width: 60%;
  margin-bottom: 6px;
}

.skeleton-list-desc {
  height: 10px;
  width: 40%;
}

.skeleton-badge {
  width: 48px;
  height: 20px;
  border-radius: var(--radius-full);
  background: linear-gradient(
    90deg,
    var(--bg-primary) 0%,
    var(--border-primary) 50%,
    var(--bg-primary) 100%
  );
  background-size: 200px 100%;
  animation: skeleton-loading 1.5s ease-in-out infinite;
}

/* 表格行骨架 */
.skeleton-table-row {
  display: flex;
  padding: var(--spacing-md);
  border-bottom: 1px solid var(--border-primary);
}

.skeleton-table-row:last-child {
  border-bottom: none;
}

.skeleton-cell {
  padding: 0 var(--spacing-sm);
}

/* 图表骨架 */
.skeleton-chart {
  background: var(--bg-card);
  border: 1px solid var(--border-primary);
  border-radius: var(--radius-lg);
  padding: var(--spacing-lg);
}

.skeleton-chart-header {
  margin-bottom: var(--spacing-lg);
}

.skeleton-chart-title {
  height: 16px;
  width: 40%;
  margin-bottom: 8px;
}

.skeleton-chart-subtitle {
  height: 10px;
  width: 25%;
}

.skeleton-chart-body {
  display: flex;
  align-items: flex-end;
  gap: var(--spacing-md);
  height: 150px;
  padding-top: var(--spacing-md);
}

.skeleton-chart-bar {
  flex: 1;
  background: linear-gradient(
    0deg,
    var(--bg-primary) 0%,
    var(--border-primary) 50%,
    var(--bg-primary) 100%
  );
  background-size: 100% 200px;
  animation: skeleton-loading 1.5s ease-in-out infinite;
  border-radius: var(--radius-sm) var(--radius-sm) 0 0;
}

/* 响应式 */
@media (max-width: 768px) {
  .skeleton-header {
    flex-direction: column;
    align-items: flex-start;
  }

  .skeleton-data-card {
    flex-direction: column;
    align-items: flex-start;
  }

  .skeleton-chart-body {
    height: 100px;
  }
}
</style>
