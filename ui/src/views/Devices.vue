<template>
  <div class="devices-root">
    <ParticleBackground :particle-count="30" color="#8b5cf6" :opacity="0.3" />
    <AppNavbar title="设备管理中心" subtitle="Device Management Center" :menu-items="menuItems" />

    <main class="devices-main">
      <!-- 设备概览 -->
      <section class="overview-section">
        <div class="section-header">
          <h2 class="section-title">
            <svg width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><rect x="2" y="3" width="20" height="14" rx="2"/><path d="M8 21h8m-4-4v4"/></svg>
            设备概览
          </h2>
          <div class="header-actions">
            <CyberButton variant="primary" size="sm" @click="showAddDevice = true">
              <svg width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><line x1="12" y1="5" x2="12" y2="19"/><line x1="5" y1="12" x2="19" y2="12"/></svg>
              添加设备
            </CyberButton>
            <CyberButton variant="ghost" size="sm" @click="refreshAllDevices">
              <svg width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M21.5 2v6h-6M2.5 22v-6h6M2 11.5a10 10 0 0 1 18.8-4.3M22 12.5a10 10 0 0 1-18.8 4.3"/></svg>
              刷新全部
            </CyberButton>
            <CyberButton variant="ghost" size="sm" @click="exportDevices">
              <svg width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M21 15v4a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2v-4"/><polyline points="17 8 12 3 7 8"/><line x1="12" y1="3" x2="12" y2="15"/></svg>
              导出配置
            </CyberButton>
          </div>
        </div>

        <div class="stats-grid">
          <div class="stat-card">
            <div class="stat-icon" style="background: rgba(59,130,246,0.2); color: #3b82f6;">📡</div>
            <div class="stat-info"><div class="stat-value">{{ totalCount }}</div><div class="stat-label">设备总数</div></div>
          </div>
          <div class="stat-card">
            <div class="stat-icon" style="background: rgba(34,197,94,0.2); color: #22c55d;">✅</div>
            <div class="stat-info"><div class="stat-value">{{ onlineCount }}</div><div class="stat-label">在线设备</div></div>
          </div>
          <div class="stat-card">
            <div class="stat-icon" style="background: rgba(239,68,68,0.2); color: #ef4444;">❌</div>
            <div class="stat-info"><div class="stat-value">{{ offlineCount }}</div><div class="stat-label">离线设备</div></div>
          </div>
          <div class="stat-card">
            <div class="stat-icon" style="background: rgba(245,158,11,0.2); color: #f59e0b;">⚠️</div>
            <div class="stat-info"><div class="stat-value">{{ alertCount }}</div><div class="stat-label">告警设备</div></div>
          </div>
        </div>
      </section>

      <!-- 筛选和搜索 -->
      <section class="filter-section">
        <div class="filter-bar">
          <div class="filter-tabs">
            <button v-for="cat in categories" :key="cat.value" class="filter-tab" :class="{ active: activeCategory === cat.value }" @click="activeCategory = cat.value">
              {{ cat.label }} <span class="tab-count">{{ cat.count }}</span>
            </button>
          </div>
          <div class="search-box">
            <svg width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><circle cx="11" cy="11" r="8"/><line x1="21" y1="21" x2="16.65" y2="16.65"/></svg>
            <input v-model="searchQuery" type="text" placeholder="搜索设备..." class="search-input" />
          </div>
        </div>
      </section>

      <!-- 设备网格 -->
      <section class="devices-grid-section">
        <div class="devices-grid">
          <div
            v-for="dev in filteredDevices"
            :key="dev.id"
            class="device-card"
            :class="{ offline: dev.status === 'offline', selected: selectedDevice?.id === dev.id }"
            @click="selectDevice(dev)"
          >
            <div class="card-header">
              <div class="device-icon" :style="{ background: dev.color + '20', color: dev.color }">
                {{ dev.icon }}
              </div>
              <div class="device-status" :class="dev.status">
                <span class="status-dot"></span>
                {{ dev.status === 'online' ? '在线' : '离线' }}
              </div>
            </div>

            <div class="card-body">
              <h3 class="device-name">{{ dev.name }}</h3>
              <p class="device-type">{{ dev.type }}</p>
              <div class="device-meta">
                <span class="meta-item">型号: {{ dev.model }}</span>
                <span class="meta-item">地址: {{ dev.address }}</span>
              </div>
              <div class="device-value">
                <span class="value-number">{{ dev.value }}</span>
                <span class="value-unit">{{ dev.unit }}</span>
              </div>
            </div>

            <div class="card-footer">
              <button class="action-btn" @click.stop="refreshDevice(dev)">刷新</button>
              <button class="action-btn" @click.stop="editDevice(dev)">编辑</button>
              <button class="action-btn danger" @click.stop="deleteDevice(dev)">删除</button>
            </div>
          </div>
        </div>
      </section>

      <!-- 设备详情面板 -->
      <Transition name="slide-right">
        <div v-if="selectedDevice" class="detail-panel">
          <div class="detail-card">
            <div class="detail-header">
              <div class="detail-icon" :style="{ background: selectedDevice.color + '20', color: selectedDevice.color }">
                {{ selectedDevice.icon }}
              </div>
              <div class="detail-info">
                <h3 class="detail-name">{{ selectedDevice.name }}</h3>
                <p class="detail-type">{{ selectedDevice.type }}</p>
              </div>
              <button class="close-btn" @click="selectedDevice = null">×</button>
            </div>

            <div class="detail-content">
              <div class="detail-section">
                <h4 class="section-title">基本信息</h4>
                <div class="detail-grid">
                  <div class="detail-item"><span class="detail-label">设备ID</span><span class="detail-value">{{ selectedDevice.id }}</span></div>
                  <div class="detail-item"><span class="detail-label">设备型号</span><span class="detail-value">{{ selectedDevice.model }}</span></div>
                  <div class="detail-item"><span class="detail-label">Modbus地址</span><span class="detail-value">{{ selectedDevice.address }}</span></div>
                  <div class="detail-item"><span class="detail-label">所属应用</span><span class="detail-value">{{ selectedDevice.app }}</span></div>
                  <div class="detail-item"><span class="detail-label">状态</span><span class="detail-value" :class="selectedDevice.status">{{ selectedDevice.status === 'online' ? '在线' : '离线' }}</span></div>
                  <div class="detail-item"><span class="detail-label">最后更新</span><span class="detail-value">{{ selectedDevice.lastUpdate }}</span></div>
                </div>
              </div>

              <div class="detail-section">
                <h4 class="section-title">当前数据</h4>
                <div class="data-display">
                  <div class="data-value-large">{{ selectedDevice.value }}<span class="data-unit">{{ selectedDevice.unit }}</span></div>
                  <div class="data-trend" :class="selectedDevice.trend">
                    {{ selectedDevice.trend === 'up' ? '↑ 上升' : selectedDevice.trend === 'down' ? '↓ 下降' : '→ 稳定' }}
                  </div>
                </div>
              </div>

              <div class="detail-section">
                <h4 class="section-title">操作</h4>
                <div class="detail-actions">
                  <CyberButton variant="primary" size="sm" @click="editDevice(selectedDevice)">编辑配置</CyberButton>
                  <CyberButton variant="ghost" size="sm" @click="refreshDevice(selectedDevice)">刷新数据</CyberButton>
                  <CyberButton variant="warning" size="sm" @click="toggleDeviceStatus(selectedDevice)">
                    {{ selectedDevice.status === 'online' ? '设为离线' : '设为在线' }}
                  </CyberButton>
                  <CyberButton variant="danger" size="sm" @click="deleteDevice(selectedDevice)">删除设备</CyberButton>
                </div>
              </div>
            </div>
          </div>
        </div>
      </Transition>
    </main>

    <!-- 添加设备弹窗 -->
    <Transition name="modal">
      <div v-if="showAddDevice" class="modal-overlay" @click.self="showAddDevice = false">
        <div class="modal-content">
          <div class="modal-header">
            <h3>添加新设备</h3>
            <button class="modal-close" @click="showAddDevice = false">×</button>
          </div>
          <div class="modal-body">
            <div class="form-grid">
              <div class="form-group"><label>设备名称 *</label><input v-model="newDevice.name" placeholder="输入设备名称" /></div>
              <div class="form-group"><label>设备类型</label><select v-model="newDevice.type"><option value="传感器">传感器</option><option value="控制器">控制器</option><option value="报警器">报警器</option></select></div>
              <div class="form-group"><label>设备型号 *</label><input v-model="newDevice.model" placeholder="输入设备型号" /></div>
              <div class="form-group"><label>Modbus地址</label><input v-model="newDevice.address" placeholder="0x30" /></div>
              <div class="form-group"><label>数据单位</label><input v-model="newDevice.unit" placeholder="℃, %, ppm" /></div>
              <div class="form-group"><label>所属应用</label><select v-model="newDevice.app"><option value="环境监测">环境监测</option><option value="安防系统">安防系统</option><option value="消防系统">消防系统</option><option value="资源池">资源池</option></select></div>
            </div>
          </div>
          <div class="modal-footer">
            <CyberButton variant="ghost" @click="showAddDevice = false">取消</CyberButton>
            <CyberButton variant="primary" @click="addDevice">添加设备</CyberButton>
          </div>
        </div>
      </div>
    </Transition>

    <!-- 编辑设备弹窗 -->
    <Transition name="modal">
      <div v-if="showEditDevice" class="modal-overlay" @click.self="showEditDevice = false">
        <div class="modal-content">
          <div class="modal-header">
            <h3>编辑设备配置</h3>
            <button class="modal-close" @click="showEditDevice = false">×</button>
          </div>
          <div class="modal-body">
            <div class="form-grid">
              <div class="form-group"><label>设备名称 *</label><input v-model="editForm.name" placeholder="输入设备名称" /></div>
              <div class="form-group"><label>设备类型</label><select v-model="editForm.type"><option value="传感器">传感器</option><option value="控制器">控制器</option><option value="报警器">报警器</option></select></div>
              <div class="form-group"><label>设备型号 *</label><input v-model="editForm.model" placeholder="输入设备型号" /></div>
              <div class="form-group"><label>Modbus地址</label><input v-model="editForm.address" placeholder="0x30" /></div>
              <div class="form-group"><label>数据单位</label><input v-model="editForm.unit" placeholder="℃, %, ppm" /></div>
              <div class="form-group"><label>所属应用</label><select v-model="editForm.app"><option value="环境监测">环境监测</option><option value="安防系统">安防系统</option><option value="消防系统">消防系统</option><option value="资源池">资源池</option></select></div>
            </div>
          </div>
          <div class="modal-footer">
            <CyberButton variant="ghost" @click="showEditDevice = false">取消</CyberButton>
            <CyberButton variant="primary" @click="saveEditDevice">保存修改</CyberButton>
          </div>
        </div>
      </div>
    </Transition>
  </div>
