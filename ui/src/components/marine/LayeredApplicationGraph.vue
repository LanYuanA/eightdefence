<script setup lang="ts">
import { computed, ref } from 'vue'
import { services } from '../../marine/model'
import type { MarineApp } from '../../marine/model'
import type { RunState } from '../../marine/runner'
import { buildApplicationFlow } from '../../marine/application-flow'
const props=defineProps<{app:MarineApp;run:RunState}>()
const flow=computed(()=>buildApplicationFlow(props.app))
const selected=ref('')
const used=computed(()=>[...flow.value.awareness,...flow.value.actions])
const unused=computed(()=>services.filter(s=>!used.value.some(n=>n.id===s.id)))
// The graph illustrates the application's complete route; it does not schedule devices.
const flowing=computed(()=>['running','completed'].includes(props.run.status))
const live=(id:string)=>flowing.value && used.value.some(service=>service.id===id)
const px=(i:number,count:number,start=205,end=1020)=>count===1?(start+end)/2:start+i*(end-start)/(count-1)
const svcX=(i:number)=>px(i,used.value.length)
const devices=computed(()=>[{id:'sensing',name:'态势数据接入',serviceIds:flow.value.awareness.map(s=>s.id)},...flow.value.abstractions.map(d=>({...d,name:d.name.replace('逻辑执行器','接口').replace('机舱','').replace('检修安全','')}))])
const hardware=computed(()=>[{id:'sensing',name:'态势感知',serviceIds:flow.value.awareness.map(s=>s.id)},...flow.value.hardware.map(d=>({...d,name:d.name.replace('机舱','').replace(' 1 号','').replace(' 2 号','').replace('检修安全','')}))])
function curve(x1:number,y1:number,x2:number,y2:number){const m=(y1+y2)/2;return `M${x1},${y1} C${x1},${m} ${x2},${m} ${x2},${y2}`}
function serviceDeviceLink(sx:number,dx:number,up:boolean){
  const gap=845/Math.max(1,unused.value.length-1)
  const target=(sx+dx)/2
  const cross=195+(Math.max(0,Math.min(unused.value.length-2,Math.round((target-195)/gap-.5)))+.5)*gap
  return up?`M${dx},474 C${dx},446 ${cross},435 ${cross},412 L${cross},366 C${cross},342 ${sx},330 ${sx},302`:`M${sx},302 C${sx},330 ${cross},342 ${cross},366 L${cross},412 C${cross},435 ${dx},446 ${dx},474`
}
const edges=computed(()=>{
  const out:{id:string;path:string;kind:string;active:boolean}[]=[]
  used.value.forEach((s,i)=>{
    const sense=s.id.startsWith('A'),x=svcX(i),active=live(s.id)
    out.push({id:'app-'+s.id,path:sense?curve(x,244,577,110):curve(627,110,x,244),kind:sense?'data':'command',active})
    if(!sense)out.push({id:'app-feedback-'+s.id,path:curve(x+12,244,639,110),kind:'feedback',active})
    const targets=devices.value.map((d,j)=>({d,j})).filter(({d})=>d.serviceIds.includes(s.id))
    targets.forEach(({d,j})=>{const dx=px(j,devices.value.length);out.push({id:d.id+s.id,path:serviceDeviceLink(x,dx,sense),kind:sense?'data':'command',active});if(!sense)out.push({id:'return-'+d.id+s.id,path:serviceDeviceLink(x+12,dx+12,true),kind:'feedback',active})})
  })
  devices.value.forEach((d,i)=>{const x=px(i,devices.value.length);out.push({id:'hw-'+d.id,path:i===0?curve(x,633,x,530):curve(x,530,x,633),kind:i===0?'data':'command',active:d.serviceIds.some(live)});if(i>0)out.push({id:'feedback-'+d.id,path:curve(x+18,633,x+18,530),kind:'feedback',active:d.serviceIds.some(live)})})
  return out
})
const selectedService=computed(()=>services.find(s=>s.id===selected.value))
</script>
<template>
 <div class="layered-graph">
  <svg viewBox="0 0 1120 750" role="img" aria-label="硬件、设备抽象、原子服务、应用四层双向调用链路">
   <defs>
    <radialGradient id="node-light"><stop stop-color="#194456"/><stop offset="1" stop-color="#0b202e"/></radialGradient>
    <radialGradient id="app-light"><stop stop-color="#245d68"/><stop offset="1" stop-color="#102f40"/></radialGradient>
    <pattern id="link-grid" width="28" height="28" patternUnits="userSpaceOnUse"><circle cx="1" cy="1" r=".6" fill="#456174"/></pattern>
   </defs>
   <rect width="1120" height="750" fill="url(#link-grid)" opacity=".24"/>
   <g class="layer-mark"><text x="20" y="76">04</text><text x="20" y="101">应用层</text><text x="20" y="270">03</text><text x="20" y="295">原子服务层</text><text x="20" y="500">02</text><text x="20" y="525">设备抽象层</text><text x="20" y="659">01</text><text x="20" y="684">硬件层</text></g>
   <path d="M140 187H1080M140 436H1080M140 583H1080" stroke="#284352" stroke-dasharray="2 7" opacity=".45"/>
   <g v-for="edge in edges" :key="edge.id" :class="['layer-edge',edge.kind,{live:edge.active}]"><path :d="edge.path"/><path v-if="edge.active" :d="edge.path" class="moving-particle"/></g>
