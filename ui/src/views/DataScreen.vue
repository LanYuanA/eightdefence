<template>
  <div class="screen-root">
    <!-- 背景效果 -->
    <div class="screen-bg">
      <div class="grid-lines"></div>
      <div class="floating-particles"></div>
    </div>

    <!-- 顶部标题栏 -->
    <header class="screen-header">
      <div class="header-left">
        <div class="system-time">{{ currentTime }}</div>
      </div>
      <div class="header-center">
        <h1 class="main-title">软件定义消防系统</h1>
        <div class="sub-title">Eight Defence Integrated Monitoring Platform</div>
      </div>
      <div class="header-right">
        <div class="system-status">
          <StatusDot status="online" size="sm" />
          <span>系统运行正常</span>
        </div>
      </div>
    </header>

    <!-- 主内容区 -->
    <main class="screen-main">
      <!-- 左侧面板 -->
      <div class="left-panel">
        <!-- 安防概览 -->
        <div class="panel-card">
          <div class="panel-title">
            <svg width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M12 22s8-4 8-10V5l-8-3-8 3v7c0 6 8 10 8 10z"/></svg>
            安防系统
          </div>
          <div class="security-grid">
            <div class="security-item" :class="{ alert: waterAlert }">
              <div class="item-icon">💧</div>
              <div class="item-info"><div class="item-label">水浸检测</div><div class="item-value">{{ waterAlert ? '异常' : '正常' }}</div></div>
            </div>
            <div class="security-item" :class="{ alert: intrusionAlert }">
              <div class="item-icon">👤</div>
              <div class="item-info"><div class="item-label">入侵检测</div><div class="item-value">{{ intrusionAlert ? '检测到' : '正常' }}</div></div>
            </div>
            <div class="security-item" :class="{ alert: gasAlert }">
              <div class="item-icon">☁️</div>
              <div class="item-info"><div class="item-label">气体检测</div><div class="item-value">{{ gasAlert ? '异常' : '正常' }}</div></div>
            </div>
          </div>
        </div>

        <!-- 环境数据 -->
        <div class="panel-card">
          <div class="panel-title">
            <svg width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M14 14.76V3.5a2.5 2.5 0 0 0-5 0v11.26a4.5 4.5 0 1 0 5 0z"/></svg>
            环境监测
          </div>
          <div class="env-grid">
            <div class="env-item">
              <div class="env-label">温度</div>
              <div class="env-value">{{ envData.temp }}<span class="env-unit">℃</span></div>
              <div class="env-bar"><div class="env-fill" :style="{ width: (envData.temp / 50 * 100) + '%' }"></div></div>
            </div>
            <div class="env-item">
              <div class="env-label">湿度</div>
              <div class="env-value">{{ envData.humi }}<span class="env-unit">%</span></div>
              <div class="env-bar"><div class="env-fill" :style="{ width: envData.humi + '%' }"></div></div>
            </div>
            <div class="env-item">
              <div class="env-label">PM2.5</div>
              <div class="env-value">{{ envData.pm25 }}<span class="env-unit">μg/m³</span></div>
              <div class="env-bar"><div class="env-fill" :style="{ width: (envData.pm25 / 200 * 100) + '%' }"></div></div>
            </div>
            <div class="env-item">
              <div class="env-label">CO₂</div>
              <div class="env-value">{{ envData.co2 }}<span class="env-unit">ppm</span></div>
              <div class="env-bar"><div class="env-fill" :style="{ width: (envData.co2 / 2000 * 100) + '%' }"></div></div>
            </div>
          </div>
        </div>
      </div>

      <!-- 中间面板 -->
      <div class="center-panel">
        <!-- 设备拓扑 -->
        <div class="panel-card topology-card">
          <div class="panel-title">
            <svg width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><circle cx="12" cy="12" r="10"/><line x1="2" y1="12" x2="22" y2="12"/><path d="M12 2a15.3 15.3 0 0 1 4 10 15.3 15.3 0 0 1-4 10 15.3 15.3 0 0 1-4-10 15.3 15.3 0 0 1 4-10z"/></svg>
            设备拓扑
          </div>
          <DeviceTopology3D />
        </div>

        <!-- 数据流向 -->
        <div class="panel-card flow-card">
          <div class="panel-title">
            <svg width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M22 12h-4l-3 9L9 3l-3 9H2"/></svg>
            数据流向
          </div>
          <DataFlowVisualizer :nodes="flowNodes" :flows="flowFlows" :show-controls="false" />
        </div>
      </div>

      <!-- 右侧面板 -->
      <div class="right-panel">
        <!-- 消防概览 -->
        <div class="panel-card">
          <div class="panel-title">
            <svg width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M8.5 14.5A2.5 2.5 0 0 0 11 12c0-1.38-.5-2-1-3-1.072-2.143-.224-4.054 2-6 .5 2.5 2 4.9 4 6.5 2 1.6 3 3.5 3 5.5a7 7 0 1 1-14 0c0-1.153.433-2.294 1-3a2.5 2.5 0 0 0 2.5 2.5z"/></svg>
            消防系统
          </div>
          <div class="fire-grid">
            <div class="fire-item" :class="{ alert: fireAlert }">
              <div class="item-icon">🔥</div>
              <div class="item-info"><div class="item-label">烟雾检测</div><div class="item-value">{{ fireAlert ? '报警' : '正常' }}</div></div>
            </div>
            <div class="fire-item">
              <div class="item-icon">🌡️</div>
              <div class="item-info"><div class="item-label">温度</div><div class="item-value">{{ fireData.temp }}℃</div></div>
            </div>
            <div class="fire-item">
              <div class="item-icon">💨</div>
              <div class="item-info"><div class="item-label">烟雾浓度</div><div class="item-value">{{ fireData.smoke }}</div></div>
            </div>
          </div>
        </div>

        <!-- 设备状态 -->
        <div class="panel-card">
          <div class="panel-title">
            <svg width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><rect x="2" y="3" width="20" height="14" rx="2"/><path d="M8 21h8m-4-4v4"/></svg>
            设备状态
          </div>
          <div class="device-stats">
            <div class="device-stat">
              <div class="stat-circle online">{{ onlineDevices }}</div>
              <div class="stat-label">在线</div>
            </div>
            <div class="device-stat">
              <div class="stat-circle offline">{{ offlineDevices }}</div>
              <div class="stat-label">离线</div>
            </div>
            <div class="device-stat">
              <div class="stat-circle alert">{{ alertDevices }}</div>
              <div class="stat-label">告警</div>
            </div>
          </div>
        </div>

        <!-- 实时事件 -->
        <div class="panel-card events-card">
          <div class="panel-title">
            <svg width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M13 2L3 14h9l-1 8 10-12h-9l1-8z"/></svg>
            实时事件
          </div>
          <div class="events-list">
            <div v-for="event in recentEvents" :key="event.id" class="event-item" :class="event.type">
              <span class="event-icon">{{ event.icon }}</span>
              <span class="event-text">{{ event.text }}</span>
              <span class="event-time">{{ event.time }}</span>
            </div>
          </div>
        </div>
      </div>
    </main>

    <!-- 底部状态栏 -->
    <footer class="screen-footer">
      <div class="footer-left">
        <span class="footer-item">数据刷新: 2s</span>
        <span class="footer-item">在线设备: {{ onlineDevices }}/{{ totalDevices }}</span>
      </div>
      <div class="footer-center">
        <span class="footer-item">系统运行时间: {{ uptime }}</span>
      </div>
      <div class="footer-right">
        <span class="footer-item">版本: v2.0.0</span>
        <span class="footer-item">© 2024 软件定义消防系统</span>
      </div>
    </footer>
  </div>
