<template>
  <header class="nav-header">
    <div class="nav-inner">
      <!-- 品牌区域 -->
      <div class="flex items-center gap-4">
        <div class="brand-icon" @click="$router.push('/')">
          <svg width="28" height="28" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.8">
            <path d="M12 22s8-4 8-10V5l-8-3-8 3v7c0 6 8 10 8 10z"/>
          </svg>
        </div>
        <div>
          <div class="brand-title">{{ title }}</div>
          <div class="brand-sub">{{ subtitle }}</div>
        </div>
      </div>

      <!-- 导航菜单 -->
      <div class="flex items-center gap-3">
        <!-- 时间显示 -->
        <div class="nav-time">
          <svg width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" class="mr-1.5 opacity-60">
            <circle cx="12" cy="12" r="10"/>
            <path d="M12 6v6l4 2"/>
          </svg>
          {{ currentTime }}
        </div>

        <!-- 快速导航下拉菜单 -->
        <div class="nav-dropdown" @mouseenter="showDropdown = true" @mouseleave="showDropdown = false">
          <button class="nav-btn nav-btn-dropdown">
            <svg width="15" height="15" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
              <rect x="3" y="3" width="7" height="7"/><rect x="14" y="3" width="7" height="7"/>
              <rect x="14" y="14" width="7" height="7"/><rect x="3" y="14" width="7" height="7"/>
            </svg>
            功能菜单
            <svg width="12" height="12" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" class="ml-1">
              <polyline points="6 9 12 15 18 9"/>
            </svg>
          </button>

          <!-- 下拉菜单 -->
          <Transition name="dropdown">
            <div v-if="showDropdown" class="dropdown-menu">
              <!-- 核心功能 -->
              <div class="dropdown-section">
                <div class="dropdown-section-title">核心功能</div>
                <button class="dropdown-item" @click="navigateTo('/')">
                  <span class="dropdown-icon">🏠</span>
                  <span class="dropdown-label">主控台</span>
                  <span class="dropdown-desc">系统概览和数据流向</span>
                </button>
                <button class="dropdown-item" @click="navigateTo('/resource-pool-manager')">
                  <span class="dropdown-icon">📦</span>
                  <span class="dropdown-label">资源池管理</span>
                  <span class="dropdown-desc">自由组合设备和服务</span>
                </button>
                <button class="dropdown-item" @click="navigateTo('/workflow')">
                  <span class="dropdown-icon">🔀</span>
                  <span class="dropdown-label">流程编排</span>
                  <span class="dropdown-desc">拖拽式流程设计</span>
                </button>
              </div>

              <!-- 业务应用 -->
              <div class="dropdown-section">
                <div class="dropdown-section-title">业务应用</div>
                <button class="dropdown-item" @click="navigateTo('/security')">
                  <span class="dropdown-icon">🛡️</span>
                  <span class="dropdown-label">安防系统</span>
                  <span class="dropdown-desc">水浸/入侵/气体检测</span>
                </button>
                <button class="dropdown-item" @click="navigateTo('/environment')">
                  <span class="dropdown-icon">🌡️</span>
                  <span class="dropdown-label">环境监测</span>
                  <span class="dropdown-desc">温湿度/PM2.5/CO2</span>
                </button>
                <button class="dropdown-item" @click="navigateTo('/fire')">
                  <span class="dropdown-icon">🔥</span>
                  <span class="dropdown-label">消防系统</span>
                  <span class="dropdown-desc">烟雾/温度火灾检测</span>
                </button>
              </div>

              <!-- 设备管理 -->
              <div class="dropdown-section">
                <div class="dropdown-section-title">设备管理</div>
                <button class="dropdown-item" @click="navigateTo('/devices')">
                  <span class="dropdown-icon">📡</span>
                  <span class="dropdown-label">设备管理</span>
                  <span class="dropdown-desc">查看和管理所有设备</span>
                </button>
                <button class="dropdown-item" @click="navigateTo('/atomic-services')">
                  <span class="dropdown-icon">⚙️</span>
                  <span class="dropdown-label">原子服务</span>
                  <span class="dropdown-desc">管理原子服务</span>
                </button>
                <button class="dropdown-item" @click="navigateTo('/device-graph')">
                  <span class="dropdown-icon">🔗</span>
                  <span class="dropdown-label">设备图谱</span>
                  <span class="dropdown-desc">设备关系可视化</span>
                </button>
              </div>

              <!-- 智能分析 -->
              <div class="dropdown-section">
                <div class="dropdown-section-title">智能分析</div>
                <button class="dropdown-item" @click="navigateTo('/ai-analysis')">
                  <span class="dropdown-icon">🧠</span>
                  <span class="dropdown-label">AI分析</span>
                  <span class="dropdown-desc">智能分析和预测</span>
                </button>
                <button class="dropdown-item" @click="navigateTo('/scene-mode')">
                  <span class="dropdown-icon">🎬</span>
                  <span class="dropdown-label">场景模式</span>
                  <span class="dropdown-desc">一键切换场景</span>
                </button>
              </div>

              <!-- 系统管理 -->
              <div class="dropdown-section">
                <div class="dropdown-section-title">系统管理</div>
                <button class="dropdown-item" @click="navigateTo('/alarms')">
                  <span class="dropdown-icon">🔔</span>
                  <span class="dropdown-label">告警中心</span>
                  <span class="dropdown-desc">查看告警记录</span>
                </button>
                <button class="dropdown-item" @click="navigateTo('/screen')">
                  <span class="dropdown-icon">📊</span>
                  <span class="dropdown-label">数据大屏</span>
                  <span class="dropdown-desc">实时监控大屏</span>
                </button>
                <button class="dropdown-item" @click="navigateTo('/logs')">
                  <span class="dropdown-icon">📝</span>
                  <span class="dropdown-label">系统日志</span>
                  <span class="dropdown-desc">查看系统日志</span>
                </button>
                <button class="dropdown-item" @click="navigateTo('/settings')">
                  <span class="dropdown-icon">⚙️</span>
                  <span class="dropdown-label">系统设置</span>
                  <span class="dropdown-desc">系统配置</span>
                </button>
              </div>
            </div>
          </Transition>
        </div>

        <!-- 常用快捷按钮 -->
        <button class="nav-btn" :class="{ 'nav-btn-active': isActive('/') }" @click="$router.push('/')">
          <svg width="15" height="15" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M3 9l9-7 9 7v11a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2z"/><polyline points="9 22 9 12 15 12 15 22"/></svg>
          主控台
        </button>
        <button class="nav-btn" :class="{ 'nav-btn-active': isActive('/resource-pool-manager') }" @click="$router.push('/resource-pool-manager')">
          <svg width="15" height="15" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M21 16V8a2 2 0 0 0-1-1.73l-7-4a2 2 0 0 0-2 0l-7 4A2 2 0 0 0 3 8v8a2 2 0 0 0 1 1.73l7 4a2 2 0 0 0 2 0l7-4A2 2 0 0 0 21 16z"/></svg>
          资源池
        </button>
        <button class="nav-btn" :class="{ 'nav-btn-active': isActive('/devices') }" @click="$router.push('/devices')">
          <svg width="15" height="15" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><rect x="2" y="3" width="20" height="14" rx="2"/><path d="M8 21h8m-4-4v4"/></svg>
          设备
        </button>

        <!-- 退出按钮 -->
        <button class="nav-btn-danger" @click="handleLogout">
          <svg width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" class="mr-1">
            <path d="M9 21H5a2 2 0 0 1-2-2V5a2 2 0 0 1 2-2h4"/>
            <polyline points="16 17 21 12 16 7"/>
            <line x1="21" y1="12" x2="9" y2="12"/>
          </svg>
          退出
        </button>
      </div>
    </div>
  </header>
