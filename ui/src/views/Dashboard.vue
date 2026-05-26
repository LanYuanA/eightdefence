<template>
  <div class="dashboard-root flex flex-col min-h-screen">
    <!-- 顶部导航栏 -->
    <header class="nav-header">
      <div class="nav-inner">
        <div class="flex items-center gap-4">
          <div class="brand-icon">
            <svg width="28" height="28" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.8">
              <path d="M12 22s8-4 8-10V5l-8-3-8 3v7c0 6 8 10 8 10z"/>
            </svg>
          </div>
          <div>
            <div class="brand-title">智慧环控安防系统</div>
            <div class="brand-sub">Smart Environment & Security Platform</div>
          </div>
        </div>
        <div class="flex items-center gap-5">
          <div class="nav-time">
            <svg width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" class="mr-1.5 opacity-60">
              <circle cx="12" cy="12" r="10"/><path d="M12 6v6l4 2"/>
            </svg>
            {{ currentTime }}
          </div>
          <button class="nav-btn" @click="$router.push('/devices')">
            <svg width="15" height="15" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" class="mr-1.5"><rect x="2" y="3" width="20" height="14" rx="2"/><path d="M8 21h8m-4-4v4"/></svg>
            设备管理
          </button>
          <button class="nav-btn" @click="$router.push('/security')">
            <svg width="15" height="15" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" class="mr-1.5"><path d="M12 22s8-4 8-10V5l-8-3-8 3v7c0 6 8 10 8 10z"/></svg>
            安防系统
          </button>
          <button class="nav-btn" @click="$router.push('/logs')">
            <svg width="15" height="15" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" class="mr-1.5"><path d="M14 2H6a2 2 0 0 0-2 2v16a2 2 0 0 0 2 2h12a2 2 0 0 0 2-2V8z"/><path d="M14 2v6h6M16 13H8M16 17H8M10 9H8"/></svg>
            系统日志
          </button>
          <button class="nav-btn-danger" @click="handleLogout">
            <svg width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" class="mr-1"><path d="M9 21H5a2 2 0 0 1-2-2V5a2 2 0 0 1 2-2h4"/><polyline points="16 17 21 12 16 7"/><line x1="21" y1="12" x2="9" y2="12"/></svg>
            退出
          </button>
        </div>
      </div>
    </header>

    <main class="flex-1 p-5 grid grid-cols-12 gap-5 overflow-hidden">
      <!-- 左列: 环境数据 + 安防报警 -->
      <div class="col-span-12 lg:col-span-3 flex flex-col gap-5">
        <!-- 环境传感器 -->
        <div class="card">
          <div class="card-header">
            <svg width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M14 14.76V3.5a2.5 2.5 0 0 0-5 0v11.26a4.5 4.5 0 1 0 5 0z"/></svg>
            <span>环境监测</span>
          </div>
          <div class="sensor-grid">
            <div class="sensor-item">
              <div class="sensor-icon temp-icon">
                <svg width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M14 14.76V3.5a2.5 2.5 0 0 0-5 0v11.26a4.5 4.5 0 1 0 5 0z"/></svg>
              </div>
              <div class="sensor-info">
                <div class="sensor-name">温度</div>
                <div class="sensor-value">{{ data.temperature }}<span class="sensor-unit">°C</span></div>
              </div>
            </div>
            <div class="sensor-item">
              <div class="sensor-icon hum-icon">
                <svg width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M12 2.69l5.66 5.66a8 8 0 1 1-11.31 0z"/></svg>
              </div>
              <div class="sensor-info">
                <div class="sensor-name">湿度</div>
                <div class="sensor-value">{{ data.humidity }}<span class="sensor-unit">%</span></div>
              </div>
            </div>
            <div class="sensor-item">
              <div class="sensor-icon pm-icon">
                <svg width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><circle cx="12" cy="12" r="3"/><path d="M12 1v2m0 18v2M4.22 4.22l1.42 1.42m12.73 12.73l1.42 1.42M1 12h2m18 0h2M4.22 19.78l1.42-1.42M18.36 5.64l1.42-1.42"/></svg>
              </div>
              <div class="sensor-info">
                <div class="sensor-name">PM2.5</div>
                <div class="sensor-value" :class="data.pm25 > 75 ? 'text-red-500' : ''">{{ data.pm25 }}<span class="sensor-unit">μg/m³</span></div>
              </div>
            </div>
            <div class="sensor-item">
              <div class="sensor-icon pm-icon">
                <svg width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><circle cx="12" cy="12" r="10"/><circle cx="12" cy="12" r="6"/><circle cx="12" cy="12" r="2"/></svg>
              </div>
              <div class="sensor-info">
                <div class="sensor-name">PM10</div>
                <div class="sensor-value">{{ data.pm10 }}<span class="sensor-unit">μg/m³</span></div>
              </div>
            </div>
            <div class="sensor-item">
              <div class="sensor-icon gas-icon">
                <svg width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M8 16l2-6 2 6m-2-3h2m-8 3h16"/></svg>
              </div>
              <div class="sensor-info">
                <div class="sensor-name">TVOC</div>
                <div class="sensor-value">{{ data.tvoc }}<span class="sensor-unit">ppb</span></div>
              </div>
            </div>
            <div class="sensor-item">
              <div class="sensor-icon gas-icon">
                <svg width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M12 22c5.523 0 10-4.477 10-10S17.523 2 12 2 2 6.477 2 12s4.477 10 10 10z"/><path d="M12 8v4l3 3"/></svg>
              </div>
              <div class="sensor-info">
                <div class="sensor-name">CO₂</div>
                <div class="sensor-value">{{ data.co2 }}<span class="sensor-unit">ppm</span></div>
              </div>
            </div>
          </div>
        </div>

        <!-- 安防报警 -->
        <div class="card flex-1">
          <div class="card-header">
            <svg width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M10.29 3.86L1.82 18a2 2 0 0 0 1.71 3h16.94a2 2 0 0 0 1.71-3L13.71 3.86a2 2 0 0 0-3.42 0z"/><line x1="12" y1="9" x2="12" y2="13"/><line x1="12" y1="17" x2="12.01" y2="17"/></svg>
            <span>安防报警</span>
            <span class="ml-auto" :class="hasAlarm ? 'alarm-badge-active' : 'alarm-badge-clear'">
              {{ hasAlarm ? '报警中' : '正常' }}
            </span>
          </div>
          <div class="flex flex-col gap-2.5 mt-3">
            <div class="alarm-item" :class="{ 'alarm-active': data.smoke }">
              <div class="alarm-dot" :class="data.smoke ? 'dot-red' : 'dot-green'"></div>
              <span class="flex-1">烟雾探测</span>
              <span class="alarm-status" :class="data.smoke ? 'status-danger' : 'status-safe'">{{ data.smoke ? '报警' : '正常' }}</span>
            </div>
            <div class="alarm-item" :class="{ 'alarm-active': data.water }">
              <div class="alarm-dot" :class="securityStatus.water?.online === false ? 'dot-gray' : data.water ? 'dot-red' : 'dot-green'"></div>
              <span class="flex-1">积水检测</span>
              <span class="alarm-status" :class="securityStatus.water?.online === false ? 'status-offline' : data.water ? 'status-danger' : 'status-safe'">{{ securityStatus.water?.online === false ? '离线' : data.water ? '报警' : '正常' }}</span>
            </div>
            <div class="alarm-item" :class="{ 'alarm-active': data.ir }">
              <div class="alarm-dot" :class="securityStatus.intrusion?.online === false ? 'dot-gray' : data.ir ? 'dot-red' : 'dot-green'"></div>
              <span class="flex-1">红外入侵</span>
              <span class="alarm-status" :class="securityStatus.intrusion?.online === false ? 'status-offline' : data.ir ? 'status-danger' : 'status-safe'">{{ securityStatus.intrusion?.online === false ? '离线' : data.ir ? '检测' : '正常' }}</span>
            </div>
            <div class="alarm-item" :class="{ 'alarm-active': data.radar }">
              <div class="alarm-dot" :class="data.radar ? 'dot-red' : 'dot-green'"></div>
              <span class="flex-1">雷达微波</span>
              <span class="alarm-status" :class="data.radar ? 'status-danger' : 'status-safe'">{{ data.radar ? '检测' : '正常' }}</span>
            </div>
          </div>
        </div>
      </div>

      <!-- 中列: 趋势图 -->
      <div class="col-span-12 lg:col-span-6 flex flex-col gap-5">
        <div class="card flex-1 flex flex-col">
          <div class="card-header">
            <svg width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><polyline points="22 12 18 12 15 21 9 3 6 12 2 12"/></svg>
            <span>数据趋势</span>
          </div>
          <div class="flex-1 mt-2">
            <v-chart class="w-full h-full min-h-[380px]" :option="chartOption" autoresize />
          </div>
        </div>
      </div>

      <!-- 右列: 设备控制 -->
      <div class="col-span-12 lg:col-span-3 flex flex-col gap-5">
        <!-- 空调控制 -->
        <div class="card">
          <div class="card-header">
            <svg width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><rect x="2" y="4" width="20" height="16" rx="2"/><path d="M2 10h20"/></svg>
            <span>空调控制</span>
          </div>
          <div class="grid grid-cols-2 gap-2.5 mt-3">
            <button class="ctrl-btn" @click="control('ac', 'cool_on')">
              <svg width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M10 2v8.29a4 4 0 1 0 4 0V2"/></svg>
              制冷
            </button>
            <button class="ctrl-btn ctrl-btn-warm" @click="control('ac', 'heat_on')">
              <svg width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M14 14.76V3.5a2.5 2.5 0 0 0-5 0v11.26a4.5 4.5 0 1 0 5 0z"/></svg>
              制热
            </button>
            <button class="ctrl-btn ctrl-btn-stop col-span-2" @click="control('ac', 'cool_off')">
              <svg width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><rect x="6" y="6" width="12" height="12" rx="1"/></svg>
              关闭空调
            </button>
          </div>
        </div>

        <!-- 空气净化器 -->
        <div class="card">
          <div class="card-header">
            <svg width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><circle cx="12" cy="12" r="10"/><path d="M8 12h8M12 8v8"/></svg>
            <span>空气净化器</span>
            <span class="ml-auto status-tag" :class="data.purifier_power ? 'tag-on' : 'tag-off'">
              {{ data.purifier_power ? '运行中' : '已关闭' }}
            </span>
          </div>
          <div class="flex flex-col gap-2.5 mt-3">
            <div class="flex gap-2">
              <button class="ctrl-btn flex-1" @click="controlVal('purifier', 'power', 1)">开机</button>
              <button class="ctrl-btn ctrl-btn-stop flex-1" @click="controlVal('purifier', 'power', 0)">关机</button>
            </div>
            <div class="section-label">运行模式</div>
            <div class="flex gap-2">
              <button class="ctrl-btn-sm flex-1" :class="{ 'ctrl-btn-active': data.purifier_run_mode === 1 }" @click="controlVal('purifier', 'run_mode', 1)">自动</button>
              <button class="ctrl-btn-sm flex-1" :class="{ 'ctrl-btn-active': data.purifier_run_mode === 2 }" @click="controlVal('purifier', 'run_mode', 2)">睡眠</button>
              <button class="ctrl-btn-sm flex-1" :class="{ 'ctrl-btn-active': data.purifier_run_mode === 3 }" @click="controlVal('purifier', 'run_mode', 3)">强力</button>
            </div>
            <div class="section-label">手动档位</div>
            <div class="flex gap-2">
              <button class="ctrl-btn-sm flex-1" @click="controlVal('purifier', 'manual', 1)">1档</button>
              <button class="ctrl-btn-sm flex-1" @click="controlVal('purifier', 'manual', 2)">2档</button>
              <button class="ctrl-btn-sm flex-1" @click="controlVal('purifier', 'manual', 3)">3档</button>
            </div>
          </div>
        </div>

        <!-- 加湿器 -->
        <div class="card flex-1">
          <div class="card-header">
            <svg width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M12 2.69l5.66 5.66a8 8 0 1 1-11.31 0z"/></svg>
            <span>加湿器</span>
            <span class="ml-auto status-tag" :class="data.humidifier_power ? 'tag-on' : 'tag-off'">
              {{ data.humidifier_power ? '运行中' : '已关闭' }}
            </span>
          </div>
          <div class="flex flex-col gap-2.5 mt-3">
            <div class="flex gap-2">
              <button class="ctrl-btn flex-1" @click="controlVal('humidifier', 'power', 1)">开机</button>
              <button class="ctrl-btn ctrl-btn-stop flex-1" @click="controlVal('humidifier', 'power', 0)">关机</button>
            </div>
            <div class="flex gap-2">
              <button class="ctrl-btn-sm flex-1" @click="controlVal('humidifier', 'humidify', 1)">加湿</button>
              <button class="ctrl-btn-sm flex-1" @click="controlVal('humidifier', 'dehumidify', 1)">除湿</button>
              <button class="ctrl-btn-sm flex-1" @click="controlVal('humidifier', 'purify', 1)">净化</button>
            </div>
            <div class="section-label">目标湿度</div>
            <div class="slider-wrapper">
              <input type="range" class="custom-slider" min="30" max="80" step="5" v-model="targetHum" @change="setConstHum">
              <div class="slider-value">{{ targetHum }}%</div>
            </div>
          </div>
        </div>
      </div>
    </main>
  </div>
