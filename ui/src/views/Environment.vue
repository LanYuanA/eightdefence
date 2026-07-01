<template>
  <div class="env-root">
    <ParticleBackground :particle-count="40" color="#22c55d" :opacity="0.3" />
    <AppNavbar title="环境监测系统" subtitle="Environment Monitoring System" :menu-items="menuItems" />

    <main class="env-main">
      <!-- 概览卡片 -->
      <section class="overview-section">
        <div class="section-header">
          <h2 class="section-title">
            <svg width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M14 14.76V3.5a2.5 2.5 0 0 0-5 0v11.26a4.5 4.5 0 1 0 5 0z"/></svg>
            环境数据概览
          </h2>
          <div class="header-actions">
            <CyberButton variant="ghost" size="sm" @click="refreshData">刷新数据</CyberButton>
            <CyberButton variant="primary" size="sm" @click="showThresholds = true">阈值设置</CyberButton>
          </div>
        </div>
        <div class="cards-grid">
          <DataCard v-for="(card, i) in envCards" :key="card.label" v-bind="card" :delay="i * 100" />
        </div>
      </section>

      <!-- 图表和数据流 -->
      <section class="charts-flow-section">
        <div class="charts-area">
          <BaseCard title="温度趋势" subtitle="24小时监测" icon-color="red">
            <template #header>
              <div class="flex items-center gap-3">
                <div class="card-icon icon-red">
                  <svg width="20" height="20" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M14 14.76V3.5a2.5 2.5 0 0 0-5 0v11.26a4.5 4.5 0 1 0 5 0z"/></svg>
                </div>
                <div><h3 class="card-title">温度趋势</h3><p class="card-subtitle">24小时监测</p></div>
              </div>
            </template>
            <div class="chart-box"><canvas ref="tempChartRef"></canvas></div>
          </BaseCard>
          <BaseCard title="湿度趋势" subtitle="24小时监测" icon-color="blue">
            <template #header>
              <div class="flex items-center gap-3">
                <div class="card-icon icon-blue">
                  <svg width="20" height="20" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M12 2.69l5.66 5.66a8 8 0 1 1-11.31 0z"/></svg>
                </div>
                <div><h3 class="card-title">湿度趋势</h3><p class="card-subtitle">24小时监测</p></div>
              </div>
            </template>
            <div class="chart-box"><canvas ref="humiChartRef"></canvas></div>
          </BaseCard>
        </div>
        <div class="flow-area">
          <BaseCard title="数据流向" subtitle="实时传输可视化" icon-color="purple">
            <template #header>
              <div class="flex items-center gap-3">
                <div class="card-icon icon-purple">
                  <svg width="20" height="20" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M22 12h-4l-3 9L9 3l-3 9H2"/></svg>
                </div>
                <div><h3 class="card-title">数据流向</h3><p class="card-subtitle">实时传输可视化</p></div>
              </div>
            </template>
            <DataFlowVisualizer :nodes="flowNodes" :flows="flowFlows" flow-color="#22c55d" :show-controls="true" />
          </BaseCard>
        </div>
      </section>

      <!-- 设备列表和控制 -->
      <section class="devices-control-section">
        <BaseCard title="在线设备" subtitle="环境监测设备" icon-color="green">
          <template #header>
            <div class="flex items-center justify-between w-full">
              <div class="flex items-center gap-3">
                <div class="card-icon icon-green">
                  <svg width="20" height="20" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><rect x="2" y="3" width="20" height="14" rx="2"/><path d="M8 21h8m-4-4v4"/></svg>
                </div>
                <div><h3 class="card-title">在线设备</h3><p class="card-subtitle">环境监测设备</p></div>
              </div>
              <CyberButton variant="ghost" size="sm" @click="showAddDevice = true">添加设备</CyberButton>
            </div>
          </template>
          <div class="device-list">
            <div v-for="dev in devices" :key="dev.id" class="device-row" :class="{ offline: !dev.online }">
              <span class="dev-icon">{{ dev.icon }}</span>
              <StatusDot :status="dev.online ? 'online' : 'offline'" size="sm" :pulse="dev.online" />
              <div class="dev-info"><div class="dev-name">{{ dev.name }}</div><div class="dev-type">{{ dev.type }}</div></div>
              <div class="dev-val">{{ dev.value }}</div>
              <div class="dev-actions">
                <button class="act-btn" @click="viewDev(dev)">详情</button>
                <button class="act-btn" @click="refreshDev(dev)">刷新</button>
              </div>
            </div>
          </div>
        </BaseCard>

        <BaseCard title="智能联动" subtitle="环境调控" icon-color="cyan">
          <template #header>
            <div class="flex items-center gap-3">
              <div class="card-icon icon-cyan">
                <svg width="20" height="20" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M12 22s8-4 8-10V5l-8-3-8 3v7c0 6 8 10 8 10z"/></svg>
              </div>
              <div><h3 class="card-title">智能联动</h3><p class="card-subtitle">环境调控</p></div>
            </div>
          </template>
          <div class="control-list">
            <div v-for="ctrl in controls" :key="ctrl.id" class="ctrl-item" :class="{ active: ctrl.active }">
              <span class="ctrl-icon">{{ ctrl.icon }}</span>
              <div class="ctrl-info"><div class="ctrl-name">{{ ctrl.name }}</div><div class="ctrl-desc">{{ ctrl.desc }}</div></div>
              <span class="ctrl-status" :class="ctrl.active ? 'on' : 'off'">{{ ctrl.active ? '运行' : '待机' }}</span>
              <button class="ctrl-btn" :class="ctrl.active ? 'btn-off' : 'btn-on'" @click="toggleCtrl(ctrl)">{{ ctrl.active ? '关闭' : '开启' }}</button>
            </div>
          </div>
        </BaseCard>
      </section>

      <!-- 实时数据流 -->
      <section class="stream-section">
        <BaseCard title="实时数据流" subtitle="系统事件监控" icon-color="yellow">
          <template #header>
            <div class="flex items-center gap-3">
              <div class="card-icon icon-yellow">
                <svg width="20" height="20" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M13 2L3 14h9l-1 8 10-12h-9l1-8z"/></svg>
              </div>
              <div><h3 class="card-title">实时数据流</h3><p class="card-subtitle">系统事件监控</p></div>
            </div>
          </template>
          <LiveDataStream :max-items="30" />
        </BaseCard>
      </section>
    </main>

    <!-- 阈值设置弹窗 -->
    <Transition name="modal">
      <div v-if="showThresholds" class="modal-overlay" @click.self="showThresholds = false">
        <div class="modal-content">
          <div class="modal-header">
            <h3>环境阈值设置</h3>
            <button class="modal-close" @click="showThresholds = false">×</button>
          </div>
          <div class="modal-body">
            <div v-for="t in thresholdList" :key="t.key" class="threshold-row">
              <span class="th-label">{{ t.label }}</span>
              <div class="th-inputs">
                <input type="number" v-model.number="t.min" class="th-input" />
                <span>~</span>
                <input type="number" v-model.number="t.max" class="th-input" />
                <span class="th-unit">{{ t.unit }}</span>
              </div>
            </div>
          </div>
          <div class="modal-footer">
            <CyberButton variant="ghost" @click="showThresholds = false">取消</CyberButton>
            <CyberButton variant="primary" @click="saveThresholds">保存</CyberButton>
          </div>
        </div>
      </div>
    </Transition>

    <!-- 添加设备弹窗 -->
    <Transition name="modal">
      <div v-if="showAddDevice" class="modal-overlay" @click.self="showAddDevice = false">
        <div class="modal-content">
          <div class="modal-header">
            <h3>添加环境监测设备</h3>
            <button class="modal-close" @click="showAddDevice = false">×</button>
          </div>
          <div class="modal-body">
            <div class="form-grid">
              <div class="form-group">
                <label>设备名称 *</label>
                <input v-model="newDevice.name" placeholder="输入设备名称" />
              </div>
              <div class="form-group">
                <label>设备类型</label>
                <select v-model="newDevice.type">
                  <option value="sensor">传感器</option>
                  <option value="controller">控制器</option>
                  <option value="alarm">报警器</option>
                </select>
              </div>
              <div class="form-group">
                <label>设备型号 *</label>
                <input v-model="newDevice.model" placeholder="输入设备型号" />
              </div>
              <div class="form-group">
                <label>Modbus地址</label>
                <input v-model="newDevice.address" placeholder="0x30" />
              </div>
              <div class="form-group">
                <label>数据单位</label>
                <input v-model="newDevice.unit" placeholder="℃, %, ppm" />
              </div>
              <div class="form-group">
                <label>所属应用</label>
                <select v-model="newDevice.app">
                  <option value="环境监测">环境监测</option>
                  <option value="安防系统">安防系统</option>
                  <option value="消防系统">消防系统</option>
                </select>
              </div>
            </div>
          </div>
          <div class="modal-footer">
            <CyberButton variant="ghost" @click="showAddDevice = false">取消</CyberButton>
            <CyberButton variant="primary" @click="addDevice">添加设备</CyberButton>
          </div>
        </div>
      </div>
    </Transition>
  </div>