</template>

<script setup lang="ts">
import { ref, reactive, computed } from 'vue'
import { ElMessage } from 'element-plus'
import ParticleBackground from '../components/ParticleBackground.vue'
import AppNavbar from '../components/AppNavbar.vue'
import CyberButton from '../components/CyberButton.vue'

const menuItems = [
  { path: '/', label: '主控台' },
  { path: '/security', label: '安防系统' },
  { path: '/environment', label: '环境监测' },
  { path: '/fire', label: '消防系统' },
  { path: '/logs', label: '系统日志' }
]

const showAddDevice = ref(false)
const showEditDevice = ref(false)
const searchQuery = ref('')
const activeCategory = ref('all')
const selectedDevice = ref<any>(null)
const editingDevice = ref<any>(null)

interface Device {
  id: string
  name: string
  type: string
  model: string
  address: string
  icon: string
  color: string
  value: number
  unit: string
  status: 'online' | 'offline'
  app: string
  lastUpdate: string
  trend: 'up' | 'down' | 'stable'
}

const newDevice = reactive({
  name: '',
  type: '传感器',
  model: '',
  address: '',
  unit: '',
  app: '环境监测'
})

const editForm = reactive({
  name: '',
  type: '传感器',
  model: '',
  address: '',
  unit: '',
  app: '环境监测'
})