</template>

<script setup lang="ts">
import { ref, onMounted, onUnmounted } from 'vue'
import { useRouter, useRoute } from 'vue-router'

const props = withDefaults(defineProps<{
  title?: string
  subtitle?: string
}>(), {
  title: '软件定义消防系统',
  subtitle: 'Software Defined Monitoring Platform'
})

const router = useRouter()
const route = useRoute()
const currentTime = ref('')
const showDropdown = ref(false)

let timeInterval: ReturnType<typeof setInterval> | null = null

// 更新时间
function updateTime() {
  const now = new Date()
  currentTime.value = now.toLocaleString('zh-CN', {
    year: 'numeric',
    month: '2-digit',
    day: '2-digit',
    hour: '2-digit',
    minute: '2-digit',
    second: '2-digit',
    hour12: false
  })
}

// 判断是否是当前活动路由
function isActive(path: string): boolean {
  if (path === '/') {
    return route.path === '/'
  }
  return route.path.startsWith(path)
}

// 导航到指定页面
function navigateTo(path: string) {
  router.push(path)
  showDropdown.value = false
}

// 退出登录
function handleLogout() {
  localStorage.removeItem('isLoggedIn')
  router.push('/login')
}

onMounted(() => {
  updateTime()
  timeInterval = setInterval(updateTime, 1000)
})

