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
