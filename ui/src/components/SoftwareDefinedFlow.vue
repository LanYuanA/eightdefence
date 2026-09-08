<script setup lang="ts">
interface FlowApplication {
  id: string
  name: string
  generated?: boolean
}

defineProps<{ onlineCount: number; live: boolean; applications: FlowApplication[] }>()

const awarenessServices = [
  ['A01', '温湿感知'], ['A02', '颗粒物感知'], ['A03', '气体感知'], ['A04', '烟雾感知'],
  ['A05', '水浸感知'], ['A06', '活动感知'], ['A07', '光照感知'],
]
const actionServices = [['01', '舱室通风'], ['02', '冷却循环'], ['03', '供水增压'], ['04', '舱底排水']]
</script>

<template>
  <div class="definition-flow">
    <div class="flow-legend">
      <span><i class="legend-data"/>态势数据向上流动</span>
      <span><i class="legend-control"/>控制指令向下流动</span>
      <strong :class="{ live }">{{ live ? `${onlineCount} / 12 路数据在线` : '等待现场数据 · 当前展示结构流向' }}</strong>
    </div>

    <div class="architecture-stack">
      <div class="flow-rail data-rail" aria-hidden="true"><span>态势数据</span><i/><i/><i/></div>
      <div class="flow-rail control-rail" aria-hidden="true"><span>控制指令</span><i/><i/><i/></div>

      <section class="stack-layer application-layer">
        <header><span class="layer-index">L4</span><div><small>APPLICATION LAYER</small><h4>应用层</h4></div><p>任务需求决定服务组合，保存的新应用自动进入本层</p></header>
        <div class="application-grid">
          <article v-for="app in applications" :key="app.id" :class="{ generated: app.generated }">
            <span>{{ app.generated ? 'NEW' : 'APP' }}</span><strong>{{ app.name }}</strong><small>{{ app.generated ? '编排生成应用' : '示例任务应用' }}</small>
          </article>
          <article v-if="!applications.some(app => app.generated)" class="application-placeholder"><span>＋</span><strong>等待生成新应用</strong><small>在任务编排页保存后自动出现</small></article>
        </div>
      </section>

      <div class="layer-bridge"><span>态势结果汇入应用</span><b>↑</b><i/><b>↓</b><span>任务指令下发</span></div>

      <section class="stack-layer service-layer">
        <header><span class="layer-index">L3</span><div><small>ATOMIC SERVICE LAYER</small><h4>原子服务层</h4></div><p>感知和执行能力统一封装、自由复用</p></header>
        <div class="service-columns">
          <div class="service-group awareness-group"><div class="group-title"><span>态势感知服务</span><small>7 项 / 覆盖 12 路数据</small></div><div class="service-grid"><article v-for="service in awarenessServices" :key="service[0]"><b>{{ service[0] }}</b><span>{{ service[1] }}</span></article></div></div>
          <div class="service-divider"><span>按任务组合</span><i/><b>↔</b></div>
          <div class="service-group action-group"><div class="group-title"><span>执行控制服务</span><small>4 项 / 软件模拟</small></div><div class="service-grid"><article v-for="service in actionServices" :key="service[0]"><b>{{ service[0] }}</b><span>{{ service[1] }}</span></article></div></div>
        </div>
      </section>

      <div class="layer-bridge"><span>统一态势接口</span><b>↑</b><i/><b>↓</b><span>统一执行接口</span></div>

      <section class="stack-layer abstraction-layer">
        <header><span class="layer-index">L2</span><div><small>DEVICE ABSTRACTION LAYER</small><h4>设备抽象层</h4></div><p>隔离设备差异，为上层提供稳定接口</p></header>
        <div class="abstraction-grid">
          <article class="data-interface"><span>DATA</span><div><strong>态势数据统一接口</strong><small>读取 · 状态 · 时间戳 · 数据质量</small></div></article>
          <article class="replaceable-interface"><span>VENT-01</span><div><strong>通风逻辑执行器</strong><small>稳定接口 · 运行时绑定可替换硬件</small></div></article>
          <article><span>EXE-02</span><div><strong>冷却执行器抽象</strong><small>启动 · 停止 · 调速 · 状态</small></div></article>
          <article><span>EXE-03</span><div><strong>水务执行器抽象</strong><small>启动 · 停止 · 调节 · 状态</small></div></article>
        </div>
      </section>

      <div class="layer-bridge"><span>现场数据上报</span><b>↑</b><i/><b>↓</b><span>硬件动作执行</span></div>

      <section class="stack-layer resource-layer">
        <header><span class="layer-index">L1</span><div><small>PHYSICAL RESOURCE LAYER</small><h4>态势感知与执行器硬件层</h4></div><p>两类物理资源位于同一层</p></header>
        <div class="resource-columns">
          <article class="situation-resource"><div class="resource-heading"><span class="resource-pulse"/><div><strong>态势感知</strong><small>现有传感器 · 12 路数据</small></div></div><div class="source-tags"><span>温度 / 湿度</span><span>PM2.5 / PM10</span><span>TVOC / 甲醛 / CO₂</span><span>烟雾 / 水浸</span><span>红外 / 雷达 / 光照</span></div></article>
          <article class="hardware-resource"><div class="resource-heading"><span class="hardware-mark">HW</span><div><strong>执行器硬件</strong><small>RS485 物理设备 · 地址差异由抽象层吸收</small></div></div><div class="hardware-list"><span>海风一号 · 0x02</span><span>海岳一号 · 0x0E</span><span>海岳二号 · 0x0F</span></div></article>
        </div>
      </section>
    </div>

    <div class="decoupling-summary"><span><b>软软解耦</b> 原子服务按任务快速组合，新应用即刻生成</span><i/><span><b>软硬解耦</b> 上层面向抽象接口，底层硬件可替换</span><RouterLink class="decoupling-link" to="/hardware-decoupling">进入替换演示 →</RouterLink></div>
  </div>
