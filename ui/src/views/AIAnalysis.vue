<template>
  <div class="ai-root">
    <ParticleBackground :particle-count="40" color="#8b5cf6" :opacity="0.4" />
    <AppNavbar title="AI智能分析" subtitle="AI Intelligent Analysis" :menu-items="menuItems" />

    <main class="ai-main">
      <!-- AI概览 -->
      <section class="overview-section">
        <div class="section-header">
          <h2 class="section-title">
            <svg width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M12 2a10 10 0 1 0 10 10 4 4 0 0 1-5-5 4 4 0 0 1-5-5"/><path d="M8.5 8.5v.01"/><path d="M16 15.5v.01"/><path d="M12 12v.01"/><path d="M11 17v.01"/><path d="M7 14v.01"/></svg>
            AI智能分析概览
          </h2>
          <div class="header-actions">
            <CyberButton variant="ghost" size="sm" @click="refreshAnalysis">
              <svg width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M21.5 2v6h-6M2.5 22v-6h6M2 11.5a10 10 0 0 1 18.8-4.3M22 12.5a10 10 0 0 1-18.8 4.3"/></svg>
              刷新分析
            </CyberButton>
            <CyberButton variant="primary" size="sm" @click="runFullAnalysis">
              <svg width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><polygon points="5 3 19 12 5 21 5 3"/></svg>
              运行完整分析
            </CyberButton>
          </div>
        </div>

        <div class="ai-stats">
          <div class="stat-card">
            <div class="stat-icon" style="background: rgba(139,92,246,0.2); color: #8b5cf6;">🧠</div>
            <div class="stat-info"><div class="stat-value">{{ analysisScore }}</div><div class="stat-label">健康评分</div></div>
          </div>
          <div class="stat-card">
            <div class="stat-icon" style="background: rgba(239,68,68,0.2); color: #ef4444;">⚠️</div>
            <div class="stat-info"><div class="stat-value">{{ riskCount }}</div><div class="stat-label">风险项</div></div>
          </div>
          <div class="stat-card">
            <div class="stat-icon" style="background: rgba(34,197,94,0.2); color: #22c55d;">💡</div>
            <div class="stat-info"><div class="stat-value">{{ suggestionCount }}</div><div class="stat-label">优化建议</div></div>
          </div>
          <div class="stat-card">
            <div class="stat-icon" style="background: rgba(59,130,246,0.2); color: #3b82f6;">📊</div>
            <div class="stat-info"><div class="stat-value">{{ predictionAccuracy }}%</div><div class="stat-label">预测准确率</div></div>
          </div>
        </div>
      </section>

      <!-- AI分析内容 -->
      <section class="analysis-section">
        <div class="analysis-grid">
          <!-- 设备健康分析 -->
          <BaseCard title="设备健康分析" subtitle="基于AI的设备状态评估" icon-color="purple">
            <template #header>
              <div class="flex items-center gap-3">
                <div class="card-icon icon-purple">🏥</div>
                <div><h3 class="card-title">设备健康分析</h3><p class="card-subtitle">基于AI的设备状态评估</p></div>
              </div>
            </template>
            <div class="health-list">
              <div v-for="device in deviceHealth" :key="device.id" class="health-item" :class="device.status">
                <div class="health-icon">{{ device.icon }}</div>
                <div class="health-info">
                  <div class="health-name">{{ device.name }}</div>
                  <div class="health-score">
                    <div class="score-bar">
                      <div class="score-fill" :style="{ width: device.score + '%', background: getScoreColor(device.score) }"></div>
                    </div>
                    <span class="score-value">{{ device.score }}分</span>
                  </div>
                  <div class="health-prediction">{{ device.prediction }}</div>
                </div>
                <div class="health-status" :class="device.status">
                  {{ device.status === 'good' ? '健康' : device.status === 'warning' ? '预警' : '异常' }}
                </div>
              </div>
            </div>
          </BaseCard>

          <!-- 异常检测 -->
          <BaseCard title="异常检测" subtitle="实时异常数据分析" icon-color="red">
            <template #header>
              <div class="flex items-center gap-3">
                <div class="card-icon icon-red">🔍</div>
                <div><h3 class="card-title">异常检测</h3><p class="card-subtitle">实时异常数据分析</p></div>
              </div>
            </template>
            <div class="anomaly-list">
              <div v-for="anomaly in anomalies" :key="anomaly.id" class="anomaly-item" :class="anomaly.level">
                <div class="anomaly-icon">{{ anomaly.icon }}</div>
                <div class="anomaly-info">
                  <div class="anomaly-title">{{ anomaly.title }}</div>
                  <div class="anomaly-desc">{{ anomaly.description }}</div>
                  <div class="anomaly-time">{{ anomaly.time }}</div>
                </div>
                <div class="anomaly-confidence">
                  <div class="confidence-value">{{ anomaly.confidence }}%</div>
                  <div class="confidence-label">置信度</div>
                </div>
              </div>
            </div>
          </BaseCard>

          <!-- 预测分析 -->
          <BaseCard title="预测分析" subtitle="基于历史数据的趋势预测" icon-color="blue">
            <template #header>
              <div class="flex items-center gap-3">
                <div class="card-icon icon-blue">📈</div>
                <div><h3 class="card-title">预测分析</h3><p class="card-subtitle">基于历史数据的趋势预测</p></div>
              </div>
            </template>
            <div class="prediction-charts">
              <div class="chart-container">
                <canvas ref="predictionChartRef"></canvas>
              </div>
              <div class="prediction-summary">
                <div class="summary-item">
                  <span class="summary-label">温度趋势</span>
                  <span class="summary-value trend-up">↑ 上升</span>
                </div>
                <div class="summary-item">
                  <span class="summary-label">湿度趋势</span>
                  <span class="summary-value trend-down">↓ 下降</span>
                </div>
                <div class="summary-item">
                  <span class="summary-label">故障概率</span>
                  <span class="summary-value trend-stable">12%</span>
                </div>
              </div>
            </div>
          </BaseCard>

          <!-- 智能建议 -->
          <BaseCard title="智能建议" subtitle="AI优化建议" icon-color="green">
            <template #header>
              <div class="flex items-center gap-3">
                <div class="card-icon icon-green">💡</div>
                <div><h3 class="card-title">智能建议</h3><p class="card-subtitle">AI优化建议</p></div>
              </div>
            </template>
            <div class="suggestion-list">
              <div v-for="suggestion in suggestions" :key="suggestion.id" class="suggestion-item">
                <div class="suggestion-icon" :class="suggestion.type">{{ suggestion.icon }}</div>
                <div class="suggestion-content">
                  <div class="suggestion-title">{{ suggestion.title }}</div>
                  <div class="suggestion-desc">{{ suggestion.description }}</div>
                  <div class="suggestion-impact">
                    <span class="impact-label">预期效果：</span>
                    <span class="impact-value">{{ suggestion.impact }}</span>
                  </div>
                </div>
                <CyberButton variant="ghost" size="sm" @click="applySuggestion(suggestion)">应用</CyberButton>
              </div>
            </div>
          </BaseCard>
        </div>
      </section>
    </main>
  </div>