onUnmounted(() => {
  if (timeInterval) {
    clearInterval(timeInterval)
  }
})
</script>

<style scoped>
.nav-header {
  background: var(--glass-bg);
  backdrop-filter: var(--glass-blur) var(--glass-saturate);
  -webkit-backdrop-filter: var(--glass-blur) var(--glass-saturate);
  border-bottom: 1px solid var(--glass-border);
  position: sticky;
  top: 0;
  z-index: var(--z-sticky);
  animation: slideDown var(--duration-normal) var(--ease-out);
}

.nav-inner {
  display: flex;
  align-items: center;
  justify-content: space-between;
  padding: var(--spacing-md) var(--spacing-xl);
  max-width: 1920px;
  margin: 0 auto;
}

.brand-icon {
  width: 48px;
  height: 48px;
  display: flex;
  align-items: center;
  justify-content: center;
  background: var(--accent-gradient);
  border-radius: var(--radius-md);
  color: white;
  box-shadow: var(--accent-glow);
  animation: breathe-glow 3s ease-in-out infinite;
  cursor: pointer;
  transition: transform var(--duration-fast);
}

.brand-icon:hover {
  transform: scale(1.05);
}

.brand-title {
  font-size: 18px;
  font-weight: 700;
  color: var(--text-primary);
  letter-spacing: 0.5px;
}

.brand-sub {
  font-size: 12px;
  color: var(--text-secondary);
  margin-top: 2px;
}

.nav-time {
  display: flex;
  align-items: center;
  padding: var(--spacing-sm) var(--spacing-md);
  background: rgba(255, 255, 255, 0.05);
  border-radius: var(--radius-md);
  font-size: 13px;
  font-family: var(--font-mono);
  color: var(--text-secondary);
  border: 1px solid var(--border-primary);
}

.nav-btn {
  display: flex;
  align-items: center;
  padding: var(--spacing-sm) var(--spacing-md);
  background: transparent;
  border: 1px solid transparent;
  border-radius: var(--radius-md);
  color: var(--text-secondary);
  font-size: 13px;
  font-weight: 500;
  cursor: pointer;
  transition: all var(--duration-normal) var(--ease-default);
  position: relative;
  overflow: hidden;
  white-space: nowrap;
}

.nav-btn::before {
  content: '';
  position: absolute;
  top: 50%;
  left: 50%;
  width: 0;
  height: 0;
  background: rgba(59, 130, 246, 0.2);
  border-radius: 50%;
  transform: translate(-50%, -50%);
  transition: width 0.6s ease, height 0.6s ease;
}

.nav-btn:hover {
  color: var(--text-primary);
  background: rgba(255, 255, 255, 0.05);
  border-color: var(--border-accent);
}

.nav-btn:hover::before {
  width: 200%;
  height: 200%;
}

