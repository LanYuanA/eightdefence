<template>
  <div class="atomic-root">
    <ParticleBackground :particle-count="30" color="#8b5cf6" :opacity="0.3" />
    <AppNavbar title="原子服务管理" subtitle="Atomic Services Management" :menu-items="menuItems" />

    <main class="atomic-main">
      <!-- 服务概览 -->
      <section class="overview-section">
        <div class="section-header">
          <h2 class="section-title">
            <svg width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M13 2L3 14h9l-1 8 10-12h-9l1-8z"/></svg>
            原子服务概览
          </h2>
          <CyberButton variant="primary" size="sm" @click="showCreateService = true">
            <svg width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><line x1="12" y1="5" x2="12" y2="19"/><line x1="5" y1="12" x2="19" y2="12"/></svg>
            创建服务
          </CyberButton>
        </div>

        <div class="stats-grid">
          <div class="stat-card">
            <div class="stat-icon" style="background: rgba(139,92,246,0.2); color: #8b5cf6;">⚙️</div>
            <div class="stat-info"><div class="stat-value">{{ lowerServices.length }}</div><div class="stat-label">下层服务</div></div>
          </div>
          <div class="stat-card">
            <div class="stat-icon" style="background: rgba(99,102,241,0.2); color: #6366f1;">🔧</div>
            <div class="stat-info"><div class="stat-value">{{ upperServices.length }}</div><div class="stat-label">上层服务</div></div>
          </div>
          <div class="stat-card">
            <div class="stat-icon" style="background: rgba(34,197,94,0.2); color: #22c55d;">✅</div>
            <div class="stat-info"><div class="stat-value">{{ activeServices }}</div><div class="stat-label">活跃服务</div></div>
          </div>
          <div class="stat-card">
            <div class="stat-icon" style="background: rgba(245,158,11,0.2); color: #f59e0b;">🔄</div>
            <div class="stat-info"><div class="stat-value">{{ systemServices }}</div><div class="stat-label">系统内置</div></div>
          </div>
        </div>
      </section>

      <!-- 服务列表 -->
      <section class="services-section">
        <div class="services-grid">
          <!-- 下层基础服务 -->
          <BaseCard title="下层基础服务" subtitle="最小功能单元" icon-color="purple">
            <template #header>
              <div class="flex items-center gap-3">
                <div class="card-icon icon-purple">⚙️</div>
                <div><h3 class="card-title">下层基础服务</h3><p class="card-subtitle">最小功能单元</p></div>
              </div>
            </template>
            <div class="service-list">
              <div v-for="service in lowerServices" :key="service.id" class="service-item">
                <div class="service-icon" :style="{ background: service.color + '20', color: service.color }">
                  {{ service.icon }}
                </div>
                <div class="service-info">
                  <div class="service-name">{{ service.name }}</div>
                  <div class="service-desc">{{ service.description }}</div>
                  <div class="service-tags">
                    <span class="tag">{{ service.category }}</span>
                    <span v-if="service.is_system" class="tag system">系统内置</span>
                  </div>
                </div>
                <div class="service-actions">
                  <button class="action-btn" @click="editService(service)">编辑</button>
                  <button class="action-btn" @click="testService(service)">测试</button>
                </div>
              </div>
            </div>
          </BaseCard>

          <!-- 上层组合服务 -->
          <BaseCard title="上层组合服务" subtitle="组合下层服务实现复杂逻辑" icon-color="blue">
            <template #header>
              <div class="flex items-center gap-3">
                <div class="card-icon icon-blue">🔧</div>
                <div><h3 class="card-title">上层组合服务</h3><p class="card-subtitle">组合下层服务实现复杂逻辑</p></div>
              </div>
            </template>
            <div class="service-list">
              <div v-for="service in upperServices" :key="service.id" class="service-item">
                <div class="service-icon" :style="{ background: service.color + '20', color: service.color }">
                  {{ service.icon }}
                </div>
                <div class="service-info">
                  <div class="service-name">{{ service.name }}</div>
                  <div class="service-desc">{{ service.description }}</div>
                  <div class="service-tags">
                    <span class="tag">{{ service.category }}</span>
                    <span v-if="service.is_system" class="tag system">系统内置</span>
                  </div>
                </div>
                <div class="service-actions">
                  <button class="action-btn" @click="editService(service)">编辑</button>
                  <button class="action-btn" @click="testService(service)">测试</button>
                </div>
              </div>
            </div>
          </BaseCard>
        </div>
      </section>
    </main>

    <!-- 创建服务弹窗 -->
    <Transition name="modal">
      <div v-if="showCreateService" class="modal-overlay" @click.self="showCreateService = false">
        <div class="modal-content">
          <div class="modal-header">
            <h3>创建原子服务</h3>
            <button class="modal-close" @click="showCreateService = false">×</button>
          </div>
          <div class="modal-body">
            <div class="form-grid">
              <div class="form-group"><label>服务名称 *</label><input v-model="newService.name" placeholder="输入服务名称" /></div>
              <div class="form-group"><label>服务类型</label><select v-model="newService.type"><option value="lower">下层服务</option><option value="upper">上层服务</option></select></div>
              <div class="form-group"><label>服务分类</label><input v-model="newService.category" placeholder="如：data, control, monitor" /></div>
              <div class="form-group"><label>图标</label><input v-model="newService.icon" placeholder="输入emoji图标" /></div>
              <div class="form-group full-width"><label>描述</label><textarea v-model="newService.description" placeholder="输入服务描述" rows="3"></textarea></div>
            </div>
          </div>
          <div class="modal-footer">
            <CyberButton variant="ghost" @click="showCreateService = false">取消</CyberButton>
            <CyberButton variant="primary" @click="createService">创建</CyberButton>
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
import BaseCard from '../components/BaseCard.vue'
import CyberButton from '../components/CyberButton.vue'