// 所有现有设备 + 资源池设备
const devices = reactive<Device[]>([
  // ===== 云测仪 SD123-E60V2 (主设备) =====
  { id: 'dev-001', name: '云测仪-温度传感器', type: '传感器', model: 'SD123-E60V2', address: '0x30', icon: '🌡️', color: '#3b82f6', value: 24.5, unit: '℃', status: 'online', app: '环境监测', lastUpdate: '2024-01-15 10:30:00', trend: 'stable' },
  { id: 'dev-002', name: '云测仪-湿度传感器', type: '传感器', model: 'SD123-E60V2', address: '0x30', icon: '💧', color: '#06b6d4', value: 65, unit: '%', status: 'online', app: '环境监测', lastUpdate: '2024-01-15 10:30:00', trend: 'down' },
  { id: 'dev-003', name: '云测仪-PM2.5传感器', type: '传感器', model: 'SD123-E60V2', address: '0x30', icon: '💨', color: '#f59e0b', value: 42, unit: 'μg/m³', status: 'online', app: '环境监测', lastUpdate: '2024-01-15 10:30:00', trend: 'stable' },
  { id: 'dev-004', name: '云测仪-CO2传感器', type: '传感器', model: 'SD123-E60V2', address: '0x30', icon: '☁️', color: '#8b5cf6', value: 520, unit: 'ppm', status: 'online', app: '环境监测', lastUpdate: '2024-01-15 10:30:00', trend: 'up' },
  { id: 'dev-005', name: '云测仪-TVOC传感器', type: '传感器', model: 'SD123-E60V2', address: '0x30', icon: '🧪', color: '#ec4899', value: 120, unit: 'ppb', status: 'online', app: '环境监测', lastUpdate: '2024-01-15 10:23:00', trend: 'stable' },
  { id: 'dev-006', name: '云测仪-甲醛传感器', type: '传感器', model: 'SD123-E60V2', address: '0x30', icon: '⚗️', color: '#14b8a6', value: 30, unit: 'ppb', status: 'online', app: '环境监测', lastUpdate: '2024-01-15 10:22:00', trend: 'down' },
  { id: 'dev-007', name: '云测仪-PM10传感器', type: '传感器', model: 'SD123-E60V2', address: '0x30', icon: '💨', color: '#f59e0b', value: 68, unit: 'μg/m³', status: 'online', app: '环境监测', lastUpdate: '2024-01-15 10:17:00', trend: 'stable' },
  { id: 'dev-008', name: '云测仪-PM1.0传感器', type: '传感器', model: 'SD123-E60V2', address: '0x30', icon: '💨', color: '#f59e0b', value: 25, unit: 'μg/m³', status: 'online', app: '环境监测', lastUpdate: '2024-01-15 10:16:00', trend: 'stable' },

  // ===== 独立设备 =====
  { id: 'dev-009', name: '烟雾报警器', type: '报警器', model: 'SMK-200', address: '0x70', icon: '🔥', color: '#ef4444', value: 0, unit: '', status: 'online', app: '消防系统', lastUpdate: '2024-01-15 10:29:00', trend: 'stable' },
  { id: 'dev-010', name: '水浸传感器', type: '传感器', model: 'WTR-100', address: '0x90', icon: '💧', color: '#06b6d4', value: 0, unit: 'cm', status: 'online', app: '安防系统', lastUpdate: '2024-01-15 10:28:00', trend: 'stable' },
  { id: 'dev-011', name: '红外探测器', type: '传感器', model: 'IRD-300', address: '0x40', icon: '👤', color: '#8b5cf6', value: 0, unit: '', status: 'online', app: '安防系统', lastUpdate: '2024-01-15 10:27:00', trend: 'stable' },
  { id: 'dev-012', name: '弱光检测传感器', type: '传感器', model: 'LS-100', address: '0x50', icon: '💡', color: '#f59e0b', value: 350, unit: 'lux', status: 'online', app: '环境监测', lastUpdate: '2024-01-15 10:24:00', trend: 'up' },
  { id: 'dev-013', name: '恒湿净化一体机', type: '控制器', model: 'HUM-400', address: '0x20', icon: '🌀', color: '#22c55d', value: 0, unit: '', status: 'online', app: '环境监测', lastUpdate: '2024-01-15 10:25:00', trend: 'stable' },
  { id: 'dev-014', name: '霉菌空气净化机', type: '控制器', model: 'AP-500', address: '0x10', icon: '🌬️', color: '#22c55d', value: 0, unit: '', status: 'online', app: '环境监测', lastUpdate: '2024-01-15 10:24:00', trend: 'stable' },
  { id: 'dev-015', name: '空调集中控制器', type: '控制器', model: 'ACT-500', address: '0x60', icon: '❄️', color: '#22c55d', value: 24, unit: '℃', status: 'online', app: '环境监测', lastUpdate: '2024-01-15 10:26:00', trend: 'down' },
  { id: 'dev-016', name: '报警装置(驱鼠器)', type: '报警器', model: 'ALM-100', address: '0x80', icon: '🔔', color: '#ef4444', value: 0, unit: '', status: 'online', app: '安防系统', lastUpdate: '2024-01-15 10:21:00', trend: 'stable' },

  // ===== 资源池设备 (传感器虚拟副本1) =====
  { id: 'pool-001', name: '[资源池]温度传感器-副本1', type: '传感器', model: 'SD123-E60V2-Copy1', address: '0x31', icon: '🌡️', color: '#3b82f6', value: 24.5, unit: '℃', status: 'online', app: '资源池', lastUpdate: '2024-01-15 10:30:00', trend: 'stable' },
  { id: 'pool-002', name: '[资源池]湿度传感器-副本1', type: '传感器', model: 'SD123-E60V2-Copy1', address: '0x31', icon: '💧', color: '#06b6d4', value: 65, unit: '%', status: 'online', app: '资源池', lastUpdate: '2024-01-15 10:30:00', trend: 'down' },
  { id: 'pool-003', name: '[资源池]PM2.5传感器-副本1', type: '传感器', model: 'SD123-E60V2-Copy1', address: '0x31', icon: '💨', color: '#f59e0b', value: 42, unit: 'μg/m³', status: 'online', app: '资源池', lastUpdate: '2024-01-15 10:30:00', trend: 'stable' },
  { id: 'pool-004', name: '[资源池]CO2传感器-副本1', type: '传感器', model: 'SD123-E60V2-Copy1', address: '0x31', icon: '☁️', color: '#8b5cf6', value: 520, unit: 'ppm', status: 'online', app: '资源池', lastUpdate: '2024-01-15 10:30:00', trend: 'up' },
  { id: 'pool-005', name: '[资源池]TVOC传感器-副本1', type: '传感器', model: 'SD123-E60V2-Copy1', address: '0x31', icon: '🧪', color: '#ec4899', value: 120, unit: 'ppb', status: 'online', app: '资源池', lastUpdate: '2024-01-15 10:23:00', trend: 'stable' },
  { id: 'pool-006', name: '[资源池]甲醛传感器-副本1', type: '传感器', model: 'SD123-E60V2-Copy1', address: '0x31', icon: '⚗️', color: '#14b8a6', value: 30, unit: 'ppb', status: 'online', app: '资源池', lastUpdate: '2024-01-15 10:22:00', trend: 'down' },
  { id: 'pool-007', name: '[资源池]PM10传感器-副本1', type: '传感器', model: 'SD123-E60V2-Copy1', address: '0x31', icon: '💨', color: '#f59e0b', value: 68, unit: 'μg/m³', status: 'online', app: '资源池', lastUpdate: '2024-01-15 10:17:00', trend: 'stable' },
  { id: 'pool-008', name: '[资源池]PM1.0传感器-副本1', type: '传感器', model: 'SD123-E60V2-Copy1', address: '0x31', icon: '💨', color: '#f59e0b', value: 25, unit: 'μg/m³', status: 'online', app: '资源池', lastUpdate: '2024-01-15 10:16:00', trend: 'stable' },
  { id: 'pool-009', name: '[资源池]烟雾报警器-副本1', type: '报警器', model: 'SMK-200-Copy1', address: '0x71', icon: '🔥', color: '#ef4444', value: 0, unit: '', status: 'online', app: '资源池', lastUpdate: '2024-01-15 10:29:00', trend: 'stable' },
  { id: 'pool-010', name: '[资源池]水浸传感器-副本1', type: '传感器', model: 'WTR-100-Copy1', address: '0x91', icon: '💧', color: '#06b6d4', value: 0, unit: 'cm', status: 'online', app: '资源池', lastUpdate: '2024-01-15 10:28:00', trend: 'stable' },
  { id: 'pool-011', name: '[资源池]红外探测器-副本1', type: '传感器', model: 'IRD-300-Copy1', address: '0x41', icon: '👤', color: '#8b5cf6', value: 0, unit: '', status: 'online', app: '资源池', lastUpdate: '2024-01-15 10:27:00', trend: 'stable' },
  { id: 'pool-012', name: '[资源池]弱光检测传感器-副本1', type: '传感器', model: 'LS-100-Copy1', address: '0x51', icon: '💡', color: '#f59e0b', value: 350, unit: 'lux', status: 'online', app: '资源池', lastUpdate: '2024-01-15 10:24:00', trend: 'up' },

  // ===== 资源池设备 (传感器虚拟副本2) =====
  { id: 'pool-013', name: '[资源池]温度传感器-副本2', type: '传感器', model: 'SD123-E60V2-Copy2', address: '0x32', icon: '🌡️', color: '#3b82f6', value: 24.5, unit: '℃', status: 'online', app: '资源池', lastUpdate: '2024-01-15 10:30:00', trend: 'stable' },
  { id: 'pool-014', name: '[资源池]湿度传感器-副本2', type: '传感器', model: 'SD123-E60V2-Copy2', address: '0x32', icon: '💧', color: '#06b6d4', value: 65, unit: '%', status: 'online', app: '资源池', lastUpdate: '2024-01-15 10:30:00', trend: 'down' },
  { id: 'pool-015', name: '[资源池]PM2.5传感器-副本2', type: '传感器', model: 'SD123-E60V2-Copy2', address: '0x32', icon: '💨', color: '#f59e0b', value: 42, unit: 'μg/m³', status: 'online', app: '资源池', lastUpdate: '2024-01-15 10:30:00', trend: 'stable' },
  { id: 'pool-016', name: '[资源池]CO2传感器-副本2', type: '传感器', model: 'SD123-E60V2-Copy2', address: '0x32', icon: '☁️', color: '#8b5cf6', value: 520, unit: 'ppm', status: 'online', app: '资源池', lastUpdate: '2024-01-15 10:30:00', trend: 'up' },
  { id: 'pool-017', name: '[资源池]TVOC传感器-副本2', type: '传感器', model: 'SD123-E60V2-Copy2', address: '0x32', icon: '🧪', color: '#ec4899', value: 120, unit: 'ppb', status: 'online', app: '资源池', lastUpdate: '2024-01-15 10:23:00', trend: 'stable' },
  { id: 'pool-018', name: '[资源池]甲醛传感器-副本2', type: '传感器', model: 'SD123-E60V2-Copy2', address: '0x32', icon: '⚗️', color: '#14b8a6', value: 30, unit: 'ppb', status: 'online', app: '资源池', lastUpdate: '2024-01-15 10:22:00', trend: 'down' },
  { id: 'pool-019', name: '[资源池]PM10传感器-副本2', type: '传感器', model: 'SD123-E60V2-Copy2', address: '0x32', icon: '💨', color: '#f59e0b', value: 68, unit: 'μg/m³', status: 'online', app: '资源池', lastUpdate: '2024-01-15 10:17:00', trend: 'stable' },
  { id: 'pool-020', name: '[资源池]PM1.0传感器-副本2', type: '传感器', model: 'SD123-E60V2-Copy2', address: '0x32', icon: '💨', color: '#f59e0b', value: 25, unit: 'μg/m³', status: 'online', app: '资源池', lastUpdate: '2024-01-15 10:16:00', trend: 'stable' },
  { id: 'pool-021', name: '[资源池]烟雾报警器-副本2', type: '报警器', model: 'SMK-200-Copy2', address: '0x72', icon: '🔥', color: '#ef4444', value: 0, unit: '', status: 'online', app: '资源池', lastUpdate: '2024-01-15 10:29:00', trend: 'stable' },
  { id: 'pool-022', name: '[资源池]水浸传感器-副本2', type: '传感器', model: 'WTR-100-Copy2', address: '0x92', icon: '💧', color: '#06b6d4', value: 0, unit: 'cm', status: 'online', app: '资源池', lastUpdate: '2024-01-15 10:28:00', trend: 'stable' },
  { id: 'pool-023', name: '[资源池]红外探测器-副本2', type: '传感器', model: 'IRD-300-Copy2', address: '0x42', icon: '👤', color: '#8b5cf6', value: 0, unit: '', status: 'online', app: '资源池', lastUpdate: '2024-01-15 10:27:00', trend: 'stable' },
  { id: 'pool-024', name: '[资源池]弱光检测传感器-副本2', type: '传感器', model: 'LS-100-Copy2', address: '0x52', icon: '💡', color: '#f59e0b', value: 350, unit: 'lux', status: 'online', app: '资源池', lastUpdate: '2024-01-15 10:24:00', trend: 'up' }
])

