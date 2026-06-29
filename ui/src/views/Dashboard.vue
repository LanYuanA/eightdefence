<template>
  <div class="dashboard-root">
    <ParticleBackground :particle-count="50" color="#3b82f6" :opacity="0.4" />
    <AppNavbar title="软件定义消防系统" subtitle="Software Defined Fire Protection System" />

    <main class="dashboard-main">
      <!-- 软件定义架构概览 -->
      <section class="architecture-section">
        <div class="section-header">
          <h2 class="section-title">
            <svg width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><rect x="2" y="3" width="20" height="14" rx="2"/><path d="M8 21h8m-4-4v4"/></svg>
            软件定义架构
          </h2>
          <div class="header-actions">
            <CyberButton variant="primary" size="sm" @click="showResourcePool = true">
              <svg width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M21 16V8a2 2 0 0 0-1-1.73l-7-4a2 2 0 0 0-2 0l-7 4A2 2 0 0 0 3 8v8a2 2 0 0 0 1 1.73l7 4a2 2 0 0 0 2 0l7-4A2 2 0 0 0 21 16z"/></svg>
              资源池
            </CyberButton>
          </div>
        </div>

        <!-- 五层架构展示 -->
        <div class="architecture-layers">
          <div class="layer application-layer">
            <div class="layer-header">
              <span class="layer-icon">📱</span>
              <span class="layer-title">应用层</span>
              <span class="layer-desc">安防/环境/消防</span>
            </div>
            <div class="layer-nodes">
              <div v-for="app in applicationLayer" :key="app.id" class="arch-node app-node" :class="{ active: app.active }">
                <span class="node-icon">{{ app.icon }}</span>
                <span class="node-name">{{ app.name }}</span>
              </div>
            </div>
          </div>

          <div class="layer atomic-upper-layer">
            <div class="layer-header">
              <span class="layer-icon">⚙️</span>
              <span class="layer-title">原子服务上层</span>
              <span class="layer-desc">组合服务</span>
            </div>
            <div class="layer-nodes">
              <div v-for="service in atomicUpperLayer" :key="service.id" class="arch-node service-node" :class="{ active: service.active }">
                <span class="node-icon">{{ service.icon }}</span>
                <span class="node-name">{{ service.name }}</span>
              </div>
            </div>
          </div>

          <div class="layer atomic-lower-layer">
            <div class="layer-header">
              <span class="layer-icon">🔧</span>
              <span class="layer-title">原子服务下层</span>
              <span class="layer-desc">基础服务</span>
            </div>
            <div class="layer-nodes">
              <div v-for="service in atomicLowerLayer" :key="service.id" class="arch-node service-node" :class="{ active: service.active }">
                <span class="node-icon">{{ service.icon }}</span>
                <span class="node-name">{{ service.name }}</span>
              </div>
            </div>
          </div>

          <div class="layer abstraction-layer">
            <div class="layer-header">
              <span class="layer-icon">🔌</span>
              <span class="layer-title">设备抽象层</span>
              <span class="layer-desc">统一接口</span>
            </div>
            <div class="layer-nodes">
              <div v-for="abstraction in abstractionLayer" :key="abstraction.id" class="arch-node abstraction-node" :class="{ active: abstraction.active }">
                <span class="node-icon">{{ abstraction.icon }}</span>
                <span class="node-name">{{ abstraction.name }}</span>
              </div>
            </div>
          </div>

          <div class="layer device-layer">
            <div class="layer-header">
              <span class="layer-icon">📡</span>
              <span class="layer-title">设备层</span>
              <span class="layer-desc">物理设备</span>
            </div>
            <div class="layer-nodes">
              <div v-for="device in deviceLayer" :key="device.id" class="arch-node device-node" :class="{ active: device.online }">
                <span class="node-icon">{{ device.icon }}</span>
                <span class="node-name">{{ device.name }}</span>
                <span class="node-value">{{ device.value }}{{ device.unit }}</span>
              </div>
            </div>
          </div>
        </div>
      </section>

      <!-- 数据流向图 - 软件定义架构 -->
      <section class="flow-section">
        <BaseCard title="软件定义数据流向" subtitle="设备层 → 设备抽象 → 原子服务 → 应用" icon-color="blue">
          <template #header>
            <div class="flex items-center gap-3">
              <div class="card-icon icon-blue">
                <svg width="20" height="20" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M22 12h-4l-3 9L9 3l-3 9H2"/></svg>
              </div>
              <div><h3 class="card-title">软件定义数据流向</h3><p class="card-subtitle">设备层 → 设备抽象 → 原子服务 → 应用</p></div>
            </div>
          </template>
          <div class="flow-canvas">
            <canvas ref="flowCanvasRef"></canvas>
          </div>
        </BaseCard>
      </section>

      <!-- 概览卡片 -->
      <section class="overview-section">
        <div class="cards-grid">
          <DataCard v-for="(card, index) in overviewCards" :key="card.label" v-bind="card" :delay="index * 100" />
        </div>
      </section>

      <!-- 设备状态和实时数据流 -->
      <section class="devices-stream-section">
        <div class="devices-area">
          <BaseCard title="设备状态" subtitle="实时设备监控" icon-color="green">
            <template #header>
              <div class="flex items-center gap-3">
                <div class="card-icon icon-green">
                  <svg width="20" height="20" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><rect x="2" y="3" width="20" height="14" rx="2"/><path d="M8 21h8m-4-4v4"/></svg>
                </div>
                <div><h3 class="card-title">设备状态</h3><p class="card-subtitle">实时设备监控</p></div>
              </div>
            </template>
            <div class="device-cards-grid">
              <DeviceCard v-for="device in devices" :key="device.name" v-bind="device" :show-chart="true" :show-actions="true" @action="handleDeviceAction" />
            </div>
          </BaseCard>
        </div>

        <div class="stream-area">
          <BaseCard title="实时数据流" subtitle="系统事件监控" icon-color="cyan">
            <template #header>
              <div class="flex items-center gap-3">
                <div class="card-icon icon-cyan">
                  <svg width="20" height="20" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M13 2L3 14h9l-1 8 10-12h-9l1-8z"/></svg>
                </div>
                <div><h3 class="card-title">实时数据流</h3><p class="card-subtitle">系统事件监控</p></div>
              </div>
            </template>
            <LiveDataStream :max-items="30" />
          </BaseCard>
        </div>
      </section>
    </main>

    <!-- 资源池面板 - 动态创建应用 -->
    <Transition name="slide-right">
      <div v-if="showResourcePool" class="resource-pool-overlay" @click.self="showResourcePool = false">
        <div class="resource-pool-panel">
          <div class="panel-header">
            <div class="flex items-center gap-3">
              <div class="panel-icon">
                <svg width="20" height="20" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M21 16V8a2 2 0 0 0-1-1.73l-7-4a2 2 0 0 0-2 0l-7 4A2 2 0 0 0 3 8v8a2 2 0 0 0 1 1.73l7 4a2 2 0 0 0 2 0l7-4A2 2 0 0 0 21 16z"/></svg>
              </div>
              <div><h3 class="panel-title">资源池</h3><p class="panel-subtitle">选择设备创建新应用</p></div>
            </div>
            <button class="panel-close" @click="showResourcePool = false">×</button>
          </div>

          <div class="pool-content">
            <!-- 资源池设备列表 -->
            <div class="pool-section">
              <div class="section-title">可用设备资源</div>
              <div class="pool-devices">
                <div v-for="device in poolDevices" :key="device.id" class="pool-device" :class="{ selected: selectedPoolDevices.includes(device.id) }" @click="togglePoolDevice(device.id)">
                  <div class="device-icon" :style="{ background: device.color + '20', color: device.color }">{{ device.icon }}</div>
                  <div class="device-info">
                    <div class="device-name">{{ device.name }}</div>
                    <div class="device-type">{{ device.type }}</div>
                  </div>
                  <div class="device-check" v-if="selectedPoolDevices.includes(device.id)">✓</div>
                </div>
              </div>
            </div>

            <!-- 创建新应用 -->
            <div class="create-app-section">
              <div class="section-title">创建新应用</div>
              <div class="app-form">
                <div class="form-group"><label>应用名称 *</label><input v-model="newAppName" placeholder="输入应用名称" /></div>
                <div class="form-group"><label>应用描述</label><textarea v-model="newAppDesc" placeholder="输入应用描述" rows="2"></textarea></div>
                <div class="form-group"><label>应用类型</label><select v-model="newAppType"><option value="monitoring">监测应用</option><option value="control">控制应用</option><option value="analysis">分析应用</option></select></div>
                <div class="selected-count">已选择 {{ selectedPoolDevices.length }} 个设备</div>
                <CyberButton variant="primary" size="sm" @click="createNewApp" :disabled="selectedPoolDevices.length === 0 || !newAppName">
                  创建应用
                </CyberButton>
              </div>
            </div>

            <!-- 已创建的应用 -->
            <div class="created-apps-section" v-if="createdApps.length > 0">
              <div class="section-title">已创建的应用</div>
              <div class="created-apps">
                <div v-for="app in createdApps" :key="app.id" class="created-app" @click="openApp(app)">
                  <div class="app-icon">{{ app.icon }}</div>
                  <div class="app-info">
                    <div class="app-name">{{ app.name }}</div>
                    <div class="app-desc">{{ app.description }}</div>
                    <div class="app-meta">
                      <span class="app-type">{{ app.type === 'monitoring' ? '监测' : app.type === 'control' ? '控制' : '分析' }}</span>
                      <span class="app-devices">{{ app.devices.length }} 个设备</span>
                    </div>
                  </div>
                  <div class="app-actions">
                    <CyberButton variant="ghost" size="sm" @click.stop="openApp(app)">打开</CyberButton>
                    <CyberButton variant="danger" size="sm" @click.stop="deleteApp(app)">删除</CyberButton>
                  </div>
                </div>
              </div>
            </div>
          </div>
        </div>
      </div>
    </Transition>

    <!-- 应用详情模态框 -->
    <Transition name="modal">
      <div v-if="showAppDetail" class="modal-overlay" @click.self="showAppDetail = false">
        <div class="modal-content modal-large">
          <div class="modal-header">
            <div class="flex items-center gap-3">
              <span class="app-modal-icon">{{ selectedApp?.icon }}</span>
              <div>
                <h3>{{ selectedApp?.name }}</h3>
                <p class="app-modal-desc">{{ selectedApp?.description }}</p>
              </div>
            </div>
            <button class="modal-close" @click="showAppDetail = false">×</button>
          </div>
          <div class="modal-body">
            <div class="app-detail-content">
              <!-- 应用数据流向图 -->
              <div class="app-flow-section">
                <div class="section-title">应用数据流向</div>
                <div class="app-flow-canvas">
                  <canvas ref="appFlowCanvasRef"></canvas>
                </div>
              </div>

              <!-- 应用设备列表 -->
              <div class="app-devices-section">
                <div class="section-title">应用设备</div>
                <div class="app-devices-list">
                  <div v-for="deviceId in selectedApp?.devices" :key="deviceId" class="app-device-item">
                    <div class="device-icon">{{ getPoolDeviceIcon(deviceId) }}</div>
                    <div class="device-name">{{ getPoolDeviceName(deviceId) }}</div>
                    <div class="device-status online">在线</div>
                  </div>
                </div>
              </div>

              <!-- 应用数据面板 -->
              <div class="app-data-section">
                <div class="section-title">实时数据</div>
                <div class="app-data-grid">
                  <div v-for="data in appDataPoints" :key="data.label" class="app-data-card">
                    <div class="data-label">{{ data.label }}</div>
                    <div class="data-value">{{ data.value }}<span class="data-unit">{{ data.unit }}</span></div>
                    <div class="data-trend" :class="data.trend">{{ data.trendText }}</div>
                  </div>
                </div>
              </div>
            </div>
          </div>
        </div>
      </div>
    </Transition>
  </div>