</template>

<script setup lang="ts">
import { ref, onMounted, onUnmounted, computed } from "vue"
import { useRouter } from "vue-router"
import { ElMessage } from "element-plus"
import axios from "axios"
import VChart from "vue-echarts"
import { use } from "echarts/core"
import { CanvasRenderer } from "echarts/renderers"
import { LineChart } from "echarts/charts"
import {
  GridComponent,
  TooltipComponent,
  LegendComponent,
  TitleComponent
} from "echarts/components"

use([CanvasRenderer, LineChart, GridComponent, TooltipComponent, LegendComponent, TitleComponent])

const router = useRouter()
const currentTime = ref("")

const data = ref({
  temperature: 0, humidity: 0, pm25: 0, pm10: 0,
  tvoc: 0, ch2o: 0, co2: 0, lux: 0,
  smoke: 0, water: 0, ir: 0, radar: 0,
  humidifier_power: 0, humidifier_fault: 0,
  purifier_power: 0, purifier_run_mode: 0
})

const securityStatus = ref<any>({
  water: { online: true },
  intrusion: { online: true }
})

const hasAlarm = computed(() => data.value.smoke || data.value.water || data.value.ir || data.value.radar)

const targetHum = ref(50)
let timeInterval = 0
let dataInterval = 0

const historyTime = ref<string[]>([])
const historyTemp = ref<number[]>([])
const historyHum = ref<number[]>([])
const historyPm25 = ref<number[]>([])

