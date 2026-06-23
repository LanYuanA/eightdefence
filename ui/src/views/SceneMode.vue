<template>
  <div class="scene-root">
    <ParticleBackground :particle-count="35" color="#f59e0b" :opacity="0.3" />
    <AppNavbar title="场景模式" subtitle="Scene Mode Management" :menu-items="menuItems" />

    <main class="scene-main">
      <!-- 场景概览 -->
      <section class="overview-section">
        <div class="section-header">
          <h2 class="section-title">
            <svg width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><rect x="2" y="3" width="20" height="14" rx="2"/><path d="M8 21h8m-4-4v4"/></svg>
            场景模式管理
          </h2>
          <CyberButton variant="primary" size="sm" @click="showCreateScene = true">
            <svg width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><line x1="12" y1="5" x2="12" y2="19"/><line x1="5" y1="12" x2="19" y2="12"/></svg>
            创建场景
          </CyberButton>
        </div>

        <div class="scene-stats">
          <div class="stat-card">
            <div class="stat-icon" style="background: rgba(245,158,11,0.2); color: #f59e0b;">🎬</div>
            <div class="stat-info"><div class="stat-value">{{ scenes.length }}</div><div class="stat-label">场景总数</div></div>
          </div>
          <div class="stat-card">
            <div class="stat-icon" style="background: rgba(34,197,94,0.2); color: #22c55d;">✅</div>
            <div class="stat-info"><div class="stat-value">{{ activeScenes }}</div><div class="stat-label">活跃场景</div></div>
          </div>
          <div class="stat-card">
            <div class="stat-icon" style="background: rgba(59,130,246,0.2); color: #3b82f6;">🔗</div>
            <div class="stat-info"><div class="stat-value">{{ totalBindings }}</div><div class="stat-label">设备绑定</div></div>
          </div>
          <div class="stat-card">
            <div class="stat-icon" style="background: rgba(139,92,246,0.2); color: #8b5cf6;">⚡</div>
            <div class="stat-info"><div class="stat-value">{{ autoScenes }}</div><div class="stat-label">自动触发</div></div>
          </div>
        </div>
      </section>

      <!-- 场景列表 -->
      <section class="scenes-section">
        <div class="scenes-grid">
          <div v-for="scene in scenes" :key="scene.id" class="scene-card" :class="{ active: scene.isActive }">
            <div class="scene-header">
              <div class="scene-icon" :style="{ background: scene.color + '20', color: scene.color }">
                {{ scene.icon }}
              </div>
              <div class="scene-info">
                <div class="scene-name">{{ scene.name }}</div>
                <div class="scene-desc">{{ scene.description }}</div>
              </div>
              <div class="scene-toggle" :class="{ on: scene.isActive }" @click="toggleScene(scene)">
                <div class="toggle-thumb"></div>
              </div>
            </div>

            <div class="scene-devices">
              <div class="devices-title">绑定设备</div>
              <div class="devices-list">
                <div v-for="device in scene.devices" :key="device.id" class="device-tag">
                  <span class="device-icon">{{ device.icon }}</span>
                  <span class="device-name">{{ device.name }}</span>
                </div>
              </div>
            </div>

            <div class="scene-rules">
              <div class="rules-title">触发规则</div>
              <div class="rules-list">
                <div v-for="rule in scene.rules" :key="rule.id" class="rule-item">
                  <span class="rule-icon">{{ rule.icon }}</span>
                  <span class="rule-text">{{ rule.text }}</span>
                </div>
              </div>
            </div>

            <div class="scene-actions">
              <CyberButton variant="ghost" size="sm" @click="editScene(scene)">编辑</CyberButton>
              <CyberButton variant="ghost" size="sm" @click="testScene(scene)">测试</CyberButton>
              <CyberButton variant="danger" size="sm" @click="deleteScene(scene)">删除</CyberButton>
            </div>
          </div>
        </div>
      </section>
    </main>

    <!-- 创建场景弹窗 -->
    <Transition name="modal">
      <div v-if="showCreateScene" class="modal-overlay" @click.self="showCreateScene = false">
        <div class="modal-content">
          <div class="modal-header">
            <h3>创建场景模式</h3>
            <button class="modal-close" @click="showCreateScene = false">×</button>
          </div>
          <div class="modal-body">
            <div class="form-grid">
              <div class="form-group"><label>场景名称 *</label><input v-model="newScene.name" placeholder="输入场景名称" /></div>
              <div class="form-group"><label>图标</label><input v-model="newScene.icon" placeholder="输入emoji图标" /></div>
              <div class="form-group full-width"><label>描述</label><textarea v-model="newScene.description" placeholder="输入场景描述" rows="3"></textarea></div>
            </div>
          </div>
          <div class="modal-footer">
            <CyberButton variant="ghost" @click="showCreateScene = false">取消</CyberButton>
            <CyberButton variant="primary" @click="createScene">创建</CyberButton>
          </div>
        </div>
      </div>
    </Transition>
  </div>