</template>

<script setup lang="ts">
import { ref, reactive, onMounted, onUnmounted } from 'vue'
import { useTime } from '../composables/useTime'
import StatusDot from '../components/StatusDot.vue'
import DeviceTopology3D from '../components/DeviceTopology3D.vue'
import DataFlowVisualizer from '../components/DataFlowVisualizer.vue'

const { currentTime } = useTime()

// 安防状态
const waterAlert = ref(false)
const intrusionAlert = ref(false)
const gasAlert = ref(false)

// 环境数据
const envData = reactive({ temp: 24.5, humi: 65, pm25: 42, co2: 520 })

// 消防数据
const fireAlert = ref(false)
const fireData = reactive({ temp: 28.5, smoke: 0.3 })

// 设备统计
const onlineDevices = ref(24)
const offlineDevices = ref(3)
const alertDevices = ref(2)
const totalDevices = ref(27)

// 系统运行时间
const uptime = ref('15天 8小时 32分钟')

// 数据流向节点
const flowNodes = [
  { id: 'sensor', x: 60, y: 80, size: 16, color: '#3b82f6', icon: '📡', label: '传感器', active: true, dataCount: 256 },
  { id: 'gateway', x: 200, y: 60, size: 20, color: '#8b5cf6', icon: '🔌', label: '网关', active: true, dataCount: 128 },
  { id: 'server', x: 340, y: 80, size: 18, color: '#22c55d', icon: '🖥️', label: '服务器', active: true, dataCount: 64 },
  { id: 'ui', x: 200, y: 140, size: 16, color: '#ec4899', icon: '📊', label: '界面', active: true, dataCount: 32 }
]

