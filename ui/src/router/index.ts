import { createRouter, createWebHistory } from 'vue-router'
import Login from '../views/Login.vue'
import Dashboard from '../views/Dashboard.vue'
import LogViewer from '../views/LogViewer.vue'
import Devices from '../views/Devices.vue'
import Security from '../views/Security.vue'
import Environment from '../views/Environment.vue'
import FireFighting from '../views/FireFighting.vue'
import AlarmCenter from '../views/AlarmCenter.vue'
import DataScreen from '../views/DataScreen.vue'
import Settings from '../views/Settings.vue'
import MarineDemo from '../views/MarineDemo.vue'
import ResourcePool from '../views/ResourcePool.vue'
import DeviceGraph from '../views/DeviceGraph.vue'
import HardwareDecoupling from '../views/HardwareDecoupling.vue'
import SoftwareDefinedPlatform from '../views/SoftwareDefinedPlatform.vue'

const router = createRouter({
  history: createWebHistory('/'),
  routes: [
    { path: '/software-defined-platform', name: 'software-defined-platform', component: SoftwareDefinedPlatform, meta: { public: true } },
    { path: '/cooling-replica', redirect: '/hardware-decoupling' },
    { path: '/legacy-dashboard', name: 'legacy-dashboard', component: Dashboard },
    { path: '/workflow', redirect: '/' },
    { path: '/marine', redirect: '/' },
    {
      path: '/login',
      name: 'login',
      component: Login
    },
    {
      path: '/',
      name: 'dashboard',
      redirect: '/software-defined-platform'
    },
    {
      path: '/logs',
      name: 'logs',
      component: LogViewer
    },
    {
      path: '/devices',
      name: 'devices',
      component: Devices
    },
    {
      path: '/security',
      name: 'security',
      component: Security
    },
    {
      path: '/environment',
      name: 'environment',
      component: Environment
    },
    {
      path: '/fire',
      name: 'fire',
      component: FireFighting
    },
    {
      path: '/alarms',
      name: 'alarms',
      component: AlarmCenter
    },
    {
      path: '/screen',
      name: 'screen',
      component: DataScreen
    },
    {
      path: '/settings',
      name: 'settings',
      component: Settings
    },
    {
      path: '/atomic-services',
      name: 'atomic-services',
      component: MarineDemo
    },
    {
      path: '/hardware-decoupling',
      name: 'hardware-decoupling',
      component: HardwareDecoupling
    },
    {
      path: '/resource-pool',
      name: 'resource-pool',
      component: ResourcePool
    },
    {
      path: '/device-graph',
      name: 'device-graph',
      component: DeviceGraph
    }
  ]
})

router.beforeEach((to) => {
  const isLoggedIn = localStorage.getItem('isLoggedIn')
  if (!to.meta.public && to.name !== 'login' && to.name !== 'dashboard' && to.name !== 'atomic-services' && to.name !== 'hardware-decoupling' && !isLoggedIn) {
    return { name: 'login', query: { redirect: to.fullPath } }
  } else if (to.name === 'login' && isLoggedIn) {
    return { name: 'dashboard' }
  }
  return true
})

export default router
