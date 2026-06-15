<template>
  <div class="fire-page">
    <!-- 顶部导航栏 -->
    <nav class="top-nav-bar">
      <div class="top-nav-inner">
        <div class="top-nav-brand">
          <svg width="20" height="20" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" style="color:#ef4444">
            <path d="M8.5 14.5A2.5 2.5 0 0 0 11 12c0-1.38-.5-2-1-3-1.072-2.143-.224-4.054 2-6 .5 2.5 2 4.9 4 6.5 2 1.6 3 3.5 3 5.5a7 7 0 1 1-14 0c0-1.153.433-2.294 1-3a2.5 2.5 0 0 0 2.5 2.5z"/>
          </svg>
          <span>智慧环控安防系统</span>
        </div>
        <div class="top-nav-links">
          <a @click.prevent="$router.push('/')" href="#">主控台</a>
          <a @click.prevent="$router.push('/security')" href="#">安防系统</a>
          <a @click.prevent="$router.push('/environment')" href="#">环境监测</a>
          <a class="active" @click.prevent="$router.push('/fire')" href="#">消防系统</a>
          <a @click.prevent="$router.push('/devices')" href="#">设备管理</a>
          <a @click.prevent="$router.push('/logs')" href="#">系统日志</a>
          <a class="nav-logout" @click.prevent="handleLogout" href="#">退出</a>
        </div>
      </div>
    </nav>
    <header>
      <h1>智能火灾预警应用分析平台</h1>
      <div style="display: flex; gap: 15px;">
        <button class="btn danger" style="padding: 5px 12px; font-size:12px;" @click="emergencyLinkage">&#x1F6A8; 应急联动处置</button>
      </div>
      <div class="status-bar">
        <div
          class="status-item status-item-clickable"
          @click="openCurrentRegionDeviceModal"
        >
          本区设备在线：<span style="color: var(--accent)">{{ onlineCountText }}</span> &#x1F50D;
        </div>
        <div class="status-item">当前时间：<span>{{ currentTime }}</span></div>
      </div>
    </header>
    <div class="fire-container">
      <div class="panel" style="overflow-y: hidden;">
        <div class="panel-title">区域火灾风险等级评估</div>
        <div class="risk-section" style="margin-bottom: 30px;">
          <div style="margin-bottom: 15px; display: flex; align-items: center; gap: 10px;">
            <span style="font-size: 14px; font-weight: bold;">当前监测区域：</span>
            <select
              v-model="currentRegion"
              style="background: var(--bg-color); color: var(--text-main); border: 1px solid var(--border); padding: 6px 12px; border-radius: 4px; font-size: 14px;"
            >
              <option value="A">区域A</option>
              <option value="B">区域B</option>
              <option value="C">区域C</option>
            </select>
          </div>
          <div class="risk-bar-container">
            <div class="risk-label">
              <span>区域{{ currentRegion }}</span>
              <span :style="{ color: riskColor }">{{ riskText }}</span>
            </div>
            <div class="progress-bg">
              <div class="progress-fill" :style="{ width: riskPercent + '%', background: riskColor }"></div>
            </div>
          </div>
        </div>

        <div style="display: flex; justify-content: space-between; align-items: center; margin-bottom: 15px;">
          <div class="panel-title" style="margin-bottom: 0;">各区域实时监测与趋势分析</div>
        </div>

        <div class="realtime-grid">
          <div class="data-card">
            <div class="label">温度 (℃)</div>
            <div class="value temp-val">{{ temperatureDisplay }}</div>
            <div class="label" style="font-size: 12px">传感器状态: <span :style="{ color: status.devices?.temperature?.online === false ? 'var(--danger)' : 'inherit' }">{{ status.devices?.temperature?.online === false ? '离线' : '在线' }}</span></div>
          </div>
          <div class="data-card">
            <div class="label">湿度 (%RH)</div>
            <div class="value hum-val">{{ humidityDisplay }}</div>
            <div class="label" style="font-size: 12px">传感器状态: 在线</div>
          </div>
          <div class="data-card">
            <div class="label">CO&#x2082; (ppm)</div>
            <div class="value gas-val">{{ co2Display }}</div>
            <div class="label" style="font-size: 12px">传感器状态: 在线</div>
          </div>
          <div class="data-card">
            <div class="label">烟雾浓度</div>
            <div class="value smoke-val">{{ status.smoke?.stateText ?? '--' }}</div>
            <div class="label" style="font-size: 12px">传感器状态: <span :style="{ color: status.smoke?.online === false ? 'var(--danger)' : 'inherit' }">{{ status.smoke?.online === false ? '离线' : (status.smoke?.state !== 0 ? '异常' : '在线') }}</span></div>
          </div>
        </div>

        <div class="panel-title">各项数据趋势分析预判</div>
        <div class="charts-grid">
          <div ref="trendChartTempRef" class="chart-box"></div>
          <div ref="trendChartHumRef" class="chart-box"></div>
          <div ref="trendChartGasRef" class="chart-box"></div>
          <div ref="trendChartSmokeRef" class="chart-box"></div>
        </div>
      </div>

      <div class="right-col" style="display: flex; flex-direction: column; gap: 20px;">
      <!-- Right Panel: Thresholds -->
      <div class="panel">
        <div class="panel-title">预警阈值设置与数据峰值</div>
        <div class="threshold-item">
          <div class="th-header">
            <span>&#x1F321; 温度报警阈值</span>
            <span style="font-size: 12px; color: var(--danger)">今日峰值: {{ tempPeak }}℃</span>
          </div>
          <div class="th-input">
            <input type="number" v-model.number="thresholds.temp" /> <span style="font-size: 12px; color: var(--text-sub)">℃</span>
            <button class="btn" style="padding: 4px 10px; font-size: 12px;" @click="updateThreshold('temp')">保存设置</button>
          </div>
        </div>
        <div class="threshold-item" style="margin-bottom: 0;">
          <div class="th-header">
            <span>&#x2623; CO&#x2082;浓度阈值</span>
            <span style="font-size: 12px; color: var(--warning)">今日峰值: {{ gasPeak }} ppm</span>
          </div>
          <div class="th-input">
            <input type="number" v-model.number="thresholds.gas" /> <span style="font-size: 12px; color: var(--text-sub)">ppm</span>
            <button class="btn" style="padding: 4px 10px; font-size: 12px;" @click="updateThreshold('gas')">保存设置</button>
          </div>
        </div>
      </div>

      <!-- Right Panel: Device Control -->
      <div class="panel">
        <div class="panel-title">设备联动控制与数据管理</div>

        <div class="device-item">
          <div>
            <strong>舱门控制开关</strong><br>
            <span style="color:var(--text-sub); font-size:12px;">常闭状态，火情联动紧急开启逃生通道</span>
          </div>
          <div
            class="ctrl-switch"
            :class="{ off: !deviceSwitches.cabin }"
            @click="toggleDevice('cabin')"
          ></div>
        </div>

        <div class="device-item">
          <div>
            <strong>排烟风机设备</strong><br>
            <span style="color:var(--text-sub); font-size:12px;">火情确认后自动开启 / 手动干预</span>
          </div>
          <div
            class="ctrl-switch"
            :class="{ off: !deviceSwitches.fan }"
            @click="toggleDevice('fan')"
          ></div>
        </div>

        <div class="device-item">
          <div>
            <strong>自动水淋灭火系统</strong><br>
            <span style="color:var(--text-sub); font-size:12px;">火情确认后自动喷淋 / 手动干预</span>
          </div>
          <div
            class="ctrl-switch"
            :class="{ off: !deviceSwitches.sprinkler }"
            @click="toggleDevice('sprinkler')"
          ></div>
        </div>

        <div class="device-item" style="margin-bottom: 20px;">
          <div>
            <strong>声光报警器</strong><br>
            <span style="color:var(--text-sub); font-size:12px;">火情确认后自动鸣响 / 手动消音</span>
          </div>
          <div
            class="ctrl-switch"
            :class="{ off: !deviceSwitches.horn }"
            @click="toggleDevice('horn')"
          ></div>
        </div>

        <div class="panel-title" style="margin-bottom: 10px;">模拟控制与数据管理</div>
        <div class="btn-group">
          <button class="btn danger" style="padding: 5px 12px; font-size:12px;" @click="doControl('fire', 'simulate')">模拟火灾</button>
          <button class="btn" style="padding: 5px 12px; font-size:12px;" @click="doControl('fire', 'reset')">解除火灾</button>
          <button class="btn" style="padding: 5px 12px; font-size:12px; background-color: var(--warning);" @click="doControl('smoke', 'simulate')">模拟烟雾</button>
          <button class="btn" style="padding: 5px 12px; font-size:12px;" @click="doControl('smoke', 'reset')">解除烟雾</button>
        </div>
        <div class="btn-group" style="margin-top: 5px;">
          <button class="btn" @click="openModal('deviceControlLog')">设备控制历史记录</button>
          <button class="btn" @click="openModal('history')">历史监测日志</button>
          <button class="btn" @click="openModal('alarm')">报警事件库</button>
          <button class="btn" @click="handleBackup">数据备份归档</button>
        </div>
        <div class="btn-group">
          <button class="btn" style="background-color: var(--text-sub)" @click="openModal('recovery')">数据恢复</button>
        </div>
      </div>
      </div>  <!-- /right-col -->
    </div>

    <!-- 火情确认弹窗 -->
    <div v-if="modals.fireConfirm" class="modal-overlay" style="display: flex;">
      <div class="modal" style="width: 500px;">
        <div class="modal-header">
          <span>&#x26A0; 火情确认</span>
          <span class="modal-close" @click="closeModal('fireConfirm')">&times;</span>
        </div>
        <div class="modal-body">
          <div style="font-size: 16px; margin-bottom: 20px; line-height: 1.6;">
            <p>检测到<span style="color: var(--danger); font-weight: bold;">{{ alarmTypeText }}</span>超出阈值！</p>
            <p>当前区域：<span style="color: var(--warning); font-weight: bold;">区域{{ currentRegion }}</span></p>
            <p>检测数值：<span style="color: var(--warning); font-weight: bold;">{{ confirmValueText }}</span></p>
            <p>阈值设置：<span style="color: var(--text-sub);">{{ confirmThresholdText }}</span></p>
          </div>
          <div class="btn-group" style="justify-content: center; gap: 20px;">
            <button class="btn danger" @click="confirmFire">确认火情</button>
            <button class="btn" @click="cancelFireAlarm">火情误报</button>
          </div>
        </div>
      </div>
    </div>

    <!-- 应急联动弹窗 -->
    <div v-if="modals.realtimeAlarm" class="modal-overlay" style="display: flex;">
      <div class="modal" style="width: 500px;">
        <div class="modal-header">
          <span>&#x1F6A8; 应急联动处置</span>
          <span class="modal-close" @click="closeModal('realtimeAlarm')">&times;</span>
        </div>
        <div class="modal-body">
          <div style="font-size: 16px; margin-bottom: 20px; text-align: center;">
            <p>是否执行应急联动处置？</p>
            <p style="color: var(--warning);">该操作将开启所有联动设备！</p>
          </div>
          <div class="btn-group" style="justify-content: center;">
            <button class="btn danger" @click="confirmEmergencyLinkage">确认执行</button>
            <button class="btn" @click="closeModal('realtimeAlarm')">取消</button>
          </div>
        </div>
      </div>
    </div>

    <!-- 设备列表弹窗 -->
    <div v-if="modals.deviceList" class="modal-overlay" style="display: flex;">
      <div class="modal" style="width: 500px;">
        <div class="modal-header">
          <span>区域{{ currentRegion }} 设备运行状态统计</span>
          <span class="modal-close" @click="closeModal('deviceList')">&times;</span>
        </div>
        <div style="display: flex; background: var(--bg-color); border-bottom: 1px solid var(--border);">
          <button
            class="btn"
            style="flex:1; border-radius:0;"
            :style="{ background: deviceTab === 'online' ? 'var(--panel-bg)' : 'var(--bg-color)' }"
            @click="deviceTab = 'online'"
          >&#x1F7E2; 在线设备 {{ onlineDeviceCount }}</button>
          <button
            class="btn"
            style="flex:1; border-radius:0;"
            :style="{ background: deviceTab === 'offline' ? 'var(--panel-bg)' : 'var(--bg-color)' }"
            @click="deviceTab = 'offline'"
          >&#x1F534; 离线设备 {{ offlineDeviceCount }}</button>
        </div>
        <div class="modal-body" style="padding:15px;">
          <div v-show="deviceTab === 'online'">
            <table class="table-striped">
              <tbody>
                <tr v-for="(dev, i) in onlineDevices" :key="'on-'+i">
                  <td>&#x1F7E2;</td>
                  <td>{{ dev.name }}</td>
                  <td>在线</td>
                </tr>
                <tr v-if="onlineDevices.length === 0">
                  <td colspan="3" style="text-align:center; color:var(--text-sub)">暂无在线设备</td>
                </tr>
              </tbody>
            </table>
          </div>
          <div v-show="deviceTab === 'offline'">
            <table class="table-striped">
              <tbody>
                <tr v-for="(dev, i) in offlineDevices" :key="'off-'+i">
                  <td>&#x1F534;</td>
                  <td>{{ dev.name }}</td>
                  <td>离线</td>
                </tr>
                <tr v-if="offlineDevices.length === 0">
                  <td colspan="3" style="text-align:center; color:var(--success)">暂无离线设备</td>
                </tr>
              </tbody>
            </table>
          </div>
        </div>
      </div>
    </div>

    <!-- 设备控制历史记录弹窗 -->
    <div v-if="modals.deviceControlLog" class="modal-overlay" style="display: flex;">
      <div class="modal">
        <div class="modal-header">
          <span>设备控制历史记录</span>
          <span class="modal-close" @click="closeModal('deviceControlLog')">&times;</span>
        </div>
        <div class="modal-body">
          <table class="table-striped">
            <thead>
              <tr>
                <th>时间</th>
                <th>区域</th>
                <th>设备名称</th>
                <th>操作状态</th>
              </tr>
            </thead>
            <tbody>
              <tr v-for="(item, i) in deviceControlLogList" :key="i">
                <td>{{ item.time }}</td>
                <td>{{ item.region }}</td>
                <td>{{ item.device }}</td>
                <td>{{ item.status }}</td>
              </tr>
              <tr v-if="deviceControlLogList.length === 0">
                <td colspan="4" style="text-align:center;color:var(--text-sub)">暂无设备操作记录</td>
              </tr>
            </tbody>
          </table>
        </div>
      </div>
    </div>

    <!-- 历史监测日志弹窗 -->
    <div v-if="modals.history" class="modal-overlay" style="display: flex;">
      <div class="modal">
        <div class="modal-header">
          <span>历史监测日志</span>
          <span class="modal-close" @click="closeModal('history')">&times;</span>
        </div>
        <div class="modal-body">
          <p style="color:var(--text-sub); text-align:center;">历史监测数据展示区域</p>
        </div>
      </div>
    </div>

    <!-- 报警事件库弹窗 -->
    <div v-if="modals.alarm" class="modal-overlay" style="display: flex;">
      <div class="modal">
        <div class="modal-header">
          <span>报警事件库</span>
          <span class="modal-close" @click="closeModal('alarm')">&times;</span>
        </div>
        <div class="modal-body">
          <p style="color:var(--text-sub); text-align:center;">报警事件数据展示区域</p>
        </div>
      </div>
    </div>

    <!-- 数据恢复弹窗 -->
    <div v-if="modals.recovery" class="modal-overlay" style="display: flex;">
      <div class="modal">
        <div class="modal-header">
          <span>数据恢复</span>
          <span class="modal-close" @click="closeModal('recovery')">&times;</span>
        </div>
        <div class="modal-body">
          <div style="margin-bottom:15px;">
            <label style="font-size:14px; margin-right:10px;">选择备份节点：</label>
            <select v-model="recoverySelect" style="background: var(--bg-color); color: var(--text-main); border: 1px solid var(--border); padding: 6px 12px; border-radius: 4px;">
              <option value="">请选择</option>
              <option value="20260422">2026-04-22 18:00</option>
              <option value="20260421">2026-04-21 18:00</option>
              <option value="20260420">2026-04-20 18:00</option>
            </select>
          </div>
          <div style="margin-bottom:15px; color:var(--text-sub); font-size:13px;">
            <span :style="{ color: recoveryStatusColor }">{{ recoveryStatusText }}</span>
          </div>
          <button class="btn" :disabled="recoveryRunning" @click="startRecovery">执行恢复</button>
        </div>
      </div>
    </div>
  </div>
