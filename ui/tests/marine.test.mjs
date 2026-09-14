import test from 'node:test'
import assert from 'node:assert/strict'
import { createId, createLeadershipDemo, createPreset, createStep, createNode, services, validateApp, plannedDuration, parseSavedApps } from '../src/marine/model.ts'
import { MarineRunner, simulateService } from '../src/marine/runner.ts'
import { demoSensorSnapshot, normalizeSensorSnapshot, sensorServiceSummary } from '../src/marine/sensors.ts'
import { buildApplicationFlow } from '../src/marine/application-flow.ts'
import {
  advanceActuatorDemo,
  confirmActuatorSwitch,
  connectActuator,
  createActuatorDemo,
  disconnectActive,
  injectActuatorFault,
  mergeRealMotorTelemetry,
  syncRealRemovalState,
  confirmRealActuatorSwitch,
} from '../src/marine/actuators.ts'
import { createMarineApi } from '../src/marine/api.ts'

test('A: awareness runs first, then ventilation, then cooling and water start together', () => {
  const app = createPreset('A'); const runner = new MarineRunner(); runner.start(app)
  const sensing = app.steps[0].nodes
  const [vent] = app.steps[1].nodes
  const [cool, water] = app.steps[2].nodes
  runner.tick(2)
  assert.ok(sensing.every(node => runner.state.nodes[node.id].status === 'running'))
  assert.equal(runner.state.nodes[vent.id].status, 'waiting')
  runner.tick(1)
  assert.ok(sensing.every(node => runner.state.nodes[node.id].status === 'completed'))
  assert.equal(runner.state.nodes[vent.id].status, 'running')
  runner.tick(5.9)
  assert.equal(runner.state.nodes[cool.id].status, 'waiting')
  assert.equal(runner.state.nodes[vent.id].status, 'running')
  runner.tick(.1)
  assert.equal(runner.state.nodes[vent.id].status, 'completed')
  assert.equal(runner.state.nodes[cool.id].status, 'running')
  assert.equal(runner.state.nodes[water.id].status, 'running')
  runner.tick(6)
  assert.equal(runner.state.status, 'completed')
  assert.equal(runner.state.elapsed, 15)
})
test('B: waits for the slower parallel service before drainage', () => {
  const app = createPreset('B'); app.steps[1].nodes[0].duration = 2; app.steps[1].nodes[1].duration = 8
  const runner = new MarineRunner(); runner.start(app)
  runner.tick(6)
  assert.equal(runner.state.nodes[app.steps[1].nodes[0].id].status, 'completed')
  assert.equal(runner.state.nodes[app.steps[2].nodes[0].id].status, 'waiting')
  runner.tick(5)
  assert.equal(runner.state.nodes[app.steps[2].nodes[0].id].status, 'running')
  runner.tick(6)
  assert.equal(runner.state.status, 'completed'); assert.equal(runner.state.elapsed, plannedDuration(app))
})
test('pause freezes time; cancellation stops all pending services; rerun resets results', () => {
  const runner = new MarineRunner(); const app = createPreset('A'); runner.start(app)
  runner.tick(2); runner.pause(); runner.tick(10); assert.equal(runner.state.elapsed, 2)
  runner.resume(); runner.tick(1); assert.equal(runner.state.elapsed, 3)
  runner.cancel(); runner.tick(50); assert.equal(runner.state.status, 'cancelled')
  assert.ok(Object.values(runner.state.nodes).every(node => ['completed', 'cancelled'].includes(node.status)))
  runner.start(app); assert.equal(runner.state.elapsed, 0); runner.tick(20); assert.equal(runner.state.status, 'completed')
})
test('failure cancels the parallel peer and prevents later steps', () => {
  const runner = new MarineRunner(); const app = createPreset('B'); runner.start(app)
  runner.tick(3)
  runner.fail(app.steps[1].nodes[0].id, 'test fault'); runner.tick(30)
  assert.equal(runner.state.status, 'failed')
  assert.equal(runner.state.nodes[app.steps[1].nodes[1].id].status, 'cancelled')
  assert.equal(runner.state.nodes[app.steps[2].nodes[0].id].status, 'cancelled')
})
test('validation rejects shared actuator conflicts, empty steps and unimplemented services', () => {
  const app = createPreset('A'); app.steps = [createStep(['03', '04'])]
  assert.match(validateApp(app).join(), /共用水务执行器/)
  assert.throws(() => new MarineRunner().start(app), /共用/)
  app.steps = [createStep(['06'])]; assert.match(validateApp(app).join(), /尚未实现/)
  app.steps = [createStep()]; assert.match(validateApp(app).join(), /没有服务/)
  app.steps = [createStep(['01'])]; app.steps[0].nodes[0].duration = NaN
  assert.match(validateApp(app).join(), /超出范围/)
})
test('awareness services keep an editable, validated threshold condition', () => {
  const node = createNode('A03')
  assert.equal(node.threshold, 800)
  assert.equal(node.thresholdOperator, 'gte')
  const app = createPreset('A'); app.steps = [createStep(['A03'])]
  app.steps[0].nodes[0].threshold = -1
  assert.match(validateApp(app).join(), /超出范围/)
})
test('motor service defaults, persists direction and rejects unsafe parameters', () => {
  const node = createNode('M01')
  assert.equal(node.motor.executorId, 'EXHAUST-FAN-01')
  assert.equal(node.motor.speedRpm, 200)
  assert.equal(node.motor.direction, 'forward')

  node.motor.direction = 'reverse'
  const app = { id: 'motor-app', name: '电机应用', description: '', steps: [{ id: 'step', nodes: [node] }] }
  const [restored] = parseSavedApps(JSON.stringify([app]))
  assert.equal(restored.steps[0].nodes[0].motor.direction, 'reverse')

  restored.steps[0].nodes[0].motor.speedRpm = 501
  assert.match(validateApp(restored).join(), /电机参数/)
})
test('motor service adds its logical executor and physical adapter to application flow', () => {
  const app = { id: 'motor-flow', name: '排烟控制', description: '', steps: [createStep(['M01'])] }
  const flow = buildApplicationFlow(app)
  assert.deepEqual(flow.actions.map(item => item.id), ['M01'])
  assert.deepEqual(flow.abstractions.map(item => item.id), ['EXHAUST-FAN-01'])
  assert.match(flow.hardware[0].name, /IDS57-R.*0x02/)
})
test('all five simulators produce distinct results and respond to strength or duration progress', () => {
  const outputs = ['01','02','03','04','05'].map(id => simulateService(createNode(id), 1))
  assert.equal(new Set(outputs.map(result => result.metric)).size, 5)
  for (const id of ['02','03','04']) {
    const node = createNode(id); node.intensity = 20; const low = simulateService(node, 1).value
    node.intensity = 100; const high = simulateService(node, 1).value
    assert.notEqual(low, high)
  }
  const vent = createNode('01'); assert.ok(simulateService(vent, .5).value < simulateService(vent, 1).value)
})
test('execution takes a snapshot; a large clock tick correctly crosses step boundaries', () => {
  const runner = new MarineRunner(); const app = createPreset('A'); runner.start(app)
  app.steps[0].nodes[0].duration = 30; app.name = 'edited'
  runner.tick(100)
  assert.equal(runner.state.status, 'completed'); assert.equal(runner.state.elapsed, 15)
  assert.notEqual(runner.state.app.name, app.name)
})
test('saved applications round trip, malformed records do not become executable', () => {
  const app = createPreset('A')
  assert.deepEqual(parseSavedApps(JSON.stringify([app])), [app])
  assert.deepEqual(parseSavedApps(JSON.stringify([{ id: 'broken', steps: null }, app])), [app])
  assert.throws(() => parseSavedApps('{broken'))
  assert.deepEqual(parseSavedApps(null), [])
})