const chartOption = computed(() => {
  return {
    backgroundColor: "transparent",
    tooltip: {
      trigger: "axis",
      backgroundColor: "rgba(15, 23, 42, 0.9)",
      borderColor: "rgba(99, 102, 241, 0.3)",
      textStyle: { color: "#e2e8f0", fontSize: 12 }
    },
    legend: {
      data: ["温度", "湿度", "PM2.5"],
      textStyle: { color: "#94a3b8", fontSize: 12 },
      top: 0, right: 0
    },
    grid: { left: "3%", right: "4%", bottom: "3%", top: "40", containLabel: true },
    xAxis: {
      type: "category",
      boundaryGap: false,
      data: historyTime.value,
      axisLine: { lineStyle: { color: "rgba(148, 163, 184, 0.2)" } },
      axisLabel: { color: "#64748b", fontSize: 11 },
      splitLine: { show: false }
    },
    yAxis: {
      type: "value",
      axisLine: { show: false },
      axisLabel: { color: "#64748b", fontSize: 11 },
      splitLine: { lineStyle: { color: "rgba(148, 163, 184, 0.08)" } }
    },
    series: [
      {
        name: "温度", type: "line", smooth: true, symbol: "none",
        lineStyle: { width: 2, color: "#f59e0b" },
        areaStyle: { color: { type: "linear", x: 0, y: 0, x2: 0, y2: 1, colorStops: [{ offset: 0, color: "rgba(245, 158, 11, 0.15)" }, { offset: 1, color: "rgba(245, 158, 11, 0)" }] } },
        data: historyTemp.value
      },
      {
        name: "湿度", type: "line", smooth: true, symbol: "none",
        lineStyle: { width: 2, color: "#3b82f6" },
        areaStyle: { color: { type: "linear", x: 0, y: 0, x2: 0, y2: 1, colorStops: [{ offset: 0, color: "rgba(59, 130, 246, 0.15)" }, { offset: 1, color: "rgba(59, 130, 246, 0)" }] } },
        data: historyHum.value
      },
      {
        name: "PM2.5", type: "line", smooth: true, symbol: "none",
        lineStyle: { width: 2, color: "#8b5cf6" },
        areaStyle: { color: { type: "linear", x: 0, y: 0, x2: 0, y2: 1, colorStops: [{ offset: 0, color: "rgba(139, 92, 246, 0.15)" }, { offset: 1, color: "rgba(139, 92, 246, 0)" }] } },
        data: historyPm25.value
      }
    ]
  }
})

