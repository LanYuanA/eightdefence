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
import AtomicServices from '../views/AtomicServices.vue'
import ResourcePool from '../views/ResourcePool.vue'
import DeviceGraph from '../views/DeviceGraph.vue'

const router = createRouter({
  history: createWebHistory('/'),
  routes: [
    {
      path: '/login',
      name: 'login',
      component: Login
    },
    {
      path: '/',
      name: 'dashboard',
      component: Dashboard
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
      component: AtomicServices
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

router.beforeEach((to, _from, next) => {
  const isLoggedIn = localStorage.getItem('isLoggedIn')
  if (to.name !== 'login' && !isLoggedIn) {
    next({ name: 'login' })
  } else if (to.name === 'login' && isLoggedIn) {
    next({ name: 'dashboard' })
  } else {
    next()
  }
})

export default router
