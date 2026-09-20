import test from 'node:test'
import assert from 'node:assert/strict'
import { buildGroupControlCommands, resolveSoftwareTask, terminateSoftwareDemoState } from '../src/marine/software-defined-tasks.ts'

test('任务一 maps to the fixed 125 plan and separates controllable motors', () => {
  assert.deepEqual(resolveSoftwareTask('任务一：机舱静音控制'), {
    key: '任务一', targets: [1, 2, 5], controllableTargets: [1, 2], simulatedTargets: [5],
  })
})

test('任务二 maps to the fixed 238 plan and only motors 2 and 3 are controllable', () => {
  assert.deepEqual(resolveSoftwareTask('任务二：低噪运行'), {
    key: '任务二', targets: [2, 3, 8], controllableTargets: [2, 3], simulatedTargets: [8],
  })
})

test('任务三 maps to the fixed 456 plan with simulation-only targets', () => {
  assert.deepEqual(resolveSoftwareTask('任务三：舱室维护'), {
    key: '任务三', targets: [4, 5, 6], controllableTargets: [], simulatedTargets: [4, 5, 6],
  })
})

test('only the first three input characters select a task branch', () => {
  assert.equal(resolveSoftwareTask('任务二：任务一 只是描述内容').key, '任务二')
  assert.equal(resolveSoftwareTask('其他任务'), null)
})

test('group control converts AI targets into stop and keep-running outputs', () => {
  const plan = resolveSoftwareTask('任务二：低噪运行')
  assert.deepEqual(buildGroupControlCommands(plan), [
    { motorId: 1, command: 'keep', state: 'on' },
    { motorId: 2, command: 'stop', state: 'off' },
    { motorId: 3, command: 'stop', state: 'off' },
    { motorId: 4, command: 'keep', state: 'on' },
    { motorId: 5, command: 'keep', state: 'on' },
    { motorId: 6, command: 'keep', state: 'on' },
    { motorId: 7, command: 'keep', state: 'on' },
    { motorId: 8, command: 'stop', state: 'off' },
  ])
  assert.deepEqual(buildGroupControlCommands(null), [])
})

test('terminating the demo clears the AI plan and only marks physical motors stopped', () => {
  assert.deepEqual(terminateSoftwareDemoState(), {
    activePlan: null,
    revealStage: 0,
    stopped: [1, 2, 3],
    state: 'pending',
  })
})
