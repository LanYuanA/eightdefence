<script setup lang="ts">
import { computed, onMounted, onUnmounted, ref } from 'vue'
import DecouplingSwitch from '../components/DecouplingSwitch.vue'
import ReferenceRegion from '../components/ReferenceRegion.vue'
import { buildGroupControlCommands, resolveSoftwareTask, type SoftwareTaskPlan } from '../marine/software-defined-tasks'
import { marineApi } from '../marine/api'

const DESIGN_WIDTH = 1678
const DESIGN_HEIGHT = 937
const viewport = ref<HTMLElement>()
const scale = ref(1)
let observer: ResizeObserver | undefined
onMounted(() => {
  observer = new ResizeObserver(([entry]) => {
    if (entry) scale.value = Math.min(entry.contentRect.width / DESIGN_WIDTH, entry.contentRect.height / DESIGN_HEIGHT)
  })
  if (viewport.value) observer.observe(viewport.value)
})
onUnmounted(() => { observer?.disconnect(); if (processTimer) clearInterval(processTimer) })

const task = ref('任务一：静音控制')
const taskEdited = ref(false)
const activePlan = ref<SoftwareTaskPlan | null>(null)
const state = ref<'pending' | 'edited' | 'executed'>('pending')
const message = ref('请生成任务方案，所有设备保持原状态')
const devices = Array.from({ length: 8 }, (_, index) => index + 1)
const revealStage = ref(0)
const selected = computed(() => revealStage.value >= 1 ? activePlan.value?.targets ?? [] : [])
const deviceNames = computed(() => selected.value.map(id => `电机${id}`).join('、'))
const untouched = computed(() => devices.filter(id => !selected.value.includes(id)))
const groupCommands = computed(() => revealStage.value >= 3 ? buildGroupControlCommands(activePlan.value) : [])
const groupCommand = (id: number) => groupCommands.value.find(command => command.motorId === id)
const isOriginalPlan = computed(() => revealStage.value >= 2 && activePlan.value?.key === '任务一')
const controllable = (id: number) => id <= 3
const simulated = (id: number) => id > 3
const stopped = ref<number[]>([])
const processKind = ref<'analysis' | 'reset' | 'execute' | 'stop' | null>(null)
const progress = ref(0)
let processTimer: ReturnType<typeof setInterval> | undefined
const processSteps = computed(() => processKind.value === 'reset'
  ? ['正在检查设备状态', '正在恢复所有电机转动', '正在同步设备状态', '设备状态重置完成']
  : processKind.value === 'stop'
  ? ['正在检查运行设备', '正在下发停止指令', '正在确认电机停止', '演示已终止']
  : processKind.value === 'execute'
  ? ['正在准备控制指令', '正在下发控制指令', '正在同步执行反馈', '控制指令执行完成']
  : ['正在检查现有设备', '正在查询设备状态', '正在匹配任务与控制服务', '正在生成执行方案'])
const processStep = computed(() => Math.min(3, Math.floor(progress.value / 25)))
async function runProcess(kind: 'analysis' | 'reset' | 'execute' | 'stop', complete: () => void | Promise<void>) {
  if (processKind.value) return false
  processKind.value = kind
  progress.value = 0
  processTimer = setInterval(() => {
    progress.value = Math.min(90, progress.value + 5)
  }, 160)
  try {
    await Promise.all([new Promise(resolve => setTimeout(resolve, 1800)), complete()])
    progress.value = 100
    await new Promise(resolve => setTimeout(resolve, 350))
    return true
  } catch (error) {
    message.value = `控制失败：${error instanceof Error ? error.message : '网关请求失败'}`
    return false
  } finally {
    if (processTimer) clearInterval(processTimer)
    processTimer = undefined
    processKind.value = null
  }
}
async function resetDevices() {
  await runProcess('reset', async () => {
    await marineApi.resetSoftwareDemo('现场演示员')
    stopped.value = []
    activePlan.value = null
    revealStage.value = 0
    taskEdited.value = false
    state.value = 'pending'
    message.value = '所有电机已恢复开启状态，其中电机1、2、3为真实设备'
  })
}