</template>

<script setup lang="ts">
import { ref, reactive, onMounted, onUnmounted } from 'vue'
import { ElMessage } from 'element-plus'
import { Chart, registerables } from 'chart.js'
import ParticleBackground from '../components/ParticleBackground.vue'
import AppNavbar from '../components/AppNavbar.vue'
import BaseCard from '../components/BaseCard.vue'
import CyberButton from '../components/CyberButton.vue'

Chart.register(...registerables)

const menuItems = [
  { path: '/', label: '主控台' },
  { path: '/security', label: '安防系统' },
  { path: '/environment', label: '环境监测' },
  { path: '/fire', label: '消防系统' },
  { path: '/devices', label: '设备管理' }
]

const predictionChartRef = ref<HTMLCanvasElement | null>(null)
let predictionChart: Chart | null = null

// AI分析数据
const analysisScore = ref(87)
const riskCount = ref(3)
const suggestionCount = ref(8)
const predictionAccuracy = ref(94)

// 设备健康数据
const deviceHealth = reactive([
  { id: 1, name: '云测仪 SD123', icon: '🌡️', score: 95, status: 'good', prediction: '预计30天内无故障' },
  { id: 2, name: '烟雾报警器', icon: '🔥', score: 88, status: 'good', prediction: '预计45天内无故障' },
  { id: 3, name: '水浸传感器', icon: '💧', score: 72, status: 'warning', prediction: '建议7天内维护' },
  { id: 4, name: '红外探测器', icon: '👤', score: 91, status: 'good', prediction: '预计60天内无故障' },
  { id: 5, name: '空调控制器', icon: '❄️', score: 65, status: 'warning', prediction: '建议3天内检查' },
  { id: 6, name: '通风净化机', icon: '🌀', score: 45, status: 'error', prediction: '需要立即维护' }
])

