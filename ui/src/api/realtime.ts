import { cppClient } from './client'

export interface DeviceValue {
  name: string
  type: string
  online: boolean
  value: number
  unit: string
  description: string
  category: string
}

export interface AllSensorData {
  pm25: number; pm25_online: boolean
  pm10: number; pm10_online: boolean
  humidity: number; humidity_online: boolean
  temperature: number; temperature_online: boolean
  tvoc: number; tvoc_online: boolean
  ch2o: number; ch2o_online: boolean
  co2: number; co2_online: boolean
  smoke: number; smoke_online: boolean
  water: number; water_online: boolean
  ir: number; radar: number; ir_online: boolean
  lux: number; light_online: boolean
  ac_online: boolean
  humidifier_power: number; humidifier_fault: number; humidifier_online: boolean
  purifier_power: number; purifier_run_mode: number; purifier_online: boolean
}

export interface BusStats {
  totalTransactions: number
  totalErrors: number
  bytesSent: number
  bytesRecv: number
  avgLatencyMs: number
  maxLatencyMs: number
  busContentionCount: number
}

export const realtimeApi = {
  // 获取全部设备实时数据
  getAllData(): Promise<{ success: boolean; data: AllSensorData }> {
    return cppClient.get('/data') as any
  },

  // 获取设备列表
  getDevices(): Promise<{ success: boolean; data: DeviceValue[] }> {
    return cppClient.get('/devices') as any
  },

  // 发送设备控制指令
  control(device: string, action: string, val?: number): Promise<{ status: string }> {
    let url = `/control?device=${device}&action=${action}`
    if (val !== undefined) url += `&val=${val}`
    return cppClient.get(url) as any
  },

  // 获取总线统计
  getBusStats(): Promise<BusStats> {
    return cppClient.get('/bus/stats') as any
  },

  // 获取轮询配置
  getPollingConfig(): Promise<{ success: boolean; data: any[] }> {
    return cppClient.get('/polling/config') as any
  },

  // 设置轮询间隔
  setPollingInterval(group: string, interval: number, timeout?: number): Promise<{ ok: boolean }> {
    let url = `/polling/set?group=${encodeURIComponent(group)}&interval=${interval}`
    if (timeout) url += `&timeout=${timeout}`
    return cppClient.get(url) as any
  }
}
