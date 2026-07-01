<template>
  <div class="security-root">
    <ParticleBackground :particle-count="50" color="#3b82f6" :opacity="0.4" />
    <AppNavbar title="智能安防系统" subtitle="Security Monitoring System" :menu-items="menuItems" />

    <main class="security-main">
      <!-- 状态概览 -->
      <section class="status-section">
        <div class="section-header">
          <h2 class="section-title">
            <svg width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M12 22s8-4 8-10V5l-8-3-8 3v7c0 6 8 10 8 10z"/></svg>
            安防系统概览
          </h2>
        </div>
        <div class="status-grid">
          <div class="status-card" :class="systemStatus">
            <div class="status-icon">🛡️</div>
            <div class="status-info">
              <div class="status-label">系统状态</div>
              <div class="status-value">{{ systemStatus === 'normal' ? '运行正常' : '存在异常' }}</div>
            </div>
            <StatusDot :status="systemStatus === 'normal' ? 'online' : 'warning'" size="lg" />
          </div>
          <div class="status-card" :class="alarmActive ? 'danger' : 'normal'">
            <div class="status-icon">🔔</div>
            <div class="status-info">
              <div class="status-label">报警状态</div>
              <div class="status-value">{{ alarmActive ? '报警中' : '待机中' }}</div>
            </div>
            <StatusDot :status="alarmActive ? 'danger' : 'online'" size="lg" />
          </div>
          <div class="status-card">
            <div class="status-icon">📹</div>
            <div class="status-info">
              <div class="status-label">在线设备</div>
              <div class="status-value">{{ onlineCount }}/{{ totalCount }}</div>
            </div>
            <StatusDot status="online" size="lg" />
          </div>
          <div class="status-card">
            <div class="status-icon">⚡</div>
            <div class="status-info">
              <div class="status-label">今日事件</div>
              <div class="status-value">{{ todayEvents }} 条</div>
            </div>
            <StatusDot status="online" size="lg" />
          </div>
        </div>
      </section>

      <!-- 安防场景卡片 -->
      <section class="scenarios-section">
        <div class="section-header">
          <h2 class="section-title">安防场景监控</h2>
        </div>
        <div class="scenarios-grid">
          <!-- 水浸检测 -->
          <BaseCard title="水浸风险安防" subtitle="水浸隐患监测与处置" icon-color="blue">
            <template #header>
              <div class="flex items-center gap-3">
                <div class="card-icon icon-blue">
                  <svg width="20" height="20" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M12 2.69l5.66 5.66a8 8 0 1 1-11.31 0z"/></svg>
                </div>
                <div><h3 class="card-title">水浸风险安防</h3><p class="card-subtitle">水浸隐患监测与处置</p></div>
              </div>
            </template>
            <div class="scenario-data">
              <div class="data-row"><span class="data-label">传感器状态</span><span class="data-value" :class="waterOnline ? 'text-success' : 'text-danger'">{{ waterOnline ? '在线' : '离线' }}</span></div>
              <div class="data-row"><span class="data-label">水浸检测</span><span class="data-value" :class="waterDetected ? 'text-danger' : ''">{{ waterDetected ? '检测到水浸' : '正常' }}</span></div>
              <div class="data-row"><span class="data-label">水位高度</span><span class="data-value">{{ waterLevel }}cm</span></div>
              <div class="data-row"><span class="data-label">风险等级</span><span class="data-value" :class="waterRiskClass">{{ waterRisk }}</span></div>
            </div>
            <div class="scenario-actions">
              <CyberButton variant="danger" size="sm" @click="simulateWater">模拟异常</CyberButton>
              <CyberButton variant="ghost" size="sm" @click="resetWater">恢复正常</CyberButton>
            </div>
          </BaseCard>

          <!-- 入侵检测 -->
          <BaseCard title="人员入侵检测" subtitle="红外+雷达双鉴探测" icon-color="purple">
            <template #header>
              <div class="flex items-center gap-3">
                <div class="card-icon icon-purple">
                  <svg width="20" height="20" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M20 21v-2a4 4 0 0 0-4-4H8a4 4 0 0 0-4 4v2"/><circle cx="12" cy="7" r="4"/></svg>
                </div>
                <div><h3 class="card-title">人员入侵检测</h3><p class="card-subtitle">红外+雷达双鉴探测</p></div>
              </div>
            </template>
            <div class="scenario-data">
              <div class="data-row"><span class="data-label">探测器状态</span><span class="data-value" :class="intrusionOnline ? 'text-success' : 'text-danger'">{{ intrusionOnline ? '在线' : '离线' }}</span></div>
              <div class="data-row"><span class="data-label">红外检测</span><span class="data-value" :class="infraredDetected ? 'text-danger' : ''">{{ infraredDetected ? '检测到移动' : '正常' }}</span></div>
              <div class="data-row"><span class="data-label">雷达检测</span><span class="data-value" :class="radarDetected ? 'text-danger' : ''">{{ radarDetected ? '检测到目标' : '正常' }}</span></div>
              <div class="data-row"><span class="data-label">风险等级</span><span class="data-value" :class="intrusionRiskClass">{{ intrusionRisk }}</span></div>
            </div>
            <div class="scenario-actions">
              <CyberButton variant="danger" size="sm" @click="simulateIntrusion">模拟入侵</CyberButton>
              <CyberButton variant="ghost" size="sm" @click="resetIntrusion">恢复正常</CyberButton>
            </div>
          </BaseCard>

          <!-- 气体检测 -->
          <BaseCard title="有害气体防护" subtitle="TVOC/CH2O/CO2监测" icon-color="yellow">
            <template #header>
              <div class="flex items-center gap-3">
                <div class="card-icon icon-yellow">
                  <svg width="20" height="20" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M18.5 8c.83 0 1.5-.67 1.5-1.5S19.33 5 18.5 5c-.17 0-.34.03-.5.08A2.5 2.5 0 0 0 16 3a2.5 2.5 0 0 0-2 4v.02A1.5 1.5 0 0 0 13 8.5c0 .83.67 1.5 1.5 1.5h4z"/></svg>
                </div>
                <div><h3 class="card-title">有害气体防护</h3><p class="card-subtitle">TVOC/CH2O/CO2监测</p></div>
              </div>
            </template>
            <div class="scenario-data">
              <div class="data-row"><span class="data-label">TVOC浓度</span><span class="data-value">{{ tvocValue }}ppb</span></div>
              <div class="data-row"><span class="data-label">甲醛浓度</span><span class="data-value">{{ ch2oValue }}ppb</span></div>
              <div class="data-row"><span class="data-label">CO₂浓度</span><span class="data-value">{{ co2Value }}ppm</span></div>
              <div class="data-row"><span class="data-label">风险等级</span><span class="data-value" :class="gasRiskClass">{{ gasRisk }}</span></div>
            </div>
            <div class="scenario-actions">
              <CyberButton variant="danger" size="sm" @click="simulateGas">模拟异常</CyberButton>
              <CyberButton variant="ghost" size="sm" @click="resetGas">恢复正常</CyberButton>
            </div>
          </BaseCard>
        </div>
      </section>

      <!-- 数据流向和实时监控 -->
      <section class="monitor-section">
        <div class="flow-panel">
          <BaseCard title="安防数据流向" subtitle="实时数据传输" icon-color="blue">
            <template #header>
              <div class="flex items-center gap-3">
                <div class="card-icon icon-blue">
                  <svg width="20" height="20" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M22 12h-4l-3 9L9 3l-3 9H2"/></svg>
                </div>
                <div><h3 class="card-title">安防数据流向</h3><p class="card-subtitle">实时数据传输</p></div>
              </div>
            </template>
            <DataFlowVisualizer :nodes="securityFlowNodes" :flows="securityFlowFlows" flow-color="#3b82f6" :show-controls="true" />
          </BaseCard>
        </div>
        <div class="stream-panel">
          <BaseCard title="安防事件流" subtitle="实时告警监控" icon-color="red">
            <template #header>
              <div class="flex items-center gap-3">
                <div class="card-icon icon-red">
                  <svg width="20" height="20" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M10.29 3.86L1.82 18a2 2 0 0 0 1.71 3h16.94a2 2 0 0 0 1.71-3L13.71 3.86a2 2 0 0 0-3.42 0z"/><line x1="12" y1="9" x2="12" y2="13"/><line x1="12" y1="17" x2="12.01" y2="17"/></svg>
                </div>
                <div><h3 class="card-title">安防事件流</h3><p class="card-subtitle">实时告警监控</p></div>
              </div>
            </template>
            <LiveDataStream :max-items="20" />
          </BaseCard>
        </div>
      </section>
    </main>
  </div>