const flowFlows = [
  { from: 'sensor', to: 'gateway', active: true, speed: 2 },
  { from: 'gateway', to: 'server', active: true, speed: 3 },
  { from: 'server', to: 'ui', active: true, speed: 2 }
]

// 最近事件
const recentEvents = reactive([
  { id: 1, icon: '🟢', text: '温度传感器数据更新', time: '10:30', type: 'normal' },
  { id: 2, icon: '🟢', text: '湿度传感器数据更新', time: '10:29', type: 'normal' },
  { id: 3, icon: '🟡', text: 'CO₂浓度偏高提醒', time: '10:28', type: 'warning' },
  { id: 4, icon: '🟢', text: '空调控制器状态正常', time: '10:27', type: 'normal' },
  { id: 5, icon: '🔴', text: '烟雾报警器触发', time: '10:26', type: 'alert' }
])

// 模拟数据更新
let updateInterval: ReturnType<typeof setInterval> | null = null

onMounted(() => {
  updateInterval = setInterval(() => {
    // 模拟数据变化
    envData.temp = 24 + Math.random() * 2
    envData.humi = 60 + Math.random() * 10
    envData.pm25 = 40 + Math.random() * 10
    envData.co2 = 500 + Math.random() * 100
    fireData.temp = 28 + Math.random() * 2
    fireData.smoke = 0.2 + Math.random() * 0.3
  }, 2000)
})

onUnmounted(() => {
  if (updateInterval) clearInterval(updateInterval)
})
</script>

