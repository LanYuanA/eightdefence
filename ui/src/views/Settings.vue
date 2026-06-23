<template>
  <div class="settings-root">
    <ParticleBackground :particle-count="30" color="#6366f1" :opacity="0.3" />
    <AppNavbar title="系统设置" subtitle="System Settings" :menu-items="menuItems" />

    <main class="settings-main">
      <div class="settings-grid">
        <!-- 侧边导航 -->
        <div class="settings-nav">
          <div v-for="section in sections" :key="section.id" class="nav-item" :class="{ active: activeSection === section.id }" @click="activeSection = section.id">
            <span class="nav-icon">{{ section.icon }}</span>
            <span class="nav-label">{{ section.label }}</span>
          </div>
        </div>

        <!-- 设置内容 -->
        <div class="settings-content">
          <!-- 通用设置 -->
          <div v-if="activeSection === 'general'" class="settings-section">
            <BaseCard title="通用设置" subtitle="系统基本配置" icon-color="blue">
              <template #header>
                <div class="flex items-center gap-3">
                  <div class="card-icon icon-blue">⚙️</div>
                  <div><h3 class="card-title">通用设置</h3><p class="card-subtitle">系统基本配置</p></div>
                </div>
              </template>
              <div class="settings-list">
                <div class="setting-item"><div class="setting-info"><div class="setting-label">系统名称</div><div class="setting-desc">显示在导航栏的系统名称</div></div><input v-model="settings.systemName" class="setting-input" /></div>
                <div class="setting-item"><div class="setting-info"><div class="setting-label">数据刷新间隔</div><div class="setting-desc">自动刷新数据的时间间隔</div></div><select v-model="settings.refreshInterval" class="setting-select"><option value="1000">1秒</option><option value="2000">2秒</option><option value="5000">5秒</option><option value="10000">10秒</option></select></div>
                <div class="setting-item"><div class="setting-info"><div class="setting-label">语言</div><div class="setting-desc">系统界面语言</div></div><select v-model="settings.language" class="setting-select"><option value="zh-CN">简体中文</option><option value="en-US">English</option></select></div>
                <div class="setting-item"><div class="setting-info"><div class="setting-label">主题</div><div class="setting-desc">界面主题风格</div></div><select v-model="settings.theme" class="setting-select"><option value="dark">深色主题</option><option value="light">浅色主题</option></select></div>
              </div>
            </BaseCard>
          </div>

          <!-- 通知设置 -->
          <div v-if="activeSection === 'notifications'" class="settings-section">
            <BaseCard title="通知设置" subtitle="告警通知配置" icon-color="yellow">
              <template #header>
                <div class="flex items-center gap-3">
                  <div class="card-icon icon-yellow">🔔</div>
                  <div><h3 class="card-title">通知设置</h3><p class="card-subtitle">告警通知配置</p></div>
                </div>
              </template>
              <div class="settings-list">
                <div class="setting-item"><div class="setting-info"><div class="setting-label">声音通知</div><div class="setting-desc">收到告警时播放声音</div></div><div class="setting-switch" :class="{ on: settings.soundEnabled }" @click="settings.soundEnabled = !settings.soundEnabled"><div class="switch-thumb"></div></div></div>
                <div class="setting-item"><div class="setting-info"><div class="setting-label">桌面通知</div><div class="setting-desc">显示桌面弹窗通知</div></div><div class="setting-switch" :class="{ on: settings.desktopEnabled }" @click="settings.desktopEnabled = !settings.desktopEnabled"><div class="switch-thumb"></div></div></div>
                <div class="setting-item"><div class="setting-info"><div class="setting-label">邮件通知</div><div class="setting-desc">发送告警邮件到指定邮箱</div></div><div class="setting-switch" :class="{ on: settings.emailEnabled }" @click="settings.emailEnabled = !settings.emailEnabled"><div class="switch-thumb"></div></div></div>
                <div class="setting-item"><div class="setting-info"><div class="setting-label">通知邮箱</div><div class="setting-desc">接收告警通知的邮箱地址</div></div><input v-model="settings.email" class="setting-input" placeholder="admin@example.com" /></div>
              </div>
            </BaseCard>
          </div>

          <!-- 设备设置 -->
          <div v-if="activeSection === 'devices'" class="settings-section">
            <BaseCard title="设备设置" subtitle="设备管理配置" icon-color="green">
              <template #header>
                <div class="flex items-center gap-3">
                  <div class="card-icon icon-green">📡</div>
                  <div><h3 class="card-title">设备设置</h3><p class="card-subtitle">设备管理配置</p></div>
                </div>
              </template>
              <div class="settings-list">
                <div class="setting-item"><div class="setting-info"><div class="setting-label">自动发现设备</div><div class="setting-desc">自动扫描并添加新设备</div></div><div class="setting-switch" :class="{ on: settings.autoDiscover }" @click="settings.autoDiscover = !settings.autoDiscover"><div class="switch-thumb"></div></div></div>
                <div class="setting-item"><div class="setting-info"><div class="setting-label">设备超时时间</div><div class="setting-desc">设备无响应判定为离线的时间</div></div><select v-model="settings.deviceTimeout" class="setting-select"><option value="30">30秒</option><option value="60">1分钟</option><option value="120">2分钟</option><option value="300">5分钟</option></select></div>
                <div class="setting-item"><div class="setting-info"><div class="setting-label">轮询间隔</div><div class="setting-desc">设备数据轮询的时间间隔</div></div><select v-model="settings.pollInterval" class="setting-select"><option value="1000">1秒</option><option value="2000">2秒</option><option value="5000">5秒</option></select></div>
              </div>
            </BaseCard>
          </div>

          <!-- 安全设置 -->
          <div v-if="activeSection === 'security'" class="settings-section">
            <BaseCard title="安全设置" subtitle="系统安全配置" icon-color="red">
              <template #header>
                <div class="flex items-center gap-3">
                  <div class="card-icon icon-red">🔒</div>
                  <div><h3 class="card-title">安全设置</h3><p class="card-subtitle">系统安全配置</p></div>
                </div>
              </template>
              <div class="settings-list">
                <div class="setting-item"><div class="setting-info"><div class="setting-label">修改密码</div><div class="setting-desc">更改登录密码</div></div><CyberButton variant="ghost" size="sm" @click="showChangePassword = true">修改</CyberButton></div>
                <div class="setting-item"><div class="setting-info"><div class="setting-label">登录超时</div><div class="setting-desc">无操作自动退出的时间</div></div><select v-model="settings.loginTimeout" class="setting-select"><option value="1800">30分钟</option><option value="3600">1小时</option><option value="7200">2小时</option><option value="0">永不</option></select></div>
                <div class="setting-item"><div class="setting-info"><div class="setting-label">操作日志</div><div class="setting-desc">记录用户操作日志</div></div><div class="setting-switch" :class="{ on: settings.auditLog }" @click="settings.auditLog = !settings.auditLog"><div class="switch-thumb"></div></div></div>
              </div>
            </BaseCard>
          </div>

          <!-- 保存按钮 -->
          <div class="settings-actions">
            <CyberButton variant="ghost" @click="resetSettings">重置默认</CyberButton>
            <CyberButton variant="primary" @click="saveSettings">保存设置</CyberButton>
          </div>
        </div>
      </div>
    </main>

    <!-- 修改密码弹窗 -->
    <Transition name="modal">
      <div v-if="showChangePassword" class="modal-overlay" @click.self="showChangePassword = false">
        <div class="modal-content">
          <div class="modal-header">
            <h3>修改密码</h3>
            <button class="modal-close" @click="showChangePassword = false">×</button>
          </div>
          <div class="modal-body">
            <div class="form-grid">
              <div class="form-group full-width">
                <label>当前密码 *</label>
                <input v-model="passwordForm.oldPassword" type="password" placeholder="输入当前密码" />
              </div>
              <div class="form-group">
                <label>新密码 *</label>
                <input v-model="passwordForm.newPassword" type="password" placeholder="输入新密码" />
              </div>
              <div class="form-group">
                <label>确认新密码 *</label>
                <input v-model="passwordForm.confirmPassword" type="password" placeholder="再次输入新密码" />
              </div>
            </div>
          </div>
          <div class="modal-footer">
            <CyberButton variant="ghost" @click="showChangePassword = false">取消</CyberButton>
            <CyberButton variant="primary" @click="changePassword">确认修改</CyberButton>
          </div>
        </div>
      </div>
    </Transition>
  </div>
