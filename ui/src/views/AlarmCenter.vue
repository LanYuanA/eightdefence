<template>
  <div class="alarm-root">
    <ParticleBackground :particle-count="50" color="#ef4444" :opacity="0.4" />
    <AppNavbar title="告警中心" subtitle="Alarm Center" :menu-items="menuItems" />

    <main class="alarm-main">
      <!-- 告警概览 -->
      <section class="overview-section">
        <div class="section-header">
          <h2 class="section-title">
            <svg width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M10.29 3.86L1.82 18a2 2 0 0 0 1.71 3h16.94a2 2 0 0 0 1.71-3L13.71 3.86a2 2 0 0 0-3.42 0z"/><line x1="12" y1="9" x2="12" y2="13"/><line x1="12" y1="17" x2="12.01" y2="17"/></svg>
            告警概览
          </h2>
        </div>
        <div class="alarm-stats">
          <div class="alarm-stat-card critical">
            <div class="stat-icon">🔴</div>
            <div class="stat-info"><div class="stat-value">{{ criticalCount }}</div><div class="stat-label">紧急告警</div></div>
          </div>
          <div class="alarm-stat-card warning">
            <div class="stat-icon">🟠</div>
            <div class="stat-info"><div class="stat-value">{{ warningCount }}</div><div class="stat-label">一般告警</div></div>
          </div>
          <div class="alarm-stat-card info">
            <div class="stat-icon">🟡</div>
            <div class="stat-info"><div class="stat-value">{{ infoCount }}</div><div class="stat-label">提示信息</div></div>
          </div>
          <div class="alarm-stat-card resolved">
            <div class="stat-icon">🟢</div>
            <div class="stat-info"><div class="stat-value">{{ resolvedCount }}</div><div class="stat-label">已处理</div></div>
          </div>
        </div>
      </section>

      <!-- 告警列表 -->
      <section class="alarm-list-section">
        <BaseCard title="告警记录" subtitle="实时告警监控" icon-color="red">
          <template #header>
            <div class="flex items-center justify-between w-full">
              <div class="flex items-center gap-3">
                <div class="card-icon icon-red">
                  <svg width="20" height="20" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M18 8A6 6 0 0 0 6 8c0 7-3 9-3 9h18s-3-2-3-9"/><path d="M13.73 21a2 2 0 0 1-3.46 0"/></svg>
                </div>
                <div><h3 class="card-title">告警记录</h3><p class="card-subtitle">实时告警监控</p></div>
              </div>
              <div class="flex gap-2">
                <CyberButton variant="ghost" size="sm" @click="clearAll">清空全部</CyberButton>
                <CyberButton variant="ghost" size="sm" @click="exportAlarms">导出日志</CyberButton>
              </div>
            </div>
          </template>

          <!-- 筛选器 -->
          <div class="alarm-filters">
            <button v-for="f in filters" :key="f.value" class="filter-btn" :class="{ active: activeFilter === f.value }" @click="activeFilter = f.value">
              <span class="filter-dot" :style="{ background: f.color }"></span>
              {{ f.label }}
            </button>
          </div>

          <!-- 告警列表 -->
          <div class="alarm-list">
            <div v-for="alarm in filteredAlarms" :key="alarm.id" class="alarm-item" :class="[`level-${alarm.level}`, { unread: !alarm.read }]">
              <div class="alarm-icon">{{ alarm.icon }}</div>
              <div class="alarm-content">
                <div class="alarm-header">
                  <span class="alarm-title">{{ alarm.title }}</span>
                  <span class="alarm-time">{{ alarm.time }}</span>
                </div>
                <div class="alarm-desc">{{ alarm.description }}</div>
                <div class="alarm-meta">
                  <span class="alarm-source">{{ alarm.source }}</span>
                  <span class="alarm-level-badge" :class="alarm.level">{{ levelText(alarm.level) }}</span>
                </div>
              </div>
              <div class="alarm-actions">
                <button class="action-btn" @click="handleAlarm(alarm)">处理</button>
                <button class="action-btn" @click="ignoreAlarm(alarm)">忽略</button>
              </div>
            </div>
            <div v-if="filteredAlarms.length === 0" class="empty-state">
              <div class="empty-icon">🔔</div>
              <div class="empty-text">暂无告警记录</div>
            </div>
          </div>
        </BaseCard>
      </section>
    </main>
  </div>
</template>