</template>

<script setup lang="ts">
import { ref, reactive, onMounted, onUnmounted } from 'vue'
import { ElMessage } from 'element-plus'
import { Chart, registerables } from 'chart.js'
import ParticleBackground from '../components/ParticleBackground.vue'
import AppNavbar from '../components/AppNavbar.vue'
import DataCard from '../components/DataCard.vue'
import BaseCard from '../components/BaseCard.vue'
import CyberButton from '../components/CyberButton.vue'
import StatusDot from '../components/StatusDot.vue'
import DataFlowVisualizer from '../components/DataFlowVisualizer.vue'
import LiveDataStream from '../components/LiveDataStream.vue'

Chart.register(...registerables)

const menuItems = [
  { path: '/', label: '主控台' },
  { path: '/security', label: '安防系统' },
  { path: '/fire', label: '消防系统' },
  { path: '/devices', label: '设备管理' },
  { path: '/logs', label: '系统日志' }
]

const showThresholds = ref(false)
const showAddDevice = ref(false)

const newDevice = reactive({
  name: '',
  type: 'sensor',
  model: '',
  address: '',
  unit: '',
  app: '环境监测'
})

const envCards = reactive([
  { label: '当前温度', value: 24.5, unit: '℃', icon: '<path d="M14 14.76V3.5a2.5 2.5 0 0 0-5 0v11.26a4.5 4.5 0 1 0 5 0z"/>', iconColor: 'red' as const, status: 'normal' as const, trend: '稳定', trendDirection: 'stable' as const, miniChartData: [24.2,24.5,24.3,24.6,24.4,24.5,24.5] },
  { label: '当前湿度', value: 65, unit: '%', icon: '<path d="M12 2.69l5.66 5.66a8 8 0 1 1-11.31 0z"/>', iconColor: 'blue' as const, status: 'normal' as const, trend: '下降', trendDirection: 'down' as const, miniChartData: [68,67,66,65.5,65,64.5,65] },
  { label: 'PM2.5', value: 42, unit: 'μg/m³', icon: '<path d="M18.5 8c.83 0 1.5-.67 1.5-1.5S19.33 5 18.5 5c-.17 0-.34.03-.5.08A2.5 2.5 0 0 0 16 3a2.5 2.5 0 0 0-2 4v.02A1.5 1.5 0 0 0 13 8.5c0 .83.67 1.5 1.5 1.5h4z"/>', iconColor: 'yellow' as const, status: 'normal' as const, trend: '正常', trendDirection: 'stable' as const, miniChartData: [40,42,45,43,41,42,42] },
  { label: 'CO₂浓度', value: 520, unit: 'ppm', icon: '<circle cx="12" cy="12" r="10"/><path d="M12 6v6l4 2"/>', iconColor: 'purple' as const, status: 'normal' as const, trend: '正常', trendDirection: 'stable' as const, miniChartData: [510,515,520,518,522,520,520] }
])

