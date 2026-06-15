<template>
  <div class="env-page">
    <!-- 左侧导航栏 -->
    <div class="sidebar">
      <div class="sidebar-logo">
        <h2>消防系统平台</h2>
        <p>软件定义消防解决方案</p>
      </div>
      <div class="sidebar-menu">
        <div class="menu-title">核心应用</div>
        <router-link to="/" class="menu-item">
          <i class="fas fa-tachometer-alt"></i>
          <span>主控台</span>
        </router-link>
        <router-link to="/security" class="menu-item">
          <i class="fas fa-shield-alt"></i>
          <span>安防系统</span>
        </router-link>
        <router-link to="/environment" class="menu-item active">
          <i class="fas fa-leaf"></i>
          <span>环境监测系统</span>
        </router-link>
        <router-link to="/fire" class="menu-item">
          <i class="fas fa-fire"></i>
          <span>火灾预警系统</span>
        </router-link>

        <div class="menu-title">在线状态</div>
        <router-link to="/devices" class="menu-item">
          <i class="fas fa-tachometer-alt"></i>
          <span>设备监控</span>
        </router-link>
      </div>
    </div>

    <!-- 主内容区 -->
    <div class="main-content">
      <!-- 顶部导航 -->
      <div class="top-nav">
        <div class="top-left">
          <div class="page-title">环境监测界面</div>
          <div class="page-desc">实时监控环境数据，智能联动处置异常</div>
          <div class="system-time">{{ systemTime }}</div>
        </div>
        <div class="top-right">
          <div class="search-box">
            <i class="fas fa-search"></i>
            <input type="text" placeholder="搜索设备/位置...">
          </div>
          <div class="user-info">
            <div class="user-avatar">
              <i class="fas fa-user"></i>
            </div>
            <div class="user-name">系统管理员 <i class="fas fa-chevron-down"></i></div>
          </div>
        </div>
      </div>

      <!-- 内容容器 -->
      <div class="content-container">
        <!-- 数据概览卡片 -->
        <div class="overview-cards">
          <div class="overview-card">
            <div class="card-info">
              <div class="card-label">当前温度</div>
              <div class="card-value">{{ statusData.temperature?.value ?? '--' }}℃</div>
              <div class="card-trend" :class="trendClass(statusData.temperature?.risk)">
                <i :class="trendIcon(statusData.temperature?.risk)"></i> {{ statusData.temperature?.risk ?? '--' }}
              </div>
            </div>
            <div class="card-icon icon-temp">
              <i class="fas fa-temperature-half"></i>
            </div>
          </div>
          <div class="overview-card">
            <div class="card-info">
              <div class="card-label">当前湿度</div>
              <div class="card-value">{{ statusData.humidity?.value ?? '--' }}%</div>
              <div class="card-trend" :class="trendClass(statusData.humidity?.risk)">
                <i :class="trendIcon(statusData.humidity?.risk)"></i> {{ statusData.humidity?.risk ?? '--' }}
              </div>
            </div>
            <div class="card-icon icon-humi">
              <i class="fas fa-droplet"></i>
            </div>
          </div>
          <div class="overview-card">
            <div class="card-info">
              <div class="card-label">PM2.5浓度</div>
              <div class="card-value">{{ statusData.airQuality?.pm25 ?? '--' }}μg/m³</div>
              <div class="card-trend" :class="trendClass(statusData.airQuality?.risk)">
                <i :class="trendIcon(statusData.airQuality?.risk)"></i> {{ statusData.airQuality?.risk ?? '--' }}
              </div>
            </div>
            <div class="card-icon icon-pm25">
              <i class="fas fa-smog"></i>
            </div>
          </div>
          <div class="overview-card">
            <div class="card-info">
              <div class="card-label">空气质量指数</div>
              <div class="card-value">{{ statusData.gas?.co2?.value ?? '--' }}</div>
              <div class="card-trend trend-normal">
                <i class="fas fa-check-circle"></i> CO₂参考值
              </div>
            </div>
            <div class="card-icon icon-air">
              <i class="fas fa-wind"></i>
            </div>
          </div>
        </div>

        <!-- 阈值设置区域 -->
        <div class="threshold-section">
          <div class="section-header">
            <div class="section-title">
              <i class="fas fa-sliders-h"></i>
              监测数据阈值设置
            </div>
          </div>
          <div class="threshold-grid">
            <div class="threshold-item">
              <div class="threshold-label">
                <span>PM2.5阈值 (μg/m³)</span>
                <span class="current-threshold">{{ thresholds.pm25.min }} ~ {{ thresholds.pm25.max }}</span>
              </div>
              <div class="threshold-range">
                <input type="number" class="threshold-input" v-model.number="thresholds.pm25.min" min="0" max="1000" placeholder="最小值">
                <span class="threshold-separator">~</span>
                <input type="number" class="threshold-input" v-model.number="thresholds.pm25.max" min="0" max="1000" placeholder="最大值">
              </div>
            </div>
            <div class="threshold-item">
              <div class="threshold-label">
                <span>PM10阈值 (μg/m³)</span>
                <span class="current-threshold">{{ thresholds.pm10.min }} ~ {{ thresholds.pm10.max }}</span>
              </div>
              <div class="threshold-range">
                <input type="number" class="threshold-input" v-model.number="thresholds.pm10.min" min="0" max="1000" placeholder="最小值">
                <span class="threshold-separator">~</span>
                <input type="number" class="threshold-input" v-model.number="thresholds.pm10.max" min="0" max="1000" placeholder="最大值">
              </div>
            </div>
            <div class="threshold-item">
              <div class="threshold-label">
                <span>CO₂浓度阈值 (ppm)</span>
                <span class="current-threshold">{{ thresholds.co2.min }} ~ {{ thresholds.co2.max }}</span>
              </div>
              <div class="threshold-range">
                <input type="number" class="threshold-input" v-model.number="thresholds.co2.min" min="0" max="5000" placeholder="最小值">
                <span class="threshold-separator">~</span>
                <input type="number" class="threshold-input" v-model.number="thresholds.co2.max" min="0" max="5000" placeholder="最大值">
              </div>
            </div>
            <div class="threshold-item">
              <div class="threshold-label">
                <span>甲醛浓度阈值 (mg/m³)</span>
                <span class="current-threshold">{{ thresholds.hcho.min }} ~ {{ thresholds.hcho.max }}</span>
              </div>
              <div class="threshold-range">
                <input type="number" step="0.01" class="threshold-input" v-model.number="thresholds.hcho.min" min="0" max="1" placeholder="最小值">
                <span class="threshold-separator">~</span>
                <input type="number" step="0.01" class="threshold-input" v-model.number="thresholds.hcho.max" min="0" max="1" placeholder="最大值">
              </div>
            </div>
            <div class="threshold-item">
              <div class="threshold-label">
                <span>TVOC浓度阈值 (mg/m³)</span>
                <span class="current-threshold">{{ thresholds.tvoc.min }} ~ {{ thresholds.tvoc.max }}</span>
              </div>
              <div class="threshold-range">
                <input type="number" step="0.01" class="threshold-input" v-model.number="thresholds.tvoc.min" min="0" max="5" placeholder="最小值">
                <span class="threshold-separator">~</span>
                <input type="number" step="0.01" class="threshold-input" v-model.number="thresholds.tvoc.max" min="0" max="5" placeholder="最大值">
              </div>
            </div>
            <div class="threshold-item">
              <div class="threshold-label">
                <span>光照强度阈值 (lux)</span>
                <span class="current-threshold">{{ thresholds.light.min }} ~ {{ thresholds.light.max }}</span>
              </div>
              <div class="threshold-range">
                <input type="number" class="threshold-input" v-model.number="thresholds.light.min" min="0" max="10000" placeholder="最小值">
                <span class="threshold-separator">~</span>
                <input type="number" class="threshold-input" v-model.number="thresholds.light.max" min="0" max="10000" placeholder="最大值">
              </div>
            </div>
            <div class="threshold-item">
              <div class="threshold-label">
                <span>空气质量阈值 (AQI)</span>
                <span class="current-threshold">{{ thresholds.air.min }} ~ {{ thresholds.air.max }}</span>
              </div>
              <div class="threshold-range">
                <input type="number" class="threshold-input" v-model.number="thresholds.air.min" min="0" max="500" placeholder="最小值">
                <span class="threshold-separator">~</span>
                <input type="number" class="threshold-input" v-model.number="thresholds.air.max" min="0" max="500" placeholder="最大值">
              </div>
            </div>
            <div class="threshold-item">
              <div class="threshold-label">
                <span>温度阈值 (℃)</span>
                <span class="current-threshold">{{ thresholds.temp.min }} ~ {{ thresholds.temp.max }}</span>
              </div>
              <div class="threshold-range">
                <input type="number" class="threshold-input" v-model.number="thresholds.temp.min" min="-20" max="60" placeholder="最小值">
                <span class="threshold-separator">~</span>
                <input type="number" class="threshold-input" v-model.number="thresholds.temp.max" min="-20" max="60" placeholder="最大值">
              </div>
            </div>
            <div class="threshold-item">
              <div class="threshold-label">
                <span>湿度阈值 (%)</span>
                <span class="current-threshold">{{ thresholds.humi.min }} ~ {{ thresholds.humi.max }}</span>
              </div>
              <div class="threshold-range">
                <input type="number" class="threshold-input" v-model.number="thresholds.humi.min" min="0" max="100" placeholder="最小值">
                <span class="threshold-separator">~</span>
                <input type="number" class="threshold-input" v-model.number="thresholds.humi.max" min="0" max="100" placeholder="最大值">
              </div>
            </div>
          </div>
        </div>

        <!-- 图表区域 -->
        <div class="charts-section">
          <div class="section-header">
            <div class="section-title">
              <i class="fas fa-chart-line"></i>
              实时监测数据
            </div>
            <div class="section-actions">
              <button class="btn btn-primary">
                <i class="fas fa-plus"></i> 添加设备
              </button>
              <button class="btn btn-default">
                <i class="fas fa-download"></i> 导出数据
              </button>
            </div>
          </div>
          <div class="charts-grid">
            <div class="single-chart-wrapper">
              <canvas ref="tempChartRef"></canvas>
            </div>
            <div class="single-chart-wrapper">
              <canvas ref="humiChartRef"></canvas>
            </div>
          </div>
        </div>

        <!-- 环境数据面板 -->
        <div class="data-panel">
          <div class="data-card">
            <div class="data-card-title">
              <i class="fas fa-smog"></i>
              <span>PM10浓度</span>
            </div>
            <div class="data-card-value">{{ statusData.airQuality?.pm10 ?? '--' }}</div>
            <div class="data-card-unit">μg/m³</div>
          </div>
          <div class="data-card">
            <div class="data-card-title">
              <i class="fas fa-wind"></i>
              <span>CO₂浓度</span>
            </div>
            <div class="data-card-value">{{ statusData.gas?.co2?.value ?? '--' }}</div>
            <div class="data-card-unit">ppm</div>
          </div>
          <div class="data-card">
            <div class="data-card-title">
              <i class="fas fa-vial"></i>
              <span>甲醛浓度</span>
            </div>
            <div class="data-card-value">{{ statusData.gas?.ch2o?.value ?? '--' }}</div>
            <div class="data-card-unit">{{ statusData.gas?.ch2o?.unit ?? 'ppb' }}</div>
          </div>
          <div class="data-card">
            <div class="data-card-title">
              <i class="fas fa-vial-circle-check"></i>
              <span>TVOC浓度</span>
            </div>
            <div class="data-card-value">{{ statusData.gas?.tvoc?.value ?? '--' }}</div>
            <div class="data-card-unit">{{ statusData.gas?.tvoc?.unit ?? 'ppb' }}</div>
          </div>
          <div class="data-card">
            <div class="data-card-title">
              <i class="fas fa-sun"></i>
              <span>光照强度</span>
            </div>
            <div class="data-card-value">{{ statusData.gas?.o3?.value ?? '--' }}</div>
            <div class="data-card-unit">{{ statusData.gas?.o3?.unit ?? 'ppb' }}</div>
          </div>
          <div class="data-card">
            <div class="data-card-title">
              <i class="fas fa-cloud"></i>
              <span>空气质量</span>
            </div>
            <div class="data-card-value">{{ statusData.airQuality?.pm25 ?? '--' }}</div>
            <div class="data-card-unit">AQI</div>
          </div>
        </div>

        <!-- 在线设备列表 -->
        <div class="devices-section">
          <div class="section-header">
            <div class="devices-collapse-header" @click="toggleDevices">
              <div class="section-title">
                <i class="fas fa-server"></i>
                <span>在线设备管理</span>
              </div>
              <i class="fas fa-chevron-down collapse-icon" :class="{ rotated: devicesCollapsed }"></i>
            </div>
            <div class="section-actions">
              <button class="btn btn-default">
                <i class="fas fa-sync-alt"></i> 刷新列表
              </button>
              <button class="btn btn-default">
                <i class="fas fa-filter"></i> 筛选
              </button>
            </div>
          </div>
          <div class="devices-table-container" :class="{ collapsed: devicesCollapsed }">
            <table class="devices-table">
              <thead>
                <tr>
                  <th>设备编号</th>
                  <th>设备名称</th>
                  <th>设备类型</th>
                  <th>安装位置</th>
                  <th>最后上报时间</th>
                  <th>状态</th>
                  <th>操作</th>
                </tr>
              </thead>
              <tbody>
                <tr v-for="device in devices" :key="device.id">
                  <td>{{ device.id }}</td>
                  <td>{{ device.name }}</td>
                  <td>{{ device.type }}</td>
                  <td>{{ device.location }}</td>
                  <td>{{ device.time }}</td>
                  <td>
                    <span class="device-status" :class="device.status === 'online' ? 'status-online' : 'status-offline'">
                      {{ device.status === 'online' ? '在线' : '离线' }}
                    </span>
                  </td>
                  <td class="device-action">
                    <button class="action-btn action-detail" @click="showDeviceDetail(device)">详情</button>
                    <button class="action-btn action-refresh" @click="refreshDevice(device)">刷新</button>
                    <button class="action-btn action-test" @click="testDevice(device)">测试</button>
                  </td>
                </tr>
              </tbody>
            </table>
          </div>
        </div>

        <!-- 报警日志区域 -->
        <div class="alarm-log-section">
          <div class="section-header">
            <div class="section-title">
              <i class="fas fa-history"></i>
              报警日志记录
            </div>
            <div class="section-actions">
              <button class="btn btn-default" @click="clearAlarmLogs">
                <i class="fas fa-trash"></i> 清空日志
              </button>
              <button class="btn btn-default">
                <i class="fas fa-download"></i> 导出日志
              </button>
            </div>
          </div>
          <table class="alarm-log-table">
            <thead>
              <tr>
                <th>时间</th>
                <th>日志类型</th>
                <th>内容描述</th>
                <th>关联数值/设备</th>
                <th>级别/状态</th>
                <th>处理状态</th>
              </tr>
            </thead>
            <tbody>
              <tr v-for="(log, index) in alarmLogs" :key="index">
                <td>{{ log.time }}</td>
                <td>
                  <span class="log-type" :class="log.type === 'alarm' ? 'log-alarm' : 'log-linkage'">
                    {{ log.type === 'alarm' ? '报警' : '联动' }}
                  </span>
                </td>
                <td>{{ log.desc }}</td>
                <td>{{ log.value }}</td>
                <td>
                  <span class="alarm-level" :class="'level-' + log.level">
                    {{ log.level === 'high' ? '高' : log.level === 'medium' ? '中' : '低' }}
                  </span>
                </td>
                <td>{{ log.handle }}</td>
              </tr>
              <tr v-if="alarmLogs.length === 0">
                <td colspan="6" style="text-align:center;color:#999;padding:20px;">暂无报警日志</td>
              </tr>
            </tbody>
          </table>
        </div>

        <!-- 智能联动处置 -->
        <div class="control-section">
          <div class="section-header">
            <div class="section-title">
              <i class="fas fa-cogs"></i>
              智能联动处置（原子服务自动执行）
            </div>
          </div>

          <!-- 手动/自动模式切换 -->
          <div class="control-mode">
            <span class="mode-label">联动模式：</span>
            <div class="mode-switch">
              <div class="mode-option" :class="{ active: controlMode === 'auto' }" @click="controlMode = 'auto'">自动</div>
              <div class="mode-option" :class="{ active: controlMode === 'manual' }" @click="controlMode = 'manual'">手动</div>
            </div>
          </div>

          <div class="control-grid">
            <div class="control-item">
              <div class="control-name">
                <span>空调控制</span>
                <span class="status" :class="statusData.services?.tempControlActive ? 'status-on' : 'status-off'">
                  {{ statusData.services?.tempControlActive ? '开启' : '关闭' }}
                </span>
              </div>
              <div class="control-desc">温度过高自动开启调温</div>
              <div class="control-manual-btn">
                <button class="manual-btn manual-on" @click="sendControl('simulate', 'temp')">模拟异常</button>
                <button class="manual-btn manual-off" @click="sendControl('reset', 'temp')">重置</button>
              </div>
            </div>
            <div class="control-item">
              <div class="control-name">
                <span>恒湿净化一体机</span>
                <span class="status" :class="statusData.gas?.ventilationActive ? 'status-on' : 'status-off'">
                  {{ statusData.gas?.ventilationActive ? '开启' : '关闭' }}
                </span>
              </div>
              <div class="control-desc">湿度/空气质量异常自动启动</div>
              <div class="control-manual-btn">
                <button class="manual-btn manual-on" @click="sendControl('simulate', 'air')">模拟异常</button>
                <button class="manual-btn manual-off" @click="sendControl('reset', 'air')">重置</button>
              </div>
            </div>
            <div class="control-item">
              <div class="control-name">
                <span>空气质量云测仪</span>
                <span class="status" :class="statusData.airQuality?.airQualityActive ? 'status-on' : 'status-off'">
                  {{ statusData.airQuality?.airQualityActive ? '开启' : '关闭' }}
                </span>
              </div>
              <div class="control-desc">实时监测空气质量数据</div>
              <div class="control-manual-btn">
                <button class="manual-btn manual-on" @click="sendControl('simulate', 'air')">模拟异常</button>
                <button class="manual-btn manual-off" @click="sendControl('reset', 'air')">重置</button>
              </div>
            </div>
            <div class="control-item">
              <div class="control-name">
                <span>弱光监测传感器</span>
                <span class="status" :class="statusData.gas?.gasResponseActive ? 'status-on' : 'status-off'">
                  {{ statusData.gas?.gasResponseActive ? '开启' : '关闭' }}
                </span>
              </div>
              <div class="control-desc">光照不足自动监测预警</div>
              <div class="control-manual-btn">
                <button class="manual-btn manual-on" @click="sendControl('simulate', 'gas')">模拟异常</button>
                <button class="manual-btn manual-off" @click="sendControl('reset', 'gas')">重置</button>
              </div>
            </div>
          </div>
        </div>
      </div>
    </div>
  </div>