<script setup lang="ts">
import { ref, reactive, computed } from 'vue'
import { ElMessage } from 'element-plus'
import ParticleBackground from '../components/ParticleBackground.vue'
import AppNavbar from '../components/AppNavbar.vue'
import BaseCard from '../components/BaseCard.vue'
import CyberButton from '../components/CyberButton.vue'

const menuItems = [
  { path: '/', label: '主控台' },
  { path: '/security', label: '安防系统' },
  { path: '/environment', label: '环境监测' },
  { path: '/fire', label: '消防系统' },
  { path: '/devices', label: '设备管理' }
]

const activeFilter = ref('all')

const filters = [
  { value: 'all', label: '全部', color: '#3b82f6' },
  { value: 'critical', label: '紧急', color: '#ef4444' },
  { value: 'warning', label: '警告', color: '#f59e0b' },
  { value: 'info', label: '提示', color: '#06b6d4' }
]

// 告警数据（带状态管理）
interface Alarm {
  id: number
  title: string
  description: string
  source: string
  level: 'critical' | 'warning' | 'info'
  icon: string
  time: string
  read: boolean
  status: 'active' | 'acknowledged' | 'resolved'
}

const alarms = reactive<Alarm[]>([
  { id: 1, title: '温度异常报警', description: '区域A温度超过阈值，当前温度58.5℃', source: '温度传感器', level: 'critical', icon: '🔴', time: '2024-01-15 10:30:00', read: false, status: 'active' },
  { id: 2, title: '烟雾浓度超标', description: '检测到烟雾浓度异常，当前2.5%obs/m', source: '烟雾报警器', level: 'critical', icon: '🔴', time: '2024-01-15 10:29:00', read: false, status: 'active' },
  { id: 3, title: '湿度偏低警告', description: '环境湿度低于设定阈值，当前25%', source: '湿度传感器', level: 'warning', icon: '🟠', time: '2024-01-15 10:28:00', read: true, status: 'acknowledged' },
  { id: 4, title: '设备离线提示', description: '通风净化机已离线超过30分钟', source: '设备监控', level: 'info', icon: '🟡', time: '2024-01-15 10:27:00', read: true, status: 'resolved' },
  { id: 5, title: 'CO₂浓度偏高', description: '室内CO₂浓度达到800ppm', source: '气体传感器', level: 'warning', icon: '🟠', time: '2024-01-15 10:26:00', read: true, status: 'acknowledged' }
])

// 统计计算
const criticalCount = computed(() => alarms.filter(a => a.level === 'critical' && a.status === 'active').length)
const warningCount = computed(() => alarms.filter(a => a.level === 'warning' && a.status === 'active').length)
const infoCount = computed(() => alarms.filter(a => a.level === 'info' && a.status === 'active').length)
const resolvedCount = computed(() => alarms.filter(a => a.status === 'resolved').length)

const filteredAlarms = computed(() => {
  if (activeFilter.value === 'all') return alarms
  return alarms.filter(a => a.level === activeFilter.value)
})

function levelText(level: string) {
  return { critical: '紧急', warning: '警告', info: '提示' }[level] || level
}

// 处理告警
function handleAlarm(alarm: Alarm) {
  alarm.status = 'resolved'
  alarm.read = true
  ElMessage.success('告警已处理')
}

// 忽略告警
function ignoreAlarm(alarm: Alarm) {
  alarm.status = 'acknowledged'
  alarm.read = true
  ElMessage.info('告警已忽略')
}

// 清空所有告警
function clearAll() {
  alarms.splice(0, alarms.length)
  ElMessage.success('告警已清空')
}

// 导出告警日志
function exportAlarms() {
  const exportData = alarms.map(a => ({
    时间: a.time,
    标题: a.title,
    描述: a.description,
    来源: a.source,
    级别: levelText(a.level),
    状态: a.status === 'active' ? '活跃' : a.status === 'acknowledged' ? '已确认' : '已处理'
  }))

  const blob = new Blob([JSON.stringify(exportData, null, 2)], { type: 'application/json' })
  const url = URL.createObjectURL(blob)
  const link = document.createElement('a')
  link.href = url
  link.download = `alarm-logs-${new Date().toISOString().split('T')[0]}.json`
  link.click()
  URL.revokeObjectURL(url)

  ElMessage.success('告警日志已导出')
}
</script>

<style scoped>
.alarm-root { min-height: 100vh; background: var(--bg-primary); position: relative; }
.alarm-main { position: relative; z-index: 1; padding: var(--spacing-xl); max-width: 1920px; margin: 0 auto; }