</template>

<script setup lang="ts">
import { ref, reactive, onMounted, onUnmounted } from 'vue'
import { ElMessage } from 'element-plus'
import ParticleBackground from '../components/ParticleBackground.vue'
import AppNavbar from '../components/AppNavbar.vue'
import DataCard from '../components/DataCard.vue'
import BaseCard from '../components/BaseCard.vue'
import CyberButton from '../components/CyberButton.vue'
import DeviceCard from '../components/DeviceCard.vue'
import LiveDataStream from '../components/LiveDataStream.vue'

const showResourcePool = ref(false)
const showAppDetail = ref(false)
const selectedApp = ref<any>(null)
const flowCanvasRef = ref<HTMLCanvasElement | null>(null)
const appFlowCanvasRef = ref<HTMLCanvasElement | null>(null)
let flowAnimationId: number | null = null
let appFlowAnimationId: number | null = null

// 资源池相关
const selectedPoolDevices = ref<string[]>([])
const newAppName = ref('')
const newAppDesc = ref('')
const newAppType = ref('monitoring')

interface CreatedApp {
  id: string
  name: string
  description: string
  type: 'monitoring' | 'control' | 'analysis'
  icon: string
  devices: string[]
  createdAt: string
}

const createdApps = reactive<CreatedApp[]>([])

// 软件定义架构层级
const applicationLayer = reactive([
  { id: 'app-security', name: '安防系统', icon: '🛡️', active: true },
  { id: 'app-environment', name: '环境监测', icon: '🌡️', active: true },
  { id: 'app-fire', name: '消防系统', icon: '🔥', active: true },
  { id: 'app-custom', name: '自定义应用', icon: '📱', active: false }
])

const atomicUpperLayer = reactive([
  { id: 'upper-monitor', name: '环境监测服务', icon: '📊', active: true },
  { id: 'upper-security', name: '安防监控服务', icon: '🛡️', active: true },
  { id: 'upper-fire', name: '消防预警服务', icon: '🔥', active: true },
  { id: 'upper-linkage', name: '设备联动服务', icon: '🔗', active: true }
])

const atomicLowerLayer = reactive([
  { id: 'lower-collect', name: '数据采集服务', icon: '📥', active: true },
  { id: 'lower-process', name: '数据处理服务', icon: '⚙️', active: true },
  { id: 'lower-store', name: '数据存储服务', icon: '💾', active: true },
  { id: 'lower-alarm', name: '报警判断服务', icon: '🔔', active: true },
  { id: 'lower-control', name: '设备控制服务', icon: '🎮', active: true }
])