const categories = computed(() => [
  { value: 'all', label: '全部', count: devices.length },
  { value: 'sensor', label: '传感器', count: devices.filter(d => d.type === '传感器').length },
  { value: 'controller', label: '控制器', count: devices.filter(d => d.type === '控制器').length },
  { value: 'alarm', label: '报警器', count: devices.filter(d => d.type === '报警器').length },
  { value: 'pool', label: '资源池', count: devices.filter(d => d.app === '资源池').length }
])

const totalCount = computed(() => devices.length)
const onlineCount = computed(() => devices.filter(d => d.status === 'online').length)
const offlineCount = computed(() => devices.filter(d => d.status === 'offline').length)
const alertCount = computed(() => devices.filter(d => d.value > 100).length)

const filteredDevices = computed(() => {
  let result = devices
  if (activeCategory.value !== 'all') {
    if (activeCategory.value === 'sensor') result = result.filter(d => d.type === '传感器')
    else if (activeCategory.value === 'controller') result = result.filter(d => d.type === '控制器')
    else if (activeCategory.value === 'alarm') result = result.filter(d => d.type === '报警器')
    else if (activeCategory.value === 'pool') result = result.filter(d => d.app === '资源池')
  }
  if (searchQuery.value) {
    const q = searchQuery.value.toLowerCase()
    result = result.filter(d => d.name.toLowerCase().includes(q) || d.type.toLowerCase().includes(q))
  }
  return result
})

