export type SoftwareTaskPlan = {
  key: '任务一' | '任务二' | '任务三'
  targets: number[]
  controllableTargets: number[]
  simulatedTargets: number[]
}

export type GroupControlCommand = {
  motorId: number
  command: 'stop' | 'keep'
  state: 'off' | 'on'
}

const taskTargets = {
  任务一: [1, 2, 5],
  任务二: [2, 3, 8],
  任务三: [4, 5, 6],
} as const

const physicalMotors = new Set([1, 2, 3])

export function resolveSoftwareTask(input: string): SoftwareTaskPlan | null {
  const key = input.slice(0, 3) as keyof typeof taskTargets
  const targets = taskTargets[key]
  if (!targets) return null
  return {
    key,
    targets: [...targets],
    controllableTargets: targets.filter(id => physicalMotors.has(id)),
    simulatedTargets: targets.filter(id => !physicalMotors.has(id)),
  }
}

export function buildGroupControlCommands(plan: SoftwareTaskPlan | null): GroupControlCommand[] {
  if (!plan) return []
  return Array.from({ length: 8 }, (_, index) => {
    const motorId = index + 1
    const stopped = plan.targets.includes(motorId)
    return { motorId, command: stopped ? 'stop' : 'keep', state: stopped ? 'off' : 'on' }
  })
}

export function terminateSoftwareDemoState() {
  return {
    activePlan: null,
    revealStage: 0,
    stopped: [1, 2, 3, 4, 5, 6, 7, 8],
    state: 'pending' as const,
  }
}

export function softwareTerminationMessage(failedMotorNumbers: number[]) {
  return failedMotorNumbers.length === 0
    ? '三台真实电机均已停止，虚拟电机已同步关闭，演示已终止'
    : `电机${failedMotorNumbers.join('、')}未确认停止，请检查设备连接后重试`
}