</template>

<script setup lang="ts">
import { ref, reactive, computed, onMounted, onBeforeUnmount } from 'vue'
import { ElMessage } from 'element-plus'
import { Chart, registerables } from 'chart.js'

Chart.register(...registerables)

// ---- Types ----
interface GasSensor {
  value: number
  unit: string
  online: boolean
}

interface StatusData {
  system?: { overallRisk: string; systemNormal: boolean; running: boolean }
  temperature?: { value: number; unit: string; risk: string; online: boolean }
  humidity?: { value: number; unit: string; risk: string; online: boolean }
  airQuality?: {
    pm25: number; pm10: number; risk: string
    pm25Online: boolean; pm10Online: boolean; airQualityActive: boolean
  }
  gas?: {
    tvoc: GasSensor; ch2o: GasSensor; o3: GasSensor; co2: GasSensor
    risk: string; gasResponseActive: boolean; ventilationActive: boolean
  }
  services?: { tempControlActive: boolean; centerAlarmActive: boolean }
}

interface AlarmLog {
  time: string
  type: string
  desc: string
  value: string
  level: string
  handle: string
}

// ---- State ----
const systemTime = ref('')
const statusData = ref<StatusData>({})
const alarmLogs = ref<AlarmLog[]>([])
const controlMode = ref<'auto' | 'manual'>('auto')
const devicesCollapsed = ref(false)