function selectDevice(dev: Device) {
  selectedDevice.value = dev
}

function refreshAllDevices() {
  // 模拟刷新所有设备数据
  devices.forEach(dev => {
    if (dev.status === 'online') {
      // 随机波动数据
      const fluctuation = (Math.random() - 0.5) * 2
      dev.value = Math.round((dev.value + fluctuation) * 10) / 10
      dev.lastUpdate = new Date().toLocaleString('zh-CN')
    }
  })
  ElMessage.success('所有设备数据已刷新')
}

function refreshDevice(dev: Device) {
  const index = devices.findIndex(d => d.id === dev.id)
  if (index !== -1) {
    // 模拟刷新数据
    const fluctuation = (Math.random() - 0.5) * 5
    devices[index].value = Math.round((devices[index].value + fluctuation) * 10) / 10
    devices[index].lastUpdate = new Date().toLocaleString('zh-CN')
    selectedDevice.value = devices[index]
    ElMessage.success(`${dev.name} 数据已刷新`)
  }
}

function editDevice(dev: Device) {
  editingDevice.value = dev
  editForm.name = dev.name
  editForm.type = dev.type
  editForm.model = dev.model
  editForm.address = dev.address
  editForm.unit = dev.unit
  editForm.app = dev.app
  showEditDevice.value = true
}

