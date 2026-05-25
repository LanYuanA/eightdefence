<template>
  <div class="dev-root flex flex-col min-h-screen">
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
          <button class="nav-btn" @click="$router.push('/')">
            <svg width="15" height="15" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" class="mr-1.5"><path d="M3 9l9-7 9 7v11a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2z"/><polyline points="9 22 9 12 15 12 15 22"/></svg>
            返回主页
          </button>
          <button class="nav-btn" @click="$router.push('/logs')">
            <svg width="15" height="15" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" class="mr-1.5"><path d="M14 2H6a2 2 0 0 0-2 2v16a2 2 0 0 0 2 2h12a2 2 0 0 0 2-2V8z"/><path d="M14 2v6h6"/><line x1="16" y1="13" x2="8" y2="13"/><line x1="16" y1="17" x2="8" y2="17"/></svg>
            日志查看
          </button>
        </div>
      </div>
    </header>

    <main class="flex-1 p-5 flex gap-5 overflow-hidden">
      <!-- 左侧面板: 设备列表 -->
      <div class="w-72 flex flex-col gap-4 shrink-0">
        <!-- 筛选器 -->
        <div class="card">
          <div class="card-header">
            <svg width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><polygon points="22 3 2 3 10 12.46 10 19 14 21 14 12.46 22 3"/></svg>
            <span>设备筛选</span>
          </div>
          <div class="flex flex-wrap gap-2 mt-3">
            <button
              v-for="cat in categories" :key="cat.value"
              class="filter-tag"
              :class="{ 'active': activeCategory === cat.value }"
              @click="activeCategory = cat.value"
            >{{ cat.label }}</button>
          </div>
          <div class="mt-3 flex items-center gap-3 text-xs text-slate-400">
            <span class="flex items-center gap-1.5">
              <span class="w-2 h-2 rounded-full bg-emerald-400 shadow-emerald"></span>
              在线 <span class="text-emerald-400 font-semibold">{{ onlineCount }}</span>
            </span>
            <span class="text-slate-600">/</span>
            <span>共 <span class="text-slate-200">{{ filteredDevices.length }}</span> 台</span>
          </div>
        </div>

        <!-- 设备列表 -->
        <div class="card flex-1 flex flex-col">
          <div class="card-header">
            <svg width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><rect x="2" y="3" width="20" height="14" rx="2"/><path d="M8 21h8m-4-4v4"/></svg>
            <span>设备列表</span>
          </div>
          <div class="mt-3 flex flex-col gap-1.5 max-h-[calc(100vh-340px)] overflow-y-auto custom-scrollbar">
            <div
              v-for="dev in filteredDevices" :key="dev.type"
              class="device-item"
              :class="{ 'active': selectedDevice?.type === dev.type, 'offline': !dev.online }"
              @click="selectDevice(dev)"
            >
              <div class="flex items-center gap-2 min-w-0">
                <span class="w-2 h-2 rounded-full shrink-0" :class="dev.online ? 'bg-emerald-400 shadow-emerald' : 'bg-red-500 shadow-red'"></span>
                <span class="text-sm truncate">{{ dev.name }}</span>
              </div>
              <span class="text-xs font-mono" :class="dev.online ? 'text-indigo-300' : 'text-red-400'">
                {{ dev.online ? formatValue(dev) : '离线' }}
              </span>
            </div>
          </div>
        </div>
      </div>

      <!-- 右侧面板: 设备详情 -->
      <div class="flex-1 flex flex-col gap-5 min-w-0">
        <template v-if="selectedDevice">
          <!-- 设备信息卡片 -->
          <div class="card">
            <div class="flex items-center justify-between mb-4">
              <div class="card-header pb-0 mb-0 border-0">
                <svg width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><circle cx="12" cy="12" r="3"/><path d="M19.4 15a1.65 1.65 0 0 0 .33 1.82l.06.06a2 2 0 0 1 0 2.83 2 2 0 0 1-2.83 0l-.06-.06a1.65 1.65 0 0 0-1.82-.33 1.65 1.65 0 0 0-1 1.51V21a2 2 0 0 1-2 2 2 2 0 0 1-2-2v-.09A1.65 1.65 0 0 0 9 19.4a1.65 1.65 0 0 0-1.82.33l-.06.06a2 2 0 0 1-2.83 0 2 2 0 0 1 0-2.83l.06-.06A1.65 1.65 0 0 0 4.68 15a1.65 1.65 0 0 0-1.51-1H3a2 2 0 0 1-2-2 2 2 0 0 1 2-2h.09A1.65 1.65 0 0 0 4.6 9a1.65 1.65 0 0 0-.33-1.82l-.06-.06a2 2 0 0 1 0-2.83 2 2 0 0 1 2.83 0l.06.06A1.65 1.65 0 0 0 9 4.68a1.65 1.65 0 0 0 1-1.51V3a2 2 0 0 1 2-2 2 2 0 0 1 2 2v.09a1.65 1.65 0 0 0 1 1.51 1.65 1.65 0 0 0 1.82-.33l.06-.06a2 2 0 0 1 2.83 0 2 2 0 0 1 0 2.83l-.06.06A1.65 1.65 0 0 0 19.4 9a1.65 1.65 0 0 0 1.51 1H21a2 2 0 0 1 2 2 2 2 0 0 1-2 2h-.09a1.65 1.65 0 0 0-1.51 1z"/></svg>
                <span>设备信息</span>
              </div>
              <div class="flex items-center gap-3">
                <span class="status-badge" :class="selectedDevice.online ? 'status-online' : 'status-offline'">
                  {{ selectedDevice.online ? '在线' : '离线' }}
                </span>
                <span class="category-badge">{{ selectedDevice.category }}</span>
              </div>
            </div>

            <div class="grid grid-cols-2 md:grid-cols-4 gap-4">
              <div class="info-box">
                <div class="info-label">设备名称</div>
                <div class="info-value text-indigo-300">{{ selectedDevice.name }}</div>
              </div>
              <div class="info-box">
                <div class="info-label">设备类型</div>
                <div class="info-value text-purple-300">{{ selectedDevice.type }}</div>
              </div>
              <div class="info-box">
                <div class="info-label">当前数值</div>
                <div class="info-value text-amber-300">
                  {{ formatValue(selectedDevice) }}
                  <span class="text-xs text-slate-500 ml-1">{{ selectedDevice.unit }}</span>
                </div>
              </div>
              <div class="info-box">
                <div class="info-label">运行状态</div>
                <div class="info-value" :class="selectedDevice.online ? 'text-emerald-400' : 'text-red-400'">
                  {{ selectedDevice.online ? '正常运行' : '通信中断' }}
                </div>
              </div>
            </div>

            <div class="mt-4 p-3 bg-slate-900/50 border border-slate-700/50 rounded-lg">
              <div class="text-xs text-slate-500 mb-1">设备描述</div>
              <div class="text-sm text-slate-300">{{ selectedDevice.description }}</div>
            </div>
          </div>

          <!-- 实时数据图表 -->
          <div class="card flex-1 flex flex-col">
            <div class="card-header">
              <svg width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M22 12h-4l-3 9L9 3l-3 9H2"/></svg>
              <span>实时数据</span>
              <span class="ml-auto text-xs text-slate-400 font-mono">最近 60 个采样点</span>
            </div>
            <div class="mt-4 flex-1">
              <div v-if="hasChart">
                <div class="flex items-center gap-4 mb-4">
                  <span class="text-xs text-slate-400">当前: <span class="text-indigo-300 font-semibold font-mono">{{ formatValue(selectedDevice) }} {{ selectedDevice.unit }}</span></span>
                </div>
                <!-- 柱状图 -->
                <div class="chart-area flex items-end gap-1 h-48 px-2">
                  <div
                    v-for="(val, idx) in valueHistory" :key="idx"
                    class="flex-1 transition-all duration-300 rounded-t-sm relative group"
                    :style="{ height: getBarHeight(val) + '%', background: getBarColor(val) }"
                  >
                    <div class="absolute -top-6 left-1/2 -translate-x-1/2 text-[10px] text-indigo-300 font-mono opacity-0 group-hover:opacity-100 transition-opacity whitespace-nowrap">
                      {{ val }}{{ selectedDevice.unit }}
                    </div>
                  </div>
                </div>
                <div class="flex justify-between text-[10px] text-slate-500 mt-2 px-2">
                  <span>{{ valueHistory.length > 0 ? valueHistory.length + ' 个采样点' : '采集中...' }}</span>
                  <span>最小: {{ historyMin }} | 最大: {{ historyMax }} | 平均: {{ historyAvg }}</span>
                </div>
              </div>

              <!-- 报警/开关设备的大状态显示 -->
              <div v-else class="flex items-center justify-center py-12">
                <div class="text-center">
                  <div class="text-8xl font-black font-mono" :class="selectedDevice.value ? 'text-red-400' : 'text-emerald-400'" style="text-shadow: 0 0 30px currentColor">
                    {{ selectedDevice.value ? '⚠' : '✓' }}
                  </div>
                  <div class="text-2xl mt-4 font-mono" :class="selectedDevice.value ? 'text-red-400' : 'text-emerald-400'">
                    {{ selectedDevice.value ? '报警触发' : '正常状态' }}
                  </div>
                </div>
              </div>
            </div>
          </div>

          <!-- 系统统计 -->
          <div class="card">
            <div class="card-header">
              <svg width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M21.21 15.89A10 10 0 1 1 8 2.83"/><path d="M22 12A10 10 0 0 0 12 2v10z"/></svg>
              <span>总线统计</span>
            </div>
            <div class="grid grid-cols-2 md:grid-cols-4 gap-4 mt-4">
              <div class="info-box">
                <div class="info-label">总事务数</div>
                <div class="info-value text-indigo-300 font-mono">{{ busStats.totalTransactions || 0 }}</div>
              </div>
              <div class="info-box">
                <div class="info-label">总错误数</div>
                <div class="info-value text-red-400 font-mono">{{ busStats.totalErrors || 0 }}</div>
              </div>
              <div class="info-box">
                <div class="info-label">平均延迟</div>
                <div class="info-value text-amber-300 font-mono">{{ busStats.avgLatencyMs?.toFixed(1) || '0' }}ms</div>
              </div>
              <div class="info-box">
                <div class="info-label">命令队列</div>
                <div class="info-value text-purple-300 font-mono">{{ cmdStats.queueSize || 0 }}</div>
              </div>
            </div>
          </div>
        </template>

        <!-- 未选择设备 -->
        <div v-else class="flex items-center justify-center h-full">
          <div class="text-center">
            <div class="empty-icon mx-auto mb-4">
              <svg width="48" height="48" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.5" class="text-slate-600">
                <rect x="2" y="3" width="20" height="14" rx="2"/><path d="M8 21h8m-4-4v4"/>
              </svg>
            </div>
            <div class="text-lg text-slate-400 font-medium">请从左侧选择设备</div>
            <div class="text-sm text-slate-600 mt-2">点击任意设备查看详细信息与实时数据</div>
          </div>
        </div>
      </div>
    </main>
  </div>
