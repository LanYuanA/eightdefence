import { createRouter, createWebHistory } from 'vue-router'
import Login from '../views/Login.vue'
import Dashboard from '../views/Dashboard.vue'
import LogViewer from '../views/LogViewer.vue'
import Devices from '../views/Devices.vue'
import Security from '../views/Security.vue'

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