// 软件定义架构数据流向
const flowNodes = [
  // 设备层
  { id: 'sensor-cloud', x: 140, y: 280, size: 18, color: '#3b82f6', icon: '🌡️', label: '云测仪', active: true, dataCount: 128 },
  { id: 'sensor-humidifier', x: 280, y: 280, size: 16, color: '#06b6d4', icon: '🌀', label: '恒湿净化', active: true, dataCount: 32 },
  { id: 'sensor-conditioner', x: 420, y: 280, size: 16, color: '#22c55d', icon: '❄️', label: '空调控制', active: true, dataCount: 16 },
  // 设备抽象层
  { id: 'abs-cloud', x: 200, y: 210, size: 20, color: '#3b82f6', icon: '☁️', label: '云测仪抽象', active: true, dataCount: 128 },
  { id: 'abs-env', x: 400, y: 210, size: 18, color: '#06b6d4', icon: '🌿', label: '环境抽象', active: true, dataCount: 48 },
  // 原子服务下层
  { id: 'lower-collect', x: 180, y: 140, size: 22, color: '#3b82f6', icon: '📥', label: '数据采集', active: true, dataCount: 96 },
  { id: 'lower-process', x: 330, y: 140, size: 20, color: '#8b5cf6', icon: '⚙️', label: '数据处理', active: true, dataCount: 64 },
  { id: 'lower-alarm', x: 480, y: 140, size: 18, color: '#ef4444', icon: '🔔', label: '报警判断', active: true, dataCount: 32 },
  // 原子服务上层
  { id: 'upper-monitor', x: 280, y: 70, size: 24, color: '#3b82f6', icon: '📊', label: '环境监测服务', active: true, dataCount: 48 },
  // 应用层
  { id: 'app-env', x: 280, y: 15, size: 20, color: '#3b82f6', icon: '🌡️', label: '环境监测', active: true, dataCount: 24 }
]