test('application IDs still work when the browser has no crypto.randomUUID', () => {
  const id = createId(undefined, () => 0.5, () => 123)
  assert.match(id, /^marine-3f-[a-z0-9]{8}$/)
})

test('leadership demo creates two independent document scenarios by reusing control services', () => {
  const demo = createLeadershipDemo()
  const actionIds = demo.apps.map(app => app.steps.flatMap(step => step.nodes)
    .map(node => node.serviceId).filter(id => ['01', '02', '03', '04', '05'].includes(id)))

  assert.deepEqual(actionIds, [['01', '05', '04'], ['05', '01', '04']])
  assert.deepEqual(demo.reusedServiceIds, ['01', '04', '05'])
  assert.notEqual(demo.apps[0].id, demo.apps[1].id)
  assert.equal(demo.newServiceCount, 0)
})

test('application flow is derived from the services actually used by each app', () => {
  const flowA = buildApplicationFlow(createPreset('A'))
  assert.equal(flowA.application.name, '开航辅助保障')
  assert.deepEqual(flowA.awareness.map(item => item.id), ['A01', 'A02', 'A03'])
  assert.deepEqual(flowA.sensors.map(item => item.label), ['温度', '湿度', 'PM2.5', 'PM10', 'TVOC', '甲醛', 'CO₂'])
  assert.deepEqual(flowA.actions.map(item => item.id), ['01', '02', '03'])
  assert.deepEqual(flowA.abstractions.map(item => item.id), ['VENT-01', 'COOL-01', 'WATER-01'])
  assert.deepEqual(flowA.hardware.map(item => item.name), ['机舱通风机 1 号', '冷却循环泵', '供水增压泵'])

  const flowB = buildApplicationFlow(createPreset('B'))
  assert.deepEqual(flowB.awareness.map(item => item.id), ['A05'])
  assert.deepEqual(flowB.sensors.map(item => item.label), ['水浸'])
  assert.deepEqual(flowB.actions.map(item => item.id), ['02', '03', '04'])
  assert.deepEqual(flowB.hardware.map(item => item.name), ['冷却循环泵', '供水增压泵', '舱底排水泵 2 号'])
})

