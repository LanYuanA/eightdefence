<template>
  <div class="log-root flex flex-col min-h-screen">
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
          <button class="nav-btn" @click="$router.push('/devices')">
            <svg width="15" height="15" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" class="mr-1.5"><rect x="2" y="3" width="20" height="14" rx="2"/><path d="M8 21h8m-4-4v4"/></svg>
            设备管理
          </button>
        </div>
      </div>
    </header>

    <main class="flex-1 p-5 flex gap-5 overflow-hidden">
      <!-- 左侧: 日志文件列表 -->
      <div class="w-72 flex flex-col gap-4 shrink-0">
        <div class="card">
          <div class="card-header">
            <svg width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M14 2H6a2 2 0 0 0-2 2v16a2 2 0 0 0 2 2h12a2 2 0 0 0 2-2V8z"/><path d="M14 2v6h6"/></svg>
            <span>日志文件</span>
            <span class="ml-auto text-xs text-slate-500">共 {{ logFiles.length }} 个</span>
          </div>
          <div class="mt-3 flex flex-col gap-1.5 max-h-[calc(100vh-220px)] overflow-y-auto custom-scrollbar">
            <div
              v-for="file in logFiles" :key="file"
              class="log-file-item"
              :class="{ 'active': selectedFile === file }"
              @click="selectFile(file)"
            >
              <div class="flex items-center gap-2 min-w-0">
                <span class="w-2 h-2 rounded-full shrink-0" :class="file === latestFile ? 'bg-emerald-400 shadow-emerald' : 'bg-slate-600'"></span>
                <span class="text-sm truncate">{{ file }}</span>
              </div>
              <span v-if="file === latestFile" class="live-tag">实时</span>
            </div>
            <div v-if="logFiles.length === 0" class="text-slate-500 text-sm text-center py-6">暂无日志文件</div>
          </div>
        </div>
      </div>

      <!-- 右侧: 日志内容 -->
      <div class="flex-1 flex flex-col gap-4 min-w-0">
        <div class="card flex-1 flex flex-col">
          <!-- 工具栏 -->
          <div class="flex items-center justify-between mb-4 flex-wrap gap-3">
            <div class="flex items-center gap-3">
              <span class="text-base font-semibold text-slate-100">
                {{ selectedFile || '请选择日志文件' }}
              </span>
              <span v-if="selectedFile === latestFile" class="live-tag">实时</span>
            </div>
            <div class="flex items-center gap-3">
              <input
                v-model="searchText"
                type="text"
                placeholder="搜索日志内容..."
                class="search-input"
              />
              <select v-model="levelFilter" class="filter-select">
                <option value="all">全部级别</option>
                <option value="INFO">INFO</option>
                <option value="WARN">WARNING</option>
                <option value="ERR">ERROR</option>
              </select>
              <button class="tool-btn" @click="fetchLogContent" :disabled="!selectedFile">
                <svg width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><polyline points="23 4 23 10 17 10"/><path d="M20.49 15a9 9 0 1 1-2.12-9.36L23 10"/></svg>
                刷新
              </button>
              <label class="flex items-center gap-2 text-xs text-slate-400 cursor-pointer select-none">
                <input type="checkbox" v-model="autoScroll" class="accent-indigo-500" />
                自动滚动
              </label>
            </div>
          </div>

          <!-- 日志内容区域 -->
          <div ref="logContainer" class="log-content flex-1 overflow-y-auto custom-scrollbar">
            <div v-if="loading" class="text-center py-12 text-slate-400">
              <svg width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" class="animate-spin mx-auto mb-3 text-indigo-400"><path d="M21 12a9 9 0 1 1-6.219-8.56"/></svg>
              加载中...
            </div>
            <div v-else-if="filteredLines.length === 0" class="text-slate-500 text-center py-12">
              {{ selectedFile ? '暂无匹配的日志条目' : '请从左侧选择一个日志文件' }}
            </div>
            <div
              v-for="(line, idx) in filteredLines"
              :key="idx"
              class="log-line"
              :class="getLineClass(line)"
            >
              <span class="line-num">{{ line.lineNum }}</span>
              <span v-html="highlightSearch(line.text)"></span>
            </div>
          </div>

          <!-- 统计栏 -->
          <div class="stats-bar">
            <span>总行数: <span class="text-slate-200">{{ totalLines }}</span></span>
            <span>过滤: <span class="text-slate-200">{{ filteredLines.length }}</span></span>
            <span class="text-red-400">错误: <span class="font-semibold">{{ errorCount }}</span></span>
            <span class="text-amber-400">警告: <span class="font-semibold">{{ warnCount }}</span></span>
            <span>大小: <span class="text-slate-200">{{ fileSize }}</span></span>
          </div>
        </div>
      </div>
    </main>
  </div>