.nav-btn-active {
  color: var(--accent-primary);
  background: rgba(59, 130, 246, 0.1);
  border-color: var(--accent-primary);
  box-shadow: 0 0 15px rgba(59, 130, 246, 0.2);
}

/* 下拉菜单按钮 */
.nav-btn-dropdown {
  background: rgba(59, 130, 246, 0.1);
  border-color: rgba(59, 130, 246, 0.3);
}

.nav-btn-dropdown:hover {
  background: rgba(59, 130, 246, 0.2);
  border-color: var(--accent-primary);
}

/* 下拉菜单容器 */
.nav-dropdown {
  position: relative;
}

/* 下拉菜单 */
.dropdown-menu {
  position: absolute;
  top: 100%;
  right: 0;
  width: 320px;
  background: var(--bg-card);
  border: 1px solid var(--border-primary);
  border-radius: var(--radius-lg);
  box-shadow: var(--shadow-xl);
  padding: var(--spacing-sm);
  z-index: var(--z-dropdown);
  margin-top: var(--spacing-xs);
}

/* 下拉菜单分组 */
.dropdown-section {
  padding: var(--spacing-xs) 0;
}

.dropdown-section:not(:last-child) {
  border-bottom: 1px solid var(--border-primary);
}

.dropdown-section-title {
  font-size: 10px;
  font-weight: 600;
  color: var(--text-tertiary);
  text-transform: uppercase;
  letter-spacing: 1px;
  padding: var(--spacing-xs) var(--spacing-sm);
  margin-bottom: var(--spacing-xs);
}

/* 下拉菜单项 */
.dropdown-item {
  display: flex;
  align-items: center;
  gap: var(--spacing-sm);
  width: 100%;
  padding: var(--spacing-sm) var(--spacing-md);
  background: transparent;
  border: none;
  border-radius: var(--radius-sm);
  cursor: pointer;
  transition: all var(--duration-fast);
  text-align: left;
}

.dropdown-item:hover {
  background: rgba(59, 130, 246, 0.1);
}

.dropdown-icon {
  font-size: 18px;
  width: 24px;
  text-align: center;
}

.dropdown-label {
  font-size: 13px;
  font-weight: 500;
  color: var(--text-primary);
  flex: 1;
}

.dropdown-desc {
  font-size: 11px;
  color: var(--text-tertiary);
}

/* 下拉菜单动画 */
.dropdown-enter-active {
  transition: all var(--duration-fast) var(--ease-out);
}

.dropdown-leave-active {
  transition: all var(--duration-fast) var(--ease-in);
}

.dropdown-enter-from,
.dropdown-leave-to {
  opacity: 0;
  transform: translateY(-10px) scale(0.95);
}

/* 退出按钮 */
.nav-btn-danger {
  display: flex;
  align-items: center;
  padding: var(--spacing-sm) var(--spacing-md);
  background: transparent;
  border: 1px solid var(--status-danger);
  border-radius: var(--radius-md);
  color: var(--status-danger);
  font-size: 13px;
  font-weight: 500;
  cursor: pointer;
  transition: all var(--duration-normal) var(--ease-default);
}

.nav-btn-danger:hover {
  background: rgba(239, 68, 68, 0.1);
  box-shadow: 0 0 15px rgba(239, 68, 68, 0.3);
  transform: translateY(-1px);
}

@keyframes slideDown {
  from {
    opacity: 0;
    transform: translateY(-100%);
  }
  to {
    opacity: 1;
    transform: translateY(0);
  }
}

/* 响应式调整 */
@media (max-width: 1200px) {
  .nav-time {
    display: none;
  }

  .nav-btn span:not(.nav-btn-icon) {
    display: none;
  }
}

@media (max-width: 768px) {
  .nav-inner {
    flex-direction: column;
    gap: var(--spacing-md);
    padding: var(--spacing-md);
  }

  .flex.items-center.gap-3 {
    flex-wrap: wrap;
    justify-content: center;
  }

  .dropdown-menu {
    width: 280px;
    right: -50px;
  }
}
</style>
