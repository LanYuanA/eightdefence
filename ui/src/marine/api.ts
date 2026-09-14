export type MarineBackendMode = 'gateway' | 'local-demo'
type FetchLike = (input: string, init?: RequestInit) => Promise<{ ok: boolean; status: number; json(): Promise<unknown> }>

export interface MarineApiClient {
  mode(): MarineBackendMode
  health(): Promise<unknown | null>
  listApps(): Promise<any[]>
  createApp(app: unknown): Promise<any>
  updateApp(id: string, app: unknown): Promise<any>
  startRun(appId: string): Promise<any>
  listRuns(): Promise<any[]>
  getRun(id: string): Promise<any>
  pauseRun(id: string): Promise<any>
  resumeRun(id: string): Promise<any>
  cancelRun(id: string): Promise<any>
  listExecutors(): Promise<any[]>
  updateBinding(id: string, request: unknown): Promise<any>
  safety(): Promise<any>
  unlock(operator: string): Promise<any>
  lock(operator: string): Promise<any>
  listMotors(): Promise<any[]>
  startMotor(executorId: string, parameters: unknown): Promise<any>
  stopDirectMotor(executorId: string, operator: string): Promise<any>
  updateMotor(runId: string, nodeId: string, parameters: unknown): Promise<any>
  stopMotor(runId: string, nodeId: string): Promise<any>
  emergencyStop(operator: string): Promise<any>
  emergencyReset(operator: string): Promise<any>
}

export function createMarineApi(fetcher: FetchLike = fetch as unknown as FetchLike): MarineApiClient {
  let backendMode: MarineBackendMode = 'local-demo'
  async function request(path: string, method = 'GET', body?: unknown, timeoutMs = 3000) {
    const controller = new AbortController()
    const timeout = setTimeout(() => controller.abort(), timeoutMs)
    try {
      const response = await fetcher(`/api/v1${path}`, {
        method,
        headers: body === undefined ? undefined : { 'Content-Type': 'application/json' },
        body: body === undefined ? undefined : JSON.stringify(body),
        signal: controller.signal,
      })
      const data = response.status === 204 ? null : await response.json()
      if (!response.ok) {
        const message = (data as any)?.error?.message || '网关请求失败'
        throw new Error(message)
      }
      return data
    } finally { clearTimeout(timeout) }
  }
  return {
    mode: () => backendMode,
    async health() {
      try { const data = await request('/health'); backendMode = 'gateway'; return data }
      catch { backendMode = 'local-demo'; return null }
    },
    listApps: () => request('/marine/apps') as Promise<any[]>,
    createApp: app => request('/marine/apps', 'POST', app),
    updateApp: (id, app) => request(`/marine/apps/${encodeURIComponent(id)}`, 'PUT', app),
    startRun: appId => request('/marine/runs', 'POST', { appId }),
    listRuns: () => request('/marine/runs') as Promise<any[]>,
    getRun: id => request(`/marine/runs/${encodeURIComponent(id)}`),
    pauseRun: id => request(`/marine/runs/${encodeURIComponent(id)}/pause`, 'POST', {}),
    resumeRun: id => request(`/marine/runs/${encodeURIComponent(id)}/resume`, 'POST', {}),
    cancelRun: id => request(`/marine/runs/${encodeURIComponent(id)}/cancel`, 'POST', {}),
    listExecutors: () => request('/marine/executors') as Promise<any[]>,
    updateBinding: (id, body) => request(`/marine/executors/${encodeURIComponent(id)}/binding`, 'PUT', body),
    safety: () => request('/safety'),
    unlock: operator => request('/safety/unlock', 'POST', { operator }),
    lock: operator => request('/safety/lock', 'POST', { operator }),
    listMotors: () => request('/marine/motors', 'GET', undefined, 12000) as Promise<any[]>,
    startMotor: (executorId, parameters) => request(`/marine/motors/${encodeURIComponent(executorId)}/start`, 'POST', parameters, 15000),
    stopDirectMotor: (executorId, operator) => request(`/marine/motors/${encodeURIComponent(executorId)}/stop`, 'POST', { operator }, 15000),
    updateMotor: (runId, nodeId, parameters) => request(`/marine/runs/${encodeURIComponent(runId)}/nodes/${encodeURIComponent(nodeId)}/motor`, 'PATCH', parameters),
    stopMotor: (runId, nodeId) => request(`/marine/runs/${encodeURIComponent(runId)}/nodes/${encodeURIComponent(nodeId)}/stop`, 'POST', {}),
    emergencyStop: operator => request('/marine/emergency-stop', 'POST', { operator }),
    emergencyReset: operator => request('/marine/emergency-reset', 'POST', { operator }),
  }
}

export const marineApi = createMarineApi()
