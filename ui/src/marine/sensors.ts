export interface SensorValue {
  label: string
  value: number
  unit: string
  online: boolean
}

export interface SensorSnapshot {
  source: 'live' | 'demo'
  updatedAt: string
  values: Record<string, SensorValue>
}

export const sensorDefinitions: Record<string, Omit<SensorValue, 'value' | 'online'>> = {
  temperature: { label: '温度', unit: '℃' },
  humidity: { label: '湿度', unit: '%' },
  pm25: { label: 'PM2.5', unit: 'μg/m³' },
  pm10: { label: 'PM10', unit: 'μg/m³' },
  tvoc: { label: 'TVOC', unit: 'ppb' },
  ch2o: { label: '甲醛', unit: 'ppb' },
  co2: { label: 'CO₂', unit: 'ppm' },
  smoke: { label: '烟雾', unit: '' },
  water: { label: '水浸', unit: '' },
  ir: { label: '红外', unit: '' },
  radar: { label: '雷达', unit: '' },
  lux: { label: '光照', unit: 'lux' },
}

const demoValues: Record<string, number> = {
  temperature: 24.6,
  humidity: 62,
  pm25: 31,
  pm10: 54,
  tvoc: 86,
  ch2o: 24,
  co2: 516,
  smoke: 0,
  water: 0,
  ir: 0,
  radar: 0,
  lux: 328,
}

export const sensorServiceKeys: Record<string, string[]> = {
  A01: ['temperature', 'humidity'],
  A02: ['pm25', 'pm10'],
  A03: ['tvoc', 'ch2o', 'co2'],
  A04: ['smoke'],
  A05: ['water'],
  A06: ['ir', 'radar'],
  A07: ['lux'],
}

export function demoSensorSnapshot(): SensorSnapshot {
  return {
    source: 'demo',
    updatedAt: new Date().toISOString(),
    values: Object.fromEntries(Object.entries(sensorDefinitions).map(([key, definition]) => [
      key,
      { ...definition, value: demoValues[key] ?? 0, online: true },
    ])),
  }
}

export function normalizeSensorSnapshot(payload: unknown): SensorSnapshot {
  if (!payload || typeof payload !== 'object') throw new Error('传感器数据格式不正确')
  const raw = payload as Record<string, unknown>
  const values: Record<string, SensorValue> = {}

  Object.entries(sensorDefinitions).forEach(([key, definition]) => {
    const rawValue = Number(raw[key])
    const divisor = key === 'temperature' || key === 'humidity' ? 10 : 1
    const onlineKey = key === 'radar' ? 'ir_online' : key === 'lux' ? 'light_online' : `${key}_online`
    values[key] = {
      ...definition,
      value: Number.isFinite(rawValue) ? Math.round(rawValue / divisor * 10) / 10 : 0,
      online: Boolean(raw[onlineKey]),
    }
  })

  return { source: 'live', updatedAt: new Date().toISOString(), values }
}

export function sensorServiceSummary(serviceId: string, snapshot: SensorSnapshot) {
  const readings = (sensorServiceKeys[serviceId] || []).map(key => snapshot.values[key]).filter(Boolean)
  const online = readings.filter(reading => reading.online)
  const sourceLabel = snapshot.source === 'live' ? '实时设备数据' : '演示数据'

  if (!readings.length) throw new Error('未找到感知服务的数据定义')
  if (!online.length) {
    return { metric: '数据状态', value: '设备离线', unit: '', detail: `本服务关联的 ${readings.map(item => item.label).join('、')} 当前均离线`, source: sourceLabel }
  }

  if (serviceId === 'A01') return combined('温湿度', online, sourceLabel)
  if (serviceId === 'A02') return combined('颗粒物', online, sourceLabel)
  if (serviceId === 'A03') return combined('气体环境', online, sourceLabel)
  if (serviceId === 'A04') return stateResult('烟雾状态', online[0]!.value > 0, '检测到烟雾', '环境正常', sourceLabel)
  if (serviceId === 'A05') return stateResult('水浸状态', online[0]!.value > 0, '检测到水浸', '舱底正常', sourceLabel)
  if (serviceId === 'A06') return stateResult('人员活动', online.some(item => item.value > 0), '检测到活动', '区域无人', sourceLabel)
  return { metric: '光照强度', value: online[0]!.value, unit: online[0]!.unit, detail: `${online[0]!.label}数据已读取`, source: sourceLabel }
}

function combined(metric: string, readings: SensorValue[], source: string) {
  const display = readings.map(item => `${item.label} ${item.value}${item.unit}`).join(' / ')
  return { metric, value: display, unit: '', detail: `已汇聚 ${readings.length} 路在线数据`, source }
}

function stateResult(metric: string, alarm: boolean, alarmText: string, normalText: string, source: string) {
  return { metric, value: alarm ? alarmText : normalText, unit: '', detail: alarm ? '状态数据触发关注条件' : '状态数据未触发关注条件', source }
}