.section-header { margin-bottom: var(--spacing-lg); }
.section-title { display: flex; align-items: center; gap: var(--spacing-sm); font-size: 20px; font-weight: 600; color: var(--text-primary); margin: 0; }
.section-title svg { color: var(--status-danger); }

.alarm-stats { display: grid; grid-template-columns: repeat(4, 1fr); gap: var(--spacing-lg); margin-bottom: var(--spacing-xl); }
.alarm-stat-card { display: flex; align-items: center; gap: var(--spacing-md); padding: var(--spacing-lg); background: var(--bg-card); border: 1px solid var(--border-primary); border-radius: var(--radius-lg); }
.alarm-stat-card.critical { border-left: 4px solid #ef4444; }
.alarm-stat-card.warning { border-left: 4px solid #f59e0b; }
.alarm-stat-card.info { border-left: 4px solid #06b6d4; }
.alarm-stat-card.resolved { border-left: 4px solid #22c55d; }
.stat-icon { font-size: 32px; }
.stat-value { font-size: 28px; font-weight: 700; color: var(--text-primary); font-family: var(--font-mono); }
.stat-label { font-size: 13px; color: var(--text-secondary); }

.alarm-filters { display: flex; gap: var(--spacing-sm); margin-bottom: var(--spacing-lg); }
.filter-btn { display: flex; align-items: center; gap: 6px; padding: 6px 12px; background: transparent; border: 1px solid var(--border-primary); border-radius: var(--radius-full); color: var(--text-secondary); font-size: 12px; cursor: pointer; transition: all var(--duration-fast); }
.filter-btn:hover { border-color: var(--accent-primary); color: var(--text-primary); }
.filter-btn.active { background: rgba(59,130,246,0.2); border-color: var(--accent-primary); color: var(--accent-primary); }
.filter-dot { width: 8px; height: 8px; border-radius: 50%; }

.alarm-list { display: flex; flex-direction: column; gap: var(--spacing-sm); max-height: 600px; overflow-y: auto; }
.alarm-item { display: flex; align-items: flex-start; gap: var(--spacing-md); padding: var(--spacing-md); background: rgba(0,0,0,0.2); border: 1px solid var(--border-primary); border-radius: var(--radius-md); transition: all var(--duration-fast); }
.alarm-item:hover { background: rgba(0,0,0,0.3); }
.alarm-item.unread { border-left: 3px solid var(--status-danger); }
.alarm-item.level-critical { border-left-color: #ef4444; }
.alarm-item.level-warning { border-left-color: #f59e0b; }
.alarm-item.level-info { border-left-color: #06b6d4; }
.alarm-icon { font-size: 24px; }
.alarm-content { flex: 1; }
.alarm-header { display: flex; justify-content: space-between; align-items: center; margin-bottom: 4px; }
.alarm-title { font-size: 14px; font-weight: 600; color: var(--text-primary); }
.alarm-time { font-size: 11px; color: var(--text-tertiary); font-family: var(--font-mono); }
.alarm-desc { font-size: 13px; color: var(--text-secondary); margin-bottom: 8px; }
.alarm-meta { display: flex; gap: var(--spacing-sm); }
.alarm-source { font-size: 11px; color: var(--text-tertiary); }
.alarm-level-badge { font-size: 11px; padding: 2px 8px; border-radius: var(--radius-full); }
.alarm-level-badge.critical { background: rgba(239,68,68,0.2); color: #ef4444; }
.alarm-level-badge.warning { background: rgba(245,158,11,0.2); color: #f59e0b; }
.alarm-level-badge.info { background: rgba(6,182,212,0.2); color: #06b6d4; }
.alarm-actions { display: flex; flex-direction: column; gap: 4px; }
.action-btn { padding: 4px 8px; background: transparent; border: 1px solid var(--border-primary); border-radius: var(--radius-sm); color: var(--text-secondary); font-size: 11px; cursor: pointer; transition: all var(--duration-fast); }
.action-btn:hover { border-color: var(--accent-primary); color: var(--accent-primary); }

.empty-state { display: flex; flex-direction: column; align-items: center; justify-content: center; padding: var(--spacing-2xl); color: var(--text-tertiary); }
.empty-icon { font-size: 48px; margin-bottom: var(--spacing-md); opacity: 0.5; }
.empty-text { font-size: 14px; }

@media (max-width: 1200px) { .alarm-stats { grid-template-columns: repeat(2, 1fr); } }
@media (max-width: 768px) { .alarm-main { padding: var(--spacing-md); } .alarm-stats { grid-template-columns: 1fr; } }
</style>