const flowFlows = [
  // 设备层 → 设备抽象层
  { from: 'sensor-cloud', to: 'abs-cloud', active: true, speed: 2 },
  { from: 'sensor-humidifier', to: 'abs-env', active: true, speed: 2 },
  { from: 'sensor-conditioner', to: 'abs-env', active: true, speed: 2 },
  // 设备抽象层 → 原子服务下层
  { from: 'abs-cloud', to: 'lower-collect', active: true, speed: 2 },
  { from: 'abs-env', to: 'lower-collect', active: true, speed: 2 },
  // 原子服务下层 → 原子服务下层
  { from: 'lower-collect', to: 'lower-process', active: true, speed: 3 },
  { from: 'lower-collect', to: 'lower-alarm', active: true, speed: 2 },
  // 原子服务下层 → 原子服务上层
  { from: 'lower-process', to: 'upper-monitor', active: true, speed: 2 },
  { from: 'lower-alarm', to: 'upper-monitor', active: true, speed: 2 },
  // 原子服务上层 → 应用层
  { from: 'upper-monitor', to: 'app-env', active: true, speed: 2 }
]

const devices = reactive([
  { id: 1, name: '温度传感器', type: '云测仪 SD123', icon: '🌡️', value: '24.5℃', online: true },
  { id: 2, name: '湿度传感器', type: '云测仪 SD123', icon: '💧', value: '65%', online: true },
  { id: 3, name: 'PM2.5传感器', type: '云测仪 SD123', icon: '💨', value: '42μg/m³', online: true },
  { id: 4, name: 'CO₂传感器', type: '云测仪 SD123', icon: '☁️', value: '520ppm', online: true },
  { id: 5, name: '空调控制器', type: '控制设备', icon: '❄️', value: '24℃', online: true },
  { id: 6, name: '恒湿净化机', type: '净化设备', icon: '🌀', value: '运行中', online: false }
])

const controls = reactive([
  { id: 1, name: '空调控制', desc: '温度过高自动调温', icon: '❄️', active: true },
  { id: 2, name: '恒湿净化', desc: '湿度异常自动启动', icon: '💧', active: false },
  { id: 3, name: '空气净化', desc: '空气质量异常净化', icon: '🌀', active: false },
  { id: 4, name: '光照调节', desc: '光照不足自动补光', icon: '💡', active: false }
])

const thresholdList = reactive([
  { key: 'temp', label: '温度阈值', min: 10, max: 35, unit: '℃' },
  { key: 'humi', label: '湿度阈值', min: 30, max: 70, unit: '%' },
  { key: 'pm25', label: 'PM2.5阈值', min: 0, max: 75, unit: 'μg/m³' },
  { key: 'co2', label: 'CO₂阈值', min: 400, max: 1000, unit: 'ppm' }
])

const tempChartRef = ref<HTMLCanvasElement | null>(null)
const humiChartRef = ref<HTMLCanvasElement | null>(null)
let tempChart: Chart | null = null
let humiChart: Chart | null = null