const menuItems = [
  { path: '/', label: '主控台' },
  { path: '/workflow', label: '流程编排' },
  { path: '/devices', label: '设备管理' },
  { path: '/settings', label: '系统设置' }
]

const showCreateService = ref(false)

interface AtomicService {
  id: string
  name: string
  type: 'upper' | 'lower'
  category: string
  description: string
  icon: string
  color: string
  is_system: boolean
  status: string
}

const services = reactive<AtomicService[]>([
  // ===== 原有服务 =====
  { id: 'as-001', name: '读取设备数据', type: 'lower', category: 'data', description: '从设备读取数据', icon: '📖', color: '#3b82f6', is_system: true, status: 'active' },
  { id: 'as-002', name: '写入设备数据', type: 'lower', category: 'control', description: '向设备写入数据', icon: '✏️', color: '#22c55d', is_system: true, status: 'active' },
  { id: 'as-003', name: '设备状态检测', type: 'lower', category: 'monitor', description: '检测设备在线状态', icon: '🔍', color: '#f59e0b', is_system: true, status: 'active' },
  { id: 'as-004', name: '数据阈值判断', type: 'lower', category: 'condition', description: '判断数据是否超阈值', icon: '❓', color: '#ef4444', is_system: true, status: 'active' },
  { id: 'as-005', name: '发送通知', type: 'lower', category: 'action', description: '发送报警通知', icon: '📧', color: '#8b5cf6', is_system: true, status: 'active' },
  { id: 'as-006', name: '记录日志', type: 'lower', category: 'action', description: '记录操作日志', icon: '📝', color: '#06b6d4', is_system: true, status: 'active' },
  { id: 'as-007', name: '环境监测服务', type: 'upper', category: 'environment', description: '环境数据监测和报警', icon: '🌡️', color: '#22c55d', is_system: true, status: 'active' },
  { id: 'as-008', name: '安防监控服务', type: 'upper', category: 'security', description: '安防设备监控和联动', icon: '🛡️', color: '#3b82f6', is_system: true, status: 'active' },
  { id: 'as-009', name: '消防预警服务', type: 'upper', category: 'fire', description: '消防设备监控和应急', icon: '🔥', color: '#ef4444', is_system: true, status: 'active' },
  { id: 'as-010', name: '设备联动服务', type: 'upper', category: 'automation', description: '设备联动控制', icon: '🔗', color: '#8b5cf6', is_system: true, status: 'active' },

  // ===== 新增：数据分析类服务 =====
  { id: 'as-011', name: '数据融合服务', type: 'lower', category: 'data-analysis', description: '多个传感器数据融合、交叉验证、异常关联分析，提高数据准确性和可靠性', icon: '🔀', color: '#8b5cf6', is_system: true, status: 'active' },
  { id: 'as-012', name: '统计分析服务', type: 'lower', category: 'data-analysis', description: '实时数据统计，计算平均值、最大值、最小值、标准差等统计指标', icon: '📊', color: '#3b82f6', is_system: true, status: 'active' },
  { id: 'as-013', name: '趋势预测服务', type: 'lower', category: 'data-analysis', description: '基于历史数据进行趋势分析、未来值预测、季节性分析和异常检测', icon: '📈', color: '#22c55d', is_system: true, status: 'active' },
  { id: 'as-014', name: '设备健康管理服务', type: 'upper', category: 'data-analysis', description: '设备健康度评估、故障预测、维护建议生成，实现预测性维护', icon: '🏥', color: '#06b6d4', is_system: true, status: 'active' },

  // ===== 新增：自动化控制类服务 =====
  { id: 'as-015', name: '定时任务服务', type: 'lower', category: 'automation', description: '定时数据采集、定时设备控制、定时报告生成，支持Cron表达式配置', icon: '⏰', color: '#f59e0b', is_system: true, status: 'active' },
  { id: 'as-016', name: '条件联动服务', type: 'lower', category: 'automation', description: '基于条件触发的自动联动，如温度>30℃自动开空调、湿度<40%自动加湿', icon: '⚡', color: '#ef4444', is_system: true, status: 'active' },
  { id: 'as-017', name: '设备冗余切换服务', type: 'lower', category: 'automation', description: '设备故障时自动切换备用设备，实现负载均衡和高可用性', icon: '🔄', color: '#8b5cf6', is_system: true, status: 'active' },
  { id: 'as-018', name: '场景模式切换服务', type: 'upper', category: 'automation', description: '支持工作日/周末、白天/夜晚等不同场景模式的自动切换和策略配置', icon: '🎬', color: '#ec4899', is_system: true, status: 'active' },

  // ===== 新增：报警管理类服务 =====
  { id: 'as-019', name: '报警升级服务', type: 'lower', category: 'alarm', description: '报警级别自动升级，普通报警→紧急报警→危机报警，逐级升级并通知', icon: '⬆️', color: '#ef4444', is_system: true, status: 'active' },
  { id: 'as-020', name: '报警确认服务', type: 'lower', category: 'alarm', description: '报警确认、处理记录、闭环管理，支持多人确认和处理流程', icon: '✅', color: '#22c55d', is_system: true, status: 'active' },
  { id: 'as-021', name: '报警统计分析服务', type: 'upper', category: 'alarm', description: '报警频率分析、高发时段统计、设备故障率统计，生成报警分析报告', icon: '📊', color: '#f59e0b', is_system: true, status: 'active' }
])