// Thresholds
const thresholds = reactive({
  pm25: { min: 0, max: 75 },
  pm10: { min: 0, max: 150 },
  co2: { min: 400, max: 1000 },
  hcho: { min: 0, max: 0.1 },
  tvoc: { min: 0, max: 0.6 },
  light: { min: 0, max: 500 },
  air: { min: 0, max: 100 },
  temp: { min: 10, max: 35 },
  humi: { min: 30, max: 70 },
})

// 设备列表 - 从 API 状态推导在线情况, 编号使用 dev-Modbus地址
const devices = computed(() => {
  const d = statusData.value
  const now = new Date().toLocaleString('zh-CN', { hour12: false }).replace(/\//g, '-')
  return [
    { id: 'dev-0x30', name: '温度传感器', type: '环境监测设备', location: '云测仪(0x30)', time: d.temperature?.online ? now : '--', status: d.temperature?.online !== false ? 'online' : 'offline', addr: '0x30' },
    { id: 'dev-0x30', name: '湿度传感器', type: '环境监测设备', location: '云测仪(0x30)', time: d.humidity?.online ? now : '--', status: d.humidity?.online !== false ? 'online' : 'offline', addr: '0x30' },
    { id: 'dev-0x30', name: 'PM2.5传感器', type: '空气质量设备', location: '云测仪(0x30)', time: d.airQuality?.pm25Online ? now : '--', status: d.airQuality?.pm25Online !== false ? 'online' : 'offline', addr: '0x30' },
    { id: 'dev-0x30', name: 'PM10传感器', type: '空气质量设备', location: '云测仪(0x30)', time: d.airQuality?.pm10Online ? now : '--', status: d.airQuality?.pm10Online !== false ? 'online' : 'offline', addr: '0x30' },
    { id: 'dev-0x30', name: 'TVOC传感器', type: '有害气体设备', location: '云测仪(0x30)', time: d.gas?.tvoc?.online ? now : '--', status: d.gas?.tvoc?.online !== false ? 'online' : 'offline', addr: '0x30' },
    { id: 'dev-0x30', name: '甲醛传感器', type: '有害气体设备', location: '云测仪(0x30)', time: d.gas?.ch2o?.online ? now : '--', status: d.gas?.ch2o?.online !== false ? 'online' : 'offline', addr: '0x30' },
    { id: 'dev-0x30', name: '臭氧传感器', type: '有害气体设备', location: '云测仪(0x30)', time: d.gas?.o3?.online ? now : '--', status: d.gas?.o3?.online !== false ? 'online' : 'offline', addr: '0x30' },
    { id: 'dev-0x30', name: 'CO₂传感器', type: '有害气体设备', location: '云测仪(0x30)', time: d.gas?.co2?.online ? now : '--', status: d.gas?.co2?.online !== false ? 'online' : 'offline', addr: '0x30' },
    { id: 'dev-0x60', name: '空调', type: '温控调节设备', location: '控制设备(0x60)', time: '--', status: 'online', addr: '0x60' },
    { id: 'dev-0x20', name: '恒湿净化一体机', type: '净化调节设备', location: '控制设备(0x20)', time: '--', status: 'online', addr: '0x20' },
  ]
})

// 设备操作: 刷新状态
function refreshDevice(_device: any) {
  fetchStatus()
  ElMessage.success('设备状态已刷新')
}

// 设备操作: 查看详情
function showDeviceDetail(device: any) {
  const info = [
    `设备编号: ${device.id}`,
    `设备名称: ${device.name}`,
    `设备类型: ${device.type}`,
    `安装位置: ${device.location}`,
    `Modbus地址: ${device.addr}`,
    `当前状态: ${device.status === 'online' ? '在线' : '离线'}`,
    `最后通信: ${device.time}`,
  ].join('\n')
  alert(info)
}

// 设备操作: 测试连接
function testDevice(device: any) {
  if (device.status === 'online') {
    ElMessage.success(`${device.name} 连接正常 (地址: ${device.addr})`)
  } else {
    ElMessage.warning(`${device.name} 设备离线 (地址: ${device.addr})`)
  }
}

// Chart refs
const tempChartRef = ref<HTMLCanvasElement | null>(null)
const humiChartRef = ref<HTMLCanvasElement | null>(null)
let tempChart: Chart | null = null
let humiChart: Chart | null = null

// Polling timers
let statusTimer: ReturnType<typeof setInterval> | null = null
let logsTimer: ReturnType<typeof setInterval> | null = null
let timeTimer: ReturnType<typeof setInterval> | null = null

// Chart data buffers
const tempHistory = ref<number[]>([24.5, 25.1, 26.2, 27.5, 27.1, 26.8])
const humiHistory = ref<number[]>([62.5, 60.1, 58.2, 56.5, 57.1, 55.2])
const chartLabels = ['00:00', '04:00', '08:00', '12:00', '16:00', '20:00']

// ---- Helpers ----
function updateSystemTime() {
  const now = new Date()
  systemTime.value = now.toLocaleString('zh-CN', {
    year: 'numeric', month: '2-digit', day: '2-digit',
    hour: '2-digit', minute: '2-digit', second: '2-digit',
    hour12: false,
  }).replace(/\//g, '-')
}

function trendClass(risk?: string) {
  if (!risk) return 'trend-normal'
  if (risk.includes('高') || risk.includes('严重')) return 'trend-up'
  if (risk.includes('中') || risk.includes('警告')) return 'trend-up'
  return 'trend-down'
}

function trendIcon(risk?: string) {
  if (!risk) return 'fas fa-check-circle'
  if (risk.includes('高') || risk.includes('严重') || risk.includes('中') || risk.includes('警告')) return 'fas fa-arrow-up'
  return 'fas fa-check-circle'
}

function toggleDevices() {
  devicesCollapsed.value = !devicesCollapsed.value
}

function clearAlarmLogs() {
  if (confirm('确定要清空报警日志吗？')) {
    alarmLogs.value = []
  }
}

// ---- API calls ----
async function fetchStatus() {
  try {
    const resp = await fetch('/environment/api/status')
    if (!resp.ok) return
    const data = await resp.json() as StatusData
    statusData.value = data

    // Update chart history
    if (data.temperature?.value != null) {
      tempHistory.value.push(data.temperature.value)
      if (tempHistory.value.length > 12) tempHistory.value.shift()
    }
    if (data.humidity?.value != null) {
      humiHistory.value.push(data.humidity.value)
      if (humiHistory.value.length > 12) humiHistory.value.shift()
    }
    updateCharts()
  } catch {
    // silently ignore fetch errors
  }
}

async function fetchLogs() {
  try {
    const resp = await fetch('/environment/api/logs')
    if (!resp.ok) return
    const data = await resp.json()
    if (Array.isArray(data)) {
      alarmLogs.value = data
    } else if (data.logs && Array.isArray(data.logs)) {
      alarmLogs.value = data.logs
    }
  } catch {
    // silently ignore fetch errors
  }
}

async function sendControl(action: string, target: string) {
  try {
    await fetch(`/environment/api/control?action=${action}&target=${target}`)
    // Refresh status after control action
    setTimeout(fetchStatus, 500)
  } catch {
    // silently ignore
  }
}

// ---- Charts ----
function initCharts() {
  if (tempChartRef.value) {
    tempChart = new Chart(tempChartRef.value, {
      type: 'line',
      data: {
        labels: [...chartLabels],
        datasets: [{
          label: '温度 (℃)',
          data: [...tempHistory.value],
          borderColor: '#F53F3F',
          backgroundColor: 'rgba(245, 63, 63, 0.1)',
          borderWidth: 2,
          tension: 0.4,
          fill: true,
        }],
      },
      options: {
        responsive: true,
        maintainAspectRatio: false,
        plugins: {
          legend: { position: 'top' },
          tooltip: { mode: 'index', intersect: false },
        },
        scales: {
          y: { min: 20, max: 30, ticks: { stepSize: 2 } },
        },
      },
    })
  }

  if (humiChartRef.value) {
    humiChart = new Chart(humiChartRef.value, {
      type: 'line',
      data: {
        labels: [...chartLabels],
        datasets: [{
          label: '湿度 (%)',
          data: [...humiHistory.value],
          borderColor: '#165DFF',
          backgroundColor: 'rgba(22, 93, 255, 0.1)',
          borderWidth: 2,
          tension: 0.4,
          fill: true,
        }],
      },
      options: {
        responsive: true,
        maintainAspectRatio: false,
        plugins: {
          legend: { position: 'top' },
          tooltip: { mode: 'index', intersect: false },
        },
        scales: {
          y: { min: 40, max: 70, ticks: { stepSize: 5 } },
        },
      },
    })
  }
}

function updateCharts() {
  const len = tempHistory.value.length
  const labels: string[] = []
  for (let i = len - 1; i >= 0; i--) {
    const h = 20 - i * 2
    labels.push(`${String(((24 + h) % 24)).padStart(2, '0')}:00`)
  }

  if (tempChart) {
    tempChart.data.labels = labels
    tempChart.data.datasets[0].data = [...tempHistory.value]
    tempChart.update('none')
  }
  if (humiChart) {
    humiChart.data.labels = labels
    humiChart.data.datasets[0].data = [...humiHistory.value]
    humiChart.update('none')
  }
}

// ---- Lifecycle ----
onMounted(() => {
  updateSystemTime()
  timeTimer = setInterval(updateSystemTime, 1000)

  initCharts()

  fetchStatus()
  fetchLogs()

  statusTimer = setInterval(fetchStatus, 2000)
  logsTimer = setInterval(fetchLogs, 5000)
})

onBeforeUnmount(() => {
  if (statusTimer) clearInterval(statusTimer)
  if (logsTimer) clearInterval(logsTimer)
  if (timeTimer) clearInterval(timeTimer)
  tempChart?.destroy()
  humiChart?.destroy()
})
</script>

<style scoped>
* {
  margin: 0;
  padding: 0;
  box-sizing: border-box;
  font-family: "Microsoft YaHei", sans-serif;
}

.env-page {
  background-color: #f5f7fa;
  color: #333;
  display: flex;
  height: 100vh;
  overflow: hidden;
}

/* 左侧导航栏 */
.sidebar {
  width: 200px;
  background: #16263D;
  color: #fff;
  height: 100%;
  padding: 20px 0;
  overflow-y: auto;
  flex-shrink: 0;
}

.sidebar-logo {
  padding: 0 20px 20px;
  border-bottom: 1px solid #29384E;
  margin-bottom: 20px;
}

.sidebar-logo h2 {
  font-size: 18px;
  font-weight: 600;
  color: #fff;
}

.sidebar-logo p {
  font-size: 12px;
  color: #99A3B8;
  margin-top: 5px;
}

.sidebar-menu {
  padding: 10px 0;
}

.menu-title {
  font-size: 12px;
  color: #6B7B94;
  padding: 0 20px;
  margin: 15px 0 5px;
  text-transform: uppercase;
}

.menu-item {
  padding: 12px 20px;
  display: flex;
  align-items: center;
  gap: 10px;
  cursor: pointer;
  transition: background 0.3s;
  font-size: 14px;
  color: #fff;
  text-decoration: none;
}

.menu-item.active {
  background: #1E88E5;
  color: #fff;
}

.menu-item:hover:not(.active) {
  background: #29384E;
}

.menu-item i {
  font-size: 16px;
  width: 20px;
  text-align: center;
}

/* 主内容区 */
.main-content {
  flex: 1;
  overflow-y: auto;
  height: 100%;
  padding-bottom: 20px;
}

/* 顶部导航 */
.top-nav {
  background: #fff;
  padding: 15px 25px;
  display: flex;
  justify-content: space-between;
  align-items: center;
  box-shadow: 0 1px 5px rgba(0,0,0,0.05);
  position: sticky;
  top: 0;
  z-index: 100;
}

.top-left {
  display: flex;
  flex-direction: column;
}

.system-time {
  font-size: 12px;
  color: #999;
  margin-top: 3px;
}

.page-title {
  font-size: 20px;
  font-weight: 600;
  color: #16263D;
}

.page-desc {
  font-size: 13px;
  color: #666;
  margin-top: 3px;
}

.top-right {
  display: flex;
  align-items: center;
  gap: 20px;
}

.search-box {
  position: relative;
}

.search-box input {
  padding: 8px 15px 8px 35px;
  border: 1px solid #E5E6EB;
  border-radius: 4px;
  font-size: 13px;
  width: 220px;
  outline: none;
}

.search-box i {
  position: absolute;
  left: 12px;
  top: 50%;
  transform: translateY(-50%);
  color: #999;
  font-size: 14px;
}

.user-info {
  display: flex;
  align-items: center;
  gap: 8px;
  cursor: pointer;
}

.user-avatar {
  width: 32px;
  height: 32px;
  border-radius: 50%;
  background: #1E88E5;
  display: flex;
  align-items: center;
  justify-content: center;
  color: #fff;
  font-size: 14px;
}

.user-name {
  font-size: 14px;
  color: #333;
}

/* 内容容器 */
.content-container {
  padding: 25px;
}

/* 数据概览卡片 */
.overview-cards {
  display: grid;
  grid-template-columns: repeat(auto-fit, minmax(220px, 1fr));
  gap: 15px;
  margin-bottom: 25px;
}

.overview-card {
  background: #fff;
  border-radius: 8px;
  padding: 18px 20px;
  box-shadow: 0 1px 3px rgba(0,0,0,0.08);
  display: flex;
  justify-content: space-between;
  align-items: center;
}

.card-info {
  flex: 1;
}

.card-label {
  font-size: 13px;
  color: #666;
  margin-bottom: 5px;
}

.card-value {
  font-size: 24px;
  font-weight: 600;
  color: #16263D;
}

.card-trend {
  font-size: 12px;
  margin-top: 3px;
  display: flex;
  align-items: center;
  gap: 5px;
}

.trend-up {
  color: #F53F3F;
}

.trend-down {
  color: #00B42A;
}

.trend-normal {
  color: #165DFF;
}

.card-icon {
  width: 48px;
  height: 48px;
  border-radius: 8px;
  display: flex;
  align-items: center;
  justify-content: center;
  font-size: 20px;
}

.icon-temp {
  background: #FDEDED;
  color: #F53F3F;
}

.icon-humi {
  background: #E8F3FF;
  color: #165DFF;
}

.icon-pm25 {
  background: #FEF5E8;
  color: #FF7D00;
}

.icon-air {
  background: #E6F7EF;
  color: #00B42A;
}

/* 阈值设置区域 */
.threshold-section {
  background: #fff;
  border-radius: 8px;
  padding: 20px;
  box-shadow: 0 1px 3px rgba(0,0,0,0.08);
  margin-bottom: 25px;
}

.threshold-grid {
  display: grid;
  grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
  gap: 15px;
  margin-top: 15px;
}

.threshold-item {
  display: flex;
  flex-direction: column;
  gap: 8px;
}

.threshold-range {
  display: flex;
  gap: 10px;
}

.threshold-range input {
  flex: 1;
}

.threshold-label {
  font-size: 13px;
  color: #666;
  display: flex;
  justify-content: space-between;
}

.threshold-input {
  padding: 8px 12px;
  border: 1px solid #E5E6EB;
  border-radius: 4px;
  font-size: 13px;
  outline: none;
  width: 100%;
}

.threshold-separator {
  text-align: center;
  padding: 8px 0;
  color: #999;
}

/* 图表区域 */
.charts-section {
  background: #fff;
  border-radius: 8px;
  padding: 20px;
  box-shadow: 0 1px 3px rgba(0,0,0,0.08);
  margin-bottom: 25px;
}

.section-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  margin-bottom: 15px;
}

.section-title {
  font-size: 16px;
  font-weight: 600;
  color: #16263D;
  display: flex;
  align-items: center;
  gap: 8px;
}

.section-title i {
  color: #1E88E5;
}

.section-actions {
  display: flex;
  gap: 10px;
}

.btn {
  padding: 6px 12px;
  border-radius: 4px;
  font-size: 13px;
  cursor: pointer;
  border: none;
  outline: none;
  display: flex;
  align-items: center;
  gap: 5px;
}

.btn-primary {
  background: #1E88E5;
  color: #fff;
}

.btn-default {
  background: #F2F3F5;
  color: #333;
}

/* 图表网格布局 */
.charts-grid {
  display: grid;
  grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));
  gap: 20px;
}