// 设备抽象层：一对一解耦，多数据设备拆分为多个虚拟设备
// 云测仪 SD123-E60V2 → 8个虚拟传感器
const abstractionLayer = reactive([
  { id: 'abs-temp', name: '虚拟温度', icon: '🌡️', active: true },
  { id: 'abs-humi', name: '虚拟湿度', icon: '💧', active: true },
  { id: 'abs-pm25', name: '虚拟PM2.5', icon: '💨', active: true },
  { id: 'abs-co2', name: '虚拟CO2', icon: '☁️', active: true },
  { id: 'abs-tvoc', name: '虚拟TVOC', icon: '🧪', active: true },
  { id: 'abs-ch2o', name: '虚拟甲醛', icon: '⚗️', active: true },
  { id: 'abs-pm10', name: '虚拟PM10', icon: '💨', active: true },
  { id: 'abs-smoke', name: '虚拟烟雾', icon: '🔥', active: true },
  { id: 'abs-water', name: '虚拟水浸', icon: '💧', active: true },
  { id: 'abs-infrared', name: '虚拟红外', icon: '👤', active: true },
  { id: 'abs-light', name: '虚拟光照', icon: '💡', active: true },
  { id: 'abs-humidifier', name: '虚拟净化', icon: '🌀', active: true }
])

// 设备层：物理设备
const deviceLayer = reactive([
  { id: 'dev-cloud', name: '云测仪(多数据)', icon: '🌡️', online: true, value: 24.5, unit: '℃' },
  { id: 'dev-smoke', name: '烟雾报警器', icon: '🔥', online: true, value: 0, unit: '' },
  { id: 'dev-water', name: '水浸传感器', icon: '💧', online: true, value: 0, unit: 'cm' },
  { id: 'dev-infrared', name: '红外探测器', icon: '👤', online: true, value: 0, unit: '' },
  { id: 'dev-light', name: '弱光传感器', icon: '💡', online: true, value: 350, unit: 'lux' },
  { id: 'dev-humidifier', name: '恒湿净化机', icon: '🌀', online: true, value: 0, unit: '' },
  { id: 'dev-conditioner', name: '空调控制器', icon: '❄️', online: true, value: 24, unit: '℃' }
])

// 概览卡片数据
const overviewCards = reactive([
  { label: '设备总数', value: 36, unit: '台', icon: '<rect x="2" y="3" width="20" height="14" rx="2"/><path d="M8 21h8m-4-4v4"/>', iconColor: 'blue' as const, status: 'normal' as const, trend: '在线', trendDirection: 'stable' as const, miniChartData: [32, 34, 35, 36, 35, 36, 36] },
  { label: '原子服务', value: 9, unit: '个', icon: '<circle cx="12" cy="12" r="3"/><path d="M12 1v6m0 6v6m-7-7h6m6 0h6"/>', iconColor: 'purple' as const, status: 'normal' as const, trend: '运行中', trendDirection: 'stable' as const, miniChartData: [9, 9, 9, 9, 9, 9, 9] },
  { label: '数据吞吐', value: 1284, unit: '条/s', icon: '<polyline points="22 12 18 12 15 21 9 3 6 12 2 12"/>', iconColor: 'green' as const, status: 'normal' as const, trend: '稳定', trendDirection: 'stable' as const, miniChartData: [1200, 1250, 1280, 1260, 1290, 1270, 1284] },
  { label: '自定义应用', value: createdApps.length, unit: '个', icon: '<rect x="3" y="3" width="18" height="18" rx="2"/><path d="M12 8v8m-4-4h8"/>', iconColor: 'cyan' as const, status: 'normal' as const, trend: '可扩展', trendDirection: 'stable' as const, miniChartData: [0, 0, 0, 0, 0, 0, createdApps.length] }
])

// 设备数据
const devices = reactive([
  { name: '云测仪 SD123', type: '环境传感器', icon: '🌡️', value: 24.5, unit: '℃', status: 'online' as const, trend: 'stable' as const, trendText: '稳定', chartData: [24.2, 24.5, 24.3, 24.6, 24.4, 24.5, 24.5], actions: [{ id: 'detail', label: '详情', icon: '<circle cx="12" cy="12" r="10"/><path d="M12 16v-4"/><path d="M12 8h.01"/>', type: 'primary' as const }] },
  { name: '烟雾报警器', type: '安防设备', icon: '🔥', value: 0, unit: '', status: 'online' as const, trend: 'stable' as const, trendText: '正常', chartData: [0, 0, 0, 0, 0, 0, 0], actions: [{ id: 'test', label: '测试', icon: '<polygon points="5 3 19 12 5 21 5 3"/>', type: 'primary' as const }] },
  { name: '水浸传感器', type: '安防设备', icon: '💧', value: 0, unit: 'cm', status: 'online' as const, trend: 'stable' as const, trendText: '正常', chartData: [0, 0, 0, 0, 0, 0, 0], actions: [{ id: 'simulate', label: '模拟', icon: '<path d="M14.7 6.3a1 1 0 0 0 0 1.4l1.6 1.6a1 1 0 0 0 1.4 0l3.77-3.77a6 6 0 0 1-7.94 7.94l-6.91 6.91a2.12 2.12 0 0 1-3-3l6.91-6.91a6 6 0 0 1 7.94-7.94l-3.76 3.76z"/>', type: 'danger' as const }] },
  { name: '红外探测器', type: '安防设备', icon: '👤', value: 0, unit: '', status: 'online' as const, trend: 'stable' as const, trendText: '正常', chartData: [0, 0, 1, 0, 0, 0, 0], actions: [{ id: 'detail', label: '详情', icon: '<circle cx="12" cy="12" r="10"/><path d="M12 16v-4"/><path d="M12 8h.01"/>', type: 'primary' as const }] },
  { name: '空调控制器', type: '控制设备', icon: '❄️', value: 24, unit: '℃', status: 'online' as const, trend: 'down' as const, trendText: '下降', chartData: [26, 25.5, 25, 24.8, 24.5, 24.2, 24], actions: [{ id: 'on', label: '开启', icon: '<path d="M18.36 6.64a9 9 0 1 1-12.73 0"/><line x1="12" y1="2" x2="12" y2="12"/>', type: 'primary' as const }] },
  { name: '通风净化机', type: '控制设备', icon: '🌀', value: 0, unit: '', status: 'offline' as const, trend: 'stable' as const, trendText: '离线', chartData: [], actions: [{ id: 'restart', label: '重启', icon: '<path d="M21.5 2v6h-6M2.5 22v-6h6M2 11.5a10 10 0 0 1 18.8-4.3M22 12.5a10 10 0 0 1-18.8 4.3"/>', type: 'primary' as const }] }
])

// 资源池设备
const poolDevices = reactive([
  { id: 'pool-001', name: '温度传感器-副本1', type: '传感器', icon: '🌡️', color: '#3b82f6' },
  { id: 'pool-002', name: '湿度传感器-副本1', type: '传感器', icon: '💧', color: '#06b6d4' },
  { id: 'pool-003', name: 'PM2.5传感器-副本1', type: '传感器', icon: '💨', color: '#f59e0b' },
  { id: 'pool-004', name: 'CO2传感器-副本1', type: '传感器', icon: '☁️', color: '#8b5cf6' },
  { id: 'pool-005', name: '烟雾报警器-副本1', type: '报警器', icon: '🔥', color: '#ef4444' },
  { id: 'pool-006', name: '水浸传感器-副本1', type: '传感器', icon: '💧', color: '#06b6d4' },
  { id: 'pool-007', name: '红外探测器-副本1', type: '传感器', icon: '👤', color: '#8b5cf6' },
  { id: 'pool-008', name: '弱光传感器-副本1', type: '传感器', icon: '💡', color: '#f59e0b' },
  { id: 'pool-009', name: '温度传感器-副本2', type: '传感器', icon: '🌡️', color: '#3b82f6' },
  { id: 'pool-010', name: '湿度传感器-副本2', type: '传感器', icon: '💧', color: '#06b6d4' },
  { id: 'pool-011', name: 'PM2.5传感器-副本2', type: '传感器', icon: '💨', color: '#f59e0b' },
  { id: 'pool-012', name: 'CO2传感器-副本2', type: '传感器', icon: '☁️', color: '#8b5cf6' }
])