const newService = reactive({
  name: '',
  type: 'lower' as 'upper' | 'lower',
  category: '',
  description: '',
  icon: '',
  color: '#3b82f6'
})

const lowerServices = computed(() => services.filter(s => s.type === 'lower'))
const upperServices = computed(() => services.filter(s => s.type === 'upper'))
const activeServices = computed(() => services.filter(s => s.status === 'active').length)
const systemServices = computed(() => services.filter(s => s.is_system).length)

function editService(service: AtomicService) {
  // 打开编辑弹窗（简化为修改描述）
  const newDesc = prompt('请输入新的服务描述:', service.description)
  if (newDesc && newDesc !== service.description) {
    service.description = newDesc
    ElMessage.success(`服务描述已更新`)
  }
}

function testService(service: AtomicService) {
  // 模拟测试服务
  ElMessage.success(`服务 "${service.name}" 测试通过，所有接口正常`)
}

function createService() {
  if (!newService.name) {
    ElMessage.warning('请输入服务名称')
    return
  }
  const service: AtomicService = {
    id: `as-${Date.now()}`,
    ...newService,
    is_system: false,
    status: 'active'
  }
  services.push(service)
  showCreateService.value = false
  ElMessage.success('服务已创建')
  newService.name = ''
  newService.category = ''
  newService.description = ''
  newService.icon = ''
}
</script>