.single-chart-wrapper {
  position: relative;
  height: 220px;
  width: 100%;
  border: 1px solid #f0f0f0;
  border-radius: 6px;
  padding: 10px;
}

/* 环境数据面板 */
.data-panel {
  display: grid;
  grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
  gap: 15px;
  margin-bottom: 25px;
}

.data-card {
  background: #fff;
  border-radius: 8px;
  padding: 20px;
  box-shadow: 0 1px 3px rgba(0,0,0,0.08);
}

.data-card-title {
  display: flex;
  align-items: center;
  gap: 8px;
  font-size: 14px;
  color: #666;
  margin-bottom: 12px;
}

.data-card-title i {
  font-size: 16px;
  color: #1E88E5;
}

.data-card-value {
  font-size: 30px;
  font-weight: 600;
  color: #16263D;
  margin-bottom: 5px;
}

.data-card-unit {
  font-size: 12px;
  color: #999;
}

.alarm {
  color: #F53F3F !important;
  animation: flash 1s infinite alternate;
}

@keyframes flash {
  from { opacity: 1; }
  to { opacity: 0.6; }
}

/* 联动控制区 */
.control-section {
  background: #fff;
  border-radius: 8px;
  padding: 20px;
  box-shadow: 0 1px 3px rgba(0,0,0,0.08);
  margin-bottom: 25px;
}