// 应用数据点 - 根据选择的设备动态生成
const appDataPoints = ref<Array<{ label: string; value: number; unit: string; trend: string; trendText: string }>>([])

// 设备数据映射
const deviceDataMap: Record<string, { label: string; value: number; unit: string; trend: string; trendText: string }> = {
  'pool-001': { label: '温度', value: 24.5, unit: '℃', trend: 'stable', trendText: '稳定' },
  'pool-002': { label: '湿度', value: 65, unit: '%', trend: 'down', trendText: '下降' },
  'pool-003': { label: 'PM2.5', value: 42, unit: 'μg/m³', trend: 'stable', trendText: '正常' },
  'pool-004': { label: 'CO2', value: 520, unit: 'ppm', trend: 'up', trendText: '上升' },
  'pool-005': { label: '烟雾浓度', value: 0, unit: '', trend: 'stable', trendText: '正常' },
  'pool-006': { label: '水浸状态', value: 0, unit: 'cm', trend: 'stable', trendText: '正常' },
  'pool-007': { label: '红外检测', value: 0, unit: '', trend: 'stable', trendText: '正常' },
  'pool-008': { label: '光照强度', value: 350, unit: 'lux', trend: 'stable', trendText: '正常' },
  'pool-009': { label: '温度2', value: 24.5, unit: '℃', trend: 'stable', trendText: '稳定' },
  'pool-010': { label: '湿度2', value: 65, unit: '%', trend: 'down', trendText: '下降' },
  'pool-011': { label: 'PM2.5-2', value: 42, unit: 'μg/m³', trend: 'stable', trendText: '正常' },
  'pool-012': { label: 'CO2-2', value: 520, unit: 'ppm', trend: 'up', trendText: '上升' }
}

// 切换资源池设备选择
function togglePoolDevice(deviceId: string) {
  const index = selectedPoolDevices.value.indexOf(deviceId)
  if (index === -1) {
    selectedPoolDevices.value.push(deviceId)
  } else {
    selectedPoolDevices.value.splice(index, 1)
  }
}

// 获取资源池设备名称
function getPoolDeviceName(deviceId: string): string {
  const device = poolDevices.find(d => d.id === deviceId)
  return device?.name || '未知设备'
}

// 获取资源池设备图标
function getPoolDeviceIcon(deviceId: string): string {
  const device = poolDevices.find(d => d.id === deviceId)
  return device?.icon || '📡'
}

// 创建新应用
function createNewApp() {
  if (!newAppName.value || selectedPoolDevices.value.length === 0) {
    ElMessage.warning('请填写应用名称并选择设备')
    return
  }

  const app: CreatedApp = {
    id: `app-${Date.now()}`,
    name: newAppName.value,
    description: newAppDesc.value || '用户自定义应用',
    type: newAppType.value as 'monitoring' | 'control' | 'analysis',
    icon: newAppType.value === 'monitoring' ? '📊' : newAppType.value === 'control' ? '🎮' : '📈',
    devices: [...selectedPoolDevices.value],
    createdAt: new Date().toLocaleString('zh-CN')
  }

  createdApps.push(app)
  ElMessage.success(`应用 "${app.name}" 创建成功`)

  // 重置表单
  newAppName.value = ''
  newAppDesc.value = ''
  selectedPoolDevices.value = []
}

// 打开应用详情
function openApp(app: CreatedApp) {
  selectedApp.value = app
  showAppDetail.value = true

  // 根据选择的设备生成数据点
  appDataPoints.value = app.devices.map((deviceId: string) => {
    const data = deviceDataMap[deviceId]
    if (data) {
      // 添加一些随机波动
      const fluctuation = (Math.random() - 0.5) * 2
      return {
        ...data,
        value: Math.round((data.value + fluctuation) * 10) / 10
      }
    }
    // 如果没有预定义数据，生成默认数据
    const device = poolDevices.find(d => d.id === deviceId)
    return {
      label: device?.name || '未知设备',
      value: Math.round(Math.random() * 100 * 10) / 10,
      unit: '',
      trend: 'stable',
      trendText: '正常'
    }
  })

  // 绘制应用数据流向图
  setTimeout(() => {
    drawAppFlowChart()
  }, 100)
}

// 删除应用
function deleteApp(app: CreatedApp) {
  const index = createdApps.findIndex(a => a.id === app.id)
  if (index !== -1) {
    createdApps.splice(index, 1)
    ElMessage.success(`应用 "${app.name}" 已删除`)
  }
}

// 处理设备操作
function handleDeviceAction(actionId: string) {
  ElMessage.info(`设备操作: ${actionId}`)
}