<g class="application-hub" :class="{live:flowing}"><circle cx="602" cy="80" r="49" class="outer-ring"/><circle cx="602" cy="80" r="38"/><path d="M587 79L602 70L617 79L602 88Z M587 86L602 95L617 86"/><text x="670" y="79">{{app.name}}</text><text x="670" y="105" class="sub">独立应用 · {{used.length}} 项服务</text></g>
   <g v-for="(service,i) in used" :key="service.id" class="service-node" :class="{live:live(service.id),command:!service.id.startsWith('A')}" @click="selected=service.id" tabindex="0" @keydown.enter="selected=service.id">
    <circle :cx="svcX(i)" cy="273" r="35" class="outer-ring"/><circle :cx="svcX(i)" cy="273" r="28"/>
    <text :x="svcX(i)" y="279" text-anchor="middle">{{service.id}}</text><text :x="svcX(i)" y="326" text-anchor="middle" class="node-name">{{service.name}}</text><title>{{service.name}} · 第 {{service.step}} 阶段</title>
   </g>
   <g v-for="(service,i) in unused" :key="service.id" class="unused-node" tabindex="0" @click="selected=service.id" @keydown.enter="selected=service.id"><circle :cx="px(i,unused.length,195,1040)" cy="388" r="16"/><text :x="px(i,unused.length,195,1040)" y="392" text-anchor="middle">{{service.id}}</text><title>{{service.name}} · {{service.available?'本应用未调用':'扩展能力'}}</title></g>
   <g v-for="(device,i) in devices" :key="device.id" class="device-node" :class="{live:device.serviceIds.some(live)}"><circle :cx="px(i,devices.length)" cy="502" r="28"/><path :d="`M${px(i,devices.length)-11} 496h22v13h-22z M${px(i,devices.length)-5} 489v7 M${px(i,devices.length)+5} 489v7`"/><text :x="px(i,devices.length)" y="553" text-anchor="middle">{{device.name}}</text></g>
   <g v-for="(device,i) in hardware" :key="device.id" class="hardware-node" :class="{live:device.serviceIds.some(live)}"><circle :cx="px(i,hardware.length)" cy="661" r="28"/><circle :cx="px(i,hardware.length)" cy="661" r="9" class="core"/><path :d="`M${px(i,hardware.length)} 642v10 M${px(i,hardware.length)} 670v10 M${px(i,hardware.length)-19} 661h10 M${px(i,hardware.length)+9} 661h10`"/><text :x="px(i,hardware.length)" y="710" text-anchor="middle">{{device.name}}</text><title>{{i===0?flow.sensors.map(s=>s.label).join('、'):device.name}}</title></g>
  </svg>
  <div class="graph-key"><span class="data">● 数据上报</span><span class="command">● 指令下发</span><span class="feedback">● 状态反馈</span><span>○ 灰色节点未调用</span></div>
  <div v-if="selectedService" class="graph-inspector"><strong>{{selectedService.name}}</strong><span>{{selectedService.description}}</span><button @click="selected=''" aria-label="关闭节点详情">×</button></div>
 </div>