.control-mode {
  display: flex;
  align-items: center;
  gap: 10px;
  margin-bottom: 15px;
}

.mode-label {
  font-size: 14px;
  color: #666;
}

.mode-switch {
  display: flex;
  background: #f0f0f0;
  border-radius: 20px;
  padding: 3px;
  width: 100px;
  cursor: pointer;
}

.mode-option {
  flex: 1;
  text-align: center;
  padding: 4px 0;
  border-radius: 15px;
  font-size: 12px;
  font-weight: 600;
  transition: all 0.3s;
}

.mode-option.active {
  background: #1E88E5;
  color: #fff;
}

.control-grid {
  display: grid;
  grid-template-columns: repeat(auto-fit, minmax(240px, 1fr));
  gap: 15px;
}

.control-item {
  border: 1px solid #E5E6EB;
  border-radius: 6px;
  padding: 15px;
}

.control-name {
  display: flex;
  justify-content: space-between;
  align-items: center;
  font-size: 14px;
  margin-bottom: 8px;
}

.control-manual-btn {
  margin-top: 10px;
  display: flex;
  gap: 8px;
}

.manual-btn {
  padding: 4px 10px;
  border-radius: 3px;
  font-size: 12px;
  cursor: pointer;
  border: none;
  outline: none;
}