</template>

<script setup lang="ts">
import { ref, computed } from 'vue'
import { ElMessage } from 'element-plus'
import ParticleBackground from '../components/ParticleBackground.vue'
import AppNavbar from '../components/AppNavbar.vue'
import BaseCard from '../components/BaseCard.vue'
import CyberButton from '../components/CyberButton.vue'
import StatusDot from '../components/StatusDot.vue'
import DataFlowVisualizer from '../components/DataFlowVisualizer.vue'
import LiveDataStream from '../components/LiveDataStream.vue'

const menuItems = [
  { path: '/', label: '主控台' },
  { path: '/environment', label: '环境监测' },
  { path: '/fire', label: '消防系统' },
  { path: '/devices', label: '设备管理' },
  { path: '/logs', label: '系统日志' }
]

// 状态数据
const systemStatus = ref('normal')
const alarmActive = ref(false)
const onlineCount = ref(8)
const totalCount = ref(10)
const todayEvents = ref(12)

// 水浸检测
const waterOnline = ref(true)
const waterDetected = ref(false)
const waterLevel = ref(0)
const waterRisk = ref('低')
const waterRiskClass = computed(() => waterRisk.value === '高' ? 'text-danger' : waterRisk.value === '中' ? 'text-warning' : '')

// 入侵检测
const intrusionOnline = ref(true)
const infraredDetected = ref(false)
const radarDetected = ref(false)
const intrusionRisk = ref('低')
const intrusionRiskClass = computed(() => intrusionRisk.value === '高' ? 'text-danger' : intrusionRisk.value === '中' ? 'text-warning' : '')