</template>

<script setup lang="ts">
import { ref, reactive, computed } from 'vue'
import { ElMessage } from 'element-plus'
import ParticleBackground from '../components/ParticleBackground.vue'
import AppNavbar from '../components/AppNavbar.vue'
import CyberButton from '../components/CyberButton.vue'

const menuItems = [
  { path: '/', label: '主控台' },
  { path: '/ai-analysis', label: 'AI分析' },
  { path: '/devices', label: '设备管理' },
  { path: '/settings', label: '系统设置' }
]

const showCreateScene = ref(false)

interface SceneDevice {
  id: string
  name: string
  icon: string
}

interface SceneRule {
  id: string
  icon: string
  text: string
}

interface Scene {
  id: string
  name: string
  description: string
  icon: string
  color: string
  isActive: boolean
  isAuto: boolean
  devices: SceneDevice[]
  rules: SceneRule[]
}

const scenes = reactive<Scene[]>([
  {
    id: 'scene-001',
    name: '日常监测模式',
    description: '常规环境监测，自动调节温湿度',
    icon: '📊',
    color: '#3b82f6',
    isActive: true,
    isAuto: true,
    devices: [
      { id: 'dev-1', name: '温度传感器', icon: '🌡️' },
      { id: 'dev-2', name: '湿度传感器', icon: '💧' },
      { id: 'dev-3', name: '空调控制器', icon: '❄️' }
    ],
    rules: [
      { id: 'rule-1', icon: '🌡️', text: '温度 > 28℃ 时开启空调' },
      { id: 'rule-2', icon: '💧', text: '湿度 < 40% 时开启加湿器' }
    ]
  },
  {
    id: 'scene-002',
    name: '安防警戒模式',
    description: '高级安防监控，异常立即报警',
    icon: '🛡️',
    color: '#ef4444',
    isActive: false,
    isAuto: true,
    devices: [
      { id: 'dev-4', name: '红外探测器', icon: '👤' },
      { id: 'dev-5', name: '烟雾报警器', icon: '🔥' },
      { id: 'dev-6', name: '声光报警器', icon: '🔔' }
    ],
    rules: [
      { id: 'rule-3', icon: '👤', text: '检测到移动时触发报警' },
      { id: 'rule-4', icon: '🔥', text: '烟雾浓度超标时启动喷淋' }
    ]
  },
  {
    id: 'scene-003',
    name: '节能模式',
    description: '降低能耗，优化设备运行',
    icon: '🔋',
    color: '#22c55d',
    isActive: false,
    isAuto: false,
    devices: [
      { id: 'dev-7', name: '空调控制器', icon: '❄️' },
      { id: 'dev-8', name: '照明系统', icon: '💡' }
    ],
    rules: [
      { id: 'rule-5', icon: '❄️', text: '非工作时间关闭空调' },
      { id: 'rule-6', icon: '💡', text: '无人时自动关灯' }
    ]
  },
  {
    id: 'scene-004',
    name: '应急响应模式',
    description: '紧急情况下的快速响应',
    icon: '🚨',
    color: '#f59e0b',
    isActive: false,
    isAuto: true,
    devices: [
      { id: 'dev-9', name: '烟雾报警器', icon: '🔥' },
      { id: 'dev-10', name: '喷淋系统', icon: '💦' },
      { id: 'dev-11', name: '排烟风机', icon: '🌀' },
      { id: 'dev-12', name: '应急照明', icon: '🔦' }
    ],
    rules: [
      { id: 'rule-7', icon: '🔥', text: '火灾确认后立即启动喷淋' },
      { id: 'rule-8', icon: '🌀', text: '自动开启排烟系统' }
    ]
  }
])

