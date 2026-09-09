<script setup lang="ts">
import { computed } from 'vue'
import type { MarineApp } from '../../marine/model'
import type { NodeRun, RunStatus } from '../../marine/runner'
import type { SensorSnapshot } from '../../marine/sensors'

const props = defineProps<{ app: MarineApp; nodes: NodeRun[]; snapshot: SensorSnapshot; status: RunStatus }>()
const maintenance = computed(() => props.app.name === '夜间机舱检修保障')
const exception = computed(() => props.app.name === '检修异常安全处置')
const statusText: Record<RunStatus, string> = { idle: '待命', running: '运行中', paused: '已暂停', completed: '已完成', cancelled: '已停止', failed: '异常' }
function sensor(key: string) { return props.snapshot.values[key] }
function service(serviceId: string) {
  const items = props.nodes.filter(item => item.node.serviceId === serviceId)
  if (!items.length) return '待命'
  if (items.some(item => item.status === 'running')) return '执行中'
  if (items.some(item => item.status === 'failed')) return '异常'
  if (items.every(item => item.status === 'completed')) return '已完成'
  return '待命'
}
const person = computed(() => sensor('ir')?.value ?? '—')
const light = computed(() => sensor('lux'))
const smoke = computed(() => sensor('smoke'))
const water = computed(() => sensor('water'))
</script>

<template>
  <section class="command-center" :class="{ maintenance, exception }">
    <header>
      <div><span>APPLICATION CONSOLE</span><h2>{{ app.name }}</h2></div>
      <strong :class="status">{{ statusText[status] }}</strong>
    </header>
    <div class="command-grid">
      <article class="command-card personnel" v-if="maintenance || exception"><span>人员状态</span><strong>{{ person }}</strong><small>机舱检修区域</small></article>
      <article class="command-card illumination" v-if="maintenance"><span>检修照度</span><strong>{{ light?.online ? `${light.value} ${light.unit}` : '设备离线' }}</strong><small>{{ light?.online && Number(light.value) < 150 ? '低照度提醒' : '照度正常' }}</small></article>
      <article class="command-card smoke"><span>烟雾火情</span><strong>{{ smoke?.online ? smoke.value : '设备离线' }}</strong><small>{{ smoke?.online ? '持续监测' : '等待设备恢复' }}</small></article>
      <article class="command-card water"><span>舱底水浸</span><strong>{{ water?.online ? water.value : '设备离线' }}</strong><small>{{ water?.online ? '持续监测' : '等待设备恢复' }}</small></article>
      <article class="command-card ventilation"><span>机舱通风机 1 号</span><strong>{{ service('01') }}</strong><small>VENT-01 · {{ maintenance ? '检修通风' : '通风安全策略' }}</small></article>
      <article class="command-card alarm"><span>检修安全报警</span><strong>{{ service('05') }}</strong><small>报警与联动状态</small></article>
      <article class="command-card drainage"><span>舱底排水泵 2 号</span><strong>{{ service('04') }}</strong><small>DRAIN-01 · 舱底排水</small></article>
    </div>
  </section>
</template>

<style scoped>
.command-center{margin:20px 0;padding:20px 22px;border:1px solid #2e5369;border-radius:10px;background:radial-gradient(circle at 75% 15%,#37a5a21c,transparent 35%),linear-gradient(130deg,#0d2030,#071522)}.command-center.exception{background:radial-gradient(circle at 75% 15%,#d26a5b20,transparent 35%),linear-gradient(130deg,#26191f,#0b1621)}.command-center header{display:flex;align-items:center;justify-content:space-between;margin-bottom:16px}.command-center header span{color:#6dddcf;font:600 10px monospace;letter-spacing:1.5px}.command-center h2{margin:5px 0 0;color:#e7f2f5;font-size:22px}.command-center header>strong{padding:6px 11px;border:1px solid #568095;border-radius:20px;color:#8eabb9;font-size:12px}.command-center header>strong.running{border-color:#59d9b2;color:#6de1cd;background:#6de1cd12}.command-center header>strong.failed{border-color:#e8736f;color:#ff9a91}.command-grid{display:grid;grid-template-columns:repeat(4,minmax(0,1fr));gap:10px}.command-card{min-height:99px;padding:13px;border:1px solid #315066;border-radius:7px;background:#0715229c}.command-card span,.command-card small{display:block;color:#7895a6;font-size:11px}.command-card strong{display:block;margin:11px 0 5px;color:#d8e8ed;font-size:19px}.command-card.ventilation strong{color:#6de1cd}.command-card.alarm strong{color:#f3b96a}.command-card.water strong,.command-card.drainage strong{color:#74b9ff}.command-card.smoke strong{color:#f58d84}@media(max-width:1000px){.command-grid{grid-template-columns:repeat(2,1fr)}}
</style>