// 异常检测数据
const anomalies = reactive([
  { id: 1, icon: '🌡️', title: '温度异常波动', description: '检测到温度在短时间内波动超过5℃', time: '10分钟前', level: 'warning', confidence: 87 },
  { id: 2, icon: '💧', title: '湿度数据异常', description: '湿度传感器读数出现间歇性跳变', time: '25分钟前', level: 'warning', confidence: 72 },
  { id: 3, icon: '⚡', title: '设备通信异常', description: '通风净化机通信延迟超过阈值', time: '1小时前', level: 'error', confidence: 95 }
])

// 智能建议数据
const suggestions = reactive([
  { id: 1, icon: '🔧', type: 'maintenance', title: '定期维护提醒', description: '建议对水浸传感器进行定期维护', impact: '降低故障率30%' },
  { id: 2, icon: '⚡', type: 'optimization', title: '能耗优化', description: '空调控制器运行时间过长，建议优化调度', impact: '节省能耗15%' },
  { id: 3, icon: '📊', type: 'analysis', title: '数据质量提升', description: '建议增加数据采样频率以提高分析精度', impact: '提高准确率10%' },
  { id: 4, icon: '🛡️', type: 'security', title: '安全加固', description: '建议更新设备固件以修复安全漏洞', impact: '提升安全性' }
])

// 获取分数颜色
function getScoreColor(score: number): string {
  if (score >= 80) return '#22c55d'
  if (score >= 60) return '#f59e0b'
  return '#ef4444'
}

// 刷新分析
function refreshAnalysis() {
  ElMessage.success('分析数据已刷新')
}

// 运行完整分析
function runFullAnalysis() {
  ElMessage.info('正在运行完整分析...')
  setTimeout(() => {
    ElMessage.success('分析完成')
  }, 2000)
}

// 应用建议
function applySuggestion(suggestion: any) {
  ElMessage.success(`已应用建议: ${suggestion.title}`)
}