</template>

<style scoped>
.definition-flow{--data:#51d1e2;--control:#6de1a8;background:radial-gradient(circle at 50% 48%,rgba(38,91,119,.18),transparent 48%),#07111d;border:1px solid rgba(69,111,139,.38);border-radius:12px;padding:14px;min-height:0;overflow:auto}.flow-legend{display:flex;align-items:center;gap:24px;color:#91aabd;font-size:13px;margin-bottom:10px}.flow-legend span{display:flex;align-items:center;gap:7px}.flow-legend i{width:26px;height:2px;display:inline-block}.legend-data{background:var(--data)}.legend-control{background:var(--control)}.flow-legend strong{margin-left:auto;color:#879cab;font-weight:500}.flow-legend strong.live{color:#69d9c7}.architecture-stack{min-width:1080px;position:relative;padding:0 82px}.stack-layer{position:relative;z-index:2;background:linear-gradient(145deg,rgba(22,48,67,.97),rgba(9,25,38,.98));border:1px solid rgba(70,108,135,.58);border-radius:10px;padding:10px 16px;box-shadow:inset 0 1px rgba(255,255,255,.035)}.stack-layer>header{display:flex;align-items:center;gap:12px;margin-bottom:8px}.stack-layer header .layer-index{font-family:monospace;color:#6f8ca2;border:1px solid #36546a;padding:4px 7px;border-radius:4px}.stack-layer header small{font-size:12px;letter-spacing:1.6px;color:#66859c}.stack-layer h4{font-size:18px;color:#e1edf6;margin:0}.stack-layer header p{margin-left:auto;color:#8ca5b9;font-size:13px}.application-layer{border-color:rgba(126,156,255,.45)}.application-grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(175px,1fr));gap:9px}.application-grid article{position:relative;display:flex;flex-direction:column;min-height:56px;padding:7px 10px;background:rgba(116,148,244,.07);border:1px solid rgba(116,148,244,.23);border-radius:6px}.application-grid article>span{font-family:monospace;font-size:12px;color:#8aa4f2}.application-grid article strong{color:#d8e3f8;font-size:14px;font-weight:500;margin:3px 0}.application-grid article small{font-size:12px;color:#748ca1}.application-grid article.generated{border-color:rgba(109,225,168,.65);background:linear-gradient(110deg,rgba(109,225,168,.12),rgba(109,225,168,.035));animation:new-app-glow 2.6s ease-in-out infinite}.application-grid article.generated>span{color:#6de1a8}.application-grid .application-placeholder{border-style:dashed;background:transparent}.service-layer{border-color:rgba(75,195,211,.42)}.service-columns{display:grid;grid-template-columns:1.65fr 74px 1fr;gap:12px;align-items:stretch}.service-group{padding:7px 10px;background:rgba(5,17,27,.36);border-radius:7px}.group-title{display:flex;justify-content:space-between;gap:12px;margin-bottom:6px;color:#bcd0df;font-size:13px}.group-title small{color:#7895aa}.service-grid{display:grid;grid-template-columns:repeat(4,1fr);gap:6px}.service-grid article{display:flex;align-items:center;gap:7px;padding:5px 7px;border:1px solid rgba(81,209,226,.2);background:rgba(81,209,226,.045);border-radius:4px}.service-grid b{font-family:monospace;font-size:12px;color:var(--data);font-weight:500}.service-grid span{font-size:12px;color:#aec5d5}.action-group .service-grid{grid-template-columns:repeat(2,1fr)}.action-group .service-grid article{border-color:rgba(109,225,168,.2);background:rgba(109,225,168,.045)}.action-group .service-grid b{color:var(--control)}.service-divider{display:flex;flex-direction:column;align-items:center;justify-content:center;color:#6e8da3;font-size:12px}.service-divider i{height:25px;width:1px;background:#31536a;margin:5px}.service-divider b{font-size:20px;color:#86a8bc}.abstraction-layer{border-color:rgba(171,151,255,.4)}.abstraction-grid{display:grid;grid-template-columns:1.15fr 1fr 1fr 1fr;gap:8px}.abstraction-grid article{display:flex;align-items:center;gap:10px;padding:8px 10px;border:1px solid rgba(171,151,255,.22);background:rgba(171,151,255,.05);border-radius:6px}.abstraction-grid article>span{font-family:monospace;font-size:12px;color:#b39fff}.abstraction-grid article.data-interface{border-color:rgba(81,209,226,.25);background:rgba(81,209,226,.05)}.abstraction-grid article.data-interface>span{color:var(--data)}.abstraction-grid article.replaceable-interface{border-color:rgba(244,189,98,.38);background:linear-gradient(100deg,rgba(244,189,98,.09),rgba(171,151,255,.04))}.abstraction-grid article.replaceable-interface>span{color:#f4bd62}.abstraction-grid strong{display:block;color:#c8d7e3;font-size:13px}.abstraction-grid small{display:block;color:#7892a6;font-size:12px;margin-top:3px}.resource-layer{border-color:rgba(109,225,168,.4)}.resource-columns{display:grid;grid-template-columns:1.4fr 1fr;gap:10px}.resource-columns>article{padding:8px 12px;border:1px solid rgba(81,209,226,.22);background:rgba(81,209,226,.04);border-radius:6px}.resource-columns>article.hardware-resource{border-color:rgba(109,225,168,.24);background:rgba(109,225,168,.045)}.resource-heading{display:flex;align-items:center;gap:11px}.resource-heading strong{display:block;color:#cce0eb;font-size:14px}.resource-heading small{display:block;color:#7896ab;font-size:12px}.resource-pulse{width:13px;height:13px;border-radius:50%;background:var(--data);box-shadow:0 0 0 5px rgba(81,209,226,.1),0 0 14px rgba(81,209,226,.45);animation:pulse 1.8s ease-in-out infinite}.hardware-mark{display:grid;place-items:center;width:32px;height:28px;border:1px solid rgba(109,225,168,.5);border-radius:4px;color:var(--control);font-family:monospace;font-size:12px}.source-tags,.hardware-list{display:flex;gap:6px;flex-wrap:wrap;margin-top:7px}.source-tags span,.hardware-list span{padding:3px 7px;background:rgba(74,126,155,.08);border:1px solid rgba(74,126,155,.22);border-radius:4px;font-size:12px;color:#9cb5c7}.hardware-list span{color:#a3c9b8;border-color:rgba(109,225,168,.19)}.layer-bridge{position:relative;z-index:2;height:26px;display:flex;align-items:center;justify-content:center;gap:10px;color:#7592a7;font-size:12px}.layer-bridge b{color:#8fb5c8;font-size:15px}.layer-bridge i{height:1px;width:32px;background:#35576d}.flow-rail{position:absolute;top:65px;bottom:65px;width:2px;z-index:1;background:linear-gradient(transparent,currentColor 8%,currentColor 92%,transparent);opacity:.8}.flow-rail:before,.flow-rail:after{content:"";position:absolute;left:-4px;width:8px;height:8px;border-left:2px solid currentColor}.data-rail{left:34px;color:var(--data)}.data-rail:before{top:0;border-top:2px solid currentColor;transform:rotate(45deg)}.control-rail{right:34px;color:var(--control)}.control-rail:after{bottom:0;border-bottom:2px solid currentColor;transform:rotate(-45deg)}.flow-rail>span{position:absolute;top:50%;writing-mode:vertical-rl;transform:translate(-50%,-50%);left:50%;padding:8px 0;background:#07111d;font-size:12px;letter-spacing:2px;color:currentColor}.flow-rail i{position:absolute;left:-3px;width:8px;height:8px;border-radius:50%;background:currentColor;box-shadow:0 0 10px currentColor}.data-rail i{animation:data-up 4s linear infinite}.control-rail i{animation:control-down 4s linear infinite}.flow-rail i:nth-of-type(2){animation-delay:-1.35s}.flow-rail i:nth-of-type(3){animation-delay:-2.7s}.decoupling-summary{display:flex;justify-content:center;align-items:center;gap:28px;border-top:1px solid rgba(69,111,139,.25);margin-top:10px;padding-top:10px;color:#89a4b7;font-size:13px}.decoupling-summary b{color:#c4d8e6;margin-right:8px}.decoupling-summary i{width:1px;height:14px;background:#315169}.decoupling-link{margin-left:auto;padding:5px 9px;border:1px solid rgba(244,189,98,.27);border-radius:4px;color:#e8b965;text-decoration:none;font-size:12px}.decoupling-link:hover{background:rgba(244,189,98,.08)}@keyframes data-up{from{bottom:0}to{bottom:100%}}@keyframes control-down{from{top:0}to{top:100%}}@keyframes pulse{50%{opacity:.5;transform:scale(.82)}}@keyframes new-app-glow{50%{box-shadow:0 0 20px rgba(109,225,168,.12)}}@media(max-width:800px){.definition-flow{padding:12px}.flow-legend{flex-wrap:wrap}.flow-legend strong{margin-left:0;width:100%}.architecture-stack{padding:0 62px}.data-rail{left:24px}.control-rail{right:24px}.decoupling-summary{justify-content:flex-start;flex-wrap:wrap}.decoupling-summary i{display:none}.decoupling-link{margin-left:0}}
</style>