</template>

<script setup lang="ts">
import { ref, computed, onMounted, onUnmounted, nextTick } from 'vue'
import axios from 'axios'

interface LogLine {
  text: string
  lineNum: number
}

const currentTime = ref('')
const logFiles = ref<string[]>([])
const latestFile = ref('')
const selectedFile = ref('')
const logRaw = ref('')
const searchText = ref('')
const levelFilter = ref('all')
const autoScroll = ref(true)
const loading = ref(false)
const logContainer = ref<HTMLElement | null>(null)

let timeInterval = 0
let refreshInterval = 0

const allLines = computed<LogLine[]>(() => {
  if (!logRaw.value) return []
  const lines = logRaw.value.split('\n')
  return lines.map((text, idx) => ({ text, lineNum: idx + 1 }))
})

const filteredLines = computed<LogLine[]>(() => {
  let lines = allLines.value
  if (levelFilter.value !== 'all') {
    lines = lines.filter(l => {
      const t = l.text.toUpperCase()
      if (levelFilter.value === 'INFO') return t.includes('[INFO]')
      if (levelFilter.value === 'WARN') return t.includes('[WARNING]') || t.includes('[WARN]')
      if (levelFilter.value === 'ERR') return t.includes('[ERROR]') || t.includes('[ERR]')
      return true
    })
  }
  if (searchText.value.trim()) {
    const q = searchText.value.toLowerCase()
    lines = lines.filter(l => l.text.toLowerCase().includes(q))
  }
  return lines
})

const totalLines = computed(() => allLines.value.length)
const errorCount = computed(() => allLines.value.filter(l => l.text.toUpperCase().includes('[ERROR]') || l.text.toUpperCase().includes('[ERR]')).length)
const warnCount = computed(() => allLines.value.filter(l => l.text.toUpperCase().includes('[WARNING]') || l.text.toUpperCase().includes('[WARN]')).length)
const fileSize = computed(() => {
  const bytes = logRaw.value.length
  if (bytes < 1024) return bytes + ' B'
  if (bytes < 1024 * 1024) return (bytes / 1024).toFixed(1) + ' KB'
  return (bytes / (1024 * 1024)).toFixed(1) + ' MB'
})

function getLineClass(line: LogLine): string {
  const t = line.text.toUpperCase()
  if (t.includes('[ERROR]') || t.includes('[ERR]')) return 'line-error'
  if (t.includes('[WARNING]') || t.includes('[WARN]')) return 'line-warn'
  if (t.includes('[INFO]')) return 'line-info'
  return ''
}

function highlightSearch(text: string): string {
  if (!searchText.value.trim()) return escapeHtml(text)
  const q = searchText.value.replace(/[.*+?^${}()|[\]\\]/g, '\\$&')
  const regex = new RegExp(`(${q})`, 'gi')
  return escapeHtml(text).replace(regex, '<mark class="bg-yellow-500/30 text-yellow-200 rounded px-0.5">$1</mark>')
}

function escapeHtml(s: string): string {
  return s.replace(/&/g, '&').replace(/</g, '<').replace(/>/g, '>')
}

const updateTime = () => {
  const d = new Date()
  currentTime.value = `${d.getFullYear()}-${(d.getMonth()+1).toString().padStart(2,'0')}-${d.getDate().toString().padStart(2,'0')} ${d.getHours().toString().padStart(2,'0')}:${d.getMinutes().toString().padStart(2,'0')}:${d.getSeconds().toString().padStart(2,'0')}`
}

async function fetchLogFiles() {
  try {
    const res = await axios.get('/api/logs/list')
    logFiles.value = res.data || []
    if (logFiles.value.length > 0) {
      latestFile.value = logFiles.value[0]
      if (!selectedFile.value) {
        selectFile(latestFile.value)
      }
    }
  } catch (e) {}
}

function selectFile(filename: string) {
  selectedFile.value = filename
  fetchLogContent()
}

async function fetchLogContent() {
  if (!selectedFile.value) return
  loading.value = true
  try {
    const res = await axios.get(`/api/logs/${selectedFile.value}`)
    if (typeof res.data === 'string') {
      logRaw.value = res.data
    } else {
      logRaw.value = JSON.stringify(res.data)
    }
  } catch (e) {
    logRaw.value = '加载日志文件失败'
  }
  loading.value = false
  await nextTick()
  scrollToBottom()
}

function scrollToBottom() {
  if (autoScroll.value && logContainer.value) {
    logContainer.value.scrollTop = logContainer.value.scrollHeight
  }
}