async function stopDemo() {
  await runProcess('stop', async () => {
    await marineApi.stopSoftwareDemo('现场演示员')
    stopped.value = [1, 2, 3]
    state.value = activePlan.value ? 'executed' : 'pending'
    message.value = '三台真实电机已停止，演示已终止'
  })
}

function generatePlan() {
  if (processKind.value) return
  const plan = resolveSoftwareTask(task.value)
  if (!plan) {
    message.value = '仅支持任务一、任务二、任务三；系统只读取输入的前三个字'
    return
  }
  activePlan.value = null
  revealStage.value = 0
  void runProcess('analysis', async () => {
    await new Promise(resolve => setTimeout(resolve, 650))
    activePlan.value = plan
    revealStage.value = 1
    await new Promise(resolve => setTimeout(resolve, 450))
    revealStage.value = 2
    await new Promise(resolve => setTimeout(resolve, 450))
    revealStage.value = 3
    await new Promise(resolve => setTimeout(resolve, 450))
    revealStage.value = 4
    state.value = 'pending'
    taskEdited.value = false
    message.value = `${plan.key}方案已生成：${plan.controllableTargets.length ? `物理可控 ${plan.controllableTargets.join('、')} 号` : '无物理控制'}；${plan.simulatedTargets.length ? `模拟展示 ${plan.simulatedTargets.join('、')} 号` : '无模拟电机'}`
  })
}

async function confirmPlan() {
  const plan = activePlan.value
  if (!plan || state.value === 'executed' || taskEdited.value || processKind.value) return
  await runProcess('execute', async () => {
    await marineApi.executeSoftwareDemo(plan.targets, '现场演示员')
    stopped.value = [...new Set([...stopped.value, ...plan.targets])]
    state.value = 'executed'
    const physical = plan.controllableTargets
    const simulation = plan.simulatedTargets
    message.value = `演示执行完成：${physical.length ? `物理可控电机 ${physical.join('、')} 已执行` : '无物理电机指令'}${simulation.length ? `；${simulation.join('、')} 为模拟展示` : ''}`
  })
}

function editTask() {
  taskEdited.value = true
  state.value = 'edited'
  message.value = '任务已修改，请按 Enter 或点击 AI生成方案；只解析前三个字'
}

const selectorX = [878, 975, 1071, 1167, 1264, 1360, 1458, 1558]
const cardX = [870, 968, 1066, 1165, 1263, 1361, 1460, 1559]
const groupPath = (index: number) => `M414 0 C414 32 ${selectorX[index]! - 850 + 43} 25 ${selectorX[index]! - 850 + 43} 72`
</script>