const updateTime = () => {
  const d = new Date()
  currentTime.value = `${d.getFullYear()}-${(d.getMonth()+1).toString().padStart(2,"0")}-${d.getDate().toString().padStart(2,"0")} ${d.getHours().toString().padStart(2,"0")}:${d.getMinutes().toString().padStart(2,"0")}:${d.getSeconds().toString().padStart(2,"0")}`
}

const handleLogout = () => {
  localStorage.removeItem("isLoggedIn")
  router.push({ name: "login" })
}

const fetchSecurityStatus = async () => {
  try {
    const res = await axios.get("/security/api/status")
    securityStatus.value = res.data
  } catch (e) {}
}

const fetchData = async () => {
  try {
    const res = await axios.get("/api/data")
    data.value = res.data
    
    const d = new Date()
    const t = `${d.getHours().toString().padStart(2,"0")}:${d.getMinutes().toString().padStart(2,"0")}:${d.getSeconds().toString().padStart(2,"0")}`
    historyTime.value.push(t)
    historyTemp.value.push(data.value.temperature)
    historyHum.value.push(data.value.humidity)
    historyPm25.value.push(data.value.pm25)
    
    if (historyTime.value.length > 30) {
      historyTime.value.shift()
      historyTemp.value.shift()
      historyHum.value.shift()
      historyPm25.value.shift()
    }
  } catch (e) {}
}

