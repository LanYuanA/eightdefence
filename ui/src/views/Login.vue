<template>
  <div class="login-root min-h-screen flex items-center justify-center">
    <!-- 背景装饰 -->
    <div class="login-bg-pattern"></div>
    
    <div class="login-card">
      <!-- 品牌区域 -->
      <div class="brand-section">
        <div class="brand-logo">
          <svg width="32" height="32" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.8">
            <path d="M12 22s8-4 8-10V5l-8-3-8 3v7c0 6 8 10 8 10z"/>
          </svg>
        </div>
        <h1 class="brand-name">智慧环控安防系统</h1>
        <p class="brand-desc">Smart Environment & Security Platform</p>
      </div>

      <!-- 表单区域 -->
      <div class="form-section">
        <div class="input-group">
          <label class="input-label">用户名</label>
          <el-input 
            v-model="loginForm.username" 
            placeholder="请输入用户名" 
            size="large"
            :prefix-icon="UserIcon"
          />
        </div>
        <div class="input-group">
          <label class="input-label">密码</label>
          <el-input 
            v-model="loginForm.password" 
            type="password" 
            placeholder="请输入密码" 
            size="large" 
            show-password 
            @keyup.enter="handleLogin"
            :prefix-icon="LockIcon"
          />
        </div>
        <button class="login-btn" @click="handleLogin">
          <svg width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" class="mr-2">
            <path d="M15 3h4a2 2 0 0 1 2 2v14a2 2 0 0 1-2 2h-4"/><polyline points="10 17 15 12 10 7"/><line x1="15" y1="12" x2="3" y2="12"/>
          </svg>
          登录系统
        </button>
      </div>

      <!-- 底部信息 -->
      <div class="login-footer">
        <div class="footer-hint">
          <svg width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" class="mr-1.5 opacity-50">
            <circle cx="12" cy="12" r="10"/><path d="M12 16v-4M12 8h.01"/>
          </svg>
          默认账户: admin / 123456
        </div>
      </div>
    </div>
  </div>
</template>

<script setup lang="ts">
import { ref, h } from 'vue'
import { useRouter } from 'vue-router'
import { ElMessage } from 'element-plus'

const router = useRouter()
const loginForm = ref({ username: '', password: '' })

// 简单的图标组件
const UserIcon = {
  render() {
    return h('svg', { 
      width: '16', height: '16', viewBox: '0 0 24 24', fill: 'none', 
      stroke: 'currentColor', 'stroke-width': '2'
    }, [
      h('path', { d: 'M20 21v-2a4 4 0 0 0-4-4H8a4 4 0 0 0-4 4v2' }),
      h('circle', { cx: '12', cy: '7', r: '4' })
    ])
  }
}

const LockIcon = {
  render() {
    return h('svg', { 
      width: '16', height: '16', viewBox: '0 0 24 24', fill: 'none', 
      stroke: 'currentColor', 'stroke-width': '2'
    }, [
      h('rect', { x: '3', y: '11', width: '18', height: '11', rx: '2', ry: '2' }),
      h('path', { d: 'M7 11V7a5 5 0 0 1 10 0v4' })
    ])
  }
}

const handleLogin = () => {
  if (loginForm.value.username === 'admin' && loginForm.value.password === '123456') {
    localStorage.setItem('isLoggedIn', 'true')
    ElMessage.success('登录成功，欢迎进入系统！')
    router.push({ name: 'dashboard' })
  } else {
    ElMessage.error('用户名或密码错误')
  }
}
</script>

<style scoped>
.login-root {
  background: #0f172a;
  font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", "PingFang SC", "Hiragino Sans GB", "Microsoft YaHei", sans-serif;
  position: relative;
  overflow: hidden;
}

.login-bg-pattern {
  position: absolute;
  inset: 0;
  background-image:
    radial-gradient(ellipse at 30% 20%, rgba(99, 102, 241, 0.12) 0%, transparent 50%),
    radial-gradient(ellipse at 70% 80%, rgba(59, 130, 246, 0.08) 0%, transparent 50%);
  pointer-events: none;
}

.login-card {
  position: relative;
  width: 400px;
  background: rgba(30, 41, 59, 0.7);
  backdrop-filter: blur(20px);
  border: 1px solid rgba(148, 163, 184, 0.1);
  border-radius: 16px;
  padding: 40px 36px 32px;
  box-shadow: 
    0 4px 24px rgba(0, 0, 0, 0.2),
    0 0 0 1px rgba(148, 163, 184, 0.05);
}

.brand-section {
  text-align: center;
  margin-bottom: 36px;
}

.brand-logo {
  display: inline-flex;
  align-items: center;
  justify-content: center;
  width: 64px;
  height: 64px;
  border-radius: 16px;
  background: linear-gradient(135deg, #6366f1, #3b82f6);
  color: white;
  margin-bottom: 16px;
  box-shadow: 0 4px 16px rgba(99, 102, 241, 0.3);
}

.brand-name {
  font-size: 22px;
  font-weight: 700;
  color: #f1f5f9;
  margin: 0;
  letter-spacing: 0.02em;
}

.brand-desc {
  font-size: 12px;
  color: #64748b;
  margin: 6px 0 0;
  letter-spacing: 0.05em;
}

.form-section {
  display: flex;
  flex-direction: column;
  gap: 18px;
}

.input-group {
  display: flex;
  flex-direction: column;
  gap: 6px;
}

.input-label {
  font-size: 13px;
  font-weight: 500;
  color: #94a3b8;
}

.login-btn {
  display: flex;
  align-items: center;
  justify-content: center;
  width: 100%;
  padding: 12px;
  margin-top: 8px;
  font-size: 15px;
  font-weight: 600;
  color: white;
  background: linear-gradient(135deg, #6366f1, #4f46e5);
  border: none;
  border-radius: 10px;
  cursor: pointer;
  transition: all 0.2s;
  box-shadow: 0 2px 8px rgba(99, 102, 241, 0.3);
}
.login-btn:hover {
  background: linear-gradient(135deg, #818cf8, #6366f1);
  box-shadow: 0 4px 16px rgba(99, 102, 241, 0.4);
  transform: translateY(-1px);
}
.login-btn:active {
  transform: translateY(0);
}

.login-footer {
  text-align: center;
  margin-top: 24px;
  padding-top: 20px;
  border-top: 1px solid rgba(148, 163, 184, 0.08);
}

.footer-hint {
  display: inline-flex;
  align-items: center;
  font-size: 12px;
  color: #64748b;
}

/* Element Plus 输入框样式覆盖 */
:deep(.el-input__wrapper) {
  background: rgba(15, 23, 42, 0.6) !important;
  border: 1px solid rgba(148, 163, 184, 0.12) !important;
  border-radius: 8px !important;
  box-shadow: none !important;
  transition: border-color 0.2s;
}
:deep(.el-input__wrapper:hover) {
  border-color: rgba(99, 102, 241, 0.3) !important;
}
:deep(.el-input__wrapper.is-focus) {
  border-color: rgba(99, 102, 241, 0.5) !important;
  box-shadow: 0 0 0 2px rgba(99, 102, 241, 0.1) !important;
}
:deep(.el-input__inner) {
  color: #e2e8f0 !important;
  font-size: 14px;
}
:deep(.el-input__inner::placeholder) {
  color: #475569 !important;
}
:deep(.el-input__prefix .el-icon) {
  color: #64748b;
}
</style>