import test from 'node:test'
import assert from 'node:assert/strict'
import * as presentation from '../src/marine/actuators.ts'

const motor = { ...presentation.createActuatorDemo().devices[1], status: 'running', speed: -237, direction: 'reverse' }
test('real motor display reports measured RPM and direction', () => {
  assert.equal(typeof presentation.motorPresentation, 'function')
  assert.deepEqual(presentation.motorPresentation(motor, true), { rpm: 237, direction: '反转', state: '运行中', connection: '在线', online: true })
})
test('stopped online motor retains reported direction', () => {
  assert.deepEqual(presentation.motorPresentation({ ...motor, status: 'standby', speed: 0 }, true), { rpm: 0, direction: '反转', state: '已停止', connection: '在线', online: true })
})
test('offline motor hides stale speed and direction', () => {
  assert.deepEqual(presentation.motorPresentation({ ...motor, status: 'offline' }, true), { rpm: null, direction: '--', state: '--', connection: '离线', online: false })
})
test('gateway failure hides cached device telemetry', () => {
  assert.deepEqual(presentation.motorPresentation(motor, false), { rpm: null, direction: '--', state: '未知', connection: '未知', online: false })
})
test('missing device never displays zero RPM as a real measurement', () => {
  assert.equal(presentation.motorPresentation(undefined, true).rpm, null)
})