function saveEditDevice() {
  if (!editForm.name || !editForm.model) {
    ElMessage.warning('请填写必填项')
    return
  }

  const index = devices.findIndex(d => d.id === editingDevice.value.id)
  if (index !== -1) {
    devices[index].name = editForm.name
    devices[index].type = editForm.type
    devices[index].model = editForm.model
    devices[index].address = editForm.address
    devices[index].unit = editForm.unit
    devices[index].app = editForm.app
    devices[index].lastUpdate = new Date().toLocaleString('zh-CN')

    // 更新图标和颜色
    if (editForm.type === '传感器') {
      devices[index].icon = '🌡️'
      devices[index].color = '#3b82f6'
    } else if (editForm.type === '控制器') {
      devices[index].icon = '🎮'
      devices[index].color = '#22c55d'
    } else if (editForm.type === '报警器') {
      devices[index].icon = '🔔'
      devices[index].color = '#ef4444'
    }

    selectedDevice.value = devices[index]
    showEditDevice.value = false
    ElMessage.success(`设备 ${editForm.name} 配置已更新`)
  }
}

function toggleDeviceStatus(dev: Device) {
  const index = devices.findIndex(d => d.id === dev.id)
  if (index !== -1) {
    devices[index].status = devices[index].status === 'online' ? 'offline' : 'online'
    devices[index].lastUpdate = new Date().toLocaleString('zh-CN')
    selectedDevice.value = devices[index]
    ElMessage.success(`${dev.name} 已${devices[index].status === 'online' ? '上线' : '下线'}`)
  }
}

function deleteDevice(dev: Device) {
  const index = devices.findIndex(d => d.id === dev.id)
  if (index !== -1) {
    devices.splice(index, 1)
    selectedDevice.value = null
    ElMessage.success(`设备 ${dev.name} 已删除`)
  }
}