</template>

<script setup lang="ts">
import { ref, reactive } from 'vue'
import { ElMessage } from 'element-plus'
import ParticleBackground from '../components/ParticleBackground.vue'
import AppNavbar from '../components/AppNavbar.vue'
import BaseCard from '../components/BaseCard.vue'
import CyberButton from '../components/CyberButton.vue'

const menuItems = [
  { path: '/', label: '主控台' },
  { path: '/security', label: '安防系统' },
  { path: '/environment', label: '环境监测' },
  { path: '/fire', label: '消防系统' },
  { path: '/devices', label: '设备管理' }
]

const activeSection = ref('general')
const showChangePassword = ref(false)

const sections = [
  { id: 'general', icon: '⚙️', label: '通用设置' },
  { id: 'notifications', icon: '🔔', label: '通知设置' },
  { id: 'devices', icon: '📡', label: '设备设置' },
  { id: 'security', icon: '🔒', label: '安全设置' }
]

const STORAGE_KEY = 'system-settings'

// 默认设置
const defaultSettings = {
  systemName: '软件定义消防系统',
  refreshInterval: '2000',
  language: 'zh-CN',
  theme: 'dark',
  soundEnabled: true,
  desktopEnabled: true,
  emailEnabled: false,
  email: 'admin@example.com',
  autoDiscover: true,
  deviceTimeout: '60',
  pollInterval: '2000',
  loginTimeout: '3600',
  auditLog: true
}