// 初始化图表
function initChart() {
  if (!predictionChartRef.value) return

  const labels = ['00:00', '04:00', '08:00', '12:00', '16:00', '20:00', '24:00']
  const actualData = [24, 24.5, 25, 26, 27, 26.5, 26]
  const predictedData = [null, null, null, null, null, 26.5, 27.5]
  const confidenceUpper = [null, null, null, null, null, 28, 29]
  const confidenceLower = [null, null, null, null, null, 25, 26]

  predictionChart = new Chart(predictionChartRef.value, {
    type: 'line',
    data: {
      labels,
      datasets: [
        {
          label: '实际值',
          data: actualData,
          borderColor: '#3b82f6',
          backgroundColor: 'rgba(59, 130, 246, 0.1)',
          borderWidth: 2,
          tension: 0.4,
          fill: false
        },
        {
          label: '预测值',
          data: predictedData,
          borderColor: '#8b5cf6',
          borderWidth: 2,
          borderDash: [5, 5],
          tension: 0.4,
          fill: false
        },
        {
          label: '置信区间上界',
          data: confidenceUpper,
          borderColor: 'transparent',
          backgroundColor: 'rgba(139, 92, 246, 0.1)',
          borderWidth: 0,
          tension: 0.4,
          fill: '+1'
        },
        {
          label: '置信区间下界',
          data: confidenceLower,
          borderColor: 'transparent',
          backgroundColor: 'rgba(139, 92, 246, 0.1)',
          borderWidth: 0,
          tension: 0.4,
          fill: '-1'
        }
      ]
    },
    options: {
      responsive: true,
      maintainAspectRatio: false,
      plugins: {
        legend: {
          position: 'top',
          labels: { color: '#94a3b8', font: { size: 11 } }
        }
      },
      scales: {
        y: {
          min: 20,
          max: 30,
          ticks: { color: '#64748b' },
          grid: { color: 'rgba(100, 116, 139, 0.1)' }
        },
        x: {
          ticks: { color: '#64748b' },
          grid: { color: 'rgba(100, 116, 139, 0.1)' }
        }
      }
    }
  })
}

onMounted(() => {
  initChart()
})

onUnmounted(() => {
  predictionChart?.destroy()
})
</script>

<style scoped>
.ai-root { min-height: 100vh; background: var(--bg-primary); position: relative; }
.ai-main { position: relative; z-index: 1; padding: var(--spacing-xl); max-width: 1920px; margin: 0 auto; }