function addDevice() {
  if (!newDevice.name || !newDevice.model) {
    ElMessage.warning('请填写必填项')
    return
  }

  const device: Device = {
    id: `dev-${Date.now()}`,
    name: newDevice.name,
    type: newDevice.type,
    model: newDevice.model,
    address: newDevice.address || '0x00',
    icon: newDevice.type === '传感器' ? '🌡️' : newDevice.type === '控制器' ? '🎮' : '🔔',
    color: newDevice.type === '传感器' ? '#3b82f6' : newDevice.type === '控制器' ? '#22c55d' : '#ef4444',
    value: 0,
    unit: newDevice.unit,
    status: 'online',
    app: newDevice.app,
    lastUpdate: new Date().toLocaleString('zh-CN'),
    trend: 'stable'
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

function exportDevices() {
  const config = devices.map(d => ({
    id: d.id,
    name: d.name,
    type: d.type,
    model: d.model,
    address: d.address,
    unit: d.unit,
    app: d.app,
    status: d.status
  }))

  const blob = new Blob([JSON.stringify(config, null, 2)], { type: 'application/json' })
  const url = URL.createObjectURL(blob)
  const a = document.createElement('a')
  a.href = url
  a.download = 'devices-config.json'
  a.click()
  URL.revokeObjectURL(url)
  ElMessage.success('设备配置已导出')
}
</script>

<style scoped>
.devices-root { min-height: 100vh; background: var(--bg-primary); position: relative; }
.devices-main { position: relative; z-index: 1; padding: var(--spacing-xl); max-width: 1920px; margin: 0 auto; }

.section-header { display: flex; align-items: center; justify-content: space-between; margin-bottom: var(--spacing-lg); }
.section-title { display: flex; align-items: center; gap: var(--spacing-sm); font-size: 20px; font-weight: 600; color: var(--text-primary); margin: 0; }
.section-title svg { color: var(--accent-primary); }
.header-actions { display: flex; gap: var(--spacing-sm); }

.stats-grid { display: grid; grid-template-columns: repeat(4, 1fr); gap: var(--spacing-lg); margin-bottom: var(--spacing-xl); }
.stat-card { display: flex; align-items: center; gap: var(--spacing-md); padding: var(--spacing-lg); background: var(--bg-card); border: 1px solid var(--border-primary); border-radius: var(--radius-lg); }
.stat-icon { width: 48px; height: 48px; display: flex; align-items: center; justify-content: center; border-radius: var(--radius-md); font-size: 24px; }
.stat-value { font-size: 28px; font-weight: 700; color: var(--text-primary); font-family: var(--font-mono); }
.stat-label { font-size: 13px; color: var(--text-secondary); }

.filter-section { margin-bottom: var(--spacing-xl); }
.filter-bar { display: flex; align-items: center; justify-content: space-between; }
.filter-tabs { display: flex; gap: var(--spacing-xs); }
.filter-tab { display: flex; align-items: center; gap: 6px; padding: 8px 16px; background: transparent; border: 1px solid var(--border-primary); border-radius: var(--radius-full); color: var(--text-secondary); font-size: 13px; cursor: pointer; transition: all var(--duration-fast); }
.filter-tab:hover { border-color: var(--accent-primary); color: var(--text-primary); }
.filter-tab.active { background: rgba(59,130,246,0.2); border-color: var(--accent-primary); color: var(--accent-primary); }
.tab-count { display: inline-flex; align-items: center; justify-content: center; min-width: 20px; height: 20px; padding: 0 4px; background: rgba(255,255,255,0.1); border-radius: 10px; font-size: 11px; }
.search-box { display: flex; align-items: center; gap: 8px; padding: 8px 12px; background: var(--bg-card); border: 1px solid var(--border-primary); border-radius: var(--radius-md); }
.search-box svg { color: var(--text-tertiary); }
.search-input { background: transparent; border: none; outline: none; color: var(--text-primary); font-size: 14px; width: 200px; }

.devices-grid { display: grid; grid-template-columns: repeat(auto-fill, minmax(280px, 1fr)); gap: var(--spacing-lg); }

.device-card { background: var(--bg-card); border: 1px solid var(--border-primary); border-radius: var(--radius-lg); overflow: hidden; transition: all var(--duration-fast); cursor: pointer; }
.device-card:hover { transform: translateY(-4px); box-shadow: var(--shadow-lg); border-color: var(--border-accent); }
.device-card.offline { opacity: 0.6; }
.device-card.selected { border-color: var(--accent-primary); box-shadow: 0 0 20px rgba(59,130,246,0.3); }

.card-header { display: flex; align-items: center; justify-content: space-between; padding: var(--spacing-md); }
.device-icon { width: 48px; height: 48px; display: flex; align-items: center; justify-content: center; border-radius: var(--radius-md); font-size: 24px; }
.device-status { display: flex; align-items: center; gap: 6px; font-size: 12px; padding: 4px 8px; border-radius: var(--radius-full); }
.device-status.online { background: rgba(34,197,94,0.2); color: #22c55d; }
.device-status.offline { background: rgba(100,116,139,0.2); color: #64748b; }
.status-dot { width: 8px; height: 8px; border-radius: 50%; }
.device-status.online .status-dot { background: #22c55d; box-shadow: 0 0 8px #22c55d; }
.device-status.offline .status-dot { background: #64748b; }

.card-body { padding: 0 var(--spacing-md) var(--spacing-md); }
.device-name { font-size: 16px; font-weight: 600; color: var(--text-primary); margin: 0 0 4px; }
.device-type { font-size: 13px; color: var(--text-secondary); margin: 0 0 8px; }
.device-meta { display: flex; gap: var(--spacing-md); margin-bottom: var(--spacing-sm); }
.meta-item { font-size: 11px; color: var(--text-tertiary); }
.device-value { display: flex; align-items: baseline; gap: 4px; }
.value-number { font-size: 24px; font-weight: 700; color: var(--text-primary); font-family: var(--font-mono); }
.value-unit { font-size: 14px; color: var(--text-secondary); }

.card-footer { display: flex; gap: var(--spacing-xs); padding: var(--spacing-sm) var(--spacing-md) var(--spacing-md); }
.action-btn { flex: 1; padding: 6px 12px; background: transparent; border: 1px solid var(--border-primary); border-radius: var(--radius-sm); color: var(--text-secondary); font-size: 12px; cursor: pointer; transition: all var(--duration-fast); }
.action-btn:hover { border-color: var(--accent-primary); color: var(--accent-primary); }
.action-btn.danger:hover { border-color: var(--status-danger); color: var(--status-danger); }

.detail-panel { position: fixed; top: 0; right: 0; width: 400px; height: 100vh; z-index: var(--z-overlay); padding: var(--spacing-md); overflow-y: auto; }
.detail-card { background: var(--bg-card); border: 1px solid var(--border-primary); border-radius: var(--radius-lg); height: 100%; display: flex; flex-direction: column; }
.detail-header { display: flex; align-items: center; gap: var(--spacing-md); padding: var(--spacing-lg); border-bottom: 1px solid var(--border-primary); }
.detail-icon { width: 56px; height: 56px; display: flex; align-items: center; justify-content: center; border-radius: var(--radius-md); font-size: 28px; }
.detail-info { flex: 1; }
.detail-name { font-size: 18px; font-weight: 600; color: var(--text-primary); margin: 0; }
.detail-type { font-size: 13px; color: var(--text-secondary); margin: 4px 0 0; }
.close-btn { width: 32px; height: 32px; display: flex; align-items: center; justify-content: center; background: transparent; border: none; color: var(--text-secondary); font-size: 20px; cursor: pointer; border-radius: var(--radius-sm); }
.close-btn:hover { background: rgba(255,255,255,0.1); color: var(--text-primary); }

.detail-content { flex: 1; overflow-y: auto; padding: var(--spacing-lg); }
.detail-section { margin-bottom: var(--spacing-lg); }
.detail-section .section-title { font-size: 12px; font-weight: 600; color: var(--text-secondary); text-transform: uppercase; letter-spacing: 0.5px; margin-bottom: var(--spacing-sm); padding-bottom: var(--spacing-xs); border-bottom: 1px solid var(--border-primary); }
.detail-grid { display: grid; grid-template-columns: repeat(2, 1fr); gap: var(--spacing-sm); }
.detail-item { display: flex; flex-direction: column; gap: 4px; }
.detail-label { font-size: 12px; color: var(--text-tertiary); }
.detail-value { font-size: 14px; color: var(--text-primary); font-weight: 500; }
.detail-value.online { color: #22c55d; }
.detail-value.offline { color: #ef4444; }

.data-display { text-align: center; padding: var(--spacing-lg); }
.data-value-large { font-size: 48px; font-weight: 700; color: var(--text-primary); font-family: var(--font-mono); }
.data-unit { font-size: 18px; color: var(--text-secondary); margin-left: 8px; }
.data-trend { font-size: 14px; margin-top: var(--spacing-sm); }
.data-trend.up { color: #ef4444; }
.data-trend.down { color: #3b82f6; }
.data-trend.stable { color: #22c55d; }

.detail-actions { display: flex; flex-wrap: wrap; gap: var(--spacing-sm); }

.modal-overlay { position: fixed; inset: 0; background: rgba(0,0,0,0.7); backdrop-filter: blur(8px); display: flex; align-items: center; justify-content: center; z-index: var(--z-modal); }
.modal-content { background: var(--bg-card); border: 1px solid var(--border-primary); border-radius: var(--radius-xl); width: 90%; max-width: 600px; }
.modal-header { display: flex; justify-content: space-between; align-items: center; padding: var(--spacing-lg); border-bottom: 1px solid var(--border-primary); }
.modal-header h3 { font-size: 18px; font-weight: 600; color: var(--text-primary); margin: 0; }
.modal-close { width: 32px; height: 32px; display: flex; align-items: center; justify-content: center; background: transparent; border: none; color: var(--text-secondary); font-size: 20px; cursor: pointer; }
.modal-body { padding: var(--spacing-lg); }
.modal-footer { display: flex; justify-content: flex-end; gap: var(--spacing-sm); padding: var(--spacing-lg); border-top: 1px solid var(--border-primary); }

.form-grid { display: grid; grid-template-columns: repeat(2, 1fr); gap: var(--spacing-md); }
.form-group { display: flex; flex-direction: column; gap: 6px; }
.form-group label { font-size: 13px; color: var(--text-secondary); }
.form-group input, .form-group select { padding: 10px 12px; background: var(--bg-primary); border: 1px solid var(--border-primary); border-radius: var(--radius-md); color: var(--text-primary); font-size: 14px; outline: none; }
.form-group input:focus, .form-group select:focus { border-color: var(--accent-primary); }

@media (max-width: 1200px) { .stats-grid { grid-template-columns: repeat(2, 1fr); } .form-grid { grid-template-columns: 1fr; } }
@media (max-width: 768px) { .devices-main { padding: var(--spacing-md); } .stats-grid { grid-template-columns: 1fr; } .filter-bar { flex-direction: column; gap: var(--spacing-md); } .detail-panel { width: 100%; } }
</style>