</template>

<script setup lang="ts">
import { ref, reactive, computed, onMounted, onUnmounted, watch, nextTick } from "vue"
import { useRouter } from "vue-router"
import { ElMessage } from "element-plus"
import axios from "axios"
import * as echarts from "echarts"

// ---- Interfaces ----
interface FireStatus {
  system: { overallRisk: string; systemNormal: boolean; running: boolean; fireSimulated: boolean }
  smoke: { state: number; stateText: string; risk: string; online: boolean }
  temperature: { value: number; unit: string; risk: string; online: boolean }
  humidity: { value: number; unit: string; online: boolean }
  co2: { value: number; unit: string; online: boolean }
  services: { alarmActive: boolean; suppressionActive: boolean; evacuationActive: boolean; centerAlarmActive: boolean }
  devices: { smoke: { online: boolean }; temperature: { online: boolean }; humidity: { online: boolean }; co2: { online: boolean }; alarm: { online: boolean } }
}

interface LogEntry {
  timestamp: string
  level: string
  event: string
  details: string
}

interface DeviceControlLogItem {
  time: string
  region: string
  device: string
  status: string
}

// ---- State ----
const router = useRouter()
const currentTime = ref("")
const currentRegion = ref("A")

const handleLogout = () => {
  localStorage.removeItem("isLoggedIn")
  router.push({ name: "login" })
}

