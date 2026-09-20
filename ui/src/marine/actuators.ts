export type ActuatorMode = 'simulation' | 'real'
export type ActuatorPhase = 'running' | 'protected' | 'awaiting-confirmation' | 'switching' | 'recovered'
export type ActuatorStatus = 'offline' | 'standby' | 'running' | 'faulted'

export interface ActuatorDevice {
  id: 'MOTOR-02' | 'MOTOR-0E' | 'MOTOR-0F'
  name: string
  type: string
  address: '0x02' | '0x0E' | '0x0F'
  status: ActuatorStatus
  compatible: boolean
  faultCode: string
  speed: number
  direction: 'forward' | 'reverse'
}

export type HardwareDisplayProtocol = 'RS485' | 'CAN' | 'RS232'

export function motorPresentation(device: ActuatorDevice | undefined, gatewayOnline: boolean) {
  if (!gatewayOnline || !device) {
    return { rpm: null, direction: '--', state: '未知', connection: '未知', online: false }
  }
  if (device.status === 'offline') {
    return { rpm: null, direction: '--', state: '--', connection: '离线', online: false }
  }
  return {
    rpm: Number.isFinite(device.speed) ? Math.abs(device.speed) : null,
    direction: device.direction === 'reverse' ? '反转' : '正转',
    state: device.status === 'running' ? '运行中' : device.status === 'faulted' ? '故障' : '已停止',
    connection: '在线',
    online: true,
  }
}

export function hardwareDisplayProtocol(address: ActuatorDevice['address']): HardwareDisplayProtocol {
  return address === '0x02' ? 'RS485' : address === '0x0E' ? 'CAN' : 'RS232'
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
    application: { id: 'APP-COOL-01', name: '中央冷却系统' },
    atomicService: { id: 'SERVICE-COOL-01', name: '冷却泵控制服务' },
    logicalExecutor: { id: 'COOL-PUMP-01', name: '中央冷却泵逻辑执行器', boundDeviceId: 'MOTOR-02' },
    devices: [
      { id: 'MOTOR-02', name: '水泵 A', type: '中央冷却泵 · 原型号', address: '0x02', status: 'offline', compatible: true, faultCode: '—', speed: 0, direction: 'forward' },
      { id: 'MOTOR-0E', name: '水泵 B', type: '中央冷却泵 · 替换型号', address: '0x0E', status: 'offline', compatible: true, faultCode: '—', speed: 0, direction: 'forward' },
      { id: 'MOTOR-0F', name: '水泵 C', type: '中央冷却泵 · 备用型号', address: '0x0F', status: 'offline', compatible: true, faultCode: '—', speed: 0, direction: 'forward' },
    ],
    candidateId: null,
    applicationChanges: 0,
    serviceChanges: 0,
    bindingUpdates: 0,
    recoveryMs: 0,
    events: [{ id: 1, tone: 'success', title: '检修通风稳定运行', detail: 'VENT-01 当前绑定机舱通风机 1 号 · 0x02' }],
    lastMessage: '检修保障应用正通过逻辑执行器运行。',
  }
}

export function mergeRealMotorTelemetry(devices: ActuatorDevice[], motors: any[]): ActuatorDevice[] {
  return devices.map(device => {
    const motor = motors.find(item => String(item.address).toLowerCase() === device.address.toLowerCase())
    if (!motor) return { ...device, status: 'offline', speed: 0, faultCode: '—' }
    const rpm = Number(motor.actualRpm) || 0
    return {
      ...device,
      status: motor.online ? (motor.running ? 'running' : 'standby') : 'offline',
      speed: rpm === 0 ? 0 : motor.direction === 'reverse' ? -Math.abs(rpm) : rpm,
      direction: motor.direction === 'reverse' ? 'reverse' : 'forward',
      faultCode: motor.statusWord === 39 ? '0x0000' : `0x${Number(motor.statusWord || 0).toString(16).padStart(4, '0')}`,
    }
  })
}

export function syncRealRemovalState(state: ActuatorDemoState): ActuatorDemoState {
  if (!['running', 'recovered'].includes(state.phase)) return state
  const bound = state.devices.find(device => device.id === state.logicalExecutor.boundDeviceId)
  if (!bound || bound.status !== 'offline') return state
  return {
    ...state,
    phase: 'protected',
    candidateId: null,
    recoveryMs: 0,
    events: event(state, 'warning', '检测到真实执行器离线', `${bound.name} · ${bound.address} 通信中断，任务已进入保护状态`),
    lastMessage: '真实设备已离线，请选择已接入的兼容水泵。',
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

export function confirmRealActuatorSwitch(state: ActuatorDemoState): ActuatorDemoState {
  const candidate = state.devices.find(device => device.id === state.candidateId)
  if (state.phase !== 'awaiting-confirmation' || !candidate || candidate.status === 'offline') return state
  const previous = state.devices.find(device => device.id === state.logicalExecutor.boundDeviceId)
  const next = {
    ...state,
    phase: 'recovered' as const,
    candidateId: null,
    logicalExecutor: { ...state.logicalExecutor, boundDeviceId: candidate.id },
    bindingUpdates: state.bindingUpdates + 1,
    recoveryMs: 0,
  }
  return {
    ...next,
    events: event(next, 'success', '真实设备切换完成', `${previous?.address ?? '—'} → ${candidate.address}，上层应用保持不变`),
    lastMessage: `${candidate.name} 已成为当前绑定设备，可按需下发运行参数。`,
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