// 气体检测
const tvocValue = ref(120)
const ch2oValue = ref(30)
const co2Value = ref(450)
const gasRisk = ref('低')
const gasRiskClass = computed(() => gasRisk.value === '高' ? 'text-danger' : gasRisk.value === '中' ? 'text-warning' : '')

// 软件定义架构数据流向
const securityFlowNodes = [
  // 设备层
  { id: 'sensor-water', x: 140, y: 280, size: 18, color: '#06b6d4', icon: '💧', label: '水浸传感器', active: true, dataCount: 32 },
  { id: 'sensor-infrared', x: 280, y: 280, size: 18, color: '#8b5cf6', icon: '👤', label: '红外探测', active: true, dataCount: 48 },
  { id: 'sensor-gas', x: 420, y: 280, size: 18, color: '#f59e0b', icon: '☁️', label: '气体传感', active: true, dataCount: 64 },
  // 设备抽象层
  { id: 'abs-alarm', x: 280, y: 210, size: 20, color: '#ef4444', icon: '🔔', label: '报警抽象', active: true, dataCount: 96 },
  // 原子服务下层
  { id: 'lower-collect', x: 180, y: 140, size: 22, color: '#3b82f6', icon: '📥', label: '数据采集', active: true, dataCount: 96 },
  { id: 'lower-alarm', x: 330, y: 140, size: 20, color: '#ef4444', icon: '🔔', label: '报警判断', active: true, dataCount: 48 },
  { id: 'lower-control', x: 480, y: 140, size: 18, color: '#22c55d', icon: '🎮', label: '设备控制', active: true, dataCount: 16 },
  // 原子服务上层
  { id: 'upper-security', x: 280, y: 70, size: 24, color: '#8b5cf6', icon: '🛡️', label: '安防监控服务', active: true, dataCount: 48 },
  // 应用层
  { id: 'app-security', x: 280, y: 15, size: 20, color: '#8b5cf6', icon: '🛡️', label: '安防系统', active: true, dataCount: 24 }
]

const securityFlowFlows = [
  // 设备层 → 设备抽象层
  { from: 'sensor-water', to: 'abs-alarm', active: true, speed: 2 },
  { from: 'sensor-infrared', to: 'abs-alarm', active: true, speed: 2 },
  { from: 'sensor-gas', to: 'abs-alarm', active: true, speed: 2 },
  // 设备抽象层 → 原子服务下层
  { from: 'abs-alarm', to: 'lower-collect', active: true, speed: 2 },
  // 原子服务下层 → 原子服务下层
  { from: 'lower-collect', to: 'lower-alarm', active: true, speed: 2 },
  { from: 'lower-alarm', to: 'lower-control', active: true, speed: 2 },
  // 原子服务下层 → 原子服务上层
  { from: 'lower-collect', to: 'upper-security', active: true, speed: 2 },
  { from: 'lower-alarm', to: 'upper-security', active: true, speed: 2 },
  { from: 'lower-control', to: 'upper-security', active: true, speed: 2 },
  // 原子服务上层 → 应用层
  { from: 'upper-security', to: 'app-security', active: true, speed: 2 }
]

