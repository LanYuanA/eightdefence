import test from 'node:test'
import assert from 'node:assert/strict'
import { createMarineApi } from '../src/marine/api.ts'

const validApplication = {
  id: 'APP-01', name: '接口测试', description: '测试',
  steps: [{ id: 'STEP-01', nodes: [{ id: 'NODE-01', serviceId: '01', area: '机舱', intensity: 70, duration: 6 }] }],
}

test('uses gateway application API when health endpoint is available', async () => {
  const requests = []
  const api = createMarineApi(async (url, options = {}) => {
    requests.push([url, options.method || 'GET'])
    return { ok: true, status: 200, json: async () => ({ version: 'v1' }) }
  })
  await api.health()
  await api.createApp(validApplication)
  assert.equal(api.mode(), 'gateway')
  assert.deepEqual(requests, [['/api/v1/health', 'GET'], ['/api/v1/marine/apps', 'POST']])
})

test('falls back to local demo when health request fails', async () => {
  const api = createMarineApi(async () => { throw new Error('offline') })
  await api.health()
  assert.equal(api.mode(), 'local-demo')
})