<style scoped>
.screen-root { min-height: 100vh; background: #0a0e1a; color: #e2e8f0; overflow: hidden; position: relative; }

/* 背景效果 */
.screen-bg { position: fixed; inset: 0; pointer-events: none; }
.grid-lines { position: absolute; inset: 0; background-image: linear-gradient(rgba(59,130,246,0.03) 1px, transparent 1px), linear-gradient(90deg, rgba(59,130,246,0.03) 1px, transparent 1px); background-size: 50px 50px; }
.floating-particles { position: absolute; inset: 0; background: radial-gradient(circle at 20% 50%, rgba(59,130,246,0.05), transparent 50%), radial-gradient(circle at 80% 50%, rgba(139,92,246,0.05), transparent 50%); }

/* 头部 */
.screen-header { display: flex; align-items: center; justify-content: space-between; padding: 15px 30px; background: rgba(15,23,42,0.8); border-bottom: 1px solid rgba(59,130,246,0.2); position: relative; z-index: 1; }
.header-left, .header-right { width: 200px; }
.system-time { font-family: 'JetBrains Mono', monospace; font-size: 14px; color: #94a3b8; }
.header-center { text-align: center; }
.main-title { font-size: 28px; font-weight: 700; margin: 0; background: linear-gradient(135deg, #3b82f6, #8b5cf6); -webkit-background-clip: text; -webkit-text-fill-color: transparent; letter-spacing: 4px; }
.sub-title { font-size: 12px; color: #64748b; margin-top: 4px; letter-spacing: 2px; }
.system-status { display: flex; align-items: center; gap: 8px; font-size: 13px; color: #22c55d; justify-content: flex-end; }

/* 主内容区 */
.screen-main { display: grid; grid-template-columns: 280px 1fr 280px; gap: 15px; padding: 15px; height: calc(100vh - 100px); position: relative; z-index: 1; }

/* 面板卡片 */
.panel-card { background: rgba(30,41,59,0.6); border: 1px solid rgba(59,130,246,0.15); border-radius: 10px; padding: 15px; backdrop-filter: blur(10px); }
.panel-title { display: flex; align-items: center; gap: 8px; font-size: 14px; font-weight: 600; color: #94a3b8; margin-bottom: 15px; text-transform: uppercase; letter-spacing: 1px; }
.panel-title svg { color: #3b82f6; }

/* 安防网格 */
.security-grid { display: flex; flex-direction: column; gap: 10px; }
.security-item { display: flex; align-items: center; gap: 12px; padding: 10px; background: rgba(0,0,0,0.2); border-radius: 8px; border: 1px solid transparent; transition: all 0.3s; }
.security-item.alert { border-color: #ef4444; background: rgba(239,68,68,0.1); animation: pulse 2s infinite; }
.item-icon { font-size: 24px; }
.item-label { font-size: 11px; color: #64748b; }
.item-value { font-size: 14px; font-weight: 600; color: #e2e8f0; }

/* 环境网格 */
.env-grid { display: flex; flex-direction: column; gap: 12px; }
.env-item { }
.env-label { font-size: 11px; color: #64748b; margin-bottom: 4px; }
.env-value { font-size: 20px; font-weight: 700; color: #e2e8f0; font-family: 'JetBrains Mono', monospace; }
.env-unit { font-size: 12px; color: #64748b; margin-left: 4px; }
.env-bar { height: 4px; background: rgba(255,255,255,0.1); border-radius: 2px; margin-top: 6px; overflow: hidden; }
.env-fill { height: 100%; background: linear-gradient(90deg, #3b82f6, #8b5cf6); border-radius: 2px; transition: width 0.5s; }

/* 中间面板 */
.center-panel { display: flex; flex-direction: column; gap: 15px; }
.topology-card { flex: 1; }
.flow-card { height: 200px; }

/* 消防网格 */
.fire-grid { display: flex; flex-direction: column; gap: 10px; }
.fire-item { display: flex; align-items: center; gap: 12px; padding: 10px; background: rgba(0,0,0,0.2); border-radius: 8px; border: 1px solid transparent; transition: all 0.3s; }
.fire-item.alert { border-color: #ef4444; background: rgba(239,68,68,0.1); animation: pulse 2s infinite; }

/* 设备统计 */
.device-stats { display: flex; justify-content: space-around; }
.device-stat { text-align: center; }
.stat-circle { width: 48px; height: 48px; display: flex; align-items: center; justify-content: center; border-radius: 50%; font-size: 18px; font-weight: 700; margin: 0 auto 8px; }
.stat-circle.online { background: rgba(34,197,94,0.2); color: #22c55d; border: 2px solid #22c55d; }
.stat-circle.offline { background: rgba(100,116,139,0.2); color: #64748b; border: 2px solid #64748b; }
.stat-circle.alert { background: rgba(239,68,68,0.2); color: #ef4444; border: 2px solid #ef4444; }
.stat-label { font-size: 12px; color: #64748b; }

/* 事件列表 */
.events-list { display: flex; flex-direction: column; gap: 8px; max-height: 200px; overflow-y: auto; }
.event-item { display: flex; align-items: center; gap: 8px; padding: 8px; background: rgba(0,0,0,0.2); border-radius: 6px; font-size: 12px; }
.event-item.warning { border-left: 2px solid #f59e0b; }
.event-item.alert { border-left: 2px solid #ef4444; }
.event-icon { font-size: 14px; }
.event-text { flex: 1; color: #94a3b8; }
.event-time { font-family: 'JetBrains Mono', monospace; color: #475569; font-size: 11px; }

/* 底部 */
.screen-footer { display: flex; align-items: center; justify-content: space-between; padding: 10px 30px; background: rgba(15,23,42,0.8); border-top: 1px solid rgba(59,130,246,0.2); position: relative; z-index: 1; }
.footer-left, .footer-center, .footer-right { display: flex; gap: 20px; }
.footer-item { font-size: 12px; color: #475569; }

@keyframes pulse { 0%, 100% { opacity: 1; } 50% { opacity: 0.7; } }

@media (max-width: 1200px) { .screen-main { grid-template-columns: 1fr; } }
</style>