.section-header { display: flex; align-items: center; justify-content: space-between; margin-bottom: var(--spacing-lg); }
.section-title { display: flex; align-items: center; gap: var(--spacing-sm); font-size: 20px; font-weight: 600; color: var(--text-primary); margin: 0; }
.section-title svg { color: #8b5cf6; }
.header-actions { display: flex; gap: var(--spacing-sm); }

.ai-stats { display: grid; grid-template-columns: repeat(4, 1fr); gap: var(--spacing-lg); margin-bottom: var(--spacing-xl); }
.stat-card { display: flex; align-items: center; gap: var(--spacing-md); padding: var(--spacing-lg); background: var(--bg-card); border: 1px solid var(--border-primary); border-radius: var(--radius-lg); }
.stat-icon { width: 48px; height: 48px; display: flex; align-items: center; justify-content: center; border-radius: var(--radius-md); font-size: 24px; }
.stat-value { font-size: 28px; font-weight: 700; color: var(--text-primary); font-family: var(--font-mono); }
.stat-label { font-size: 13px; color: var(--text-secondary); }

.analysis-grid { display: grid; grid-template-columns: 1fr 1fr; gap: var(--spacing-xl); }

.health-list { display: flex; flex-direction: column; gap: var(--spacing-sm); max-height: 400px; overflow-y: auto; }
.health-item { display: flex; align-items: center; gap: var(--spacing-md); padding: var(--spacing-md); background: rgba(0,0,0,0.2); border: 1px solid var(--border-primary); border-radius: var(--radius-md); }
.health-item.good { border-left: 3px solid #22c55d; }
.health-item.warning { border-left: 3px solid #f59e0b; }
.health-item.error { border-left: 3px solid #ef4444; }
.health-icon { font-size: 24px; }
.health-info { flex: 1; }
.health-name { font-size: 14px; font-weight: 500; color: var(--text-primary); }
.health-score { display: flex; align-items: center; gap: 8px; margin-top: 4px; }
.score-bar { flex: 1; height: 6px; background: rgba(255,255,255,0.1); border-radius: 3px; overflow: hidden; }
.score-fill { height: 100%; border-radius: 3px; transition: width 0.5s; }
.score-value { font-size: 12px; font-weight: 600; color: var(--text-primary); font-family: var(--font-mono); }
.health-prediction { font-size: 11px; color: var(--text-tertiary); margin-top: 4px; }
.health-status { font-size: 12px; padding: 4px 8px; border-radius: var(--radius-full); }
.health-status.good { background: rgba(34,197,94,0.2); color: #22c55d; }
.health-status.warning { background: rgba(245,158,11,0.2); color: #f59e0b; }
.health-status.error { background: rgba(239,68,68,0.2); color: #ef4444; }

.anomaly-list { display: flex; flex-direction: column; gap: var(--spacing-sm); max-height: 400px; overflow-y: auto; }
.anomaly-item { display: flex; align-items: flex-start; gap: var(--spacing-md); padding: var(--spacing-md); background: rgba(0,0,0,0.2); border: 1px solid var(--border-primary); border-radius: var(--radius-md); }
.anomaly-item.warning { border-left: 3px solid #f59e0b; }
.anomaly-item.error { border-left: 3px solid #ef4444; }
.anomaly-icon { font-size: 24px; }
.anomaly-info { flex: 1; }
.anomaly-title { font-size: 14px; font-weight: 500; color: var(--text-primary); }
.anomaly-desc { font-size: 12px; color: var(--text-secondary); margin-top: 4px; }
.anomaly-time { font-size: 11px; color: var(--text-tertiary); margin-top: 4px; }
.anomaly-confidence { text-align: center; }
.confidence-value { font-size: 18px; font-weight: 700; color: var(--text-primary); font-family: var(--font-mono); }
.confidence-label { font-size: 10px; color: var(--text-tertiary); }

.prediction-charts { display: flex; flex-direction: column; gap: var(--spacing-md); }
.chart-container { height: 250px; }
.prediction-summary { display: flex; gap: var(--spacing-md); }
.summary-item { flex: 1; display: flex; flex-direction: column; align-items: center; gap: 4px; padding: var(--spacing-sm); background: rgba(0,0,0,0.2); border-radius: var(--radius-sm); }
.summary-label { font-size: 11px; color: var(--text-tertiary); }
.summary-value { font-size: 14px; font-weight: 600; color: var(--text-primary); }
.trend-up { color: #ef4444; }
.trend-down { color: #3b82f6; }
.trend-stable { color: #22c55d; }

.suggestion-list { display: flex; flex-direction: column; gap: var(--spacing-sm); max-height: 400px; overflow-y: auto; }
.suggestion-item { display: flex; align-items: flex-start; gap: var(--spacing-md); padding: var(--spacing-md); background: rgba(0,0,0,0.2); border: 1px solid var(--border-primary); border-radius: var(--radius-md); }
.suggestion-icon { width: 40px; height: 40px; display: flex; align-items: center; justify-content: center; border-radius: var(--radius-md); font-size: 20px; }
.suggestion-icon.maintenance { background: rgba(59,130,246,0.2); }
.suggestion-icon.optimization { background: rgba(34,197,94,0.2); }
.suggestion-icon.analysis { background: rgba(139,92,246,0.2); }
.suggestion-icon.security { background: rgba(239,68,68,0.2); }
.suggestion-content { flex: 1; }
.suggestion-title { font-size: 14px; font-weight: 500; color: var(--text-primary); }
.suggestion-desc { font-size: 12px; color: var(--text-secondary); margin-top: 4px; }
.suggestion-impact { font-size: 11px; color: var(--text-tertiary); margin-top: 6px; }
.impact-label { color: var(--text-tertiary); }
.impact-value { color: #22c55d; font-weight: 500; }

@media (max-width: 1200px) { .ai-stats { grid-template-columns: repeat(2, 1fr); } .analysis-grid { grid-template-columns: 1fr; } }
@media (max-width: 768px) { .ai-main { padding: var(--spacing-md); } .ai-stats { grid-template-columns: 1fr; } }
</style>