const control = async (device: string, action: string) => {
  try {
    const res = await axios.get(`/api/control`, { params: { device, action } })
    if (res.data.status === "success") {
      ElMessage.success("指令发送成功")
    } else {
      ElMessage.warning("指令发送失败")
    }
  } catch (e) {
    ElMessage.error("网络请求失败")
  }
}

const controlVal = async (device: string, action: string, val: number) => {
  try {
    const res = await axios.get(`/api/control`, { params: { device, action, val } })
    if (res.data.status === "success") {
      ElMessage.success("指令发送成功")
    } else {
      ElMessage.warning("指令发送失败")
    }
  } catch (e) {
    ElMessage.error("网络请求失败")
  }
}

const setConstHum = () => {
  controlVal('humidifier', "const_hum", targetHum.value)
}

onMounted(() => {
  updateTime()
  timeInterval = window.setInterval(updateTime, 1000)
  fetchData()
  fetchSecurityStatus()
  dataInterval = window.setInterval(() => { fetchData(); fetchSecurityStatus() }, 2000)
})

onUnmounted(() => {
  window.clearInterval(timeInterval)
  window.clearInterval(dataInterval)
})
</script>

<style scoped>
/* ===== 根容器 ===== */
.dashboard-root {
  background: #0f172a;
  background-image:
    radial-gradient(ellipse at 20% 50%, rgba(99, 102, 241, 0.06) 0%, transparent 60%),
    radial-gradient(ellipse at 80% 20%, rgba(59, 130, 246, 0.04) 0%, transparent 50%);
  font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", "PingFang SC", "Hiragino Sans GB", "Microsoft YaHei", sans-serif;
  color: #e2e8f0;
}