// 绘制软件定义架构数据流向图
function drawFlowChart() {
  if (!flowCanvasRef.value) return

  const canvas = flowCanvasRef.value
  const ctx = canvas.getContext('2d')
  if (!ctx) return

  const container = canvas.parentElement
  if (!container) return

  canvas.width = container.clientWidth
  canvas.height = 400

  // 软件定义架构节点 - 居中布局
  const ox = 120 // 向右偏移居中
  const layers = [
    {
      name: '设备层',
      y: 350,
      nodes: [
        { id: 'sensor-cloud', x: 80 + ox, icon: '🌡️', label: '云测仪(多数据)', color: '#3b82f6' },
        { id: 'sensor-smoke', x: 200 + ox, icon: '🔥', label: '烟雾报警器', color: '#ef4444' },
        { id: 'sensor-water', x: 320 + ox, icon: '💧', label: '水浸传感器', color: '#06b6d4' },
        { id: 'sensor-infrared', x: 440 + ox, icon: '👤', label: '红外探测器', color: '#8b5cf6' },
        { id: 'sensor-light', x: 560 + ox, icon: '💡', label: '弱光传感器', color: '#f59e0b' },
        { id: 'sensor-humidifier', x: 680 + ox, icon: '🌀', label: '恒湿净化机', color: '#22c55d' }
      ]
    },
    {
      name: '设备抽象层（一对一解耦）',
      y: 270,
      nodes: [
        { id: 'abs-temp', x: 10 + ox, icon: '🌡️', label: '虚拟温度', color: '#3b82f6' },
        { id: 'abs-humi', x: 80 + ox, icon: '💧', label: '虚拟湿度', color: '#06b6d4' },
        { id: 'abs-pm25', x: 150 + ox, icon: '💨', label: '虚拟PM2.5', color: '#f59e0b' },
        { id: 'abs-co2', x: 220 + ox, icon: '☁️', label: '虚拟CO2', color: '#8b5cf6' },
        { id: 'abs-tvoc', x: 290 + ox, icon: '🧪', label: '虚拟TVOC', color: '#ec4899' },
        { id: 'abs-ch2o', x: 360 + ox, icon: '⚗️', label: '虚拟甲醛', color: '#14b8a6' },
        { id: 'abs-pm10', x: 430 + ox, icon: '💨', label: '虚拟PM10', color: '#f59e0b' },
        { id: 'abs-smoke', x: 510 + ox, icon: '🔥', label: '虚拟烟雾', color: '#ef4444' },
        { id: 'abs-water', x: 580 + ox, icon: '💧', label: '虚拟水浸', color: '#06b6d4' },
        { id: 'abs-infrared', x: 650 + ox, icon: '👤', label: '虚拟红外', color: '#8b5cf6' },
        { id: 'abs-light', x: 720 + ox, icon: '💡', label: '虚拟光照', color: '#f59e0b' },
        { id: 'abs-humidifier', x: 790 + ox, icon: '🌀', label: '虚拟净化', color: '#22c55d' }
      ]
    },
    {
      name: '原子服务下层',
      y: 190,
      nodes: [
        { id: 'lower-collect', x: 100 + ox, icon: '📥', label: '数据采集', color: '#3b82f6' },
        { id: 'lower-process', x: 250 + ox, icon: '⚙️', label: '数据处理', color: '#8b5cf6' },
        { id: 'lower-alarm', x: 400 + ox, icon: '🔔', label: '报警判断', color: '#ef4444' },
        { id: 'lower-store', x: 550 + ox, icon: '💾', label: '数据存储', color: '#f59e0b' },
        { id: 'lower-control', x: 700 + ox, icon: '🎮', label: '设备控制', color: '#22c55d' }
      ]
    },
    {
      name: '原子服务上层',
      y: 110,
      nodes: [
        { id: 'upper-monitor', x: 200 + ox, icon: '📊', label: '环境监测服务', color: '#3b82f6' },
        { id: 'upper-security', x: 450 + ox, icon: '🛡️', label: '安防监控服务', color: '#8b5cf6' },
        { id: 'upper-fire', x: 650 + ox, icon: '🔥', label: '消防预警服务', color: '#ef4444' }
      ]
    },
    {
      name: '应用层',
      y: 30,
      nodes: [
        { id: 'app-env', x: 200 + ox, icon: '🌡️', label: '环境监测', color: '#3b82f6' },
        { id: 'app-security', x: 450 + ox, icon: '🛡️', label: '安防系统', color: '#8b5cf6' },
        { id: 'app-fire', x: 650 + ox, icon: '🔥', label: '消防系统', color: '#ef4444' }
      ]
    }
  ]

  // 连接定义 - 修正设备抽象层连接
  const connections = [
    // 设备层 → 设备抽象层（云测仪解耦成8个虚拟传感器）
    { from: 'sensor-cloud', to: 'abs-temp', active: true },
    { from: 'sensor-cloud', to: 'abs-humi', active: true },
    { from: 'sensor-cloud', to: 'abs-pm25', active: true },
    { from: 'sensor-cloud', to: 'abs-co2', active: true },
    { from: 'sensor-cloud', to: 'abs-tvoc', active: true },
    { from: 'sensor-cloud', to: 'abs-ch2o', active: true },
    { from: 'sensor-cloud', to: 'abs-pm10', active: true },
    // 独立设备一对一抽象
    { from: 'sensor-smoke', to: 'abs-smoke', active: true },
    { from: 'sensor-water', to: 'abs-water', active: true },
    { from: 'sensor-infrared', to: 'abs-infrared', active: true },
    { from: 'sensor-light', to: 'abs-light', active: true },
    { from: 'sensor-humidifier', to: 'abs-humidifier', active: true },
    // 设备抽象层 → 原子服务下层
    { from: 'abs-temp', to: 'lower-collect', active: true },
    { from: 'abs-humi', to: 'lower-collect', active: true },
    { from: 'abs-pm25', to: 'lower-collect', active: true },
    { from: 'abs-co2', to: 'lower-collect', active: true },
    { from: 'abs-smoke', to: 'lower-collect', active: true },
    { from: 'abs-smoke', to: 'lower-alarm', active: true },
    { from: 'abs-water', to: 'lower-collect', active: true },
    { from: 'abs-water', to: 'lower-alarm', active: true },
    { from: 'abs-infrared', to: 'lower-collect', active: true },
    { from: 'abs-infrared', to: 'lower-alarm', active: true },
    { from: 'abs-light', to: 'lower-collect', active: true },
    { from: 'abs-humidifier', to: 'lower-collect', active: true },
    { from: 'abs-humidifier', to: 'lower-control', active: true },
    // 原子服务下层 → 原子服务下层（服务间调用）
    { from: 'lower-collect', to: 'lower-process', active: true },  // 数据采集 → 数据处理
    { from: 'lower-collect', to: 'lower-alarm', active: true },    // 数据采集 → 报警判断
    { from: 'lower-process', to: 'lower-store', active: true },    // 数据处理 → 数据存储
    // 原子服务下层 → 原子服务上层（一对多）
    { from: 'lower-collect', to: 'upper-monitor', active: true },  // 数据采集 → 环境监测
    { from: 'lower-collect', to: 'upper-security', active: true }, // 数据采集 → 安防监控
    { from: 'lower-collect', to: 'upper-fire', active: true },     // 数据采集 → 消防预警
    { from: 'lower-process', to: 'upper-monitor', active: true },  // 数据处理 → 环境监测
    { from: 'lower-process', to: 'upper-security', active: true }, // 数据处理 → 安防监控
    { from: 'lower-alarm', to: 'upper-security', active: true },   // 报警判断 → 安防监控
    { from: 'lower-alarm', to: 'upper-fire', active: true },       // 报警判断 → 消防预警
    { from: 'lower-control', to: 'upper-security', active: true }, // 设备控制 → 安防监控
    { from: 'lower-control', to: 'upper-fire', active: true },     // 设备控制 → 消防预警
    { from: 'lower-store', to: 'upper-monitor', active: true },    // 数据存储 → 环境监测
    // 原子服务上层 → 应用层
    { from: 'upper-monitor', to: 'app-env', active: true },
    { from: 'upper-security', to: 'app-security', active: true },
    { from: 'upper-fire', to: 'app-fire', active: true }
  ]

  let time = 0

  function animate() {
    if (!ctx) return

    ctx.clearRect(0, 0, canvas.width, canvas.height)
    time += 0.02

    // 绘制层级背景
    layers.forEach((layer, index) => {
      ctx.fillStyle = index % 2 === 0 ? 'rgba(59, 130, 246, 0.03)' : 'rgba(139, 92, 246, 0.03)'
      ctx.fillRect(0, layer.y - 10, canvas.width, 70)

      // 层级标签
      ctx.font = '12px sans-serif'
      ctx.fillStyle = '#64748b'
      ctx.textAlign = 'left'
      ctx.fillText(layer.name, 10, layer.y + 25)
    })

    // 绘制连接线
    connections.forEach(conn => {
      const fromLayer = layers.find(l => l.nodes.some(n => n.id === conn.from))
      const toLayer = layers.find(l => l.nodes.some(n => n.id === conn.to))
      if (!fromLayer || !toLayer) return

      const fromNode = fromLayer.nodes.find(n => n.id === conn.from)
      const toNode = toLayer.nodes.find(n => n.id === conn.to)
      if (!fromNode || !toNode) return

      // 绘制路径
      ctx.beginPath()
      ctx.moveTo(fromNode.x, fromLayer.y + 30)
      ctx.lineTo(toNode.x, toLayer.y + 30)
      ctx.strokeStyle = conn.active ? '#3b82f6' : '#334155'
      ctx.lineWidth = conn.active ? 2 : 1
      ctx.globalAlpha = conn.active ? 0.6 : 0.2
      ctx.stroke()

      // 绘制流动粒子
      if (conn.active) {
        const t = (time % 2) / 2
        const px = fromNode.x + (toNode.x - fromNode.x) * t
        const py = fromLayer.y + 30 + (toLayer.y + 30 - fromLayer.y - 30) * t

        ctx.beginPath()
        ctx.arc(px, py, 4, 0, Math.PI * 2)
        ctx.fillStyle = '#3b82f6'
        ctx.globalAlpha = 0.9
        ctx.fill()

        // 发光效果
        ctx.beginPath()
        ctx.arc(px, py, 8, 0, Math.PI * 2)
        ctx.fillStyle = '#3b82f6'
        ctx.globalAlpha = 0.3
        ctx.fill()
      }
    })

    ctx.globalAlpha = 1

    // 绘制节点
    layers.forEach(layer => {
      layer.nodes.forEach(node => {
        // 节点背景
        ctx.beginPath()
        ctx.arc(node.x, layer.y + 30, 20, 0, Math.PI * 2)
        ctx.fillStyle = node.color + '30'
        ctx.strokeStyle = node.color
        ctx.lineWidth = 2
        ctx.fill()
        ctx.stroke()

        // 节点图标
        ctx.font = '16px serif'
        ctx.textAlign = 'center'
        ctx.textBaseline = 'middle'
        ctx.fillStyle = '#ffffff'
        ctx.fillText(node.icon, node.x, layer.y + 30)

        // 节点标签
        ctx.font = '10px sans-serif'
        ctx.fillStyle = node.color
        ctx.fillText(node.label, node.x, layer.y + 55)
      })
    })

    flowAnimationId = requestAnimationFrame(animate)
  }

  animate()
}