</template>

<script setup lang="ts">
import { ref, computed, onMounted, onUnmounted } from 'vue'
import axios from 'axios'

interface DeviceInfo {
  name: string
  type: string
  online: number
  value: number
  unit: string
  description: string
  category: string
}

const currentTime = ref('')
const devices = ref<DeviceInfo[]>([])
const selectedDevice = ref<DeviceInfo | null>(null)
const activeCategory = ref('all')
const valueHistory = ref<number[]>([])
const busStats = ref<any>({})
const cmdStats = ref<any>({})

let timeInterval = 0
let dataInterval = 0

const categories = [
  { value: 'all', label: '全部' },
  { value: 'sensor', label: '传感器' },
  { value: 'alarm', label: '报警器' },
  { value: 'control', label: '控制器' }
]

const filteredDevices = computed(() => {
  if (activeCategory.value === 'all') return devices.value
  return devices.value.filter(d => d.category === activeCategory.value)
})

const onlineCount = computed(() => filteredDevices.value.filter(d => d.online).length)

const hasChart = computed(() => {
  if (!selectedDevice.value) return false
  const chartTypes = ['cloud_pm25', 'cloud_pm10', 'cloud_temp', 'cloud_humidity', 'cloud_tvoc', 'cloud_ch2o', 'cloud_o3', 'cloud_co2', 'light']
  return chartTypes.includes(selectedDevice.value.type)
})