const status = ref<FireStatus>({
  system: { overallRisk: "安全", systemNormal: true, running: true, fireSimulated: false },
  smoke: { state: 0, stateText: "正常", risk: "安全", online: true },
  temperature: { value: 25, unit: "°C", risk: "安全", online: true },
  humidity: { value: 50, unit: "%", online: true },
  co2: { value: 400, unit: "ppm", online: true },
  services: { alarmActive: false, suppressionActive: false, evacuationActive: false, centerAlarmActive: false },
  devices: { smoke: { online: true }, temperature: { online: true }, humidity: { online: true }, co2: { online: true }, alarm: { online: true } }
})

const logs = ref<LogEntry[]>([])

const thresholds = reactive({ temp: 45, gas: 1000 })
const tempPeak = ref("--")
const gasPeak = ref("--")

// 火情误报屏蔽标志: 点击"火情误报"后置true, 数据恢复正常后自动重置
const alarmDismissed = ref(false)

const deviceSwitches = reactive({ cabin: false, fan: false, sprinkler: false, horn: false })

const deviceControlLogList = ref<DeviceControlLogItem[]>([])

// Modals
const modals = reactive({
  fireConfirm: false,
  realtimeAlarm: false,
  deviceList: false,
  deviceControlLog: false,
  history: false,
  alarm: false,
  recovery: false
})