// 绘制应用数据流向图
function drawAppFlowChart() {
  if (!appFlowCanvasRef.value || !selectedApp.value) return

  const canvas = appFlowCanvasRef.value
  const ctx = canvas.getContext('2d')
  if (!ctx) return

  const container = canvas.parentElement
  if (!container) return

  canvas.width = container.clientWidth
  canvas.height = 300

  const app = selectedApp.value

  // 使用应用保存的nodes和connections
  const nodes = app.nodes || []
  const connections = app.connections || []

  // 如果没有保存的nodes，使用默认布局
  if (nodes.length === 0) {
    drawDefaultAppChart(ctx, canvas, app)
    return
  }

  // 自动缩放节点位置到画布范围
  let minX = Infinity, maxX = -Infinity, minY = Infinity, maxY = -Infinity
  nodes.forEach((n: any) => {
    if (n.x < minX) minX = n.x
    if (n.x > maxX) maxX = n.x
    if (n.y < minY) minY = n.y
    if (n.y > maxY) maxY = n.y
  })

  const rangeX = maxX - minX || 1
  const rangeY = maxY - minY || 1
  const padding = 60
  const scaleX = (canvas.width - padding * 2) / rangeX
  const scaleY = (canvas.height - padding * 2) / rangeY
  const scale = Math.min(scaleX, scaleY, 1.5)

  const offsetX = (canvas.width - rangeX * scale) / 2 - minX * scale
  const offsetY = (canvas.height - rangeY * scale) / 2 - minY * scale

  // 缩放后的节点
  const scaledNodes = nodes.map((n: any) => ({
    ...n,
    sx: n.x * scale + offsetX,
    sy: n.y * scale + offsetY
  }))

  let time = 0

  function animate() {
    if (!ctx) return

    ctx.clearRect(0, 0, canvas.width, canvas.height)
    time += 0.015

    // 绘制连接线
    connections.forEach((conn: any) => {
      const fromNode = scaledNodes.find((n: any) => n.id === conn.from)
      const toNode = scaledNodes.find((n: any) => n.id === conn.to)
      if (!fromNode || !toNode) return

      // 贝塞尔曲线
      const midX = (fromNode.sx + toNode.sx) / 2
      ctx.beginPath()
      ctx.moveTo(fromNode.sx, fromNode.sy)
      ctx.bezierCurveTo(midX, fromNode.sy, midX, toNode.sy, toNode.sx, toNode.sy)
      ctx.strokeStyle = conn.active ? '#3b82f6' : '#334155'
      ctx.lineWidth = conn.active ? 2 : 1
      ctx.globalAlpha = conn.active ? 0.6 : 0.2
      ctx.stroke()

      // 流动粒子
      if (conn.active) {
        const t = (time % 2) / 2
        // 贝塞尔曲线插值
        const t1 = 1 - t
        const px = t1 * t1 * t1 * fromNode.sx + 3 * t1 * t1 * t * midX + 3 * t1 * t * t * midX + t * t * t * toNode.sx
        const py = t1 * t1 * t1 * fromNode.sy + 3 * t1 * t1 * t * fromNode.sy + 3 * t1 * t * t * toNode.sy + t * t * t * toNode.sy

        ctx.beginPath()
        ctx.arc(px, py, 4, 0, Math.PI * 2)
        ctx.fillStyle = '#3b82f6'
        ctx.globalAlpha = 0.9
        ctx.fill()

        // 发光
        ctx.beginPath()
        ctx.arc(px, py, 8, 0, Math.PI * 2)
        ctx.fillStyle = '#3b82f6'
        ctx.globalAlpha = 0.3
        ctx.fill()
      }
    })

    ctx.globalAlpha = 1

    // 绘制节点
    scaledNodes.forEach((node: any) => {
      const size = 22
      const color = node.color || '#3b82f6'

      // 外圈发光
      const gradient = ctx.createRadialGradient(node.sx, node.sy, size, node.sx, node.sy, size * 2)
      gradient.addColorStop(0, color + '30')
      gradient.addColorStop(1, 'transparent')
      ctx.beginPath()
      ctx.arc(node.sx, node.sy, size * 2, 0, Math.PI * 2)
      ctx.fillStyle = gradient
      ctx.fill()

      // 节点主体
      ctx.beginPath()
      ctx.arc(node.sx, node.sy, size, 0, Math.PI * 2)
      ctx.fillStyle = color + '30'
      ctx.strokeStyle = color
      ctx.lineWidth = 2
      ctx.fill()
      ctx.stroke()

      // 图标
      ctx.font = '18px serif'
      ctx.textAlign = 'center'
      ctx.textBaseline = 'middle'
      ctx.fillStyle = '#ffffff'
      ctx.fillText(node.icon || '📡', node.sx, node.sy)

      // 标签
      ctx.font = '11px sans-serif'
      ctx.fillStyle = color
      ctx.fillText(node.label || node.typeLabel || '', node.sx, node.sy + size + 14)
    })

    requestAnimationFrame(animate)
  }

  animate()
}

