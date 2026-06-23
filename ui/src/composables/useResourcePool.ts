import { ref, computed } from 'vue'

export interface DataPoint {
  id: string
  deviceId: string
  deviceName: string
  registerName: string
  label: string
  value: number
  unit: string
  timestamp: number
  status: 'normal' | 'warning' | 'danger' | 'offline'
  category: 'sensor' | 'environment' | 'security' | 'fire'
}

export interface DataBinding {
  id: string
  sourceId: string
  targetComponentId: string
  transform?: (value: number) => number
  format?: (value: number) => string
}

export interface DataSource {
  id: string
  deviceId: string
  deviceName: string
  registerName: string
  label: string
  unit: string
  category: string
  icon: string
  color: string
  currentValue: number
  status: 'online' | 'offline' | 'warning' | 'danger'
}

// 全局数据池
const dataPool = ref<Map<string, DataPoint>>(new Map())
const bindings = ref<DataBinding[]>([])
const subscribers = ref<Map<string, Set<(data: DataPoint) => void>>>(new Map())

export function useResourcePool() {
  // 获取所有数据点
  const allDataPoints = computed(() => {
    return Array.from(dataPool.value.values())
  })

  // 按分类获取数据点
  const dataByCategory = computed(() => {
    const categories: Record<string, DataPoint[]> = {
      sensor: [],
      environment: [],
      security: [],
      fire: []
    }

    dataPool.value.forEach(point => {
      if (categories[point.category]) {
        categories[point.category].push(point)
      }
    })

    return categories
  })

  // 获取数据源列表（用于UI展示）
  const dataSources = computed<DataSource[]>(() => {
    return Array.from(dataPool.value.values()).map(point => ({
      id: point.id,
      deviceId: point.deviceId,
      deviceName: point.deviceName,
      registerName: point.registerName,
      label: point.label,
      unit: point.unit,
      category: point.category,
      icon: getIconForCategory(point.category),
      color: getColorForStatus(point.status),
      currentValue: point.value,
      status: point.status === 'offline' ? 'offline' :
              point.status === 'danger' ? 'danger' :
              point.status === 'warning' ? 'warning' : 'online'
    }))
  })

  // 发布数据更新
  function publish(data: Omit<DataPoint, 'id' | 'timestamp'>) {
    const id = `${data.deviceId}.${data.registerName}`
    const dataPoint: DataPoint = {
      ...data,
      id,
      timestamp: Date.now()
    }

    dataPool.value.set(id, dataPoint)

    // 通知订阅者
    const subs = subscribers.value.get(id)
    if (subs) {
      subs.forEach(callback => callback(dataPoint))
    }

    // 通知全局订阅者
    const globalSubs = subscribers.value.get('*')
    if (globalSubs) {
      globalSubs.forEach(callback => callback(dataPoint))
    }
  }

  // 订阅数据更新
  function subscribe(
    sourceId: string,
    callback: (data: DataPoint) => void
  ): () => void {
    if (!subscribers.value.has(sourceId)) {
      subscribers.value.set(sourceId, new Set())
    }

    subscribers.value.get(sourceId)!.add(callback)

    // 返回取消订阅函数
    return () => {
      const subs = subscribers.value.get(sourceId)
      if (subs) {
        subs.delete(callback)
        if (subs.size === 0) {
          subscribers.value.delete(sourceId)
        }
      }
    }
  }

  // 订阅所有数据更新
  function subscribeAll(callback: (data: DataPoint) => void): () => void {
    return subscribe('*', callback)
  }

  // 获取单个数据点
  function getData(sourceId: string): DataPoint | undefined {
    return dataPool.value.get(sourceId)
  }

  // 获取多个数据点
  function getDataBatch(sourceIds: string[]): DataPoint[] {
    return sourceIds
      .map(id => dataPool.value.get(id))
      .filter((d): d is DataPoint => d !== undefined)
  }

  // 添加数据绑定
  function addBinding(binding: Omit<DataBinding, 'id'>): string {
    const id = `binding_${Date.now()}_${Math.random().toString(36).substr(2, 9)}`
    bindings.value.push({ ...binding, id })
    return id
  }

  // 移除数据绑定
  function removeBinding(bindingId: string) {
    const index = bindings.value.findIndex(b => b.id === bindingId)
    if (index !== -1) {
      bindings.value.splice(index, 1)
    }
  }

  // 获取组件的数据绑定
  function getBindingsForComponent(componentId: string): DataBinding[] {
    return bindings.value.filter(b => b.targetComponentId === componentId)
  }

  // 清除所有绑定
  function clearBindings() {
    bindings.value = []
  }

  // 计算聚合数据
  function aggregate(
    sourceIds: string[],
    operation: 'avg' | 'sum' | 'min' | 'max' | 'count'
  ): number {
    const points = getDataBatch(sourceIds)
    if (points.length === 0) return 0

    const values = points.map(p => p.value)

    switch (operation) {
      case 'avg':
        return values.reduce((a, b) => a + b, 0) / values.length
      case 'sum':
        return values.reduce((a, b) => a + b, 0)
      case 'min':
        return Math.min(...values)
      case 'max':
        return Math.max(...values)
      case 'count':
        return values.length
      default:
        return 0
    }
  }

  // 导出配置
  function exportConfig(): string {
    return JSON.stringify({
      bindings: bindings.value,
      timestamp: Date.now()
    }, null, 2)
  }

  // 导入配置
  function importConfig(configJson: string): boolean {
    try {
      const config = JSON.parse(configJson)
      if (config.bindings && Array.isArray(config.bindings)) {
        bindings.value = config.bindings
        return true
      }
      return false
    } catch {
      return false
    }
  }

  // 清空数据池
  function clearPool() {
    dataPool.value.clear()
    subscribers.value.clear()
  }

  // 辅助函数：根据分类获取图标
  function getIconForCategory(category: string): string {
    const icons: Record<string, string> = {
      sensor: '<circle cx="12" cy="12" r="3"/><path d="M12 1v6m0 6v6m-7-7h6m6 0h6"/>',
      environment: '<path d="M12 2.69l5.66 5.66a8 8 0 1 1-11.31 0z"/>',
      security: '<path d="M12 22s8-4 8-10V5l-8-3-8 3v7c0 6 8 10 8 10z"/>',
      fire: '<path d="M8.5 14.5A2.5 2.5 0 0 0 11 12c0-1.38-.5-2-1-3-1.072-2.143-.224-4.054 2-6 .5 2.5 2 4.9 4 6.5 2 1.6 3 3.5 3 5.5a7 7 0 1 1-14 0c0-1.153.433-2.294 1-3a2.5 2.5 0 0 0 2.5 2.5z"/>'
    }
    return icons[category] || icons.sensor
  }

  // 辅助函数：根据状态获取颜色
  function getColorForStatus(status: string): string {
    const colors: Record<string, string> = {
      normal: 'blue',
      warning: 'yellow',
      danger: 'red',
      offline: 'gray'
    }
    return colors[status] || 'blue'
  }

  return {
    dataPool,
    allDataPoints,
    dataByCategory,
    dataSources,
    bindings,
    publish,
    subscribe,
    subscribeAll,
    getData,
    getDataBatch,
    addBinding,
    removeBinding,
    getBindingsForComponent,
    clearBindings,
    aggregate,
    exportConfig,
    importConfig,
    clearPool
  }
}
