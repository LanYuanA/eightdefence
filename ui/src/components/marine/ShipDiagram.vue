<script setup lang="ts">
import type { NodeRun } from '../../marine/runner'
import { services } from '../../marine/model'
const props = defineProps<{ nodes: NodeRun[]; paused: boolean }>()
const actionServices = services.filter(service => service.kind === 'action' && service.available)
function active(id: string) { return props.nodes.some(node => node.node.serviceId === id && node.status === 'running') }
function completed(id: string) { return props.nodes.some(node => node.node.serviceId === id && node.status === 'completed') }
const servicePositions: Record<string, { x: number; y: number; labelY: number; labelWidth: number }> = {
  '01': { x: 260, y: 105, labelY: 33, labelWidth: 140 },
  '02': { x: 430, y: 236, labelY: 58, labelWidth: 140 },
  '03': { x: 595, y: 236, labelY: 58, labelWidth: 140 },
  '04': { x: 760, y: 236, labelY: 58, labelWidth: 140 },
  '05': { x: 860, y: 187, labelY: 108, labelWidth: 150 },
}
function position(id: string) { return servicePositions[id] || servicePositions['01'] }
</script>
<template>
  <div class="ship-diagram" :class="{ paused }">
    <div class="ship-caption"><span>民用船舶 / 能力分布</span><span>软件模拟 · 非真实遥测</span></div>
    <svg viewBox="0 0 1000 350" role="img" aria-label="民用船舶示意图，显示通风、冷却、供水和排水服务运行区域">
      <defs>
        <linearGradient id="hull" x1="0" y1="0" x2="0" y2="1"><stop stop-color="#27455d"/><stop offset="1" stop-color="#122637"/></linearGradient>
        <pattern id="sea-grid" width="40" height="40" patternUnits="userSpaceOnUse"><path d="M40 0H0V40" fill="none" stroke="#28475d" stroke-width=".6"/></pattern>
      </defs>
      <rect width="1000" height="350" fill="url(#sea-grid)" opacity=".45"/>
      <path d="M30 280H970M65 295H930M110 310H875" stroke="#4d9daf" opacity=".25"/>
      <path d="M90 212L875 212L940 187L899 248Q882 275 838 277H180Q133 275 90 212Z" fill="url(#hull)" stroke="#6a9bb4" stroke-width="1.6"/>
      <path d="M120 226H902M192 257H872" fill="none" stroke="#49728e"/>
      <path d="M188 211V138H337V211M205 138V107H318V138M218 107V86H288V107M240 86V50M230 65H281" fill="#1a354b" stroke="#7a9eb4" stroke-width="1.5"/>
      <path d="M212 151H230V171H212ZM241 151H259V171H241ZM270 151H288V171H270ZM299 151H317V171H299Z" fill="#83c4d6" opacity=".5"/>
      <path d="M214 119H304M364 210V159H482V210M502 210V159H620V210M640 210V159H758V210M778 210V172H850V210" fill="#172f43" stroke="#5b8097"/>
      <path d="M390 163V207M417 163V207M444 163V207M528 163V207M555 163V207M582 163V207M666 163V207M693 163V207M720 163V207" stroke="#355c77"/>
      <path d="M120 209V196H181M343 198H864M859 199V174M849 183H869" fill="none" stroke="#83a6b9"/>
      <g v-for="service in actionServices" :key="service.id" :style="{color:service.color}" :class="{ 'ship-active':active(service.id), 'ship-completed':completed(service.id) }">
        <path :d="`M${position(service.id).x} ${position(service.id).y} V${position(service.id).labelY + 31}`" class="ship-link" stroke="currentColor" fill="none" stroke-dasharray="4 5"/>
        <circle :cx="position(service.id).x" :cy="position(service.id).y" r="15" class="ship-halo" fill="currentColor" opacity=".12"/>
        <circle :cx="position(service.id).x" :cy="position(service.id).y" r="5" fill="currentColor"/>
        <rect :x="position(service.id).x - position(service.id).labelWidth / 2" :y="position(service.id).labelY" :width="position(service.id).labelWidth" height="31" rx="5" fill="#0d2031" stroke="currentColor" opacity=".9"/>
        <text :x="position(service.id).x" :y="position(service.id).labelY + 21" text-anchor="middle" fill="currentColor" font-size="14">{{ service.id }} {{ service.name }}</text>
      </g>
      <text x="780" y="249" fill="#8daec2" font-size="12" letter-spacing="4">SD VESSEL</text>
    </svg>
    <div class="ship-legend"><span><i class="legend-running"/>运行中</span><span><i/>待命</span><span>统一执行器接口 · 设备接入待测试</span></div>
  </div>
</template>
