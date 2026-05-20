<template>
  <div class="min-h-screen flex items-center justify-center bg-[url('https://source.unsplash.com/1600x900/?technology,cyberpunk')] bg-cover bg-center">
    <div class="glass-card p-10 rounded-2xl w-96 text-center">
      <div class="text-4xl mb-6 neon-text font-bold text-sky-400">
        <i class="fa-solid w-full fa-shield-halved"></i><br/>安防网关
      </div>
      <el-input v-model="loginForm.username" placeholder="请输入用户名" class="mb-4" size="large">
      </el-input>
      <el-input v-model="loginForm.password" type="password" placeholder="请输入密码" class="mb-6" size="large" show-password @keyup.enter="handleLogin">
      </el-input>
      <el-button type="primary" size="large" class="w-full bg-sky-500 hover:bg-sky-400 border-none" @click="handleLogin">
        登录系统
      </el-button>
    </div>
  </div>
</template>

<script setup lang="ts">
import { ref } from 'vue'
import { useRouter } from 'vue-router'
import { ElMessage } from 'element-plus'

const router = useRouter()
const loginForm = ref({ username: '', password: '' })

const handleLogin = () => {
  if (loginForm.value.username === 'admin' && loginForm.value.password === '123456') {
    localStorage.setItem('isLoggedIn', 'true')
    ElMessage.success('登录成功，欢迎进入系统！')
    router.push({ name: 'dashboard' })
  } else {
    ElMessage.error('用户名或密码错误！ (admin/123456)')
  }
}
</script>

<style scoped>
.neon-text { text-shadow: 0 0 10px #38bdf8, 0 0 20px #0284c7; }
.glass-card {
  background: rgba(30, 41, 59, 0.7);
  backdrop-filter: blur(10px);
  -webkit-backdrop-filter: blur(10px);
  border: 1px solid rgba(255, 255, 255, 0.1);
  box-shadow: 0 8px 32px 0 rgba(0, 0, 0, 0.37);
}
</style>