<style scoped>
.atomic-root { min-height: 100vh; background: var(--bg-primary); position: relative; }
.atomic-main { position: relative; z-index: 1; padding: var(--spacing-xl); max-width: 1920px; margin: 0 auto; }

.section-header { display: flex; align-items: center; justify-content: space-between; margin-bottom: var(--spacing-lg); }
.section-title { display: flex; align-items: center; gap: var(--spacing-sm); font-size: 20px; font-weight: 600; color: var(--text-primary); margin: 0; }
.section-title svg { color: var(--accent-primary); }

.stats-grid { display: grid; grid-template-columns: repeat(4, 1fr); gap: var(--spacing-lg); margin-bottom: var(--spacing-xl); }
.stat-card { display: flex; align-items: center; gap: var(--spacing-md); padding: var(--spacing-lg); background: var(--bg-card); border: 1px solid var(--border-primary); border-radius: var(--radius-lg); }
.stat-icon { width: 48px; height: 48px; display: flex; align-items: center; justify-content: center; border-radius: var(--radius-md); font-size: 24px; }
.stat-value { font-size: 28px; font-weight: 700; color: var(--text-primary); font-family: var(--font-mono); }
.stat-label { font-size: 13px; color: var(--text-secondary); }

.services-grid { display: grid; grid-template-columns: 1fr 1fr; gap: var(--spacing-xl); }

.service-list { display: flex; flex-direction: column; gap: var(--spacing-sm); max-height: 500px; overflow-y: auto; }
.service-item { display: flex; align-items: center; gap: var(--spacing-md); padding: var(--spacing-md); background: rgba(0,0,0,0.2); border: 1px solid var(--border-primary); border-radius: var(--radius-md); transition: all var(--duration-fast); }
.service-item:hover { background: rgba(0,0,0,0.3); border-color: var(--border-accent); }
.service-icon { width: 40px; height: 40px; display: flex; align-items: center; justify-content: center; border-radius: var(--radius-md); font-size: 20px; }
.service-info { flex: 1; }
.service-name { font-size: 14px; font-weight: 500; color: var(--text-primary); }
.service-desc { font-size: 12px; color: var(--text-secondary); margin-top: 2px; }
.service-tags { display: flex; gap: 4px; margin-top: 6px; }
.tag { display: inline-flex; padding: 2px 8px; background: rgba(59,130,246,0.2); border: 1px solid rgba(59,130,246,0.3); border-radius: var(--radius-full); font-size: 11px; color: var(--accent-primary); }
.tag.system { background: rgba(139,92,246,0.2); border-color: rgba(139,92,246,0.3); color: #8b5cf6; }
.service-actions { display: flex; gap: var(--spacing-xs); }
.action-btn { padding: 4px 8px; background: transparent; border: 1px solid var(--border-primary); border-radius: var(--radius-sm); color: var(--text-secondary); font-size: 12px; cursor: pointer; transition: all var(--duration-fast); }
.action-btn:hover { border-color: var(--accent-primary); color: var(--accent-primary); }

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
.form-group input, .form-group select, .form-group textarea { padding: 10px 12px; background: var(--bg-primary); border: 1px solid var(--border-primary); border-radius: var(--radius-md); color: var(--text-primary); font-size: 14px; outline: none; }
.form-group input:focus, .form-group select:focus, .form-group textarea:focus { border-color: var(--accent-primary); }

@media (max-width: 1200px) { .stats-grid { grid-template-columns: repeat(2, 1fr); } .services-grid { grid-template-columns: 1fr; } }
@media (max-width: 768px) { .atomic-main { padding: var(--spacing-md); } .stats-grid { grid-template-columns: 1fr; } .form-grid { grid-template-columns: 1fr; } .form-group.full-width { grid-column: span 1; } }
</style>