const settings = reactive({ ...defaultSettings })

// 密码表单
const passwordForm = reactive({
  oldPassword: '',
  newPassword: '',
  confirmPassword: ''
})

// 从localStorage加载设置
function loadSettings() {
  try {
    const saved = localStorage.getItem(STORAGE_KEY)
    if (saved) {
      const parsed = JSON.parse(saved)
      Object.assign(settings, parsed)
    }
  } catch (e) {
    console.error('加载设置失败:', e)
  }
}

// 保存设置
function saveSettings() {
  try {
    localStorage.setItem(STORAGE_KEY, JSON.stringify(settings))
    ElMessage.success('设置已保存')
  } catch (e) {
    ElMessage.error('保存设置失败')
  }
}

// 重置设置
function resetSettings() {
  Object.assign(settings, defaultSettings)
  localStorage.removeItem(STORAGE_KEY)
  ElMessage.success('设置已重置为默认值')
}

// 修改密码
function changePassword() {
  if (!passwordForm.oldPassword || !passwordForm.newPassword || !passwordForm.confirmPassword) {
    ElMessage.warning('请填写所有密码字段')
    return
  }
  if (passwordForm.newPassword !== passwordForm.confirmPassword) {
    ElMessage.warning('两次输入的新密码不一致')
    return
  }
  if (passwordForm.newPassword.length < 6) {
    ElMessage.warning('新密码长度不能少于6位')
    return
  }

  // 模拟修改密码成功
  showChangePassword.value = false
  passwordForm.oldPassword = ''
  passwordForm.newPassword = ''
  passwordForm.confirmPassword = ''
  ElMessage.success('密码修改成功')
}

// 组件挂载时加载设置
loadSettings()
</script>

<style scoped>
.settings-root { min-height: 100vh; background: var(--bg-primary); position: relative; }
.settings-main { position: relative; z-index: 1; padding: var(--spacing-xl); max-width: 1400px; margin: 0 auto; }

.settings-grid { display: grid; grid-template-columns: 220px 1fr; gap: var(--spacing-xl); }

.settings-nav { display: flex; flex-direction: column; gap: var(--spacing-xs); }
.nav-item { display: flex; align-items: center; gap: var(--spacing-sm); padding: var(--spacing-md); background: var(--bg-card); border: 1px solid var(--border-primary); border-radius: var(--radius-md); cursor: pointer; transition: all var(--duration-fast); }
.nav-item:hover { border-color: var(--accent-primary); }
.nav-item.active { background: rgba(59,130,246,0.2); border-color: var(--accent-primary); }
.nav-icon { font-size: 20px; }
.nav-label { font-size: 14px; font-weight: 500; color: var(--text-primary); }

.settings-content { }
.settings-section { margin-bottom: var(--spacing-xl); }

.settings-list { display: flex; flex-direction: column; }
.setting-item { display: flex; align-items: center; justify-content: space-between; padding: var(--spacing-md) 0; border-bottom: 1px solid var(--border-primary); }
.setting-item:last-child { border-bottom: none; }
.setting-label { font-size: 14px; font-weight: 500; color: var(--text-primary); }
.setting-desc { font-size: 12px; color: var(--text-secondary); margin-top: 2px; }
.setting-input { width: 250px; padding: 8px 12px; background: var(--bg-primary); border: 1px solid var(--border-primary); border-radius: var(--radius-md); color: var(--text-primary); font-size: 14px; outline: none; }
.setting-input:focus { border-color: var(--accent-primary); }
.setting-select { width: 250px; padding: 8px 12px; background: var(--bg-primary); border: 1px solid var(--border-primary); border-radius: var(--radius-md); color: var(--text-primary); font-size: 14px; outline: none; }

.setting-switch { width: 48px; height: 24px; background: var(--bg-primary); border: 1px solid var(--border-primary); border-radius: 12px; cursor: pointer; position: relative; transition: all var(--duration-fast); }
.setting-switch.on { background: var(--status-success); border-color: var(--status-success); }
.switch-thumb { position: absolute; top: 2px; left: 2px; width: 18px; height: 18px; background: white; border-radius: 50%; transition: transform var(--duration-fast); }
.setting-switch.on .switch-thumb { transform: translateX(24px); }

.settings-actions { display: flex; justify-content: flex-end; gap: var(--spacing-sm); margin-top: var(--spacing-xl); }

@media (max-width: 768px) { .settings-grid { grid-template-columns: 1fr; } }
</style>