// 默认应用图表（当没有保存nodes时）
function drawDefaultAppChart(ctx: CanvasRenderingContext2D, canvas: HTMLCanvasElement, app: any) {
  const deviceCount = app.devices?.length || 0
  const appNode = { x: canvas.width / 2, y: 50, icon: app.icon, label: app.name, color: '#3b82f6' }
  const serviceNodes = [
    { x: canvas.width / 2 - 150, y: 150, icon: '📥', label: '数据采集', color: '#8b5cf6' },
    { x: canvas.width / 2, y: 150, icon: '⚙️', label: '数据处理', color: '#22c55d' },
    { x: canvas.width / 2 + 150, y: 150, icon: '💾', label: '数据存储', color: '#f59e0b' }
  ]
  const deviceNodes = (app.devices || []).map((_deviceId: string, index: number) => ({
    x: 100 + (index * ((canvas.width - 200) / (deviceCount - 1 || 1))),
    y: 250,
    icon: '📡',
    label: '设备' + (index + 1),
    color: '#06b6d4'
  }))

  let time = 0

  function animate() {
    if (!ctx) return
    ctx.clearRect(0, 0, canvas.width, canvas.height)
    time += 0.02

    // 绘制连接线
    deviceNodes.forEach((dn: any) => {
      serviceNodes.forEach(sn => {
        ctx.beginPath()
        ctx.moveTo(dn.x, dn.y)
        ctx.lineTo(sn.x, sn.y)
        ctx.strokeStyle = '#3b82f6'
        ctx.lineWidth = 1
        ctx.globalAlpha = 0.4
        ctx.stroke()

        const t = (time % 2) / 2
        ctx.beginPath()
        ctx.arc(dn.x + (sn.x - dn.x) * t, dn.y + (sn.y - dn.y) * t, 3, 0, Math.PI * 2)
        ctx.fillStyle = '#3b82f6'
        ctx.globalAlpha = 0.8
        ctx.fill()
      })
    })

    serviceNodes.forEach(sn => {
      ctx.beginPath()
      ctx.moveTo(sn.x, sn.y)
      ctx.lineTo(appNode.x, appNode.y)
      ctx.strokeStyle = '#8b5cf6'
      ctx.lineWidth = 2
      ctx.globalAlpha = 0.6
      ctx.stroke()
    })

    ctx.globalAlpha = 1

    // 绘制节点
    const allNodes = [appNode, ...serviceNodes, ...deviceNodes]
    allNodes.forEach((node: any) => {
      const size = node === appNode ? 25 : 20
      ctx.beginPath()
      ctx.arc(node.x, node.y, size, 0, Math.PI * 2)
      ctx.fillStyle = node.color + '30'
      ctx.strokeStyle = node.color
      ctx.lineWidth = 2
      ctx.fill()
      ctx.stroke()

      ctx.font = (node === appNode ? '20px' : '16px') + ' serif'
      ctx.textAlign = 'center'
      ctx.textBaseline = 'middle'
      ctx.fillStyle = '#fff'
      ctx.fillText(node.icon, node.x, node.y)

      ctx.font = '11px sans-serif'
      ctx.fillStyle = node.color
      ctx.fillText(node.label, node.x, node.y + size + 14)
    })

    requestAnimationFrame(animate)
  }

  animate()
}

// 从localStorage加载自定义应用
function loadCustomApps() {
  try {
    const saved = localStorage.getItem('custom-apps')
    if (saved) {
      const apps = JSON.parse(saved)
      apps.forEach((app: any) => {
        // 避免重复加载
        if (!createdApps.find(a => a.id === app.id)) {
          createdApps.push(app)
        }
      })
    }
  } catch { /* ignore */ }
}

onMounted(() => {
  drawFlowChart()
  loadCustomApps()
})

onUnmounted(() => {
  if (flowAnimationId) {
    cancelAnimationFrame(flowAnimationId)
  }
  if (appFlowAnimationId) {
    cancelAnimationFrame(appFlowAnimationId)
  }
})
</script>

<style scoped>
.dashboard-root { min-height: 100vh; background: var(--bg-primary); position: relative; }
.dashboard-main { position: relative; z-index: 1; padding: var(--spacing-xl); max-width: 1920px; margin: 0 auto; }

.section-header { display: flex; align-items: center; justify-content: space-between; margin-bottom: var(--spacing-lg); }
.section-title { display: flex; align-items: center; gap: var(--spacing-sm); font-size: 20px; font-weight: 600; color: var(--text-primary); margin: 0; }
.section-title svg { color: var(--accent-primary); }
.header-actions { display: flex; gap: var(--spacing-sm); }

/* 软件定义架构 */
.architecture-section { margin-bottom: var(--spacing-xl); }
.architecture-layers { display: flex; flex-direction: column; gap: var(--spacing-md); }
.layer { background: var(--bg-card); border: 1px solid var(--border-primary); border-radius: var(--radius-lg); padding: var(--spacing-md); }
.layer-header { display: flex; align-items: center; gap: var(--spacing-sm); margin-bottom: var(--spacing-sm); }
.layer-icon { font-size: 20px; }
.layer-title { font-size: 14px; font-weight: 600; color: var(--text-primary); }
.layer-desc { font-size: 12px; color: var(--text-secondary); margin-left: auto; }
.layer-nodes { display: flex; gap: var(--spacing-md); overflow-x: auto; padding: var(--spacing-xs) 0; }
.arch-node { display: flex; flex-direction: column; align-items: center; gap: 4px; padding: var(--spacing-sm) var(--spacing-md); background: rgba(0,0,0,0.2); border: 1px solid var(--border-primary); border-radius: var(--radius-md); min-width: 80px; transition: all var(--duration-fast); }
.arch-node:hover { border-color: var(--accent-primary); background: rgba(59,130,246,0.1); }
.arch-node.active { border-color: var(--accent-primary); }
.node-icon { font-size: 24px; }
.node-name { font-size: 11px; color: var(--text-secondary); text-align: center; white-space: nowrap; }
.node-value { font-size: 12px; font-weight: 600; color: var(--text-primary); font-family: var(--font-mono); }

/* 数据流向图 */
.flow-section { margin-bottom: var(--spacing-xl); }
.flow-canvas { height: 400px; background: rgba(0,0,0,0.2); border-radius: var(--radius-md); overflow: hidden; }
.flow-canvas canvas { width: 100%; height: 100%; }

.overview-section { margin-bottom: var(--spacing-xl); }
.cards-grid { display: grid; grid-template-columns: repeat(4, 1fr); gap: var(--spacing-lg); }

.devices-stream-section { display: grid; grid-template-columns: 1.5fr 1fr; gap: var(--spacing-xl); }
.device-cards-grid { display: grid; grid-template-columns: repeat(2, 1fr); gap: var(--spacing-md); }

/* 资源池面板 */
.resource-pool-overlay { position: fixed; top: 0; right: 0; width: 450px; height: 100vh; background: rgba(0,0,0,0.5); backdrop-filter: blur(4px); z-index: var(--z-overlay); }
.resource-pool-panel { height: 100%; background: var(--bg-card); border-left: 1px solid var(--border-primary); display: flex; flex-direction: column; }
.panel-header { display: flex; align-items: center; justify-content: space-between; padding: var(--spacing-lg); border-bottom: 1px solid var(--border-primary); }
.panel-icon { width: 40px; height: 40px; display: flex; align-items: center; justify-content: center; background: rgba(59,130,246,0.2); border-radius: var(--radius-md); color: var(--accent-primary); }
.panel-title { font-size: 16px; font-weight: 600; color: var(--text-primary); margin: 0; }
.panel-subtitle { font-size: 12px; color: var(--text-secondary); margin: 2px 0 0; }
.panel-close { width: 32px; height: 32px; display: flex; align-items: center; justify-content: center; background: transparent; border: none; color: var(--text-secondary); font-size: 20px; cursor: pointer; }