function initCharts() {
  if (tempChartRef.value) {
    tempChart = new Chart(tempChartRef.value, {
      type: 'line',
      data: { labels: ['00:00','04:00','08:00','12:00','16:00','20:00'], datasets: [{ label: '温度', data: [24.2,24.5,24.3,24.6,24.4,24.5], borderColor: '#ef4444', backgroundColor: 'rgba(239,68,68,0.1)', borderWidth: 2, tension: 0.4, fill: true }] },
      options: { responsive: true, maintainAspectRatio: false, plugins: { legend: { display: false } }, scales: { y: { min: 20, max: 30 } } }
    })
  }
  if (humiChartRef.value) {
    humiChart = new Chart(humiChartRef.value, {
      type: 'line',
      data: { labels: ['00:00','04:00','08:00','12:00','16:00','20:00'], datasets: [{ label: '湿度', data: [62,60,58,56,57,55], borderColor: '#3b82f6', backgroundColor: 'rgba(59,130,246,0.1)', borderWidth: 2, tension: 0.4, fill: true }] },
      options: { responsive: true, maintainAspectRatio: false, plugins: { legend: { display: false } }, scales: { y: { min: 40, max: 70 } } }
    })
  }
}

function refreshData() { ElMessage.success('数据已刷新') }
function viewDev(dev: any) { ElMessage.info(`查看: ${dev.name}`) }
function refreshDev(dev: any) { ElMessage.success(`${dev.name} 已刷新`) }
function toggleCtrl(ctrl: any) { ctrl.active = !ctrl.active; ElMessage.success(`${ctrl.name} 已${ctrl.active ? '开启' : '关闭'}`) }
function saveThresholds() { showThresholds.value = false; ElMessage.success('阈值已保存') }

function addDevice() {
  if (!newDevice.name || !newDevice.model) {
    ElMessage.warning('请填写必填项')
    return
  }

  const device = {
    id: devices.length + 1,
    name: newDevice.name,
    type: newDevice.type === 'sensor' ? '传感器' : newDevice.type === 'controller' ? '控制器' : '报警器',
    icon: newDevice.type === 'sensor' ? '🌡️' : newDevice.type === 'controller' ? '🎮' : '🔔',
    value: '0',
    online: true
  }

  devices.push(device)
  showAddDevice.value = false
  ElMessage.success(`设备 ${device.name} 已添加`)

  // 重置表单
  newDevice.name = ''
  newDevice.model = ''
  newDevice.address = ''
  newDevice.unit = ''
}

onMounted(() => { initCharts() })
onUnmounted(() => { tempChart?.destroy(); humiChart?.destroy() })
</script>

<style scoped>
.env-root { min-height: 100vh; background: var(--bg-primary); position: relative; }
.env-main { position: relative; z-index: 1; padding: var(--spacing-xl); max-width: 1920px; margin: 0 auto; }

.section-header { display: flex; align-items: center; justify-content: space-between; margin-bottom: var(--spacing-lg); }
.section-title { display: flex; align-items: center; gap: var(--spacing-sm); font-size: 20px; font-weight: 600; color: var(--text-primary); margin: 0; }
.section-title svg { color: var(--accent-primary); }
.header-actions { display: flex; gap: var(--spacing-sm); }

.overview-section { margin-bottom: var(--spacing-xl); }
.cards-grid { display: grid; grid-template-columns: repeat(4, 1fr); gap: var(--spacing-lg); }

.charts-flow-section { display: grid; grid-template-columns: 1fr 1fr; gap: var(--spacing-xl); margin-bottom: var(--spacing-xl); }
.charts-area { display: flex; flex-direction: column; gap: var(--spacing-xl); }
.chart-box { height: 200px; padding: var(--spacing-md); }

.devices-control-section { display: grid; grid-template-columns: 1.2fr 1fr; gap: var(--spacing-xl); margin-bottom: var(--spacing-xl); }
.stream-section { margin-bottom: var(--spacing-xl); }

