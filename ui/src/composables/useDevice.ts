import { ref, computed, onMounted, onUnmounted } from 'vue'
import axios from 'axios'

export interface Device {
  type: string
  name: string
  category: 'sensor' | 'alarm' | 'controller'
  online: boolean
  value?: number
  unit?: string
  description?: string
  status?: string
  lastUpdate?: Date
}

export interface DeviceConfig {
  type: string
  name: string
  category: 'sensor' | 'alarm' | 'controller'
  model: string
  protocol: string
  address: number
  unit: string
  description: string
  registers: DeviceRegister[]
  thresholds?: DeviceThreshold[]
}

export interface DeviceRegister {
  name: string
  address: number
  dataType: string
  scaleFactor: number
  offset: number
  readOnly: boolean
}

export interface DeviceThreshold {
  registerName: string
  minValue?: number
  maxValue?: number
  alarmLevel: 'info' | 'warning' | 'danger'
}

export function useDevice() {
  const devices = ref<Device[]>([])
  const loading = ref(false)
  const error = ref<string | null>(null)
  let pollInterval: ReturnType<typeof setInterval> | null = null

  // 设备分类
  const sensors = computed(() => devices.value.filter(d => d.category === 'sensor'))
  const alarms = computed(() => devices.value.filter(d => d.category === 'alarm'))
  const controllers = computed(() => devices.value.filter(d => d.category === 'controller'))

  // 在线统计
  const onlineCount = computed(() => devices.value.filter(d => d.online).length)
  const totalCount = computed(() => devices.value.length)
  const onlineRate = computed(() => {
    if (totalCount.value === 0) return 0
    return Math.round((onlineCount.value / totalCount.value) * 100)
  })

  // 获取设备列表
  async function fetchDevices() {
    loading.value = true
    error.value = null

    try {
      const response = await axios.get('/api/devices')
      devices.value = response.data.devices || response.data || []
    } catch (err: any) {
      error.value = err.message || '获取设备列表失败'
      console.error('获取设备列表失败:', err)
    } finally {
      loading.value = false
    }
  }

  // 获取设备详情
  async function getDeviceDetail(type: string): Promise<Device | null> {
    try {
      const response = await axios.get(`/api/devices/${type}`)
      return response.data
    } catch (err: any) {
      console.error('获取设备详情失败:', err)
      return null
    }
  }

  // 控制设备
  async function controlDevice(
    device: string,
    action: string,
    value?: string | number
  ): Promise<boolean> {
    try {
      const params: any = { device, action }
      if (value !== undefined) {
        params.val = value
      }
      await axios.get('/api/control', { params })
      return true
    } catch (err: any) {
      console.error('设备控制失败:', err)
      return false
    }
  }

  // 添加设备（需要后端支持）
  async function addDevice(config: DeviceConfig): Promise<boolean> {
    try {
      await axios.post('/api/device-db/add', config)
      await fetchDevices() // 刷新列表
      return true
    } catch (err: any) {
      console.error('添加设备失败:', err)
      return false
    }
  }

  // 更新设备配置（需要后端支持）
  async function updateDevice(type: string, config: Partial<DeviceConfig>): Promise<boolean> {
    try {
      await axios.put(`/api/device-db/update?id=${type}`, config)
      await fetchDevices() // 刷新列表
      return true
    } catch (err: any) {
      console.error('更新设备失败:', err)
      return false
    }
  }

  // 删除设备（需要后端支持）
  async function deleteDevice(type: string): Promise<boolean> {
    try {
      await axios.delete(`/api/device-db/delete?id=${type}`)
      await fetchDevices() // 刷新列表
      return true
    } catch (err: any) {
      console.error('删除设备失败:', err)
      return false
    }
  }

  // 启动轮询
  function startPolling(interval: number = 5000) {
    stopPolling()
    pollInterval = setInterval(fetchDevices, interval)
  }

  // 停止轮询
  function stopPolling() {
    if (pollInterval) {
      clearInterval(pollInterval)
      pollInterval = null
    }
  }

  // 获取设备状态颜色
  function getDeviceStatusColor(device: Device): string {
    if (!device.online) return 'var(--text-tertiary)'
    if (device.status === 'alarm') return 'var(--status-danger)'
    if (device.status === 'warning') return 'var(--status-warning)'
    return 'var(--status-success)'
  }

  // 获取设备状态文本
  function getDeviceStatusText(device: Device): string {
    if (!device.online) return '离线'
    if (device.status === 'alarm') return '报警'
    if (device.status === 'warning') return '预警'
    return '正常'
  }

  // 格式化设备值
  function formatDeviceValue(device: Device): string {
    if (!device.online || device.value === undefined) return '--'
    return `${device.value.toFixed(1)}${device.unit || ''}`
  }

  onMounted(() => {
    fetchDevices()
  })

  onUnmounted(() => {
    stopPolling()
  })

  return {
    devices,
    loading,
    error,
    sensors,
    alarms,
    controllers,
    onlineCount,
    totalCount,
    onlineRate,
    fetchDevices,
    getDeviceDetail,
    controlDevice,
    addDevice,
    updateDevice,
    deleteDevice,
    startPolling,
    stopPolling,
    getDeviceStatusColor,
    getDeviceStatusText,
    formatDeviceValue
  }
}