onMounted(() => {
  updateTime()
  timeInterval = window.setInterval(updateTime, 1000)
  fetchLogFiles()
  refreshInterval = window.setInterval(() => {
    if (selectedFile.value === latestFile.value || !selectedFile.value) {
      fetchLogFiles()
    }
  }, 5000)
})

onUnmounted(() => {
  window.clearInterval(timeInterval)
  window.clearInterval(refreshInterval)
})
</script>

<style scoped>
.log-root {
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

/* ===== 日志文件列表 ===== */
.log-file-item {
  display: flex; align-items: center; justify-content: space-between; gap: 8px;
  padding: 8px 12px; background: rgba(15, 23, 42, 0.4); border: 1px solid rgba(148, 163, 184, 0.06);
  border-radius: 8px; cursor: pointer; transition: all 0.15s; font-size: 13px; color: #94a3b8;
  font-family: "JetBrains Mono", "SF Mono", "Fira Code", monospace;
}
.log-file-item:hover { background: rgba(99, 102, 241, 0.08); border-color: rgba(99, 102, 241, 0.2); }
.log-file-item.active {
  background: rgba(99, 102, 241, 0.12); border-color: rgba(99, 102, 241, 0.35);
  color: #c7d2fe;
}
.shadow-emerald { box-shadow: 0 0 6px rgba(52, 211, 153, 0.5); }
.live-tag {
  font-size: 10px; padding: 1px 6px; border-radius: 8px; font-weight: 600;
  background: rgba(16, 185, 129, 0.12); color: #34d399; flex-shrink: 0;
}

/* ===== 工具栏 ===== */
.search-input {
  background: rgba(15, 23, 42, 0.6); border: 1px solid rgba(148, 163, 184, 0.12);
  color: #e2e8f0; font-size: 13px; padding: 6px 12px; border-radius: 8px; outline: none;
  width: 180px; transition: border-color 0.2s;
}
.search-input:focus { border-color: rgba(99, 102, 241, 0.5); box-shadow: 0 0 0 2px rgba(99, 102, 241, 0.1); }
.search-input::placeholder { color: #475569; }

.filter-select {
  background: rgba(15, 23, 42, 0.6); border: 1px solid rgba(148, 163, 184, 0.12);
  color: #94a3b8; font-size: 12px; padding: 6px 10px; border-radius: 8px; outline: none; cursor: pointer;
}
.filter-select option { background: #1e293b; color: #e2e8f0; }

.tool-btn {
  display: flex; align-items: center; gap: 5px; padding: 6px 12px; font-size: 12px;
  color: #94a3b8; background: rgba(148, 163, 184, 0.06); border: 1px solid rgba(148, 163, 184, 0.12);
  border-radius: 8px; cursor: pointer; transition: all 0.2s;
}
.tool-btn:hover { color: #e2e8f0; background: rgba(99, 102, 241, 0.1); border-color: rgba(99, 102, 241, 0.25); }
.tool-btn:disabled { opacity: 0.4; cursor: not-allowed; }

/* ===== 日志内容 ===== */
.log-content {
  background: rgba(15, 23, 42, 0.5);
  border: 1px solid rgba(148, 163, 184, 0.06);
  border-radius: 10px;
  padding: 14px;
  font-family: "JetBrains Mono", "SF Mono", "Fira Code", monospace;
  font-size: 12px;
  line-height: 1.7;
  min-height: 300px;
}
.log-line {
  padding: 1px 6px 1px 8px; white-space: pre-wrap; word-break: break-all;
  border-left: 2px solid transparent; transition: background 0.15s; border-radius: 2px;
}
.log-line:hover { background: rgba(99, 102, 241, 0.05); }
.log-line.line-error { color: #f87171; border-left-color: #ef4444; background: rgba(239, 68, 68, 0.05); }
.log-line.line-warn { color: #fbbf24; border-left-color: #f59e0b; background: rgba(245, 158, 11, 0.05); }
.log-line.line-info { color: #94a3b8; }

.line-num {
  display: inline-block; min-width: 3em; text-align: right; margin-right: 12px;
  color: #334155; user-select: none;
}

/* ===== 统计栏 ===== */
.stats-bar {
  display: flex; align-items: center; gap: 24px; margin-top: 12px; padding-top: 12px;
  border-top: 1px solid rgba(148, 163, 184, 0.08); font-size: 12px; color: #64748b;
}

/* ===== 滚动条 ===== */
.custom-scrollbar::-webkit-scrollbar { width: 6px; }
.custom-scrollbar::-webkit-scrollbar-track { background: transparent; }
.custom-scrollbar::-webkit-scrollbar-thumb { background: rgba(148, 163, 184, 0.15); border-radius: 3px; }
.custom-scrollbar::-webkit-scrollbar-thumb:hover { background: rgba(148, 163, 184, 0.3); }
</style>