const deviceTab = ref("online")

// Fire confirm modal data
const alarmTypeText = ref("")
const confirmValueText = ref("")
const confirmThresholdText = ref("")
const pendingAlarmType = ref("")

// Recovery modal
const recoverySelect = ref("")
const recoveryStatusText = ref("")
const recoveryStatusColor = ref("var(--text-sub)")
const recoveryRunning = ref(false)

// Chart refs
const trendChartTempRef = ref<HTMLDivElement>()
const trendChartHumRef = ref<HTMLDivElement>()
const trendChartGasRef = ref<HTMLDivElement>()
const trendChartSmokeRef = ref<HTMLDivElement>()

let chartTemp: echarts.ECharts | null = null
let chartHum: echarts.ECharts | null = null
let chartGas: echarts.ECharts | null = null
let chartSmoke: echarts.ECharts | null = null

// Historical data for charts
const chartData = reactive({
  temp: [] as number[],
  hum: [] as number[],
  gas: [] as number[],
  smoke: [] as number[],
  labels: [] as string[]
})

// ---- Computed ----
const temperatureDisplay = computed(() => {
  if (status.value.temperature?.online === false) return "--"
  return status.value.temperature?.value?.toFixed(1) ?? "--"
})

const humidityDisplay = computed(() => {
  const val = status.value.humidity?.value
  return val != null ? val.toFixed(1) : "--"
})

const co2Display = computed(() => {
  const val = status.value.co2?.value
  return val != null ? String(val) : "--"
})

const onlineCountText = computed(() => {
  const list = allDevices.value
  if (!list || list.length === 0) return "--"
  const online = list.filter(d => d.online).length
  return `${online}/${list.length}`
})

const riskPercent = computed(() => {
  const risk = status.value.system?.overallRisk
  if (risk === "火灾") return 100
  if (risk === "高风险") return 88
  if (risk === "预警") return 65
  if (risk === "中风险") return 50
  // compute from temperature risk
  const tRisk = status.value.temperature?.risk
  if (tRisk === "高风险") return 80
  if (tRisk === "中风险") return 50
  return 15
})

const riskColor = computed(() => {
  const p = riskPercent.value
  if (p >= 80) return "var(--danger)"
  if (p >= 50) return "var(--warning)"
  return "var(--success)"
})

const riskText = computed(() => {
  const p = riskPercent.value
  if (p >= 80) return `高风险 ${p}%`
  if (p >= 50) return `中风险 ${p}%`
  return `低风险 ${p}%`
})

