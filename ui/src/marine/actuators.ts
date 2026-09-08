export type ActuatorMode = 'simulation' | 'real'
export type ActuatorPhase = 'running' | 'protected' | 'awaiting-confirmation' | 'switching' | 'recovered'
export type ActuatorStatus = 'offline' | 'standby' | 'running' | 'faulted'

export interface ActuatorDevice {
  id: 'MOTOR-02' | 'MOTOR-0E' | 'MOTOR-0F'
  name: string
  type: '短电机' | '长电机'
  address: '0x02' | '0x0E' | '0x0F'
  status: ActuatorStatus
  compatible: boolean
  faultCode: string
  speed: number
}

export interface ActuatorEvent {
  id: number
  tone: 'normal' | 'warning' | 'success'
  title: string
  detail: string
}

export interface ActuatorDemoState {
  mode: ActuatorMode
  phase: ActuatorPhase
  application: { id: string; name: string }
  atomicService: { id: string; name: string }
  logicalExecutor: { id: string; name: string; boundDeviceId: ActuatorDevice['id'] }
  devices: ActuatorDevice[]
  candidateId: ActuatorDevice['id'] | null
  applicationChanges: number
  serviceChanges: number
  bindingUpdates: number
  recoveryMs: number
  events: ActuatorEvent[]
  lastMessage: string
}

const switchDurationMs = 1600

function event(state: ActuatorDemoState, tone: ActuatorEvent['tone'], title: string, detail: string) {
  return [...state.events, { id: state.events.length + 1, tone, title, detail }]
}

function updateDevice(
  devices: ActuatorDevice[],
  id: ActuatorDevice['id'],
  update: Partial<ActuatorDevice>,
) {
  return devices.map(device => device.id === id ? { ...device, ...update } : { ...device })
}

export function createActuatorDemo(): ActuatorDemoState {
  return {
    mode: 'simulation',
    phase: 'running',
    application: { id: 'APP-VENT-01', name: '机舱通风保障' },
    atomicService: { id: 'SERVICE-01', name: '舱室智能通风' },
    logicalExecutor: { id: 'VENT-01', name: '通风执行器', boundDeviceId: 'MOTOR-02' },
    devices: [
      { id: 'MOTOR-02', name: '海风一号', type: '短电机', address: '0x02', status: 'running', compatible: true, faultCode: '0x0000', speed: 860 },
      { id: 'MOTOR-0E', name: '海岳一号', type: '长电机', address: '0x0E', status: 'offline', compatible: true, faultCode: '—', speed: 0 },
      { id: 'MOTOR-0F', name: '海岳二号', type: '长电机', address: '0x0F', status: 'offline', compatible: true, faultCode: '—', speed: 0 },
    ],
    candidateId: null,
    applicationChanges: 0,
    serviceChanges: 0,
    bindingUpdates: 0,
    recoveryMs: 0,
    events: [{ id: 1, tone: 'success', title: '任务稳定运行', detail: 'VENT-01 当前绑定海风一号 · 0x02' }],
    lastMessage: '应用正通过逻辑执行器稳定运行。',
  }
}

export function disconnectActive(state: ActuatorDemoState): ActuatorDemoState {
  if (!['running', 'recovered'].includes(state.phase)) return state
  const activeId = state.logicalExecutor.boundDeviceId
  const active = state.devices.find(device => device.id === activeId)
  return {
    ...state,
    phase: 'protected',
    candidateId: null,
    recoveryMs: 0,
    devices: updateDevice(state.devices, activeId, { status: 'offline', speed: 0, faultCode: '—' }),
    events: event(state, 'warning', '检测到执行器离线', `${active?.name ?? activeId} 连续 3 次通信超时，任务已保护暂停`),
    lastMessage: '当前执行器已离线，应用与原子服务保持不变。',
  }
}

export function injectActuatorFault(state: ActuatorDemoState): ActuatorDemoState {
  if (!['running', 'recovered'].includes(state.phase)) return state
  const activeId = state.logicalExecutor.boundDeviceId
  const active = state.devices.find(device => device.id === activeId)
  return {
    ...state,
    phase: 'protected',
    candidateId: null,
    recoveryMs: 0,
    devices: updateDevice(state.devices, activeId, { status: 'faulted', speed: 0, faultCode: '0x8311' }),
    events: event(state, 'warning', '执行器异常保护', `${active?.name ?? activeId} 返回 0x8311 过载，控制下发已暂停`),
    lastMessage: '故障被限制在执行器硬件层，等待兼容设备接入。',
  }
}

export function connectActuator(state: ActuatorDemoState, deviceId: ActuatorDevice['id']): ActuatorDemoState {
  if (deviceId === state.logicalExecutor.boundDeviceId) return state
  const device = state.devices.find(item => item.id === deviceId)
  if (!device || !device.compatible || !['protected', 'awaiting-confirmation'].includes(state.phase)) return state
  const devices = updateDevice(state.devices, deviceId, { status: 'standby', faultCode: '0x0000', speed: 0 })
  const next = { ...state, devices }
  return {
    ...next,
    phase: 'awaiting-confirmation',
    candidateId: deviceId,
    events: event(next, 'normal', '发现兼容执行器', `${device.name} · ${device.address} 已上线并通过演示能力检查`),
    lastMessage: `已推荐 ${device.name}，请由操作员确认切换。`,
  }
}

export function confirmActuatorSwitch(state: ActuatorDemoState): ActuatorDemoState {
  const candidate = state.devices.find(device => device.id === state.candidateId)
  if (state.phase !== 'awaiting-confirmation' || !candidate || candidate.status !== 'standby') {
    return { ...state, lastMessage: '尚未发现可切换的兼容执行器。' }
  }
  const previous = state.devices.find(device => device.id === state.logicalExecutor.boundDeviceId)
  const next = {
    ...state,
    phase: 'switching' as const,
    candidateId: null,
    logicalExecutor: { ...state.logicalExecutor, boundDeviceId: candidate.id },
    bindingUpdates: state.bindingUpdates + 1,
    recoveryMs: 0,
  }
  return {
    ...next,
    events: event(next, 'normal', '操作员确认切换', `${state.logicalExecutor.id}：${previous?.address ?? '—'} → ${candidate.address}，正在初始化`),
    lastMessage: '设备抽象层正在更新绑定并执行初始化检查。',
  }
}

export function advanceActuatorDemo(state: ActuatorDemoState, milliseconds: number): ActuatorDemoState {
  if (state.phase !== 'switching' || milliseconds <= 0) return state
  const recoveryMs = state.recoveryMs + milliseconds
  if (recoveryMs < switchDurationMs) return { ...state, recoveryMs }
  const active = state.devices.find(device => device.id === state.logicalExecutor.boundDeviceId)
  const next = {
    ...state,
    phase: 'recovered' as const,
    recoveryMs,
    devices: updateDevice(state.devices, state.logicalExecutor.boundDeviceId, { status: 'running', faultCode: '0x0000', speed: 860 }),
  }
  return {
    ...next,
    events: event(next, 'success', '任务恢复运行', `${active?.name ?? state.logicalExecutor.boundDeviceId} 已接管，应用与原子服务零修改`),
    lastMessage: '切换完成，原应用继续通过 VENT-01 运行。',
  }
}
