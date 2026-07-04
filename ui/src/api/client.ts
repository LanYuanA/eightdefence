import axios from 'axios'

// C++ 内嵌HTTP服务器 (实时数据) — 端口8080
const cppClient = axios.create({
  baseURL: 'http://localhost:8080/api',
  timeout: 5000,
  headers: { 'Content-Type': 'application/json' }
})

// Express.js 服务器 (持久化数据) — 端口3000
const expressClient = axios.create({
  baseURL: 'http://localhost:3000/api',
  timeout: 10000,
  headers: { 'Content-Type': 'application/json' }
})

// Express请求拦截器: 自动附加JWT Token
expressClient.interceptors.request.use(config => {
  const token = localStorage.getItem('token')
  if (token) config.headers.Authorization = `Bearer ${token}`
  return config
})

// 统一错误处理
;[cppClient, expressClient].forEach(client => {
  client.interceptors.response.use(
    r => r.data,
    error => {
      const msg = error.response?.data?.message || error.message || '网络错误'
      console.warn('[API]', msg)
      return Promise.reject(error)
    }
  )
})

export { cppClient, expressClient }