</template>
<style scoped>
.layered-graph{height:100%;min-height:0;position:relative;display:flex;flex-direction:column}.layered-graph svg{width:100%;flex:1;min-height:0}.layer-mark text{fill:#627f92;font-size:17px}.layer-mark text:nth-child(odd){fill:#35596d;font-size:25px}.layer-edge path{fill:none;stroke:#4faac5;stroke-width:1.15;opacity:.22}.layer-edge.command path{stroke:#d5ae66}.layer-edge.feedback path{stroke:#71c6a6;stroke-dasharray:3 5}.layer-edge.live path{opacity:.65}.layer-edge .moving-particle{stroke-width:3;stroke-dasharray:1 30;stroke-linecap:round;opacity:1;animation:link-stream 2.5s linear infinite;filter:drop-shadow(0 0 4px currentColor)}.application-hub circle{fill:url(#app-light);stroke:#71c7d4}.application-hub path{fill:none;stroke:#aeedf0;stroke-width:2}.application-hub text{fill:#c2e6ec;font-size:20px}.application-hub .sub{font-size:14px;fill:#678d9f}.outer-ring{fill:none!important;stroke-dasharray:3 8;opacity:.4}.service-node{cursor:pointer}.service-node circle{fill:url(#node-light);stroke:#4794b0}.service-node.command circle{stroke:#aa915c}.service-node text{fill:#a6d9e4;font-size:16px}.service-node .node-name{font-size:16px;fill:#9cb5c6}.live circle{filter:drop-shadow(0 0 7px #52bfd16b);stroke-width:1.8}.live .outer-ring{animation:ring-turn 15s linear infinite;transform-box:fill-box;transform-origin:center;opacity:.8}.unused-node{cursor:pointer}.unused-node circle{fill:#101e2c;stroke:#314453}.unused-node text{fill:#617385;font-size:10px}.unused-node:hover circle{stroke:#83acbb}.device-node circle,.hardware-node circle{fill:#102b38;stroke:#477889}.device-node path,.hardware-node path{stroke:#80aebb;stroke-width:1.5;fill:none}.device-node text,.hardware-node text{fill:#9bb5c4;font-size:16px}.hardware-node .core{fill:#5a9fba;stroke:none}.graph-key{display:flex;justify-content:center;gap:20px;color:#5d7589;font-size:11px;padding:10px 0}.graph-key .data{color:#65adc7}.graph-key .command{color:#c6a567}.graph-key .feedback{color:#69b897}.graph-inspector{position:absolute;bottom:40px;left:15%;right:8%;display:flex;align-items:center;gap:12px;background:#112a3a;border:1px solid #395a70;padding:15px;border-radius:8px;color:#b7d5df;font-size:13px}.graph-inspector strong{white-space:nowrap}.graph-inspector span{color:#83a1b2}.graph-inspector button{margin-left:auto;background:none;border:none;color:#aacad6;cursor:pointer;font-size:20px}@keyframes link-stream{to{stroke-dashoffset:-62}}@keyframes ring-turn{to{transform:rotate(360deg)}}@media(prefers-reduced-motion:reduce){*{animation:none!important}}
</style>