test('all 12 existing sensor readings are grouped into seven atomic awareness services', () => {
  const snapshot = demoSensorSnapshot()
  const awareness = ['A01', 'A02', 'A03', 'A04', 'A05', 'A06', 'A07']
  assert.equal(Object.keys(snapshot.values).length, 12)
  awareness.forEach(id => {
    const result = sensorServiceSummary(id, snapshot)
    assert.equal(result.source, '演示数据')
    assert.notEqual(result.value, '')
  })
})

test('real sensor payload is scaled and offline state stays visible', () => {
  const snapshot = normalizeSensorSnapshot({
    temperature: 246, temperature_online: 1, humidity: 623, humidity_online: 1,
    pm25: 30, pm25_online: 1, pm10: 51, pm10_online: 1,
    tvoc: 80, tvoc_online: 1, ch2o: 20, ch2o_online: 1, co2: 515, co2_online: 1,
    smoke: 0, smoke_online: 0, water: 0, water_online: 1,
    ir: 1, radar: 0, ir_online: 1, lux: 320, light_online: 1,
  })
  assert.equal(snapshot.values.temperature.value, 24.6)
  assert.equal(snapshot.values.humidity.value, 62.3)
  assert.equal(sensorServiceSummary('A04', snapshot).value, '设备离线')
  assert.equal(sensorServiceSummary('A06', snapshot).value, '检测到活动')
  assert.equal(sensorServiceSummary('A01', snapshot).source, '实时设备数据')
})

test('software base runs multiple application instances independently', () => {
  const first = createPreset('A')
  const second = createPreset('B')
  const firstRunner = new MarineRunner()
  const secondRunner = new MarineRunner()

  firstRunner.start(first)
  secondRunner.start(second)
  firstRunner.tick(2)
  secondRunner.tick(4)

  assert.equal(firstRunner.state.status, 'running')
  assert.equal(secondRunner.state.status, 'running')
  assert.equal(firstRunner.state.elapsed, 2)
  assert.equal(secondRunner.state.elapsed, 4)

  firstRunner.pause()
  firstRunner.tick(5)
  secondRunner.tick(20)
  assert.equal(firstRunner.state.status, 'paused')
  assert.equal(firstRunner.state.elapsed, 2)
  assert.equal(secondRunner.state.status, 'completed')
})

test('operator-confirmed replacement changes only the physical binding', () => {
  const initial = createActuatorDemo()
  const protectedState = disconnectActive(initial)
  const candidateState = connectActuator(protectedState, 'MOTOR-0F')

  assert.equal(initial.logicalExecutor.boundDeviceId, 'MOTOR-02')
  assert.equal(protectedState.phase, 'protected')
  assert.equal(protectedState.devices.find(device => device.id === 'MOTOR-02')?.status, 'offline')
  assert.equal(candidateState.phase, 'awaiting-confirmation')
  assert.equal(candidateState.candidateId, 'MOTOR-0F')

  const switching = confirmActuatorSwitch(candidateState)
  assert.equal(switching.phase, 'switching')
  assert.equal(switching.logicalExecutor.boundDeviceId, 'MOTOR-0F')
  assert.equal(switching.application.id, initial.application.id)
  assert.equal(switching.atomicService.id, initial.atomicService.id)
  assert.deepEqual(
    [switching.applicationChanges, switching.serviceChanges, switching.bindingUpdates],
    [0, 0, 1],
  )

  const recovered = advanceActuatorDemo(switching, 1800)
  assert.equal(recovered.phase, 'recovered')
  assert.equal(recovered.devices.find(device => device.id === 'MOTOR-0F')?.status, 'running')
  assert.equal(recovered.recoveryMs, 1800)
})

test('hardware replacement screen starts without invented RPM and merges real motor telemetry', () => {
  const initial = createActuatorDemo()
  assert.ok(initial.devices.every(device => device.speed === 0))
  assert.ok(initial.devices.every(device => device.status === 'offline'))

  const devices = mergeRealMotorTelemetry(initial.devices, [
    { address: '0x0E', online: true, running: true, actualRpm: 237, direction: 'reverse', statusWord: 39 },
  ])
  assert.equal(devices.find(device => device.address === '0x0E')?.speed, -237)
  assert.equal(devices.find(device => device.address === '0x0E')?.status, 'running')
  assert.equal(devices.find(device => device.address === '0x02')?.speed, 0)
})

