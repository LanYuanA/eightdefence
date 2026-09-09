export interface MarineService {
  id: string; name: string; category: string; description: string; color: string
  available: boolean; kind: 'awareness' | 'action'; resource?: string; metric?: string; unit?: string; target?: number
}
export const services: MarineService[] = [
  { id: 'A01', name: '环境温湿感知', category: '态势感知', description: '汇聚温度与湿度，形成舱室环境状态', color: '#4ed7c8', available: true, kind: 'awareness' },
  { id: 'A02', name: '颗粒物感知', category: '态势感知', description: '汇聚 PM2.5 与 PM10，形成空气颗粒物状态', color: '#58c4e8', available: true, kind: 'awareness' },
  { id: 'A03', name: '气体环境感知', category: '态势感知', description: '汇聚 TVOC、甲醛与 CO₂ 数据', color: '#7f9cff', available: true, kind: 'awareness' },
  { id: 'A04', name: '烟雾火情感知', category: '态势感知', description: '读取烟雾状态，输出火情关注结果', color: '#ef8d78', available: true, kind: 'awareness' },
  { id: 'A05', name: '舱底水浸感知', category: '态势感知', description: '读取水浸状态，输出舱底积水风险', color: '#62b7ff', available: true, kind: 'awareness' },
  { id: 'A06', name: '舱室活动感知', category: '态势感知', description: '融合红外与雷达，判断人员活动状态', color: '#ac98ff', available: true, kind: 'awareness' },
  { id: 'A07', name: '舱室光照感知', category: '态势感知', description: '读取光照强度，输出区域照明状态', color: '#e8c66f', available: true, kind: 'awareness' },
  { id: '01', name: '舱室通风', category: '执行控制', description: '按目标强度完成指定区域换气', color: '#66dfce', available: true, kind: 'action', resource: '通风执行器', metric: '换气进度', unit: '%', target: 100 },
  { id: '02', name: '冷却循环', category: '执行控制', description: '建立并维持稳定冷却循环', color: '#75b6ff', available: true, kind: 'action', resource: '冷却执行器', metric: '循环流量', unit: 'm³/h', target: 48 },
  { id: '03', name: '供水增压', category: '执行控制', description: '按作业需求建立稳定供水', color: '#ae9bff', available: true, kind: 'action', resource: '水务执行器', metric: '供水压力', unit: 'MPa', target: 0.5 },
  { id: '04', name: '舱底排水', category: '执行控制', description: '完成指定区域的积水排放', color: '#f1be74', available: true, kind: 'action', resource: '水务执行器', metric: '剩余水位', unit: 'cm', target: 5 },
  { id: '05', name: '检修安全报警', category: '执行控制', description: '按检修安全策略触发声光提示与报警联动', color: '#ef8d78', available: true, kind: 'action', resource: '安全报警执行器', metric: '报警状态', unit: '', target: 1 },
  { id: '06', name: '压载调节', category: '扩展能力', description: '按目标调整压载分配', color: '#75b6ff', available: false, kind: 'action' },
  { id: '07', name: '消防供水', category: '扩展能力', description: '建立消防供水能力', color: '#f1be74', available: false, kind: 'action' },
  { id: '08', name: '应急通风', category: '扩展能力', description: '执行应急区域换气', color: '#66dfce', available: false, kind: 'action' },
  { id: '09', name: '舱门控制', category: '扩展能力', description: '执行指定舱门开闭', color: '#ae9bff', available: false, kind: 'action' },
  { id: '10', name: '作业照明', category: '扩展能力', description: '按区域调整照明', color: '#f1be74', available: false, kind: 'action' },
  { id: '11', name: '岸电切换', category: '扩展能力', description: '执行供电模式切换', color: '#75b6ff', available: false, kind: 'action' },
  { id: '12', name: '负载管理', category: '扩展能力', description: '按优先级调配用电负载', color: '#75b6ff', available: false, kind: 'action' },
  { id: '13', name: '系泊收放', category: '扩展能力', description: '执行系泊收放任务', color: '#ae9bff', available: false, kind: 'action' },
  { id: '14', name: '锚机收放', category: '扩展能力', description: '执行起锚与放锚任务', color: '#f1be74', available: false, kind: 'action' },
]
export const serviceById = (id: string) => services.find(service => service.id === id)!
export const areas = ['机舱', '生活舱', '作业舱', '全船']
export interface TaskNode { id: string; serviceId: string; area: string; intensity: number; duration: number }
export interface TaskStep { id: string; nodes: TaskNode[] }
export interface MarineApp { id: string; name: string; description: string; steps: TaskStep[] }
export function createId(randomUUID?: () => string, random = Math.random, now = Date.now) {
  if (randomUUID) return randomUUID()
  const time = now().toString(36)
  const entropy = Math.floor(random() * 36 ** 8).toString(36).padStart(8, '0')
  return `marine-${time}-${entropy}`
}
export function uid() {
  const randomUUID = typeof globalThis.crypto?.randomUUID === 'function'
    ? () => globalThis.crypto.randomUUID()
    : undefined
  return createId(randomUUID)
}
export function createNode(serviceId: string): TaskNode {
  const service = serviceById(serviceId)
  return { id: uid(), serviceId, area: '机舱', intensity: 70, duration: service?.kind === 'awareness' ? 3 : 6 }
}
export function createStep(ids: string[] = []): TaskStep { return { id: uid(), nodes: ids.map(createNode) } }
export function createPreset(which: 'A' | 'B' | 'C' | 'D' | 'E'): MarineApp {
  if (which === 'A') return { id: uid(), name: '开航辅助保障', description: '先汇聚环境态势，再完成通风准备，并行建立冷却与供水，为开航提供辅助保障。', steps: [createStep(['A01', 'A02', 'A03']), createStep(['01']), createStep(['02', '03'])] }
  if (which === 'B') return { id: uid(), name: '作业后恢复保障', description: '读取舱底水浸态势，并行完成冷却与供水保障，随后执行舱底排水。', steps: [createStep(['A05']), createStep(['02', '03']), createStep(['04'])] }
  if (which === 'C') return { id: uid(), name: '全船安全巡检', description: '并行复用全部态势感知能力，一次完成环境、消防、安防和舱底状态巡检。', steps: [createStep(['A01', 'A02', 'A03']), createStep(['A04', 'A05', 'A06', 'A07'])] }
  if (which === 'D') return { id: uid(), name: '夜间机舱检修保障', description: '靠港夜间检修期间，为机舱作业提供人员、照度、火情、水浸、通风、报警和排水保障。', steps: [createStep(['A06', 'A07', 'A04', 'A05']), createStep(['01']), createStep(['05']), createStep(['04'])] }
  return { id: uid(), name: '检修异常安全处置', description: '检修过程中发生烟雾或舱底积水时，执行报警、通风安全策略与排水处置。', steps: [createStep(['A04', 'A05', 'A06']), createStep(['05', '01']), createStep(['04'])] }
}
export function createLeadershipDemo() {
  return {
    apps: [createPreset('D'), createPreset('E')] as [MarineApp, MarineApp],
    reusedServiceIds: ['01', '04', '05'],
    newServiceCount: 0,
  }
}
export function validateApp(app: MarineApp): string[] {
  const errors: string[] = []
  if (!app.name.trim()) errors.push('请填写应用名称。')
  if (!app.steps.length) errors.push('请先添加至少一个步骤。')
  const nodeIds = new Set<string>()
  app.steps.forEach((step, index) => {
    if (!step.nodes.length) errors.push(`步骤 ${index + 1} 还没有服务。`)
    const resources = new Set<string>()
    step.nodes.forEach(node => {
      const service = serviceById(node.serviceId)
      if (!service?.available) { errors.push(`步骤 ${index + 1} 包含尚未实现的服务。`); return }
      if (nodeIds.has(node.id)) errors.push('存在重复的服务实例。')
      nodeIds.add(node.id)
      if (!areas.includes(node.area) || !Number.isFinite(node.intensity) || node.intensity < 10 || node.intensity > 100 || !Number.isFinite(node.duration) || node.duration < 2 || node.duration > 30) errors.push(`${service.name} 的参数不完整或超出范围。`)
      if (service.resource && resources.has(service.resource)) errors.push(`步骤 ${index + 1} 的服务共用${service.resource}，请拆成顺序步骤。`)
      if (service.resource) resources.add(service.resource)
    })
  })
  return [...new Set(errors)]
}
export function plannedDuration(app: MarineApp) { return app.steps.reduce((sum, step) => sum + Math.max(0, ...step.nodes.map(node => node.duration)), 0) }
export function parseSavedApps(raw: string | null): MarineApp[] {
  if (!raw) return []
  const data = JSON.parse(raw)
  if (!Array.isArray(data)) throw new Error('应用记录格式不正确')
  return data.filter((app): app is MarineApp => {
    try {
      return typeof app.id === 'string' && typeof app.name === 'string' && typeof app.description === 'string' && Array.isArray(app.steps) && app.steps.every((step: TaskStep) => typeof step.id === 'string' && Array.isArray(step.nodes) && step.nodes.every(node => typeof node.id === 'string')) && validateApp(app).length === 0
    } catch { return false }
  })
}