// Device list for modal
interface DeviceInfo { name: string; online: boolean }
const allDevices = computed<DeviceInfo[]>(() => {
  const region = currentRegion.value
  const devices = status.value.devices
  return [
    { name: `区域${region} - 烟雾探测器`, online: devices?.smoke?.online ?? true },
    { name: `区域${region} - 温度传感器`, online: devices?.temperature?.online ?? true },
    { name: `区域${region} - 湿度传感器`, online: devices?.humidity?.online ?? true },
    { name: `区域${region} - CO₂传感器`, online: devices?.co2?.online ?? true },
    { name: `区域${region} - 声光报警器`, online: devices?.alarm?.online ?? true }
  ]
})

const onlineDevices = computed(() => allDevices.value.filter(d => d.online))
const offlineDevices = computed(() => allDevices.value.filter(d => !d.online))
const onlineDeviceCount = computed(() => onlineDevices.value.length)
const offlineDeviceCount = computed(() => offlineDevices.value.length)

// ---- Methods ----
function updateTime() {
  const d = new Date()
  currentTime.value = `${d.getFullYear()}-${(d.getMonth() + 1).toString().padStart(2, "0")}-${d.getDate().toString().padStart(2, "0")} ${d.getHours().toString().padStart(2, "0")}:${d.getMinutes().toString().padStart(2, "0")}:${d.getSeconds().toString().padStart(2, "0")}`
}

function openModal(name: string) {
  ;(modals as any)[name] = true
}

function closeModal(name: string) {
  ;(modals as any)[name] = false
}

function addDeviceControlLog(deviceName: string, deviceStatus: string) {
  deviceControlLogList.value.unshift({
    time: new Date().toLocaleString("zh-CN"),
    region: "区域" + currentRegion.value,
    device: deviceName,
    status: deviceStatus
  })
}

function toggleDevice(device: "cabin" | "fan" | "sprinkler" | "horn") {
  deviceSwitches[device] = !deviceSwitches[device]
  const names: Record<string, string> = {
    cabin: "舱门控制开关",
    fan: "排烟风机设备",
    sprinkler: "自动水淋灭火系统",
    horn: "声光报警器"
  }
  addDeviceControlLog(names[device], deviceSwitches[device] ? "开启" : "关闭")
}

function openCurrentRegionDeviceModal() {
  deviceTab.value = "online"
  openModal("deviceList")
}

function emergencyLinkage() {
  openModal("realtimeAlarm")
}

function confirmEmergencyLinkage() {
  closeModal("realtimeAlarm")
  deviceSwitches.cabin = true
  deviceSwitches.fan = true
  deviceSwitches.sprinkler = true
  deviceSwitches.horn = true
  addDeviceControlLog("舱门控制开关", "开启")
  addDeviceControlLog("排烟风机设备", "开启")
  addDeviceControlLog("自动水淋灭火系统", "开启")
  addDeviceControlLog("声光报警器", "开启")
  ElMessage.success("应急联动处置已执行！所有设备已开启")
}

function confirmFire() {
  closeModal("fireConfirm")
  deviceSwitches.cabin = true
  deviceSwitches.fan = true
  deviceSwitches.sprinkler = true
  deviceSwitches.horn = true
  addDeviceControlLog("舱门控制开关", "开启")
  addDeviceControlLog("排烟风机设备", "开启")
  addDeviceControlLog("自动水淋灭火系统", "开启")
  addDeviceControlLog("声光报警器", "开启")
  ElMessage.success("已确认火情并执行安全防御！")
}

function cancelFireAlarm() {
  closeModal("fireConfirm")
  alarmDismissed.value = true
  ElMessage.info("已标记为火情误报，数据恢复正常后将自动重置")
}

function updateThreshold(type: string) {
  if (type === "temp") {
    ElMessage.success("温度阈值已更新为：" + thresholds.temp + "℃")
  } else if (type === "gas") {
    ElMessage.success("CO₂阈值已更新为：" + thresholds.gas + "ppm")
  }
}

function handleBackup() {
  ElMessage.success("数据已完成归档备份")
}

function startRecovery() {
  if (!recoverySelect.value) {
    ElMessage.warning("请选择一个备份节点！")
    return
  }
  recoveryRunning.value = true
  recoveryStatusText.value = "正在还原系统快照..."
  recoveryStatusColor.value = "var(--warning)"
  setTimeout(() => {
    recoveryStatusText.value = "数据恢复成功！系统状态已刷新。"
    recoveryStatusColor.value = "var(--success)"
    recoveryRunning.value = false
    setTimeout(() => closeModal("recovery"), 2000)
  }, 2000)
}

function checkThresholds() {
  const tempVal = status.value.temperature?.value
  const tempExceeded = tempVal != null && status.value.temperature?.online !== false && tempVal > thresholds.temp
  const smokeExceeded = status.value.smoke?.state !== 0

  // 数据恢复正常时, 重置屏蔽标志
  if (!tempExceeded && !smokeExceeded) {
    alarmDismissed.value = false
    return
  }

  // 已点击"火情误报", 不再弹窗
  if (alarmDismissed.value) return

  if (tempExceeded) {
    alarmTypeText.value = "温度"
    confirmValueText.value = tempVal!.toFixed(1) + "℃"
    confirmThresholdText.value = thresholds.temp + "℃"
    pendingAlarmType.value = "temp"
    openModal("fireConfirm")
    return
  }
  if (smokeExceeded) {
    alarmTypeText.value = "烟雾浓度"
    confirmValueText.value = status.value.smoke?.stateText ?? "异常"
    confirmThresholdText.value = "正常状态"
    pendingAlarmType.value = "smoke"
    openModal("fireConfirm")
  }
}

