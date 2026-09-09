import { serviceById } from './model.ts'
import type { MarineApp } from './model.ts'
import { sensorDefinitions, sensorServiceKeys } from './sensors.ts'

export interface ApplicationFlowSensor {
  key: string
  label: string
  unit: string
}

export interface ApplicationFlowService {
  id: string
  name: string
  color: string
  step: number
}

export interface ApplicationFlowEndpoint {
  id: string
  name: string
  serviceIds: string[]
}

export interface ApplicationFlow {
  application: { id: string; name: string }
  sensors: ApplicationFlowSensor[]
  awareness: ApplicationFlowService[]
  actions: ApplicationFlowService[]
  abstractions: ApplicationFlowEndpoint[]
  hardware: ApplicationFlowEndpoint[]
}

const actionBindings: Record<string, { abstraction: [string, string]; hardware: [string, string] }> = {
  '01': { abstraction: ['VENT-01', '机舱通风逻辑执行器'], hardware: ['HW-VENT', '机舱通风机 1 号'] },
  '02': { abstraction: ['COOL-01', '冷却逻辑执行器'], hardware: ['HW-COOL', '冷却循环泵'] },
  '03': { abstraction: ['WATER-01', '供水逻辑执行器'], hardware: ['HW-WATER', '供水增压泵'] },
  '04': { abstraction: ['DRAIN-01', '舱底排水逻辑执行器'], hardware: ['HW-DRAIN', '舱底排水泵 2 号'] },
  '05': { abstraction: ['ALARM-01', '检修安全报警逻辑执行器'], hardware: ['HW-ALARM', '检修安全报警设备'] },
}

export function buildApplicationFlow(app: MarineApp): ApplicationFlow {
  const seenServices = new Set<string>()
  const orderedServices: ApplicationFlowService[] = []

  app.steps.forEach((step, stepIndex) => {
    step.nodes.forEach(node => {
      if (seenServices.has(node.serviceId)) return
      seenServices.add(node.serviceId)
      const service = serviceById(node.serviceId)
      orderedServices.push({ id: service.id, name: service.name, color: service.color, step: stepIndex + 1 })
    })
  })

  const awareness = orderedServices.filter(service => service.id.startsWith('A'))
  const actions = orderedServices.filter(service => !service.id.startsWith('A'))
  const sensorKeys = [...new Set(awareness.flatMap(service => sensorServiceKeys[service.id] || []))]
  const sensors = sensorKeys.map(key => ({ key, ...sensorDefinitions[key]! }))

  const abstractions = combineEndpoints(actions, 'abstraction')
  const hardware = combineEndpoints(actions, 'hardware')

  return {
    application: { id: app.id, name: app.name },
    sensors,
    awareness,
    actions,
    abstractions,
    hardware,
  }
}

function combineEndpoints(actions: ApplicationFlowService[], kind: 'abstraction' | 'hardware') {
  const endpoints = new Map<string, ApplicationFlowEndpoint>()
  actions.forEach(service => {
    const binding = actionBindings[service.id]?.[kind]
    if (!binding) return
    const [id, name] = binding
    const endpoint = endpoints.get(id)
    if (endpoint) endpoint.serviceIds.push(service.id)
    else endpoints.set(id, { id, name, serviceIds: [service.id] })
  })
  return [...endpoints.values()]
}