<template>
  <div ref="viewport" class="sdp-viewport">
    <div class="sdp-fit" :style="{ width: `${DESIGN_WIDTH * scale}px`, height: `${DESIGN_HEIGHT * scale}px` }">
      <main class="sdp-artboard" :style="{ transform: `scale(${scale})` }" aria-label="软件定义平台">
        <section class="sdp-illustration" aria-label="软件定义平台四层架构：应用软件、软件模块、设备抽象、现场硬件设备">
          <ReferenceRegion :x="0" :y="0" :width="850" :height="937" />
          <div class="left-group-module" :class="{ active: revealStage >= 2 }">
            <span class="group-module-icon">◎</span>
            <strong>电机群控模块</strong>
            <small>批量启停 · 状态编排</small>
          </div>
        </section>

        <section class="sdp-workspace" :aria-busy="!!processKind" aria-label="任务决策与执行控制台">
          <section class="task-panel" aria-label="04 应用软件 · 任务输入">
            <ReferenceRegion :x="850" :y="0" :width="828" :height="84" />
            <div class="task-content">
              <label class="task-field"><span class="sr-only">任务输入</span><input v-model="task" :disabled="!!processKind" aria-label="任务输入" spellcheck="false" @input="editTask" @keydown.enter="generatePlan" /></label>
              <button class="generate-button sprite-button" :disabled="!!processKind" aria-label="AI生成方案" title="生成静音控制演示方案" @click="generatePlan"><ReferenceRegion :x="1450" :y="84" :width="220" :height="84" /></button>
            </div>
            <ReferenceRegion class="task-bottom" :x="850" :y="163" :width="828" :height="22" />
          </section>

          <section class="decision-panel" :class="{ 'stage-active': processKind === 'analysis' && revealStage < 3 }" aria-label="03 软件模块 · AI决策方案">
            <ReferenceRegion class="decision-heading" :x="850" :y="185" :width="828" :height="62" />
            <div class="decision-title"><b>03</b><strong>软件模块 · 电机群控模块</strong></div>
            <span v-if="!activePlan || state !== 'pending'" class="plan-state" role="status">{{ !activePlan ? '等待生成方案' : state === 'executed' ? '✓ 演示完成' : '◷ 待人工确认' }}</span>
            <ReferenceRegion class="goal-row" :x="850" :y="247" :width="828" :height="41" />
            <ReferenceRegion class="decision-labels" :x="850" :y="288" :width="196" :height="79" />
            <div class="decision-values" :key="`${activePlan?.key}-${revealStage >= 1}`" :class="{ 'result-reveal': revealStage >= 1 }">
              <div class="decision-value accent" aria-label="关闭电机">
                <ReferenceRegion v-if="isOriginalPlan" :x="1046" :y="288" :width="584" :height="40" />
                <span v-else>{{ selected.join('、') || '无' }}</span>
                <span v-if="isOriginalPlan" class="sr-only">1、2、5</span>
              </div>
              <div class="decision-value remaining" :style="{ opacity: processKind === 'analysis' && revealStage < 2 ? .25 : 1 }" aria-label="其余电机">
                <ReferenceRegion v-if="isOriginalPlan" :x="1046" :y="328" :width="584" :height="39" />
                <span v-else><em>{{ untouched.join('、') || '无' }}</em> 保持原状态</span>
                <span v-if="isOriginalPlan" class="sr-only">3、4、6、7、8 保持原状态</span>
              </div>
            </div>
            <ReferenceRegion class="decision-right-edge" :x="1630" :y="288" :width="48" :height="79" />
            <div class="group-service" :class="{ active: revealStage >= 2 }">
              <div><small>AI方案输入</small><strong>{{ revealStage >= 2 ? `关闭 ${selected.join('、') || '无'}；保持 ${untouched.join('、') || '无'}` : '等待AI方案' }}</strong></div>
              <span>电机群控模块</span>
              <div><small>统一输出</small><strong class="stop-output">{{ revealStage >= 2 ? 'stop' : '--' }}</strong></div>
            </div>
            <button class="confirm-button sprite-button" :disabled="!activePlan || state === 'executed' || taskEdited || !!processKind" aria-label="人工确认执行" @click="confirmPlan"><ReferenceRegion :x="1360" :y="367" :width="318" :height="89" /></button>
            <ReferenceRegion class="decision-bottom" :x="850" :y="456" :width="828" :height="9" />
          </section>

          <section class="mapping-panel" :class="{ 'stage-active': !!processKind && (revealStage >= 3 || processKind === 'execute') }" aria-label="02 设备抽象 · 指令映射">
            <ReferenceRegion :x="850" :y="465" :width="828" :height="53" />
            <svg v-if="groupCommands.length" class="group-flow-lines" viewBox="0 0 828 75" preserveAspectRatio="none" aria-label="电机群控模块到设备抽象的数据流">
              <defs><marker id="group-arrow-red" markerWidth="6" markerHeight="6" refX="5" refY="3" orient="auto"><path d="M0,0 L6,3 L0,6 Z" fill="#ff5267"/></marker><marker id="group-arrow-green" markerWidth="6" markerHeight="6" refX="5" refY="3" orient="auto"><path d="M0,0 L6,3 L0,6 Z" fill="#35e6a4"/></marker></defs>
              <path v-for="(command, index) in groupCommands" :key="command.motorId" :class="['group-flow', command.command, { executing: processKind === 'execute' }]" :d="groupPath(index)" :marker-end="`url(#group-arrow-${command.command === 'stop' ? 'red' : 'green'})`" />
              <template v-if="processKind === 'execute'">
                <g v-for="(command, index) in groupCommands" :key="`particles-${command.motorId}`" :class="['flow-particles', command.command]">
                  <circle r="3.6">
                    <animateMotion :path="groupPath(index)" dur="1.7s" begin=".1s" repeatCount="1" fill="freeze" />
                  </circle>
                </g>
              </template>
            </svg>
            <div class="selector-row" aria-label="任务方案目标电机">
              <span v-for="(id, index) in devices" :key="id" class="motor-selector" :class="[groupCommand(id)?.command, { physical: controllable(id), simulated: simulated(id) }]" :style="{ left: `${selectorX[index]! - 850}px` }" :aria-label="`电机${id}，${groupCommand(id)?.command === 'stop' ? '关闭' : '保持开启'}`">
                <span class="selector-rebuilt"><span>{{ id }}</span><em>{{ groupCommand(id)?.command === 'stop' ? 'STOP' : groupCommand(id) ? 'ON' : '--' }}</em></span>
              </span>
            </div>
            <div class="target-row">
              <ReferenceRegion :x="850" :y="563" :width="214" :height="35" />
              <div class="target-value"><ReferenceRegion v-if="isOriginalPlan && revealStage >= 3" :x="1064" :y="563" :width="566" :height="35" /><span v-else>{{ revealStage >= 3 ? deviceNames : '无' }}</span><span v-if="isOriginalPlan && revealStage >= 3" class="sr-only">{{ deviceNames }}</span></div>
              <ReferenceRegion :x="1630" :y="563" :width="48" :height="35" />
            </div>
            <ReferenceRegion class="mapping-static" :x="850" :y="598" :width="828" :height="102" />
            <div v-if="revealStage < 3" class="empty-mapping"><span>无</span><span>无</span></div>
          </section>

          <section class="feedback-panel" :class="{ 'stage-active': !!processKind && processKind !== 'analysis' }" aria-label="01 现场硬件设备 · 执行反馈">
            <ReferenceRegion :x="850" :y="700" :width="828" :height="48" />
            <button class="stop-demo" :disabled="!!processKind" @click="stopDemo">■ 停止电机</button>
            <button class="reset-devices" :disabled="!!processKind" @click="resetDevices">↻ 重置设备状态</button>
            <div class="motor-grid">
              <span v-for="(id, index) in devices" :key="id" class="feedback-motor" :class="{ off: stopped.includes(id), on: !stopped.includes(id), physical: controllable(id), simulated: simulated(id) }" :style="{ left: `${cardX[index]! - 850}px` }" :aria-label="`电机${id}，${controllable(id) ? '真实电机' : '虚拟电机'}，${stopped.includes(id) ? '关' : '开'}`">
                <span class="device-type-badge">{{ controllable(id) ? '真实' : '虚拟' }}</span>
                <ReferenceRegion class="motor-artwork" :x="cardX[index]!" :y="748" :width="94" :height="90" />
                <span class="motor-feedback repaint" :class="{ executed: stopped.includes(id) }">
                  <span class="feedback-pill" :class="{ chosen: stopped.includes(id) }">{{ stopped.includes(id) ? '关' : '开' }}</span>
                </span>
              </span>
            </div>
            <div class="feedback-footer" role="status">
              <ReferenceRegion v-if="message === '等待人工确认后执行'" :x="850" :y="879" :width="828" :height="58" />
              <div class="feedback-notice"><ReferenceRegion :x="850" :y="879" :width="828" :height="58" /><p>{{ processKind ? processSteps[processStep] : message }}</p><div v-if="processKind" class="inline-progress" role="progressbar" :aria-valuenow="progress" aria-valuemin="0" aria-valuemax="100" aria-label="处理进度"><i :style="{ width: `${progress}%` }" /></div></div>
            </div>
          </section>
        </section>
      </main>
    </div>
    <DecouplingSwitch :disabled="!!processKind" />
  </div>
</template>

<style scoped src="../styles/software-defined-platform.css"></style>