// ---- API ----
async function fetchStatus() {
  try {
    const res = await axios.get("/fire/api/status")
    status.value = res.data
    // Update chart data
    const now = new Date()
    const label = `${now.getHours().toString().padStart(2, "0")}:${now.getMinutes().toString().padStart(2, "0")}:${now.getSeconds().toString().padStart(2, "0")}`
    chartData.labels.push(label)
    chartData.temp.push(res.data.temperature?.value ?? 0)
    chartData.hum.push(res.data.humidity?.value ?? 0)
    chartData.gas.push(res.data.co2?.value ?? 0)
    chartData.smoke.push(res.data.smoke?.state ?? 0)
    // Keep last 20 points
    if (chartData.labels.length > 20) {
      chartData.labels.shift()
      chartData.temp.shift()
      chartData.hum.shift()
      chartData.gas.shift()
      chartData.smoke.shift()
    }
    updateCharts()
    // Update peak values
    if (res.data.temperature?.value != null) {
      const tv = res.data.temperature.value
      if (tempPeak.value === "--" || tv > parseFloat(tempPeak.value)) {
        tempPeak.value = tv.toFixed(1)
      }
    }
    // Check thresholds after status update
    checkThresholds()
  } catch (e) {
    // silent
  }
}

async function fetchLogs() {
  try {
    const res = await axios.get("/fire/api/logs", { params: { limit: 50 } })
    logs.value = res.data || []
  } catch (e) {
    // silent
  }
}

async function doControl(target: string, action: string) {
  try {
    const res = await axios.get("/fire/api/control", { params: { target, action } })
    if (res.data.status === "success") {
      ElMessage.success(res.data.message || "指令发送成功")
    } else {
      ElMessage.warning(res.data.message || "指令发送失败")
    }
    fetchStatus()
    fetchLogs()
  } catch (e) {
    ElMessage.error("网络请求失败")
  }
}

// ---- ECharts ----
const commonXAxis = {
  type: "category" as const,
  boundaryGap: false,
  data: [] as string[],
  axisLine: { lineStyle: { color: "#94a3b8" } }
}
const commonGrid = { left: "12%", right: "5%", bottom: "15%", top: "25%", containLabel: true }

function initCharts() {
  if (trendChartTempRef.value) {
    chartTemp = echarts.init(trendChartTempRef.value)
    chartTemp.setOption({
      backgroundColor: "transparent",
      title: { text: "温度趋势 (℃)", textStyle: { color: "#e2e8f0", fontSize: 13 } },
      tooltip: { trigger: "axis" },
      grid: commonGrid,
      xAxis: { ...commonXAxis, data: chartData.labels },
      yAxis: { type: "value", splitLine: { lineStyle: { color: "#1e293b" } }, axisLine: { lineStyle: { color: "#94a3b8" } } },
      series: [{ name: "温度", type: "line", smooth: true, data: chartData.temp, itemStyle: { color: "#f59e0b" }, areaStyle: { opacity: 0.1, color: "#f59e0b" } }]
    })
  }
  if (trendChartHumRef.value) {
    chartHum = echarts.init(trendChartHumRef.value)
    chartHum.setOption({
      backgroundColor: "transparent",
      title: { text: "湿度趋势 (%RH)", textStyle: { color: "#e2e8f0", fontSize: 13 } },
      tooltip: { trigger: "axis" },
      grid: commonGrid,
      xAxis: { ...commonXAxis, data: chartData.labels },
      yAxis: { type: "value", splitLine: { lineStyle: { color: "#1e293b" } }, axisLine: { lineStyle: { color: "#94a3b8" } } },
      series: [{ name: "湿度", type: "line", smooth: true, data: chartData.hum, itemStyle: { color: "#3b82f6" }, areaStyle: { opacity: 0.1, color: "#3b82f6" } }]
    })
  }
  if (trendChartGasRef.value) {
    chartGas = echarts.init(trendChartGasRef.value)
    chartGas.setOption({
      backgroundColor: "transparent",
      title: { text: "CO₂浓度 (ppm)", textStyle: { color: "#e2e8f0", fontSize: 13 } },
      tooltip: { trigger: "axis" },
      grid: commonGrid,
      xAxis: { ...commonXAxis, data: chartData.labels },
      yAxis: { type: "value", splitLine: { lineStyle: { color: "#1e293b" } }, axisLine: { lineStyle: { color: "#94a3b8" } } },
      series: [{ name: "CO₂", type: "line", smooth: true, data: chartData.gas, itemStyle: { color: "#10b981" }, areaStyle: { opacity: 0.1, color: "#10b981" } }]
    })
  }
  if (trendChartSmokeRef.value) {
    chartSmoke = echarts.init(trendChartSmokeRef.value)
    chartSmoke.setOption({
      backgroundColor: "transparent",
      title: { text: "烟雾浓度状态", textStyle: { color: "#e2e8f0", fontSize: 13 } },
      tooltip: { trigger: "axis" },
      grid: commonGrid,
      xAxis: { ...commonXAxis, data: chartData.labels },
      yAxis: {
        type: "category",
        data: ["异常", "正常"],
        inverse: true,
        splitLine: { lineStyle: { color: "#1e293b" } },
        axisLine: { lineStyle: { color: "#94a3b8" } }
      },
      series: [{
        name: "烟雾状态",
        type: "line",
        smooth: true,
        data: chartData.smoke.map(v => v === 0 ? "正常" : "异常"),
        itemStyle: { color: "#ef4444" },
        areaStyle: { opacity: 0.1, color: "#ef4444" }
      }]
    })
  }
}