const newScene = reactive({
  name: '',
  description: '',
  icon: '🎬',
  color: '#3b82f6'
})

const activeScenes = computed(() => scenes.filter(s => s.isActive).length)
const totalBindings = computed(() => scenes.reduce((sum, s) => sum + s.devices.length, 0))
const autoScenes = computed(() => scenes.filter(s => s.isAuto).length)

function toggleScene(scene: Scene) {
  scene.isActive = !scene.isActive
  ElMessage.success(`场景 "${scene.name}" 已${scene.isActive ? '激活' : '停用'}`)
}

function editScene(scene: Scene) {
  // 打开编辑弹窗（这里简化为修改名称）
  const newName = prompt('请输入新的场景名称:', scene.name)
  if (newName && newName !== scene.name) {
    scene.name = newName
    ElMessage.success(`场景名称已更新为 "${newName}"`)
  }
}

function testScene(scene: Scene) {
  // 模拟测试场景
  ElMessage.success(`场景 "${scene.name}" 测试通过，所有设备状态正常`)
}

function deleteScene(scene: Scene) {
  const index = scenes.findIndex(s => s.id === scene.id)
  if (index !== -1) {
    scenes.splice(index, 1)
    ElMessage.success(`场景 "${scene.name}" 已删除`)
  }
}

function createScene() {
  if (!newScene.name) {
    ElMessage.warning('请输入场景名称')
    return
  }
  const scene: Scene = {
    id: `scene-${Date.now()}`,
    ...newScene,
    isActive: false,
    isAuto: false,
    devices: [],
    rules: []
  }
  scenes.push(scene)
  showCreateScene.value = false
  ElMessage.success('场景已创建')
  newScene.name = ''
  newScene.description = ''
}
</script>

<style scoped>
.scene-root { min-height: 100vh; background: var(--bg-primary); position: relative; }
.scene-main { position: relative; z-index: 1; padding: var(--spacing-xl); max-width: 1920px; margin: 0 auto; }

