<template>
  <div class="cyber-container flex flex-col min-h-screen text-slate-100 font-sans">
    <!-- Header -->
    <header class="cyber-header flex items-center justify-between px-8 py-4 relative">
      <div class="glow-line top"></div>
      <div class="flex items-center gap-4 relative z-10">
        <div class="text-3xl font-black text-cyan-400 tracking-wider uppercase cyber-title">CYBER·MONITOR</div>
        <div class="text-xs text-cyan-200 mt-2 tracking-widest hidden md:block">ENVIRONMENTAL CONTROL SYSTEM</div>
      </div>
      <div class="flex items-center gap-6 relative z-10">
        <div class="font-mono text-cyan-300 tracking-widest">{{ currentTime }}</div>
        <button class="cyber-btn-danger" @click="handleLogout">SYSTEM LOGOUT</button>
      </div>
      <div class="glow-line bottom"></div>
    </header>

    <main class="flex-1 p-6 grid grid-cols-12 gap-6 relative z-10 overflow-hidden">
      <!-- Left Panel: Sensors -->
      <div class="col-span-12 lg:col-span-3 flex flex-col gap-6">
        <div class="cyber-card p-5">
          <h2 class="section-title">SENSOR ARRAY</h2>
          <div class="grid grid-cols-2 gap-4 mt-4">
            <div class="sensor-box">
              <div class="sensor-label">TEMP</div>
              <div class="sensor-val"><span class="num text-cyan-300">{{ data.temperature }}</span><span class="unit">°C</span></div>
            </div>
            <div class="sensor-box">
              <div class="sensor-label">HUMIDITY</div>
              <div class="sensor-val"><span class="num text-cyan-300">{{ data.humidity }}</span><span class="unit">%</span></div>
            </div>
            <div class="sensor-box">
              <div class="sensor-label">PM2.5</div>
              <div class="sensor-val"><span class="num" :class="data.pm25 > 75 ? 'text-red-500' : 'text-cyan-300'">{{ data.pm25 }}</span><span class="unit">μg</span></div>
            </div>
            <div class="sensor-box">
              <div class="sensor-label">PM10</div>
              <div class="sensor-val"><span class="num text-cyan-300">{{ data.pm10 }}</span><span class="unit">μg</span></div>
            </div>
            <div class="sensor-box">
              <div class="sensor-label">TVOC</div>
              <div class="sensor-val"><span class="num text-purple-400">{{ data.tvoc }}</span><span class="unit">ppb</span></div>
            </div>
            <div class="sensor-box">
              <div class="sensor-label">CH2O</div>
              <div class="sensor-val"><span class="num text-purple-400">{{ data.ch2o }}</span><span class="unit">ppb</span></div>
            </div>
          </div>
        </div>

        <div class="cyber-card p-5 flex-1">
          <h2 class="section-title">ALERT SYSTEM</h2>
          <div class="mt-4 flex flex-col gap-3">
            <div class="alert-item" :class="{'active': data.smoke}">
              <div class="icon"></div>
              <span>SMOKE DETECTION</span>
              <span class="status">{{ data.smoke ? "FATAL" : "CLEAR" }}</span>
            </div>
            <div class="alert-item" :class="{'active': data.water}">
              <div class="icon"></div>
              <span>WATER LEAKAGE</span>
              <span class="status">{{ data.water ? "FATAL" : "CLEAR" }}</span>
            </div>
            <div class="alert-item" :class="{'active': data.ir}">
              <div class="icon"></div>
              <span>INFRARED INTRUSION</span>
              <span class="status">{{ data.ir ? "DETECTED" : "CLEAR" }}</span>
            </div>
            <div class="alert-item" :class="{'active': data.radar}">
              <div class="icon"></div>
              <span>RADAR MOTION</span>
              <span class="status">{{ data.radar ? "DETECTED" : "CLEAR" }}</span>
            </div>
          </div>
        </div>
      </div>

      <!-- Center Panel: Large Vis -->
      <div class="col-span-12 lg:col-span-6 flex flex-col gap-6">
        <div class="cyber-card p-5 flex-1 w-full bg-grid">
           <h2 class="section-title">ENVIRONMENT CHART</h2>
           <v-chart class="chart w-full h-[400px]" :option="chartOption" autoresize />
        </div>
      </div>

      <!-- Right Panel: Controls -->
      <div class="col-span-12 lg:col-span-3 flex flex-col gap-6">
        <!-- Air Conditioner -->
        <div class="cyber-card p-5">
          <h2 class="section-title">HVAC CONTROL</h2>
          <div class="mt-4 grid grid-cols-2 gap-3">
            <button class="cyber-btn" @click="control('ac', 'cool_on')">COOL ON</button>
            <button class="cyber-btn text-orange-400 border-orange-500/50" @click="control('ac', 'heat_on')">HEAT ON</button>
            <button class="cyber-btn col-span-2 text-red-500 border-red-500/50" @click="control('ac', 'cool_off')">SHUTDOWN</button>
          </div>
        </div>
        
        <!-- Air Purifier -->
        <div class="cyber-card p-5">
          <h2 class="section-title flex justify-between">
            <span>PURIFIER SYSTEM</span>
            <span :class="data.purifier_power ? 'text-green-400' : 'text-red-400'">{{ data.purifier_power ? "ON" : "OFF" }}</span>
          </h2>
          <div class="mt-4 flex flex-col gap-3">
            <div class="flex gap-2">
              <button class="cyber-btn flex-1" @click="controlVal('purifier', 'power', 1)">PWR ON</button>
              <button class="cyber-btn flex-1 text-red-500 border-red-500/50" @click="controlVal('purifier', 'power', 0)">PWR OFF</button>
            </div>
            <div class="text-xs text-cyan-500 mb-1 mt-2">MODE SELECT</div>
            <div class="flex gap-2">
              <button class="cyber-btn px-2 text-xs" :class="{'active': data.purifier_run_mode===1}" @click="controlVal('purifier', 'run_mode', 1)">AUTO</button>
              <button class="cyber-btn px-2 text-xs" :class="{'active': data.purifier_run_mode===2}" @click="controlVal('purifier', 'run_mode', 2)">SLEEP</button>
              <button class="cyber-btn px-2 text-xs" :class="{'active': data.purifier_run_mode===3}" @click="controlVal('purifier', 'run_mode', 3)">HEAVY</button>
            </div>
            <div class="text-xs text-cyan-500 mb-1 mt-2">MANUAL LEVEL</div>
            <div class="flex gap-2">
              <button class="cyber-btn px-2 text-xs" @click="controlVal('purifier', 'manual', 1)">LVL 1</button>
              <button class="cyber-btn px-2 text-xs" @click="controlVal('purifier', 'manual', 2)">LVL 2</button>
              <button class="cyber-btn px-2 text-xs" @click="controlVal('purifier', 'manual', 3)">LVL 3</button>
            </div>
          </div>
        </div>

        <!-- Humidifier -->
        <div class="cyber-card p-5">
          <h2 class="section-title flex justify-between">
            <span>HUMIDIFIER SYS</span>
            <span :class="data.humidifier_power ? 'text-green-400' : 'text-red-400'">{{ data.humidifier_power ? "ON" : "OFF" }}</span>
          </h2>
          <div class="grid grid-cols-2 gap-3 mt-4">
             <button class="cyber-btn" @click="controlVal('humidifier', 'power', 1)">START</button>
             <button class="cyber-btn text-red-500 border-red-500/50" @click="controlVal('humidifier', 'power', 0)">STOP</button>
             <button class="cyber-btn text-xs" @click="controlVal('humidifier', 'humidify', 1)">HUMIDIFY</button>
             <button class="cyber-btn text-xs" @click="controlVal('humidifier', 'dehumidify', 1)">DE-HUMIDIFY</button>
             <button class="cyber-btn text-xs col-span-2" @click="controlVal('humidifier', 'purify', 1)">PURIFY MODE</button>
             
             <!-- Const Hum Slider -->
             <div class="col-span-2 mt-2">
               <div class="text-xs text-cyan-500 mb-2">TARGET HUMIDITY %</div>
               <input type="range" class="w-full range-cyber" min="30" max="80" step="5" v-model="targetHum" @change="setConstHum">
               <div class="text-center text-cyan-300 font-mono mt-1">{{ targetHum }}%</div>
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
    tooltip: { trigger: "axis" },
    legend: { data: ["TEMP", "HUMIDITY", "PM2.5"], textStyle: { color: "#67e8f9" } },
    grid: { left: "3%", right: "4%", bottom: "3%", containLabel: true },
    xAxis: {
      type: "category",
      boundaryGap: false,
      data: historyTime.value,
      axisLine: { lineStyle: { color: "#0ea5e9" } },
      axisLabel: { color: "#0ea5e9" }
    },
    yAxis: {
      type: "value",
      axisLine: { lineStyle: { color: "#0ea5e9" } },
      splitLine: { lineStyle: { color: "rgba(14, 165, 233, 0.2)", type: "dashed" } }
    },
    series: [
      {
        name: "TEMP", type: "line",
        itemStyle: { color: "#f59e0b" },
        areaStyle: { color: "rgba(245, 158, 11, 0.2)" },
        data: historyTemp.value
      },
      {
        name: "HUMIDITY", type: "line",
        itemStyle: { color: "#06b6d4" },
        areaStyle: { color: "rgba(6, 182, 212, 0.2)" },
        data: historyHum.value
      },
      {
        name: "PM2.5", type: "line",
        itemStyle: { color: "#8b5cf6" },
        areaStyle: { color: "rgba(139, 92, 246, 0.2)" },
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

const fetchData = async () => {
  try {
    const res = await axios.get("/api/data")
    data.value = res.data
    
    // Update chart
    const d = new Date()
    const t = `${d.getHours().toString().padStart(2,"0")}:${d.getMinutes().toString().padStart(2,"0")}:${d.getSeconds().toString().padStart(2,"0")}`
    historyTime.value.push(t)
    historyTemp.value.push(data.value.temperature)
    historyHum.value.push(data.value.humidity)
    historyPm25.value.push(data.value.pm25)
    
    if (historyTime.value.length > 20) {
      historyTime.value.shift()
      historyTemp.value.shift()
      historyHum.value.shift()
      historyPm25.value.shift()
    }
  } catch (e) {}
}

const control = async (device: string, action: string) => {
  try {
    ElMessage.info(`[${device.toUpperCase()}] EXEC: ${action.toUpperCase()}`)
    const res = await axios.get(`/api/control`, { params: { device, action } })
    if (res.data.status === "success") {
      ElMessage.success("CTRL_SUCCESS")
    } else {
      ElMessage.warning("CTRL_FAILURE")
    }
  } catch (e) {
    ElMessage.error("CTRL_ERROR: NET_FAIL")
  }
}

const controlVal = async (device: string, action: string, val: number) => {
  try {
    ElMessage.info(`[${device.toUpperCase()}] EXEC: ${action.toUpperCase()} VAL:${val}`)
    const res = await axios.get(`/api/control`, { params: { device, action, val } })
    if (res.data.status === "success") {
      ElMessage.success("CTRL_SUCCESS")
    } else {
      ElMessage.warning("CTRL_FAILURE")
    }
  } catch (e) {
    ElMessage.error("CTRL_ERROR: NET_FAIL")
  }
}

const setConstHum = () => {
  controlVal('humidifier', "const_hum", targetHum.value)
}

onMounted(() => {
  updateTime()
  timeInterval = window.setInterval(updateTime, 1000)
  fetchData()
  dataInterval = window.setInterval(fetchData, 2000)
})

onUnmounted(() => {
  window.clearInterval(timeInterval)
  window.clearInterval(dataInterval)
})
</script>

<style scoped>
@import url("https://fonts.googleapis.com/css2?family=Orbitron:wght@400;700;900&family=Rajdhani:wght@400;600&display=swap");

.cyber-container {
  background: #020617;
  background-image: 
    radial-gradient(circle at 15% 50%, rgba(6, 182, 212, 0.1) 0%, transparent 50%),
    radial-gradient(circle at 85% 30%, rgba(139, 92, 246, 0.1) 0%, transparent 50%);
  font-family: "Rajdhani", sans-serif;
}

.cyber-header {
  background: rgba(15, 23, 42, 0.6);
  backdrop-filter: blur(12px);
  border-bottom: 1px solid rgba(6, 182, 212, 0.3);
}

.cyber-title {
  font-family: "Orbitron", sans-serif;
  text-shadow: 0 0 10px rgba(34, 211, 238, 0.6);
}

.glow-line {
  position: absolute;
  left: 0;
  width: 100%;
  height: 1px;
  background: linear-gradient(90deg, transparent, rgba(34,211,238,0.8), transparent);
}
.glow-line.top { top: 0; }
.glow-line.bottom { bottom: 0; }

.cyber-card {
  background: rgba(15, 23, 42, 0.7);
  border: 1px solid rgba(6, 182, 212, 0.2);
  box-shadow: 
    inset 0 0 20px rgba(6, 182, 212, 0.05),
    0 4px 6px rgba(0, 0, 0, 0.3);
  position: relative;
}

.cyber-card::before, .cyber-card::after {
  content: "";
  position: absolute;
  width: 10px;
  height: 10px;
  border: 1px solid #22d3ee;
}
.cyber-card::before { top: -1px; left: -1px; border-right: none; border-bottom: none; }
.cyber-card::after { bottom: -1px; right: -1px; border-left: none; border-top: none; }

.bg-grid {
  background-image: linear-gradient(rgba(34,211,238,0.05) 1px, transparent 1px),
  linear-gradient(90deg, rgba(34,211,238,0.05) 1px, transparent 1px);
  background-size: 20px 20px;
}

.section-title {
  font-family: "Orbitron", sans-serif;
  color: #22d3ee;
  font-size: 1rem;
  letter-spacing: 0.1em;
  border-bottom: 1px solid rgba(34,211,238,0.3);
  padding-bottom: 0.5rem;
  text-shadow: 0 0 5px rgba(34,211,238,0.5);
}

.sensor-box {
  background: rgba(2, 6, 23, 0.8);
  border: 1px solid rgba(34,211,238,0.15);
  padding: 0.75rem;
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: center;
  clip-path: polygon(10px 0, 100% 0, 100% calc(100% - 10px), calc(100% - 10px) 100%, 0 100%, 0 10px);
}
.sensor-label {
  font-size: 0.75rem;
  color: #94a3b8;
  letter-spacing: 0.05em;
}
.sensor-val .num {
  font-family: "Orbitron", sans-serif;
  font-size: 1.5rem;
  font-weight: 700;
  text-shadow: 0 0 8px currentColor;
}
.sensor-val .unit {
  font-size: 0.75rem;
  color: #64748b;
  margin-left: 2px;
}

.alert-item {
  display: flex;
  align-items: center;
  background: rgba(15, 23, 42, 0.8);
  border: 1px solid rgba(148, 163, 184, 0.2);
  padding: 0.75rem;
  border-radius: 4px;
  font-family: "Orbitron", sans-serif;
  font-size: 0.8rem;
  color: #94a3b8;
  transition: all 0.3s ease;
}
.alert-item .icon {
  width: 10px; height: 10px;
  background: #64748b;
  border-radius: 50%;
  margin-right: 1rem;
  box-shadow: 0 0 5px #64748b;
}
.alert-item .status {
  margin-left: auto;
}
.alert-item.active {
  background: rgba(220, 38, 38, 0.15);
  border-color: rgba(220, 38, 38, 0.5);
  color: #f87171;
}
.alert-item.active .icon {
  background: #ef4444;
  box-shadow: 0 0 15px #ef4444;
  animation: blink 1s infinite alternate;
}

@keyframes blink {
  0% { opacity: 1; transform: scale(1); }
  100% { opacity: 0.4; transform: scale(0.8); }
}

.cyber-btn {
  background: rgba(34, 211, 238, 0.05);
  border: 1px solid rgba(34, 211, 238, 0.4);
  color: #22d3ee;
  font-family: "Orbitron", sans-serif;
  font-size: 0.85rem;
  padding: 0.5rem 1rem;
  cursor: pointer;
  transition: all 0.2s;
  text-transform: uppercase;
  clip-path: polygon(0 0, calc(100% - 8px) 0, 100% 8px, 100% 100%, 8px 100%, 0 calc(100% - 8px));
}
.cyber-btn:hover, .cyber-btn.active {
  background: rgba(34, 211, 238, 0.2);
  box-shadow: 0 0 15px rgba(34, 211, 238, 0.3);
  text-shadow: 0 0 5px currentColor;
}
.cyber-btn:active {
  transform: translateY(1px);
}

.cyber-btn-danger {
  background: rgba(239, 68, 68, 0.1);
  border: 1px solid rgba(239, 68, 68, 0.5);
  color: #ef4444;
  font-family: "Orbitron", sans-serif;
  font-size: 0.8rem;
  padding: 0.4rem 1rem;
  clip-path: polygon(0 0, calc(100% - 8px) 0, 100% 8px, 100% 100%, 8px 100%, 0 calc(100% - 8px));
  transition: all 0.2s;
}
.cyber-btn-danger:hover {
  background: rgba(239, 68, 68, 0.3);
  box-shadow: 0 0 15px rgba(239, 68, 68, 0.4);
}

.range-cyber {
  -webkit-appearance: none;
  background: rgba(34,211,238,0.2);
  height: 4px;
  outline: none;
  border-radius: 2px;
}
.range-cyber::-webkit-slider-thumb {
  -webkit-appearance: none;
  width: 14px;
  height: 14px;
  background: #22d3ee;
  border-radius: 0;
  cursor: pointer;
  box-shadow: 0 0 10px #22d3ee;
  clip-path: polygon(50% 0, 100% 50%, 50% 100%, 0 50%);
}
</style>