const historyMin = computed(() => {
  if (valueHistory.value.length === 0) return 0
  return Math.min(...valueHistory.value)
})

const historyMax = computed(() => {
  if (valueHistory.value.length === 0) return 0
  return Math.max(...valueHistory.value)
})

const historyAvg = computed(() => {
  if (valueHistory.value.length === 0) return 0
  return (valueHistory.value.reduce((a, b) => a + b, 0) / valueHistory.value.length).toFixed(1)
})

function formatValue(dev: DeviceInfo): string {
  if (dev.category === 'alarm') return dev.value ? '报警' : '正常'
  if (dev.type === 'humidifier') return dev.value ? '开启' : '关闭'
  if (dev.type === 'purifier') return dev.value ? '开启' : '关闭'
  return String(dev.value)
}

function getBarHeight(val: number): number {
  if (valueHistory.value.length === 0) return 0
  const max = historyMax.value || 1
  return Math.max(2, (val / max) * 100)
}

function getBarColor(val: number): string {
  if (!selectedDevice.value) return 'rgba(99, 102, 241, 0.6)'
  if (selectedDevice.value.type === 'cloud_pm25') {
    if (val > 75) return 'rgba(239, 68, 68, 0.7)'
    if (val > 35) return 'rgba(245, 158, 11, 0.7)'
    return 'rgba(99, 102, 241, 0.7)'
  }
  return 'rgba(99, 102, 241, 0.7)'
}