.section-header { display: flex; align-items: center; justify-content: space-between; margin-bottom: var(--spacing-lg); }
.section-title { display: flex; align-items: center; gap: var(--spacing-sm); font-size: 20px; font-weight: 600; color: var(--text-primary); margin: 0; }
.section-title svg { color: #f59e0b; }

.scene-stats { display: grid; grid-template-columns: repeat(4, 1fr); gap: var(--spacing-lg); margin-bottom: var(--spacing-xl); }
.stat-card { display: flex; align-items: center; gap: var(--spacing-md); padding: var(--spacing-lg); background: var(--bg-card); border: 1px solid var(--border-primary); border-radius: var(--radius-lg); }
.stat-icon { width: 48px; height: 48px; display: flex; align-items: center; justify-content: center; border-radius: var(--radius-md); font-size: 24px; }
.stat-value { font-size: 28px; font-weight: 700; color: var(--text-primary); font-family: var(--font-mono); }
.stat-label { font-size: 13px; color: var(--text-secondary); }

.scenes-grid { display: grid; grid-template-columns: repeat(auto-fill, minmax(350px, 1fr)); gap: var(--spacing-xl); }

.scene-card { background: var(--bg-card); border: 1px solid var(--border-primary); border-radius: var(--radius-lg); padding: var(--spacing-lg); transition: all var(--duration-fast); }
.scene-card:hover { border-color: var(--border-accent); transform: translateY(-2px); box-shadow: var(--shadow-md); }
.scene-card.active { border-color: #f59e0b; background: rgba(245,158,11,0.05); }

.scene-header { display: flex; align-items: center; gap: var(--spacing-md); margin-bottom: var(--spacing-lg); }
.scene-icon { width: 48px; height: 48px; display: flex; align-items: center; justify-content: center; border-radius: var(--radius-md); font-size: 24px; }
.scene-info { flex: 1; }
.scene-name { font-size: 16px; font-weight: 600; color: var(--text-primary); }
.scene-desc { font-size: 12px; color: var(--text-secondary); margin-top: 4px; }
.scene-toggle { width: 48px; height: 24px; background: var(--bg-primary); border: 1px solid var(--border-primary); border-radius: 12px; cursor: pointer; position: relative; transition: all var(--duration-fast); }
.scene-toggle.on { background: #22c55d; border-color: #22c55d; }
.toggle-thumb { position: absolute; top: 2px; left: 2px; width: 18px; height: 18px; background: white; border-radius: 50%; transition: transform var(--duration-fast); }
.scene-toggle.on .toggle-thumb { transform: translateX(24px); }

.scene-devices, .scene-rules { margin-bottom: var(--spacing-md); }
.devices-title, .rules-title { font-size: 12px; font-weight: 600; color: var(--text-secondary); text-transform: uppercase; letter-spacing: 0.5px; margin-bottom: var(--spacing-sm); }
.devices-list { display: flex; flex-wrap: wrap; gap: var(--spacing-xs); }
.device-tag { display: inline-flex; align-items: center; gap: 4px; padding: 4px 8px; background: rgba(59,130,246,0.2); border: 1px solid rgba(59,130,246,0.3); border-radius: var(--radius-full); font-size: 11px; color: var(--accent-primary); }
.device-icon { font-size: 12px; }
.rules-list { display: flex; flex-direction: column; gap: var(--spacing-xs); }
.rule-item { display: flex; align-items: center; gap: 8px; padding: 6px 10px; background: rgba(0,0,0,0.2); border-radius: var(--radius-sm); font-size: 12px; color: var(--text-secondary); }
.rule-icon { font-size: 14px; }

.scene-actions { display: flex; gap: var(--spacing-sm); padding-top: var(--spacing-md); border-top: 1px solid var(--border-primary); }

.modal-overlay { position: fixed; inset: 0; background: rgba(0,0,0,0.7); backdrop-filter: blur(8px); display: flex; align-items: center; justify-content: center; z-index: var(--z-modal); }
.modal-content { background: var(--bg-card); border: 1px solid var(--border-primary); border-radius: var(--radius-xl); width: 90%; max-width: 600px; }
.modal-header { display: flex; justify-content: space-between; align-items: center; padding: var(--spacing-lg); border-bottom: 1px solid var(--border-primary); }
.modal-header h3 { font-size: 18px; font-weight: 600; color: var(--text-primary); margin: 0; }
.modal-close { width: 32px; height: 32px; display: flex; align-items: center; justify-content: center; background: transparent; border: none; color: var(--text-secondary); font-size: 20px; cursor: pointer; }
.modal-body { padding: var(--spacing-lg); }
.modal-footer { display: flex; justify-content: flex-end; gap: var(--spacing-sm); padding: var(--spacing-lg); border-top: 1px solid var(--border-primary); }

.form-grid { display: grid; grid-template-columns: repeat(2, 1fr); gap: var(--spacing-md); }
.form-group { display: flex; flex-direction: column; gap: 6px; }
.form-group.full-width { grid-column: span 2; }
.form-group label { font-size: 13px; color: var(--text-secondary); }
.form-group input, .form-group textarea { padding: 10px 12px; background: var(--bg-primary); border: 1px solid var(--border-primary); border-radius: var(--radius-md); color: var(--text-primary); font-size: 14px; outline: none; }
.form-group input:focus, .form-group textarea:focus { border-color: var(--accent-primary); }

@media (max-width: 1200px) { .scene-stats { grid-template-columns: repeat(2, 1fr); } }
@media (max-width: 768px) { .scene-main { padding: var(--spacing-md); } .scene-stats { grid-template-columns: 1fr; } .scenes-grid { grid-template-columns: 1fr; } .form-grid { grid-template-columns: 1fr; } .form-group.full-width { grid-column: span 1; } }
</style>