.device-list { max-height: 350px; overflow-y: auto; }
.device-row { display: flex; align-items: center; gap: var(--spacing-md); padding: var(--spacing-sm) var(--spacing-md); border-bottom: 1px solid var(--border-primary); transition: all var(--duration-fast); }
.device-row:hover { background: rgba(255,255,255,0.03); }
.device-row.offline { opacity: 0.5; }
.dev-icon { font-size: 20px; }
.dev-info { flex: 1; }
.dev-name { font-size: 14px; font-weight: 500; color: var(--text-primary); }
.dev-type { font-size: 12px; color: var(--text-secondary); }
.dev-val { font-family: var(--font-mono); font-weight: 600; color: var(--text-primary); }
.dev-actions { display: flex; gap: 4px; }
.act-btn { padding: 4px 8px; background: transparent; border: 1px solid var(--border-primary); border-radius: var(--radius-sm); color: var(--text-secondary); font-size: 12px; cursor: pointer; }
.act-btn:hover { border-color: var(--accent-primary); color: var(--accent-primary); }

.control-list { display: flex; flex-direction: column; gap: var(--spacing-sm); }
.ctrl-item { display: flex; align-items: center; gap: var(--spacing-md); padding: var(--spacing-md); background: rgba(0,0,0,0.2); border: 1px solid var(--border-primary); border-radius: var(--radius-md); transition: all var(--duration-fast); }
.ctrl-item.active { border-color: var(--status-success); }
.ctrl-icon { font-size: 24px; }
.ctrl-info { flex: 1; }
.ctrl-name { font-size: 14px; font-weight: 500; color: var(--text-primary); }
.ctrl-desc { font-size: 12px; color: var(--text-secondary); }
.ctrl-status { font-size: 12px; padding: 2px 8px; border-radius: var(--radius-full); }
.ctrl-status.on { background: rgba(34,197,94,0.2); color: var(--status-success); }
.ctrl-status.off { background: rgba(100,116,139,0.2); color: var(--text-tertiary); }
.ctrl-btn { padding: 6px 12px; border: 1px solid var(--border-primary); border-radius: var(--radius-sm); font-size: 12px; cursor: pointer; }
.btn-on { background: rgba(34,197,94,0.2); border-color: var(--status-success); color: var(--status-success); }
.btn-off { background: rgba(239,68,68,0.2); border-color: var(--status-danger); color: var(--status-danger); }

.modal-overlay { position: fixed; inset: 0; background: rgba(0,0,0,0.7); backdrop-filter: blur(8px); display: flex; align-items: center; justify-content: center; z-index: var(--z-modal); }
.modal-content { background: var(--bg-card); border: 1px solid var(--border-primary); border-radius: var(--radius-xl); width: 90%; max-width: 500px; }
.modal-header { display: flex; justify-content: space-between; align-items: center; padding: var(--spacing-lg); border-bottom: 1px solid var(--border-primary); }
.modal-header h3 { font-size: 18px; font-weight: 600; color: var(--text-primary); margin: 0; }
.modal-close { width: 32px; height: 32px; display: flex; align-items: center; justify-content: center; background: transparent; border: none; color: var(--text-secondary); font-size: 20px; cursor: pointer; }
.modal-body { padding: var(--spacing-lg); max-height: 60vh; overflow-y: auto; }
.modal-footer { display: flex; justify-content: flex-end; gap: var(--spacing-sm); padding: var(--spacing-lg); border-top: 1px solid var(--border-primary); }

.threshold-row { display: flex; align-items: center; justify-content: space-between; padding: var(--spacing-sm) 0; border-bottom: 1px solid var(--border-primary); }
.th-label { font-size: 14px; color: var(--text-primary); }
.th-inputs { display: flex; align-items: center; gap: var(--spacing-sm); }
.th-input { width: 70px; padding: 6px; background: var(--bg-primary); border: 1px solid var(--border-primary); border-radius: var(--radius-sm); color: var(--text-primary); text-align: center; font-size: 13px; }
.th-unit { font-size: 12px; color: var(--text-secondary); }

@media (max-width: 1200px) {
  .cards-grid { grid-template-columns: repeat(2, 1fr); }
  .charts-flow-section { grid-template-columns: 1fr; }
  .devices-control-section { grid-template-columns: 1fr; }
}
@media (max-width: 768px) {
  .env-main { padding: var(--spacing-md); }
  .cards-grid { grid-template-columns: 1fr; }
}
</style>