function updateCharts() {
  if (chartTemp) {
    chartTemp.setOption({
      xAxis: { data: chartData.labels },
      series: [{ data: chartData.temp }]
    })
  }
  if (chartHum) {
    chartHum.setOption({
      xAxis: { data: chartData.labels },
      series: [{ data: chartData.hum }]
    })
  }
  if (chartGas) {
    chartGas.setOption({
      xAxis: { data: chartData.labels },
      series: [{ data: chartData.gas }]
    })
  }
  if (chartSmoke) {
    chartSmoke.setOption({
      xAxis: { data: chartData.labels },
      series: [{ data: chartData.smoke.map(v => v === 0 ? "正常" : "异常") }]
    })
  }
}

function handleResize() {
  chartTemp?.resize()
  chartHum?.resize()
  chartGas?.resize()
  chartSmoke?.resize()
}

// ---- Lifecycle ----
let timeInterval = 0
let statusInterval = 0
let logsInterval = 0

onMounted(() => {
  updateTime()
  timeInterval = window.setInterval(updateTime, 1000)
  fetchStatus()
  fetchLogs()
  statusInterval = window.setInterval(fetchStatus, 2000)
  logsInterval = window.setInterval(fetchLogs, 5000)
  nextTick(() => {
    initCharts()
  })
  window.addEventListener("resize", handleResize)
})

onUnmounted(() => {
  window.clearInterval(timeInterval)
  window.clearInterval(statusInterval)
  window.clearInterval(logsInterval)
  window.removeEventListener("resize", handleResize)
  chartTemp?.dispose()
  chartHum?.dispose()
  chartGas?.dispose()
  chartSmoke?.dispose()
})

// Watch region changes to update device modal data
watch(currentRegion, () => {
  // region changed, device list auto-updates via computed
})
</script>

<style scoped>
/* 顶部导航栏 */
.top-nav-bar {
  background: #151a27;
  border-bottom: 1px solid #1e293b;
  padding: 0 20px;
  flex-shrink: 0;
}
.top-nav-inner {
  display: flex;
  align-items: center;
  justify-content: space-between;
  height: 48px;
}
.top-nav-brand {
  display: flex;
  align-items: center;
  gap: 8px;
  font-size: 15px;
  font-weight: 600;
  color: #e2e8f0;
}
.top-nav-links {
  display: flex;
  align-items: center;
  gap: 4px;
}
.top-nav-links a {
  padding: 6px 14px;
  font-size: 13px;
  color: #94a3b8;
  text-decoration: none;
  border-radius: 6px;
  transition: all 0.2s;
}
.top-nav-links a:hover {
  color: #e2e8f0;
  background: rgba(239, 68, 68, 0.1);
}
.top-nav-links a.active {
  color: #fff;
  background: rgba(239, 68, 68, 0.2);
}
.top-nav-links a.nav-logout {
  color: #94a3b8;
  margin-left: 8px;
  border: 1px solid rgba(239, 68, 68, 0.2);
}
.top-nav-links a.nav-logout:hover {
  color: #fca5a5;
  background: rgba(239, 68, 68, 0.1);
}