// 模拟操作
function simulateWater() {
  waterDetected.value = true
  waterLevel.value = 5.2
  waterRisk.value = '高'
  alarmActive.value = true
  ElMessage.warning('模拟水浸异常')
}

function resetWater() {
  waterDetected.value = false
  waterLevel.value = 0
  waterRisk.value = '低'
  alarmActive.value = false
  ElMessage.success('水浸状态已重置')
}

function simulateIntrusion() {
  infraredDetected.value = true
  radarDetected.value = true
  intrusionRisk.value = '高'
  alarmActive.value = true
  ElMessage.warning('模拟人员入侵')
}

function resetIntrusion() {
  infraredDetected.value = false
  radarDetected.value = false
  intrusionRisk.value = '低'
  alarmActive.value = false
  ElMessage.success('入侵状态已重置')
}

function simulateGas() {
  tvocValue.value = 580
  ch2oValue.value = 120
  co2Value.value = 1200
  gasRisk.value = '高'
  alarmActive.value = true
  ElMessage.warning('模拟气体异常')
}

function resetGas() {
  tvocValue.value = 120
  ch2oValue.value = 30
  co2Value.value = 450
  gasRisk.value = '低'
  alarmActive.value = false
  ElMessage.success('气体状态已重置')
}
</script>

<style scoped>
.security-root { min-height: 100vh; background: var(--bg-primary); position: relative; }
.security-main { position: relative; z-index: 1; padding: var(--spacing-xl); max-width: 1920px; margin: 0 auto; }

.section-header { margin-bottom: var(--spacing-lg); }
.section-title { display: flex; align-items: center; gap: var(--spacing-sm); font-size: 20px; font-weight: 600; color: var(--text-primary); margin: 0; }
.section-title svg { color: var(--accent-primary); }

.status-section { margin-bottom: var(--spacing-xl); }
.status-grid { display: grid; grid-template-columns: repeat(4, 1fr); gap: var(--spacing-lg); }
.status-card { display: flex; align-items: center; gap: var(--spacing-md); padding: var(--spacing-lg); background: var(--bg-card); border: 1px solid var(--border-primary); border-radius: var(--radius-lg); transition: all var(--duration-fast); }
.status-card:hover { transform: translateY(-2px); box-shadow: var(--shadow-md); }
.status-card.normal { border-left: 4px solid var(--status-success); }
.status-card.warning { border-left: 4px solid var(--status-warning); }
.status-card.danger { border-left: 4px solid var(--status-danger); animation: alarm-border 2s infinite; }
.status-icon { font-size: 32px; }
.status-info { flex: 1; }
.status-label { font-size: 12px; color: var(--text-secondary); text-transform: uppercase; }
.status-value { font-size: 18px; font-weight: 600; color: var(--text-primary); }

.scenarios-section { margin-bottom: var(--spacing-xl); }
.scenarios-grid { display: grid; grid-template-columns: repeat(3, 1fr); gap: var(--spacing-xl); }

.scenario-data { margin-bottom: var(--spacing-md); }
.data-row { display: flex; justify-content: space-between; align-items: center; padding: var(--spacing-sm) 0; border-bottom: 1px solid var(--border-primary); }
.data-row:last-child { border-bottom: none; }
.data-label { font-size: 13px; color: var(--text-secondary); }
.data-value { font-size: 14px; font-weight: 600; color: var(--text-primary); font-family: var(--font-mono); }
.text-success { color: var(--status-success); }
.text-warning { color: var(--status-warning); }
.text-danger { color: var(--status-danger); }

.scenario-actions { display: flex; gap: var(--spacing-sm); padding-top: var(--spacing-md); border-top: 1px solid var(--border-primary); }

.monitor-section { display: grid; grid-template-columns: 1.2fr 1fr; gap: var(--spacing-xl); }

@media (max-width: 1200px) {
  .status-grid { grid-template-columns: repeat(2, 1fr); }
  .scenarios-grid { grid-template-columns: 1fr; }
  .monitor-section { grid-template-columns: 1fr; }
}
@media (max-width: 768px) {
  .security-main { padding: var(--spacing-md); }
  .status-grid { grid-template-columns: 1fr; }
}
</style>