const updateTime = () => {
  const d = new Date()
  currentTime.value = `${d.getFullYear()}-${(d.getMonth()+1).toString().padStart(2,'0')}-${d.getDate().toString().padStart(2,'0')} ${d.getHours().toString().padStart(2,'0')}:${d.getMinutes().toString().padStart(2,'0')}:${d.getSeconds().toString().padStart(2,'0')}`
}

async function fetchDevices() {
  try {
    const res = await axios.get('/api/devices')
    const newDevices: DeviceInfo[] = res.data || []
    for (const newDev of newDevices) {
      const existing = devices.value.find(d => d.type === newDev.type)
      if (existing) {
        Object.assign(existing, newDev)
      } else {
        devices.value.push(newDev)
      }
    }
    if (devices.value.length === 0) devices.value = newDevices

    if (selectedDevice.value) {
      const updated = newDevices.find(d => d.type === selectedDevice.value!.type)
      if (updated) {
        selectedDevice.value = updated
        if (hasChart.value) {
          valueHistory.value.push(updated.value)
          if (valueHistory.value.length > 60) valueHistory.value.shift()
        }
      }
    }
  } catch (e) {}
}

async function fetchBusStats() {
  try {
    const [busRes, cmdRes] = await Promise.all([
      axios.get('/api/bus/stats'),
      axios.get('/api/cmd/stats')
    ])
    busStats.value = busRes.data || {}
    cmdStats.value = cmdRes.data || {}
  } catch (e) {}
}

function selectDevice(dev: DeviceInfo) {
  selectedDevice.value = dev
  valueHistory.value = []
  if (hasChart.value) valueHistory.value.push(dev.value)
}

onMounted(() => {
  updateTime()
  timeInterval = window.setInterval(updateTime, 1000)
  fetchDevices()
  fetchBusStats()
  dataInterval = window.setInterval(() => {
    fetchDevices()
    fetchBusStats()
  }, 2000)
})

onUnmounted(() => {
  window.clearInterval(timeInterval)
  window.clearInterval(dataInterval)
})
</script>

<style scoped>
.dev-root {
  background: #0f172a;
  background-image:
    radial-gradient(ellipse at 20% 50%, rgba(99, 102, 241, 0.06) 0%, transparent 60%),
    radial-gradient(ellipse at 80% 20%, rgba(59, 130, 246, 0.04) 0%, transparent 50%);
  font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", "PingFang SC", "Hiragino Sans GB", "Microsoft YaHei", sans-serif;
  color: #e2e8f0;
}