.pool-content { flex: 1; overflow-y: auto; padding: var(--spacing-md); }
.pool-section { margin-bottom: var(--spacing-lg); }
.section-title { font-size: 13px; font-weight: 600; color: var(--text-secondary); text-transform: uppercase; letter-spacing: 0.5px; margin-bottom: var(--spacing-sm); }
.pool-devices { display: flex; flex-direction: column; gap: var(--spacing-xs); max-height: 250px; overflow-y: auto; }
.pool-device { display: flex; align-items: center; gap: var(--spacing-sm); padding: var(--spacing-sm); background: rgba(0,0,0,0.2); border: 1px solid var(--border-primary); border-radius: var(--radius-sm); cursor: pointer; transition: all var(--duration-fast); }
.pool-device:hover { border-color: var(--accent-primary); background: rgba(59,130,246,0.1); }
.pool-device.selected { border-color: var(--accent-primary); background: rgba(59,130,246,0.2); }
.device-icon { width: 32px; height: 32px; display: flex; align-items: center; justify-content: center; border-radius: var(--radius-sm); font-size: 16px; }
.device-info { flex: 1; }
.device-name { font-size: 12px; font-weight: 500; color: var(--text-primary); }
.device-type { font-size: 10px; color: var(--text-tertiary); }
.device-check { width: 20px; height: 20px; display: flex; align-items: center; justify-content: center; background: var(--accent-primary); border-radius: 50%; color: white; font-size: 12px; }

.create-app-section { margin-bottom: var(--spacing-lg); padding: var(--spacing-md); background: rgba(0,0,0,0.2); border-radius: var(--radius-md); }
.app-form { display: flex; flex-direction: column; gap: var(--spacing-sm); }
.form-group { display: flex; flex-direction: column; gap: 4px; }
.form-group label { font-size: 12px; color: var(--text-secondary); }
.form-group input, .form-group textarea, .form-group select { padding: 8px 10px; background: var(--bg-primary); border: 1px solid var(--border-primary); border-radius: var(--radius-sm); color: var(--text-primary); font-size: 13px; outline: none; }
.form-group input:focus, .form-group textarea:focus, .form-group select:focus { border-color: var(--accent-primary); }
.selected-count { font-size: 12px; color: var(--text-secondary); text-align: center; }

.created-apps-section { margin-bottom: var(--spacing-lg); }
.created-apps { display: flex; flex-direction: column; gap: var(--spacing-sm); }
.created-app { display: flex; align-items: center; gap: var(--spacing-md); padding: var(--spacing-md); background: rgba(0,0,0,0.2); border: 1px solid var(--border-primary); border-radius: var(--radius-md); cursor: pointer; transition: all var(--duration-fast); }
.created-app:hover { border-color: var(--accent-primary); background: rgba(59,130,246,0.1); }
.app-icon { font-size: 28px; }
.app-info { flex: 1; }
.app-name { font-size: 14px; font-weight: 600; color: var(--text-primary); }
.app-desc { font-size: 11px; color: var(--text-secondary); margin-top: 2px; }
.app-meta { display: flex; gap: var(--spacing-sm); margin-top: 4px; }
.app-type { font-size: 10px; padding: 2px 6px; background: rgba(59,130,246,0.2); border-radius: var(--radius-full); color: var(--accent-primary); }
.app-devices { font-size: 10px; color: var(--text-tertiary); }
.app-actions { display: flex; gap: var(--spacing-xs); }

/* 应用详情模态框 */
.modal-overlay { position: fixed; inset: 0; background: rgba(0,0,0,0.7); backdrop-filter: blur(8px); display: flex; align-items: center; justify-content: center; z-index: var(--z-modal); }
.modal-content { background: var(--bg-card); border: 1px solid var(--border-primary); border-radius: var(--radius-xl); width: 90%; max-width: 1200px; max-height: 90vh; overflow: hidden; }
.modal-large { width: 95%; max-width: 1400px; }
.modal-header { display: flex; align-items: center; justify-content: space-between; padding: var(--spacing-lg); border-bottom: 1px solid var(--border-primary); }
.modal-header h3 { font-size: 18px; font-weight: 600; color: var(--text-primary); margin: 0; }
.app-modal-icon { font-size: 32px; }
.app-modal-desc { font-size: 13px; color: var(--text-secondary); margin: 4px 0 0; }
.modal-close { width: 36px; height: 36px; display: flex; align-items: center; justify-content: center; background: transparent; border: none; border-radius: var(--radius-md); color: var(--text-secondary); cursor: pointer; transition: all var(--duration-fast); }
.modal-close:hover { background: rgba(255,255,255,0.1); color: var(--text-primary); }
.modal-body { padding: var(--spacing-lg); max-height: 70vh; overflow-y: auto; }

.app-detail-content { display: flex; flex-direction: column; gap: var(--spacing-xl); }
.app-flow-section { }
.app-flow-canvas { height: 250px; background: rgba(0,0,0,0.2); border-radius: var(--radius-md); overflow: hidden; }
.app-flow-canvas canvas { width: 100%; height: 100%; }

.app-devices-section { }
.app-devices-list { display: flex; flex-wrap: wrap; gap: var(--spacing-sm); }
.app-device-item { display: flex; align-items: center; gap: var(--spacing-sm); padding: var(--spacing-sm) var(--spacing-md); background: rgba(0,0,0,0.2); border: 1px solid var(--border-primary); border-radius: var(--radius-sm); }
.app-device-item .device-icon { font-size: 16px; }
.app-device-item .device-name { font-size: 12px; color: var(--text-primary); }
.app-device-item .device-status { font-size: 10px; padding: 2px 6px; border-radius: var(--radius-full); }
.app-device-item .device-status.online { background: rgba(34,197,94,0.2); color: #22c55d; }

.app-data-section { }
.app-data-grid { display: grid; grid-template-columns: repeat(4, 1fr); gap: var(--spacing-md); }
.app-data-card { background: rgba(0,0,0,0.2); border: 1px solid var(--border-primary); border-radius: var(--radius-md); padding: var(--spacing-md); text-align: center; }
.data-label { font-size: 12px; color: var(--text-secondary); margin-bottom: 4px; }
.data-value { font-size: 24px; font-weight: 700; color: var(--text-primary); font-family: var(--font-mono); }
.data-unit { font-size: 12px; color: var(--text-secondary); margin-left: 4px; }
.data-trend { font-size: 11px; margin-top: 4px; }
.data-trend.up { color: #ef4444; }
.data-trend.down { color: #3b82f6; }
.data-trend.stable { color: #22c55d; }

@media (max-width: 1200px) { .cards-grid { grid-template-columns: repeat(2, 1fr); } .devices-stream-section { grid-template-columns: 1fr; } .app-data-grid { grid-template-columns: repeat(2, 1fr); } }
@media (max-width: 768px) { .dashboard-main { padding: var(--spacing-md); } .cards-grid { grid-template-columns: 1fr; } .resource-pool-overlay { width: 100%; } .layer-nodes { flex-wrap: wrap; } }
</style>
