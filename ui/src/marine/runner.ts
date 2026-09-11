import { serviceById, validateApp } from './model.ts'
import type { MarineApp, TaskNode } from './model.ts'
import { demoSensorSnapshot, sensorServiceSummary } from './sensors.ts'
import type { SensorSnapshot } from './sensors.ts'

export type RunStatus = 'idle' | 'running' | 'paused' | 'completed' | 'cancelled' | 'failed'
export type NodeStatus = 'waiting' | 'running' | 'completed' | 'cancelled' | 'failed'
export interface ServiceResult { metric: string; value: number | string; unit: string; detail: string; source: string }
export interface NodeRun { node: TaskNode; status: NodeStatus; elapsed: number; progress: number; result: ServiceResult }
export interface RunEvent { time: number; message: string; kind: 'info' | 'success' | 'error' }
export interface RunState { status: RunStatus; app: MarineApp | null; stepIndex: number; elapsed: number; nodes: Record<string, NodeRun>; events: RunEvent[] }
export const emptyRun = (): RunState => ({ status: 'idle', app: null, stepIndex: 0, elapsed: 0, nodes: {}, events: [] })

// Business services consume the same task contract. Hardware adapters can later replace
// these simulators without changing the composition or scheduling contract.
type Simulation = (node: TaskNode, progress: number, sensors: SensorSnapshot) => ServiceResult
const simulations: Record<string, Simulation> = {
  '01': (node, progress) => ({ metric: '换气进度', value: Math.round(progress * 100), unit: '%', detail: `${node.area}换气 · 强度 ${node.intensity}%`, source: '软件模拟' }),
  '02': (node, progress) => ({ metric: '循环流量', value: Number((48 * node.intensity / 100 * Math.min(1, progress * 2)).toFixed(1)), unit: 'm³/h', detail: `${node.area}冷却循环已${progress < .5 ? '启动' : '稳定'}`, source: '软件模拟' }),
  '03': (node, progress) => ({ metric: '供水压力', value: Number((.5 * node.intensity / 100 * Math.min(1, progress * 2)).toFixed(2)), unit: 'MPa', detail: `${node.area}供水${progress < .5 ? '增压中' : '压力稳定'}`, source: '软件模拟' }),
  '04': (node, progress) => ({ metric: '剩余水位', value: Number(Math.max(5, 60 - 55 * progress * node.intensity / 100).toFixed(1)), unit: 'cm', detail: `${node.area}已完成本轮定时排水`, source: '软件模拟' }),
  '05': (node, progress) => ({ metric: '报警状态', value: progress >= 1 ? '已就绪' : '联动检查中', unit: '', detail: `${node.area}检修安全报警联动${progress >= 1 ? '已就绪' : '检查中'}`, source: '软件模拟' }),
  'M01': (node) => ({ metric: '实际转速', value: (node.motor?.direction === 'reverse' ? -1 : 1) * (node.motor?.speedRpm ?? 200), unit: 'rpm', detail: `${node.motor?.executorId ?? '电机'} · ${node.motor?.direction === 'reverse' ? '反转' : '正转'}`, source: '模拟演示' }),
}
export function simulateService(node: TaskNode, progress: number, sensors: SensorSnapshot = demoSensorSnapshot()): ServiceResult {
  if (serviceById(node.serviceId)?.kind === 'awareness') return sensorServiceSummary(node.serviceId, sensors)
  const simulate = simulations[node.serviceId]
  if (!simulate) throw new Error('该服务尚未实现')
  return simulate(node, Math.max(0, Math.min(1, progress)), sensors)
}
export class MarineRunner {
  state: RunState = emptyRun()
  private sensors = demoSensorSnapshot()
  start(app: MarineApp, sensors: SensorSnapshot = demoSensorSnapshot()) {
    if (this.state.status === 'running' || this.state.status === 'paused') throw new Error('请先结束当前任务')
    const errors = validateApp(app)
    if (errors.length) throw new Error(errors.join('\n'))
    const snapshot: MarineApp = JSON.parse(JSON.stringify(app))
    this.sensors = JSON.parse(JSON.stringify(sensors))
    this.state = { ...emptyRun(), status: 'running', app: snapshot }
    for (const step of snapshot.steps) for (const node of step.nodes) {
      this.state.nodes[node.id] = { node, status: 'waiting', elapsed: 0, progress: 0, result: simulateService(node, 0, this.sensors) }
    }
    this.event(`任务「${snapshot.name}」开始 · 软件模拟`)
    this.startStep()
  }
  private event(message: string, kind: RunEvent['kind'] = 'info') { this.state.events.push({ time: this.state.elapsed, message, kind }) }
  private startStep() {
    const step = this.state.app?.steps[this.state.stepIndex]
    if (!step) return
    this.event(`步骤 ${this.state.stepIndex + 1} · ${step.nodes.length > 1 ? '并行执行' : '顺序执行'}`)
    for (const node of step.nodes) {
      this.state.nodes[node.id]!.status = 'running'
      this.event(`${serviceById(node.serviceId).name}开始 · ${node.area} · ${node.duration} 秒`)
    }
  }
  tick(seconds: number) {
    if (this.state.status !== 'running' || !Number.isFinite(seconds) || seconds <= 0) return
    let remaining = seconds
    while (remaining > 0 && this.state.status === 'running') {
      const step = this.state.app!.steps[this.state.stepIndex]!
      const active = step.nodes.map(node => this.state.nodes[node.id]!).filter(node => node.status === 'running')
      const delta = Math.min(remaining, ...active.map(run => run.node.duration - run.elapsed))
      this.state.elapsed += delta
      remaining -= delta
      for (const run of active) {
        run.elapsed = Math.min(run.node.duration, run.elapsed + delta)
        run.progress = run.elapsed / run.node.duration
        try { run.result = simulateService(run.node, run.progress, this.sensors) }
        catch (error) { this.fail(run.node.id, error instanceof Error ? error.message : '服务执行失败'); return }
        if (run.progress >= 1) { run.status = 'completed'; this.event(`${serviceById(run.node.serviceId).name}完成`, 'success') }
      }
      if (step.nodes.every(node => this.state.nodes[node.id]!.status === 'completed')) {
        if (this.state.stepIndex === this.state.app!.steps.length - 1) {
          this.state.status = 'completed'; this.event('所有步骤已完成 · 任务结果已生成', 'success')
        } else { this.state.stepIndex++; this.startStep() }
      }
    }
  }
  pause() { if (this.state.status === 'running') { this.state.status = 'paused'; this.event('任务已暂停') } }
  resume() { if (this.state.status === 'paused') { this.state.status = 'running'; this.event('任务继续执行') } }
  cancel() {
    if (!['running', 'paused'].includes(this.state.status)) return
    this.state.status = 'cancelled'
    Object.values(this.state.nodes).forEach(run => { if (run.status !== 'completed') run.status = 'cancelled' })
    this.event('任务已停止 · 未完成服务已取消')
  }
  fail(nodeId: string, reason: string) {
    const run = this.state.nodes[nodeId]
    if (!run || run.status !== 'running' || !['running', 'paused'].includes(this.state.status)) return
    run.status = 'failed'; this.state.status = 'failed'
    Object.values(this.state.nodes).forEach(other => { if (other.status === 'running' || other.status === 'waiting') other.status = 'cancelled' })
    this.event(`${serviceById(run.node.serviceId).name}异常：${reason}。后续任务已停止。`, 'error')
  }
}