/* ===== 顶部导航 ===== */
.nav-header {
  background: rgba(15, 23, 42, 0.85);
  backdrop-filter: blur(16px);
  border-bottom: 1px solid rgba(148, 163, 184, 0.1);
  padding: 0 24px;
  position: sticky;
  top: 0;
  z-index: 50;
}
.nav-inner {
  display: flex;
  align-items: center;
  justify-content: space-between;
  height: 60px;
}
.brand-icon {
  display: flex;
  align-items: center;
  justify-content: center;
  width: 40px;
  height: 40px;
  border-radius: 10px;
  background: linear-gradient(135deg, #6366f1, #3b82f6);
  color: white;
  box-shadow: 0 2px 8px rgba(99, 102, 241, 0.3);
}
.brand-title {
  font-size: 18px;
  font-weight: 700;
  color: #f1f5f9;
  letter-spacing: 0.02em;
}
.brand-sub {
  font-size: 11px;
  color: #64748b;
  letter-spacing: 0.03em;
  margin-top: -1px;
}
.nav-time {
  display: flex;
  align-items: center;
  font-size: 13px;
  color: #94a3b8;
  font-variant-numeric: tabular-nums;
}
.nav-btn {
  display: flex;
  align-items: center;
  padding: 7px 14px;
  font-size: 13px;
  color: #94a3b8;
  background: rgba(148, 163, 184, 0.06);
  border: 1px solid rgba(148, 163, 184, 0.12);
  border-radius: 8px;
  cursor: pointer;
  transition: all 0.2s;
}
.nav-btn:hover {
  color: #e2e8f0;
  background: rgba(99, 102, 241, 0.1);
  border-color: rgba(99, 102, 241, 0.3);
}
.nav-btn-danger {
  display: flex;
  align-items: center;
  padding: 7px 14px;
  font-size: 13px;
  color: #94a3b8;
  background: rgba(239, 68, 68, 0.06);
  border: 1px solid rgba(239, 68, 68, 0.15);
  border-radius: 8px;
  cursor: pointer;
  transition: all 0.2s;
}
.nav-btn-danger:hover {
  color: #fca5a5;
  background: rgba(239, 68, 68, 0.12);
  border-color: rgba(239, 68, 68, 0.3);
}

/* ===== 卡片 ===== */
.card {
  background: rgba(30, 41, 59, 0.6);
  border: 1px solid rgba(148, 163, 184, 0.08);
  border-radius: 12px;
  padding: 18px;
  backdrop-filter: blur(8px);
}
.card-header {
  display: flex;
  align-items: center;
  gap: 8px;
  font-size: 14px;
  font-weight: 600;
  color: #e2e8f0;
  padding-bottom: 12px;
  border-bottom: 1px solid rgba(148, 163, 184, 0.08);
}
.card-header svg {
  color: #6366f1;
  flex-shrink: 0;
}

/* ===== 传感器网格 ===== */
.sensor-grid {
  display: grid;
  grid-template-columns: 1fr 1fr;
  gap: 10px;
  margin-top: 12px;
}
.sensor-item {
  display: flex;
  align-items: center;
  gap: 10px;
  background: rgba(15, 23, 42, 0.5);
  border: 1px solid rgba(148, 163, 184, 0.06);
  border-radius: 10px;
  padding: 12px;
  transition: border-color 0.2s;
}
.sensor-item:hover {
  border-color: rgba(99, 102, 241, 0.2);
}
.sensor-icon {
  display: flex;
  align-items: center;
  justify-content: center;
  width: 36px;
  height: 36px;
  border-radius: 8px;
  flex-shrink: 0;
}
.temp-icon { background: rgba(245, 158, 11, 0.12); color: #f59e0b; }
.hum-icon { background: rgba(59, 130, 246, 0.12); color: #3b82f6; }
.pm-icon { background: rgba(139, 92, 246, 0.12); color: #8b5cf6; }
.gas-icon { background: rgba(16, 185, 129, 0.12); color: #10b981; }
.sensor-name {
  font-size: 12px;
  color: #64748b;
}
.sensor-value {
  font-size: 20px;
  font-weight: 700;
  color: #f1f5f9;
  font-variant-numeric: tabular-nums;
  line-height: 1.1;
}
.sensor-unit {
  font-size: 11px;
  font-weight: 400;
  color: #64748b;
  margin-left: 2px;
}

/* ===== 报警项 ===== */
.alarm-badge-active {
  font-size: 11px;
  padding: 2px 8px;
  border-radius: 10px;
  background: rgba(239, 68, 68, 0.15);
  color: #f87171;
  font-weight: 600;
  animation: pulse-badge 2s infinite;
}
.alarm-badge-clear {
  font-size: 11px;
  padding: 2px 8px;
  border-radius: 10px;
  background: rgba(16, 185, 129, 0.1);
  color: #34d399;
}
@keyframes pulse-badge {
  0%, 100% { opacity: 1; }
  50% { opacity: 0.6; }
}
.alarm-item {
  display: flex;
  align-items: center;
  gap: 10px;
  padding: 10px 12px;
  background: rgba(15, 23, 42, 0.4);
  border: 1px solid rgba(148, 163, 184, 0.06);
  border-radius: 8px;
  font-size: 13px;
  color: #94a3b8;
  transition: all 0.3s;
}
.alarm-item.alarm-active {
  background: rgba(239, 68, 68, 0.08);
  border-color: rgba(239, 68, 68, 0.2);
  color: #fca5a5;
}
.alarm-dot {
  width: 8px;
  height: 8px;
  border-radius: 50%;
  flex-shrink: 0;
}
.dot-green {
  background: #34d399;
  box-shadow: 0 0 6px rgba(52, 211, 153, 0.4);
}
.dot-red {
  background: #ef4444;
  box-shadow: 0 0 8px rgba(239, 68, 68, 0.5);
  animation: blink-dot 1s infinite alternate;
}
.dot-gray {
  background: #64748b;
  box-shadow: 0 0 4px rgba(100, 116, 139, 0.3);
}
@keyframes blink-dot {
  0% { opacity: 1; box-shadow: 0 0 8px rgba(239, 68, 68, 0.5); }
  100% { opacity: 0.5; box-shadow: 0 0 14px rgba(239, 68, 68, 0.8); }
}
.alarm-status {
  font-size: 12px;
  font-weight: 500;
}
.status-safe { color: #34d399; }
.status-danger { color: #ef4444; }
.status-offline { color: #64748b; }

/* ===== 控制按钮 ===== */
.ctrl-btn {
  display: flex;
  align-items: center;
  justify-content: center;
  gap: 6px;
  padding: 9px 12px;
  font-size: 13px;
  font-weight: 500;
  color: #e2e8f0;
  background: rgba(99, 102, 241, 0.08);
  border: 1px solid rgba(99, 102, 241, 0.2);
  border-radius: 8px;
  cursor: pointer;
  transition: all 0.2s;
}
.ctrl-btn:hover {
  background: rgba(99, 102, 241, 0.18);
  border-color: rgba(99, 102, 241, 0.4);
}
.ctrl-btn:active { transform: scale(0.97); }
.ctrl-btn-warm {
  background: rgba(245, 158, 11, 0.08);
  border-color: rgba(245, 158, 11, 0.2);
}
.ctrl-btn-warm:hover {
  background: rgba(245, 158, 11, 0.18);
  border-color: rgba(245, 158, 11, 0.4);
}
.ctrl-btn-stop {
  background: rgba(239, 68, 68, 0.06);
  border-color: rgba(239, 68, 68, 0.15);
  color: #fca5a5;
}
.ctrl-btn-stop:hover {
  background: rgba(239, 68, 68, 0.12);
  border-color: rgba(239, 68, 68, 0.3);
}
.ctrl-btn-sm {
  display: flex;
  align-items: center;
  justify-content: center;
  padding: 7px 8px;
  font-size: 12px;
  color: #94a3b8;
  background: rgba(148, 163, 184, 0.06);
  border: 1px solid rgba(148, 163, 184, 0.1);
  border-radius: 6px;
  cursor: pointer;
  transition: all 0.2s;
}
.ctrl-btn-sm:hover {
  color: #e2e8f0;
  background: rgba(99, 102, 241, 0.1);
  border-color: rgba(99, 102, 241, 0.25);
}
.ctrl-btn-active {
  color: #a5b4fc !important;
  background: rgba(99, 102, 241, 0.15) !important;
  border-color: rgba(99, 102, 241, 0.35) !important;
}

.section-label {
  font-size: 12px;
  color: #64748b;
  margin-top: 4px;
}

/* ===== 状态标签 ===== */
.status-tag {
  font-size: 11px;
  padding: 2px 8px;
  border-radius: 10px;
}
.tag-on {
  background: rgba(16, 185, 129, 0.1);
  color: #34d399;
}
.tag-off {
  background: rgba(148, 163, 184, 0.08);
  color: #64748b;
}

/* ===== 滑块 ===== */
.slider-wrapper {
  display: flex;
  align-items: center;
  gap: 12px;
}
.custom-slider {
  flex: 1;
  -webkit-appearance: none;
  appearance: none;
  height: 4px;
  background: rgba(99, 102, 241, 0.2);
  border-radius: 2px;
  outline: none;
}
.custom-slider::-webkit-slider-thumb {
  -webkit-appearance: none;
  width: 16px;
  height: 16px;
  background: #6366f1;
  border-radius: 50%;
  cursor: pointer;
  box-shadow: 0 0 6px rgba(99, 102, 241, 0.4);
}
.slider-value {
  font-size: 14px;
  font-weight: 600;
  color: #a5b4fc;
  min-width: 36px;
  text-align: right;
  font-variant-numeric: tabular-nums;
}
</style>