.fire-page {
  margin: 0;
  padding: 0;
  background-color: var(--bg-color, #0b0f19);
  color: var(--text-main, #e2e8f0);
  font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
  display: flex;
  flex-direction: column;
  width: 100%;
  height: 100vh;
  --bg-color: #0b0f19;
  --panel-bg: #151a27;
  --text-main: #e2e8f0;
  --text-sub: #94a3b8;
  --accent: #3b82f6;
  --danger: #ef4444;
  --warning: #f59e0b;
  --success: #10b981;
  --border: #1e293b;
}

.fire-page header {
  background-color: var(--panel-bg);
  padding: 15px 30px;
  display: flex;
  justify-content: space-between;
  align-items: center;
  border-bottom: 2px solid var(--border);
}

.fire-page header h1 {
  margin: 0;
  font-size: 24px;
  color: var(--accent);
}

.status-bar {
  display: flex;
  gap: 20px;
  font-size: 14px;
}

.status-item span {
  font-weight: bold;
}

.status-item-clickable {
  cursor: pointer;
  padding: 2px 8px;
  border-radius: 4px;
  background: rgba(59, 130, 246, 0.1);
  border: 1px solid var(--accent);
  transition: 0.3s;
}

.status-item-clickable:hover {
  background: rgba(59, 130, 246, 0.2);
}

.fire-container {
  display: grid;
  grid-template-columns: 2fr 1fr;
  grid-template-rows: 1fr;
  gap: 20px;
  padding: 20px;
  flex: 1;
  width: 100%;
  box-sizing: border-box;
  overflow: hidden;
}

.risk-section {
  margin-bottom: 20px;
  background: rgba(0, 0, 0, 0.2);
  padding: 15px;
  border-radius: 6px;
  border: 1px solid var(--border);
}

.risk-bar-container {
  margin-bottom: 12px;
}

.risk-bar-container:last-child {
  margin-bottom: 0;
}

.risk-label {
  display: flex;
  justify-content: space-between;
  font-size: 13px;
  margin-bottom: 5px;
}

.progress-bg {
  width: 100%;
  height: 8px;
  background: var(--bg-color);
  border-radius: 4px;
  overflow: hidden;
}

.progress-fill {
  height: 100%;
  border-radius: 4px;
  position: relative;
  transition: width 0.5s ease, background 0.5s ease;
}

.threshold-item {
  background: var(--bg-color);
  padding: 10px;
  border-radius: 4px;
  margin-bottom: 10px;
  border: 1px solid var(--border);
}

.th-header {
  display: flex;
  justify-content: space-between;
  font-size: 13px;
  margin-bottom: 8px;
  font-weight: bold;
  align-items: center;
}

.th-input {
  display: flex;
  align-items: center;
  gap: 10px;
}

.th-input input {
  background: var(--panel-bg);
  color: var(--text-main);
  border: 1px solid var(--border);
  padding: 5px 8px;
  border-radius: 4px;
  width: 60px;
  text-align: center;
}

.panel {
  background-color: var(--panel-bg);
  border-radius: 8px;
  padding: 20px;
  border: 1px solid var(--border);
  display: flex;
  flex-direction: column;
}

.panel-title {
  font-size: 16px;
  font-weight: bold;
  margin-bottom: 8px;
  color: var(--text-main);
  border-left: 4px solid var(--accent);
  padding-left: 10px;
}

.realtime-grid {
  display: grid;
  grid-template-columns: repeat(4, 1fr);
  gap: 15px;
  margin-bottom: 20px;
}

.data-card {
  background-color: var(--bg-color);
  border: 1px solid var(--border);
  border-radius: 6px;
  padding: 15px;
  text-align: center;
}

.data-card .value {
  font-size: 28px;
  font-weight: bold;
  margin: 10px 0;
}

.data-card .label {
  color: var(--text-sub);
  font-size: 14px;
}

.temp-val {
  color: var(--warning);
}

.hum-val {
  color: var(--accent);
}

.smoke-val {
  color: var(--danger);
}

.gas-val {
  color: var(--success);
}

.charts-grid {
  display: grid;
  grid-template-columns: 1fr 1fr;
  grid-template-rows: 1fr 1fr;
  gap: 15px;
  flex: 1;
  width: 100%;
  min-height: 260px;
}

.chart-box {
  width: 100%;
  height: 100%;
  min-height: 110px;
}

.log-table {
  width: 100%;
  border-collapse: collapse;
  font-size: 13px;
}

.log-table th,
.log-table td {
  padding: 10px;
  text-align: left;
  border-bottom: 1px solid var(--border);
}

.log-table th {
  color: var(--text-sub);
}

.tag {
  padding: 3px 8px;
  border-radius: 4px;
  font-size: 12px;
  font-weight: bold;
}

.tag.danger {
  background: rgba(239, 68, 68, 0.2);
  color: var(--danger);
}

.tag.warning {
  background: rgba(245, 158, 11, 0.2);
  color: var(--warning);
}

.tag.system {
  background: rgba(59, 130, 246, 0.2);
  color: var(--accent);
}

.btn-group {
  display: flex;
  gap: 10px;
  margin-top: 10px;
}

.btn {
  background-color: var(--accent);
  color: white;
  border: none;
  padding: 8px 15px;
  border-radius: 4px;
  cursor: pointer;
  font-size: 14px;
  transition: background 0.3s;
}

.btn.danger {
  background-color: var(--danger);
}

.btn:hover {
  opacity: 0.8;
}

.btn:disabled {
  opacity: 0.5;
  cursor: not-allowed;
}

.device-item {
  display: flex;
  justify-content: space-between;
  align-items: center;
  background: var(--bg-color);
  padding: 10px;
  border-radius: 4px;
  margin-bottom: 10px;
}

.ctrl-switch {
  width: 40px;
  height: 20px;
  background: var(--success);
  border-radius: 10px;
  position: relative;
  cursor: pointer;
  transition: background 0.3s;
}

.ctrl-switch::after {
  content: '';
  position: absolute;
  width: 16px;
  height: 16px;
  background: white;
  border-radius: 50%;
  top: 2px;
  right: 2px;
  transition: 0.3s;
}

.ctrl-switch.off {
  background: var(--danger);
}

.ctrl-switch.off::after {
  right: 22px;
}

.modal-overlay {
  position: fixed;
  top: 0;
  left: 0;
  width: 100%;
  height: 100%;
  background: rgba(0, 0, 0, 0.7);
  display: flex;
  justify-content: center;
  align-items: center;
  z-index: 1000;
}

.modal {
  background: var(--panel-bg);
  border: 1px solid var(--border);
  border-radius: 8px;
  width: 650px;
  max-width: 90%;
  display: flex;
  flex-direction: column;
  box-shadow: 0 10px 25px rgba(0, 0, 0, 0.5);
}

.modal-header {
  padding: 15px 20px;
  border-bottom: 1px solid var(--border);
  display: flex;
  justify-content: space-between;
  align-items: center;
  font-weight: bold;
  font-size: 16px;
}

.modal-close {
  cursor: pointer;
  color: var(--text-sub);
  font-size: 20px;
}

.modal-close:hover {
  color: var(--danger);
}

.modal-body {
  padding: 20px;
  max-height: 400px;
  overflow-y: auto;
}

.table-striped {
  width: 100%;
  border-collapse: collapse;
  font-size: 13px;
  text-align: left;
}

.table-striped th,
.table-striped td {
  padding: 10px;
  border-bottom: 1px solid var(--border);
}

.table-striped tbody tr:nth-child(even) {
  background-color: rgba(255, 255, 255, 0.02);
}
</style>