test('real replacement flow enters protection only when the bound motor stops responding', () => {
  const online = createActuatorDemo()
  online.devices = mergeRealMotorTelemetry(online.devices, [
    { address: '0x02', online: true, running: false, actualRpm: 0, direction: 'forward', statusWord: 39 },
  ])
  assert.equal(syncRealRemovalState(online).phase, 'running')
  online.devices = mergeRealMotorTelemetry(online.devices, [])
  const removed = syncRealRemovalState(online)
  assert.equal(removed.phase, 'protected')
  assert.match(removed.lastMessage, /真实设备已离线/)
})

test('real replacement confirmation binds the selected online motor without inventing motion', () => {
  let state = createActuatorDemo()
  state.devices = mergeRealMotorTelemetry(state.devices, [
    { address: '0x0E', online: true, running: false, actualRpm: 0, direction: 'forward', statusWord: 39 },
  ])
  state = syncRealRemovalState(state)
  state = connectActuator(state, 'MOTOR-0E')
  state = confirmRealActuatorSwitch(state)
  assert.equal(state.phase, 'recovered')
  assert.equal(state.logicalExecutor.boundDeviceId, 'MOTOR-0E')
  assert.equal(state.devices.find(device => device.id === 'MOTOR-0E')?.speed, 0)
})

test('real motor inventory request is allowed to outlive the default three-second API timeout', async () => {
  const fetcher = (_input, init) => new Promise((resolve, reject) => {
    const timer = setTimeout(() => resolve({ ok: true, status: 200, json: async () => [] }), 3100)
    init?.signal?.addEventListener('abort', () => { clearTimeout(timer); reject(new Error('aborted')) })
  })
  const api = createMarineApi(fetcher)
  assert.deepEqual(await api.listMotors(), [])
})

test('hardware decoupling motor controls use the direct gateway endpoints', async () => {
  const requests = []
  const api = createMarineApi(async (input, init) => {
    requests.push({ input, init })
    return { ok: true, status: 200, json: async () => ({ running: true }) }
  })
  await api.startMotor('FIRE-PUMP-01', { speedRpm: 300, direction: 'forward', acceleration: 10, deceleration: 10, operator: '演示员' })
  await api.stopDirectMotor('FIRE-PUMP-01', '演示员')
  assert.equal(requests[0].input, '/api/v1/marine/motors/FIRE-PUMP-01/start')
  assert.equal(requests[0].init.method, 'POST')
  assert.equal(requests[1].input, '/api/v1/marine/motors/FIRE-PUMP-01/stop')
})

test('fault protection cannot switch until an eligible motor is connected', () => {
  const faulted = injectActuatorFault(createActuatorDemo())
  assert.equal(faulted.phase, 'protected')
  assert.equal(faulted.devices.find(device => device.id === 'MOTOR-02')?.faultCode, '0x8311')

  const rejected = confirmActuatorSwitch(faulted)
  assert.equal(rejected.phase, 'protected')
  assert.equal(rejected.bindingUpdates, 0)
  assert.match(rejected.lastMessage, /尚未发现可切换/)

  const candidate = connectActuator(rejected, 'MOTOR-0F')
  assert.equal(candidate.phase, 'awaiting-confirmation')
  assert.equal(candidate.candidateId, 'MOTOR-0F')
})

test('document scenarios combine the night-maintenance and abnormal-response services', () => {
  const maintenance = createPreset('D')
  const response = createPreset('E')

  assert.equal(maintenance.name, '夜间机舱检修保障')
  assert.deepEqual(
    maintenance.steps.map(step => step.nodes.map(node => node.serviceId)),
    [['A06', 'A07', 'A04', 'A05'], ['01'], ['05'], ['04']],
  )
  assert.equal(response.name, '检修异常安全处置')
  assert.deepEqual(
    response.steps.map(step => step.nodes.map(node => node.serviceId)),
    [['A04', 'A05', 'A06'], ['05', '01'], ['04']],
  )
  assert.equal(services.find(service => service.id === '05')?.name, '检修安全报警')
  assert.equal(services.find(service => service.id === '05')?.available, true)
})

test('hardware demo names physical devices by their shipboard responsibility', () => {
  const devices = createActuatorDemo().devices
  assert.deepEqual(devices.map(device => device.name), ['水泵 A', '水泵 B', '水泵 C'])
  assert.equal(devices.find(device => device.id === 'MOTOR-0E')?.compatible, true)
  assert.equal(devices.find(device => device.id === 'MOTOR-0F')?.compatible, true)
})