.manual-on {
  background: #00B42A;
  color: #fff;
}

.manual-off {
  background: #86909C;
  color: #fff;
}

.status {
  padding: 3px 8px;
  border-radius: 12px;
  font-size: 11px;
  font-weight: 600;
  color: #fff;
}

.status-on {
  background: #00B42A;
}

.status-off {
  background: #86909C;
}

.control-desc {
  font-size: 12px;
  color: #666;
}

/* 在线设备列表样式 */
.devices-section {
  background: #fff;
  border-radius: 8px;
  padding: 20px;
  box-shadow: 0 1px 3px rgba(0,0,0,0.08);
  margin-bottom: 25px;
}

.devices-collapse-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  cursor: pointer;
  padding: 8px 0;
}

.collapse-icon {
  transition: transform 0.3s;
}

.collapse-icon.rotated {
  transform: rotate(180deg);
}

.devices-table-container {
  max-height: 400px;
  overflow-y: auto;
  transition: max-height 0.3s ease, opacity 0.3s ease;
}

.devices-table-container.collapsed {
  max-height: 0;
  opacity: 0;
  overflow: hidden;
}

.devices-table {
  width: 100%;
  border-collapse: collapse;
  margin-top: 15px;
}

.devices-table th,
.devices-table td {
  padding: 12px 15px;
  text-align: left;
  border-bottom: 1px solid #f0f0f0;
  font-size: 13px;
}