/* ===== 导航栏 ===== */
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
.brand-title { font-size: 18px; font-weight: 700; color: #f1f5f9; }
.brand-sub { font-size: 11px; color: #64748b; margin-top: -1px; }
.nav-time { display: flex; align-items: center; font-size: 13px; color: #94a3b8; font-variant-numeric: tabular-nums; }
.nav-btn {
  display: flex; align-items: center; padding: 7px 14px; font-size: 13px; color: #94a3b8;
  background: rgba(148, 163, 184, 0.06); border: 1px solid rgba(148, 163, 184, 0.12);
  border-radius: 8px; cursor: pointer; transition: all 0.2s;
}
.nav-btn:hover { color: #e2e8f0; background: rgba(99, 102, 241, 0.1); border-color: rgba(99, 102, 241, 0.3); }

/* ===== 卡片 ===== */
.card {
  background: rgba(30, 41, 59, 0.6);
  border: 1px solid rgba(148, 163, 184, 0.08);
  border-radius: 12px;
  padding: 18px;
  backdrop-filter: blur(8px);
}
.card-header {
  display: flex; align-items: center; gap: 8px; font-size: 14px; font-weight: 600;
  color: #e2e8f0; padding-bottom: 12px; border-bottom: 1px solid rgba(148, 163, 184, 0.08);
}
.card-header svg { color: #6366f1; flex-shrink: 0; }

/* ===== 筛选标签 ===== */
.filter-tag {
  padding: 4px 12px; font-size: 12px; border-radius: 16px; cursor: pointer; transition: all 0.15s;
  background: rgba(148, 163, 184, 0.06); border: 1px solid rgba(148, 163, 184, 0.12); color: #94a3b8;
}
.filter-tag:hover { background: rgba(99, 102, 241, 0.1); border-color: rgba(99, 102, 241, 0.3); color: #c7d2fe; }
.filter-tag.active {
  background: rgba(99, 102, 241, 0.15); border-color: rgba(99, 102, 241, 0.5); color: #a5b4fc;
  box-shadow: 0 0 8px rgba(99, 102, 241, 0.15);
}

/* ===== 设备列表项 ===== */
.device-item {
  display: flex; align-items: center; justify-content: space-between; gap: 8px;
  padding: 8px 12px; background: rgba(15, 23, 42, 0.4); border: 1px solid rgba(148, 163, 184, 0.06);
  border-radius: 8px; cursor: pointer; transition: all 0.15s; font-size: 13px; color: #94a3b8;
}
.device-item:hover { background: rgba(99, 102, 241, 0.08); border-color: rgba(99, 102, 241, 0.2); }
.device-item.active {
  background: rgba(99, 102, 241, 0.12); border-color: rgba(99, 102, 241, 0.35); color: #c7d2fe;
}
.device-item.offline { opacity: 0.5; }

.shadow-emerald { box-shadow: 0 0 6px rgba(52, 211, 153, 0.5); }
.shadow-red { box-shadow: 0 0 6px rgba(239, 68, 68, 0.5); }

/* ===== 状态标签 ===== */
.status-badge {
  display: inline-flex; align-items: center; padding: 3px 10px; font-size: 11px;
  font-weight: 600; border-radius: 8px; letter-spacing: 0.05em;
}
.status-online { background: rgba(16, 185, 129, 0.1); color: #34d399; border: 1px solid rgba(16, 185, 129, 0.25); }
.status-offline { background: rgba(239, 68, 68, 0.1); color: #f87171; border: 1px solid rgba(239, 68, 68, 0.25); }
.category-badge {
  display: inline-flex; padding: 3px 10px; font-size: 11px; font-weight: 500;
  background: rgba(99, 102, 241, 0.1); color: #a5b4fc; border: 1px solid rgba(99, 102, 241, 0.25);
  border-radius: 8px;
}

/* ===== 信息框 ===== */
.info-box {
  background: rgba(15, 23, 42, 0.5); border: 1px solid rgba(148, 163, 184, 0.08);
  padding: 12px 14px; border-radius: 8px;
}
.info-label { font-size: 11px; color: #64748b; margin-bottom: 4px; letter-spacing: 0.03em; }
.info-value { font-size: 18px; font-weight: 600; }

/* ===== 图表 ===== */
.chart-area {
  background: rgba(15, 23, 42, 0.5); border: 1px solid rgba(148, 163, 184, 0.06);
  border-radius: 8px; padding: 14px;
}

/* ===== 空状态 ===== */
.empty-icon {
  display: flex; align-items: center; justify-content: center;
  width: 80px; height: 80px; border-radius: 20px;
  background: rgba(99, 102, 241, 0.06); border: 1px solid rgba(148, 163, 184, 0.08);
}

/* ===== 滚动条 ===== */
.custom-scrollbar::-webkit-scrollbar { width: 6px; }
.custom-scrollbar::-webkit-scrollbar-track { background: transparent; }
.custom-scrollbar::-webkit-scrollbar-thumb { background: rgba(148, 163, 184, 0.15); border-radius: 3px; }
.custom-scrollbar::-webkit-scrollbar-thumb:hover { background: rgba(148, 163, 184, 0.3); }
</style>