.devices-table th {
  background-color: #f8f9fa;
  font-weight: 600;
  color: #16263D;
  position: sticky;
  top: 0;
}

.devices-table tr:hover {
  background-color: #f8f9fa;
}

.device-status {
  padding: 3px 8px;
  border-radius: 12px;
  font-size: 11px;
  font-weight: 600;
  display: inline-block;
}

.status-online {
  background: #E6F7EF;
  color: #00B42A;
}

.status-offline {
  background: #FDF2F8;
  color: #F53F3F;
}

.device-action {
  display: flex;
  gap: 8px;
}

.action-btn {
  padding: 4px 8px;
  border-radius: 3px;
  font-size: 11px;
  cursor: pointer;
  border: none;
  outline: none;
}

.action-detail {
  background: #E8F3FF;
  color: #165DFF;
}

.action-test {
  background: #E6F7EF;
  color: #00B42A;
}

.action-refresh {
  background: #F7F3E9;
  color: #FF7D00;
}

/* 报警日志样式 */
.alarm-log-section {
  background: #fff;
  border-radius: 8px;
  padding: 20px;
  box-shadow: 0 1px 3px rgba(0,0,0,0.08);
  margin-bottom: 25px;
}

.alarm-log-table {
  width: 100%;
  border-collapse: collapse;
  margin-top: 15px;
  max-height: 200px;
  overflow-y: auto;
  display: block;
}

.alarm-log-table th,
.alarm-log-table td {
  padding: 10px 15px;
  text-align: left;
  border-bottom: 1px solid #f0f0f0;
  font-size: 13px;
}

.alarm-log-table th {
  background-color: #f8f9fa;
  font-weight: 600;
  color: #16263D;
  position: sticky;
  top: 0;
}

.alarm-level {
  padding: 2px 6px;
  border-radius: 10px;
  font-size: 10px;
  font-weight: 600;
  display: inline-block;
}

.level-high {
  background: #FDEDED;
  color: #F53F3F;
}

.level-medium {
  background: #FEF5E8;
  color: #FF7D00;
}

.level-low {
  background: #E8F3FF;
  color: #165DFF;
}

.log-type {
  padding: 2px 6px;
  border-radius: 10px;
  font-size: 10px;
  font-weight: 600;
  display: inline-block;
}

.log-alarm {
  background: #FDEDED;
  color: #F53F3F;
}

.log-linkage {
  background: #E8F3FF;
  color: #165DFF;
}
</style>
