<template>
  <div class="security-root flex flex-col min-h-screen">
    <!-- 顶部导航栏 -->
    <header class="nav-header">
      <div class="nav-inner">
        <div class="flex items-center gap-4">
          <div class="brand-icon">
            <svg width="28" height="28" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.8">
              <path d="M12 22s8-4 8-10V5l-8-3-8 3v7c0 6 8 10 8 10z"/>
            </svg>
          </div>
          <div>
            <div class="brand-title">智慧环控安防系统</div>
            <div class="brand-sub">Smart Environment & Security Platform</div>
          </div>
        </div>
        <div class="flex items-center gap-5">
          <div class="nav-time">
            <svg width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" class="mr-1.5 opacity-60">
              <circle cx="12" cy="12" r="10"/><path d="M12 6v6l4 2"/>
            </svg>
            {{ currentTime }}
          </div>
          <button class="nav-btn" @click="$router.push('/')">
            <svg width="15" height="15" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" class="mr-1.5"><path d="M3 9l9-7 9 7v11a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2z"/><polyline points="9 22 9 12 15 12 15 22"/></svg>
            主控台
          </button>
          <button class="nav-btn" @click="$router.push('/devices')">
            <svg width="15" height="15" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" class="mr-1.5"><rect x="2" y="3" width="20" height="14" rx="2"/><path d="M8 21h8m-4-4v4"/></svg>
            设备管理
          </button>
          <button class="nav-btn" @click="$router.push('/logs')">
            <svg width="15" height="15" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" class="mr-1.5"><path d="M14 2H6a2 2 0 0 0-2 2v16a2 2 0 0 0 2 2h12a2 2 0 0 0 2-2V8z"/><path d="M14 2v6h6M16 13H8M16 17H8M10 9H8"/></svg>
            系统日志
          </button>
          <button class="nav-btn-danger" @click="handleLogout">
            <svg width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" class="mr-1"><path d="M9 21H5a2 2 0 0 1-2-2V5a2 2 0 0 1 2-2h4"/><polyline points="16 17 21 12 16 7"/><line x1="21" y1="12" x2="9" y2="12"/></svg>
            退出
          </button>
        </div>
      </div>
    </header>

    <main class="flex-1 p-5 overflow-y-auto">
      <!-- 页面标题 -->
      <div class="page-title-bar mb-5">
        <div>
          <h1 class="page-title">智能安防系统监控平台</h1>
          <p class="page-subtitle">环境安全监测与应急防护系统 - 多维度安防场景实时监控</p>
        </div>
      </div>

      <!-- 系统概览 -->
      <div class="overview-bar mb-5">
        <div class="status-card">
          <h3>系统整体状态</h3>
          <div class="flex items-center justify-center gap-2 mt-2">
            <span class="status-dot" :class="statusDotClass(status.system?.overallRisk)"></span>
            <span class="status-text">{{ status.system?.systemNormal ? '运行正常' : '存在异常' }}</span>
          </div>
          <p class="status-desc">{{ status.system?.systemNormal ? '所有子系统工作正常' : '请检查下方报警信息' }}</p>
        </div>
        <div class="status-card">
          <h3>报警服务状态</h3>
          <div class="flex items-center justify-center gap-2 mt-2">
            <span class="status-dot" :class="status.alarm?.soundActive || status.alarm?.centerActive ? 'dot-alert' : 'dot-normal'"></span>
            <span class="status-text">{{ status.alarm?.soundActive || status.alarm?.centerActive ? '报警中' : '待机中' }}</span>
          </div>
          <p class="status-desc">{{ status.alarm?.soundActive || status.alarm?.centerActive ? '声音/指挥中心报警已激活' : '无当前报警' }}</p>
        </div>
        <div class="status-card">
          <h3>防护处置状态</h3>
          <div class="flex items-center justify-center gap-2 mt-2">
            <span class="status-dot" :class="status.water?.controlActive || status.gas?.ventilationActive ? 'dot-warning' : 'dot-normal'"></span>
            <span class="status-text">{{ status.water?.controlActive || status.gas?.ventilationActive ? '处置中' : '待机中' }}</span>
          </div>
          <p class="status-desc">{{ status.water?.controlActive ? '排水/阀门控制中' : status.gas?.ventilationActive ? '通风设备运行中' : '无处置指令' }}</p>
        </div>
      </div>

      <!-- 三张应用卡片 -->
      <div class="app-grid mb-5">
        <!-- 水浸风险安防 -->
        <div class="app-card">
          <div class="app-card-header">
            <svg width="28" height="28" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.8"><path d="M12 2.69l5.66 5.66a8 8 0 1 1-11.31 0z"/></svg>
            <h2>水浸风险安防</h2>
          </div>
          <p class="app-card-desc">监测水浸隐患，触发预警与处置流程</p>
          <div class="data-row">
            <span class="data-label">水浸传感器</span>
            <span>
              <span class="data-value" :class="status.water?.online === false ? 'text-danger' : 'text-safe'">{{ status.water?.online === false ? '离线' : '在线' }}</span>
            </span>
          </div>
          <div class="data-row">
            <span class="data-label">水浸传感器状态</span>
            <span class="data-value" :class="status.water?.online === false ? 'text-unknown' : status.water?.sensorState !== 0 ? 'text-danger' : ''">{{ status.water?.online === false ? '未知' : status.water?.sensorState !== 0 ? '异常' : '正常' }}</span>
          </div>
          <div class="data-row">
            <span class="data-label">水位监测</span>
            <span class="data-value" :class="status.water?.online === false ? 'text-unknown' : ''">{{ status.water?.online === false ? '--' : (status.water?.level?.toFixed(1) ?? '--') + 'cm' }}</span>
          </div>
          <div class="data-row">
            <span class="data-label">风险评估</span>
            <span class="data-value" :class="status.water?.online === false ? 'text-unknown' : riskClass(status.water?.risk)">{{ status.water?.online === false ? '未知' : (status.water?.risk ?? '--') }}</span>
          </div>
          <div class="flex gap-2 mt-4">
            <button class="action-btn water-btn" @click="doControl('water', 'simulate')">模拟水浸异常</button>
            <button class="action-btn" @click="doControl('water', 'reset')">恢复正常</button>
          </div>
        </div>

        <!-- 人员入侵检测 -->
        <div class="app-card">
          <div class="app-card-header">
            <svg width="28" height="28" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.8"><path d="M20 21v-2a4 4 0 0 0-4-4H8a4 4 0 0 0-4 4v2"/><circle cx="12" cy="7" r="4"/></svg>
            <h2>人员入侵检测</h2>
          </div>
          <p class="app-card-desc">识别非法闯入行为，联动报警与记录</p>
          <div class="data-row">
            <span class="data-label">红外探测器</span>
            <span>
              <span class="data-value" :class="status.intrusion?.online === false ? 'text-danger' : 'text-safe'">{{ status.intrusion?.online === false ? '离线' : '在线' }}</span>
            </span>
          </div>
          <div class="data-row">
            <span class="data-label">红外探测器状态</span>
            <span class="data-value" :class="status.intrusion?.online === false ? 'text-unknown' : status.intrusion?.infraredState !== 0 ? 'text-danger' : ''">{{ status.intrusion?.online === false ? '未知' : status.intrusion?.infraredState !== 0 ? '检测中' : '正常' }}</span>
          </div>
          <div class="data-row">
            <span class="data-label">当前检测状态</span>
            <span class="data-value" :class="status.intrusion?.online === false ? 'text-unknown' : (status.intrusion?.infraredState !== 0 || status.intrusion?.radarState !== 0) ? 'text-danger' : ''">{{ status.intrusion?.online === false ? '未知' : intrusionStatusText }}</span>
          </div>
          <div class="data-row">
            <span class="data-label">风险评估</span>
            <span class="data-value" :class="status.intrusion?.online === false ? 'text-unknown' : riskClass(status.intrusion?.risk)">{{ status.intrusion?.online === false ? '未知' : (status.intrusion?.risk ?? '--') }}</span>
          </div>
          <div class="flex gap-2 mt-4">
            <button class="action-btn intrusion-btn" @click="doControl('intrusion', 'simulate')">模拟人员入侵</button>
            <button class="action-btn" @click="doControl('intrusion', 'reset')">恢复正常</button>
          </div>
        </div>

        <!-- 有害气体安全防护 -->
        <div class="app-card">
          <div class="app-card-header">
            <svg width="28" height="28" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.8"><path d="M18.5 8c.83 0 1.5-.67 1.5-1.5S19.33 5 18.5 5c-.17 0-.34.03-.5.08A2.5 2.5 0 0 0 16 3a2.5 2.5 0 0 0-2 4v.02A1.5 1.5 0 0 0 13 8.5c0 .83.67 1.5 1.5 1.5h4z"/><path d="M12 9V2"/><path d="M4.93 10.93a10 10 0 1 0 14.14 0"/></svg>
            <h2>有害气体安全防护</h2>
          </div>
          <p class="app-card-desc">检测有害气体浓度，超限时触发通风/报警</p>
          <div class="data-row">
            <span class="data-label">有害气体探测器</span>
            <span>
              <span class="data-value" :class="status.gas?.online === false ? 'text-danger' : 'text-safe'">{{ status.gas?.online === false ? '离线' : '在线' }}</span>
            </span>
          </div>
          <div class="data-row">
            <span class="data-label">TVOC (挥发性有机物)</span>
            <span class="data-value" :class="gasItemClass(status.gas?.tvoc, 200)">{{ gasItemText(status.gas?.tvoc) }}</span>
          </div>
          <div class="data-row">
            <span class="data-label">CH2O (甲醛)</span>
            <span class="data-value" :class="gasItemClass(status.gas?.ch2o, 50)">{{ gasItemText(status.gas?.ch2o) }}</span>
          </div>
          <div class="data-row">
            <span class="data-label">CO2 (二氧化碳)</span>
            <span class="data-value" :class="gasItemClass(status.gas?.co2, 800)">{{ gasItemText(status.gas?.co2) }}</span>
          </div>
          <div class="data-row">
            <span class="data-label">风险评估</span>
            <span class="data-value" :class="status.gas?.online === false ? 'text-unknown' : riskClass(status.gas?.risk)">{{ status.gas?.online === false ? '未知' : (status.gas?.risk ?? '--') }}</span>
          </div>
          <div class="flex gap-2 mt-4">
            <button class="action-btn gas-btn" @click="doControl('gas', 'simulate')">模拟气体泄漏</button>
            <button class="action-btn" @click="doControl('gas', 'reset')">恢复正常</button>
          </div>
        </div>
      </div>

      <!-- 联动服务状态 -->
      <div class="services-section mb-5">
        <h3 class="section-title">
          <svg width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M18 8A6 6 0 0 0 6 8c0 7-3 9-3 9h18s-3-2-3-9"/><path d="M13.73 21a2 2 0 0 1-3.46 0"/></svg>
          联动服务状态
        </h3>
        <p class="section-desc">当安防风险评估达到高风险时，系统自动触发以下联动服务</p>

        <div class="services-grid">
          <!-- 声光报警 -->
          <div class="service-card" :class="{ 'service-active': status.alarm?.soundActive }">
            <div class="service-header">
              <span class="service-icon alarm-icon">
                <svg width="22" height="22" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><polygon points="11 5 6 9 2 9 2 15 6 15 11 19 11 5"/><path d="M15.54 8.46a5 5 0 0 1 0 7.07"/></svg>
              </span>
              <div>
                <h4>声音报警服务</h4>
                <span class="service-status" :class="status.alarm?.soundActive ? 'status-on' : 'status-off'">{{ status.alarm?.soundActive ? '激活中' : '待机' }}</span>
              </div>
            </div>
            <div class="service-body">
              <div class="service-detail">
                <span class="detail-label">触发条件</span>
                <span class="detail-value">水浸/入侵/气体任一达到高风险</span>
              </div>
              <div class="service-detail">
                <span class="detail-label">执行动作</span>
                <span class="detail-value">驱动报警器蜂鸣</span>
              </div>
              <div class="service-detail">
                <span class="detail-label">停止条件</span>
                <span class="detail-value">所有风险项恢复低风险</span>
              </div>
            </div>
          </div>

          <!-- 指挥中心报警 -->
          <div class="service-card" :class="{ 'service-active': status.alarm?.centerActive }">
            <div class="service-header">
              <span class="service-icon center-icon">
                <svg width="22" height="22" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><rect x="2" y="3" width="20" height="14" rx="2" ry="2"/><line x1="8" y1="21" x2="16" y2="21"/><line x1="12" y1="17" x2="12" y2="21"/></svg>
              </span>
              <div>
                <h4>指挥中心报警</h4>
                <span class="service-status" :class="status.alarm?.centerActive ? 'status-on' : 'status-off'">{{ status.alarm?.centerActive ? '警报中' : '待机' }}</span>
              </div>
            </div>
            <div class="service-body">
              <div class="service-detail">
                <span class="detail-label">触发条件</span>
                <span class="detail-value">水浸/入侵/气体任一达到高风险</span>
              </div>
              <div class="service-detail">
                <span class="detail-label">执行动作</span>
                <span class="detail-value">记录告警事件 + 写入系统日志</span>
              </div>
              <div class="service-detail">
                <span class="detail-label">停止条件</span>
                <span class="detail-value">所有风险项恢复低风险</span>
              </div>
            </div>
          </div>

          <!-- 通风设备 / 净化器 -->
          <div class="service-card" :class="{ 'service-active': status.gas?.ventilationActive }">
            <div class="service-header">
              <span class="service-icon vent-icon">
                <svg width="22" height="22" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M9.59 4.59A2 2 0 1 1 11 8H2m10.59 11.41A2 2 0 1 0 14 16H2m15.73-8.27A2.5 2.5 0 1 1 19.5 12H2"/></svg>
              </span>
              <div>
                <h4>通风净化服务</h4>
                <span class="service-status" :class="status.gas?.ventilationActive ? 'status-on' : 'status-off'">{{ status.gas?.ventilationActive ? '运行中' : '待机' }}</span>
              </div>
            </div>
            <div class="service-body">
              <div class="service-detail">
                <span class="detail-label">触发条件</span>
                <span class="detail-value">有害气体(TVOC/CH2O/CO2)超标</span>
              </div>
              <div class="service-detail">
                <span class="detail-label">执行动作</span>
                <span class="detail-value">开启空气净化器 + 恒湿机净化模式</span>
              </div>
              <div class="service-detail">
                <span class="detail-label">停止条件</span>
                <span class="detail-value">气体浓度全部恢复正常</span>
              </div>
            </div>
          </div>

          <!-- 水浸联动 / 排水 -->
          <div class="service-card" :class="{ 'service-active': status.water?.controlActive }">
            <div class="service-header">
              <span class="service-icon water-icon">
                <svg width="22" height="22" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><circle cx="12" cy="12" r="10"/><path d="M8 12h8"/></svg>
              </span>
              <div>
                <h4>排水联动服务</h4>
                <span class="service-status" :class="status.water?.controlActive ? 'status-on' : 'status-off'">{{ status.water?.controlActive ? '控制中' : '待机' }}</span>
              </div>
            </div>
            <div class="service-body">
              <div class="service-detail">
                <span class="detail-label">触发条件</span>
                <span class="detail-value">水浸传感器检测到水位超限(>3cm)</span>
              </div>
              <div class="service-detail">
                <span class="detail-label">执行动作</span>
                <span class="detail-value">启动排水泵 + 关闭水源阀门(模拟)</span>
              </div>
              <div class="service-detail">
                <span class="detail-label">停止条件</span>
                <span class="detail-value">水位恢复正常(<1cm)</span>
              </div>
            </div>
          </div>
        </div>
      </div>

      <!-- 设备在线状态 -->
      <div class="hardware-section mb-5">
        <h3 class="service-title">
          <svg width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><rect x="2" y="2" width="20" height="8" rx="2" ry="2"/><rect x="2" y="14" width="20" height="8" rx="2" ry="2"/><line x1="6" y1="6" x2="6.01" y2="6"/><line x1="6" y1="18" x2="6.01" y2="18"/></svg>
          设备在线状态
        </h3>
        <p class="hardware-desc">安防系统关联的所有硬件设备通信状态</p>
        <div class="hardware-grid">
          <div class="hardware-item" :class="{ 'hardware-offline': status.devices?.water?.online === false }">
            <svg width="32" height="32" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.5"><path d="M12 2.69l5.66 5.66a8 8 0 1 1-11.31 0z"/></svg>
            <p>水浸传感器</p>
            <div class="hw-status">
              <span class="hw-dot" :class="status.devices?.water?.online === false ? 'hw-dot-off' : 'hw-dot-on'"></span>
              <span :class="status.devices?.water?.online === false ? 'text-danger' : 'text-safe'">{{ status.devices?.water?.online === false ? '离线' : '在线' }}</span>
            </div>
          </div>
          <div class="hardware-item" :class="{ 'hardware-offline': status.devices?.infrared?.online === false }">
            <svg width="32" height="32" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.5"><path d="M1 12s4-8 11-8 11 8 11 8-4 8-11 8-11-8-11-8z"/><circle cx="12" cy="12" r="3"/></svg>
            <p>红外探测器</p>
            <div class="hw-status">
              <span class="hw-dot" :class="status.devices?.infrared?.online === false ? 'hw-dot-off' : 'hw-dot-on'"></span>
              <span :class="status.devices?.infrared?.online === false ? 'text-danger' : 'text-safe'">{{ status.devices?.infrared?.online === false ? '离线' : '在线' }}</span>
            </div>
          </div>
          <div class="hardware-item" :class="{ 'hardware-offline': status.devices?.smoke?.online === false }">
            <svg width="32" height="32" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.5"><path d="M8 14s1.5 2 4 2 4-2 4-2"/><line x1="9" y1="9" x2="9.01" y2="9"/><line x1="15" y1="9" x2="15.01" y2="9"/><path d="M12 2C6.5 2 2 6.5 2 12s4.5 10 10 10 10-4.5 10-10S17.5 2 12 2z"/></svg>
            <p>烟雾报警器</p>
            <div class="hw-status">
              <span class="hw-dot" :class="status.devices?.smoke?.online === false ? 'hw-dot-off' : 'hw-dot-on'"></span>
              <span :class="status.devices?.smoke?.online === false ? 'text-danger' : 'text-safe'">{{ status.devices?.smoke?.online === false ? '离线' : '在线' }}</span>
            </div>
          </div>
          <div class="hardware-item" :class="{ 'hardware-offline': status.devices?.alarm?.online === false }">
            <svg width="32" height="32" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.5"><polygon points="11 5 6 9 2 9 2 15 6 15 11 19 11 5"/><path d="M15.54 8.46a5 5 0 0 1 0 7.07"/></svg>
            <p>声音报警装置</p>
            <div class="hw-status">
              <span class="hw-dot" :class="status.devices?.alarm?.online === false ? 'hw-dot-off' : 'hw-dot-on'"></span>
              <span :class="status.devices?.alarm?.online === false ? 'text-danger' : 'text-safe'">{{ status.devices?.alarm?.online === false ? '离线' : '在线' }}</span>
            </div>
          </div>
          <div class="hardware-item" :class="{ 'hardware-offline': status.devices?.purifier?.online === false }">
            <svg width="32" height="32" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.5"><path d="M9.59 4.59A2 2 0 1 1 11 8H2m10.59 11.41A2 2 0 1 0 14 16H2m15.73-8.27A2.5 2.5 0 1 1 19.5 12H2"/></svg>
            <p>空气净化器</p>
            <div class="hw-status">
              <span class="hw-dot" :class="status.devices?.purifier?.online === false ? 'hw-dot-off' : 'hw-dot-on'"></span>
              <span :class="status.devices?.purifier?.online === false ? 'text-danger' : 'text-safe'">{{ status.devices?.purifier?.online === false ? '离线' : '在线' }}</span>
            </div>
          </div>
          <div class="hardware-item" :class="{ 'hardware-offline': status.devices?.humidifier?.online === false }">
            <svg width="32" height="32" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.5"><circle cx="12" cy="12" r="10"/><path d="M8 12h8"/></svg>
            <p>恒湿净化一体机</p>
            <div class="hw-status">
              <span class="hw-dot" :class="status.devices?.humidifier?.online === false ? 'hw-dot-off' : 'hw-dot-on'"></span>
              <span :class="status.devices?.humidifier?.online === false ? 'text-danger' : 'text-safe'">{{ status.devices?.humidifier?.online === false ? '离线' : '在线' }}</span>
            </div>
          </div>
        </div>
      </div>

      <!-- 系统事件日志 -->
      <div class="log-section">
        <div class="log-section-header">
          <h3 class="service-title">
            <svg width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M14 2H6a2 2 0 0 0-2 2v16a2 2 0 0 0 2 2h12a2 2 0 0 0 2-2V8z"/><path d="M14 2v6h6"/><line x1="16" y1="13" x2="8" y2="13"/><line x1="16" y1="17" x2="8" y2="17"/></svg>
            系统事件日志
          </h3>
        </div>
        <div class="log-container">
          <div v-for="(entry, i) in logs" :key="i" class="log-entry" :class="'log-' + entry.level">
            <div class="log-time">{{ entry.timestamp }}</div>
            <div class="log-event">{{ entry.event }}</div>
            <div class="log-details">{{ entry.details }}</div>
          </div>
          <div v-if="logs.length === 0" class="log-empty">暂无日志</div>
        </div>
      </div>
    </main>
  </div>
</template>

<script setup lang="ts">
import { ref, computed, onMounted, onUnmounted } from "vue"
import { useRouter } from "vue-router"
import { ElMessage } from "element-plus"
import axios from "axios"

const router = useRouter()
const currentTime = ref("")

interface SecurityStatus {
  system: { overallRisk: string; systemNormal: boolean; running: boolean }
  water: { level: number; sensorState: number; risk: string; controlActive: boolean; online: boolean }
  intrusion: { infraredState: number; radarState: number; risk: string; online: boolean }
  gas: { online: boolean; tvoc: { value: number; unit: string; online: boolean }; ch2o: { value: number; unit: string; online: boolean }; co2: { value: number; unit: string; online: boolean }; risk: string; ventilationActive: boolean }
  alarm: { soundActive: boolean; centerActive: boolean }
  devices?: { water?: { online: boolean }; infrared?: { online: boolean }; smoke?: { online: boolean }; alarm?: { online: boolean }; purifier?: { online: boolean }; humidifier?: { online: boolean } }
}

interface LogEntry {
  timestamp: string
  level: string
  event: string
  details: string
}

const status = ref<SecurityStatus>({
  system: { overallRisk: "低风险", systemNormal: true, running: true },
  water: { level: 0, sensorState: 0, risk: "低风险", controlActive: false, online: true },
  intrusion: { infraredState: 0, radarState: 0, risk: "低风险", online: true },
  gas: { online: true, tvoc: { value: 0, unit: "ppb", online: true }, ch2o: { value: 0, unit: "ppb", online: true }, co2: { value: 0, unit: "ppm", online: true }, risk: "低风险", ventilationActive: false },
  alarm: { soundActive: false, centerActive: false },
  devices: { water: { online: true }, infrared: { online: true }, smoke: { online: true }, alarm: { online: true }, purifier: { online: true }, humidifier: { online: true } }
})

const logs = ref<LogEntry[]>([])

const intrusionStatusText = computed(() => {
  const ir = status.value.intrusion?.infraredState ?? 0
  const radar = status.value.intrusion?.radarState ?? 0
  if (ir !== 0 || radar !== 0) return "检测到非法入侵"
  return "无入侵"
})

function statusDotClass(risk?: string) {
  if (risk === "高风险") return "dot-alert"
  if (risk === "中风险") return "dot-warning"
  return "dot-normal"
}

function riskClass(risk?: string) {
  if (risk === "高风险") return "text-danger"
  if (risk === "中风险") return "text-warning"
  return "text-safe"
}

function gasItemClass(item?: { value: number; unit: string; online: boolean }, warnThreshold?: number) {
  if (!item || item.online === false) return "text-unknown"
  if (warnThreshold && item.value > warnThreshold) return "text-danger"
  return ""
}

function gasItemText(item?: { value: number; unit: string; online: boolean }) {
  if (!item || item.online === false) return "--"
  return item.value + " " + item.unit
}

const updateTime = () => {
  const d = new Date()
  currentTime.value = `${d.getFullYear()}-${(d.getMonth()+1).toString().padStart(2,"0")}-${d.getDate().toString().padStart(2,"0")} ${d.getHours().toString().padStart(2,"0")}:${d.getMinutes().toString().padStart(2,"0")}:${d.getSeconds().toString().padStart(2,"0")}`
}

const handleLogout = () => {
  localStorage.removeItem("isLoggedIn")
  router.push({ name: "login" })
}

const fetchStatus = async () => {
  try {
    const res = await axios.get("/security/api/status")
    status.value = res.data
  } catch (e) {}
}

const fetchLogs = async () => {
  try {
    const res = await axios.get("/security/api/logs", { params: { limit: 50 } })
    logs.value = res.data || []
  } catch (e) {}
}

const doControl = async (target: string, action: string) => {
  try {
    const res = await axios.get("/security/api/control", { params: { target, action } })
    if (res.data.status === "success") {
      ElMessage.success(res.data.message || "指令发送成功")
    } else {
      ElMessage.warning("指令发送失败")
    }
    fetchStatus()
    fetchLogs()
  } catch (e) {
    ElMessage.error("网络请求失败")
  }
}

let timeInterval = 0
let statusInterval = 0
let logsInterval = 0

onMounted(() => {
  updateTime()
  timeInterval = window.setInterval(updateTime, 1000)
  fetchStatus()
  fetchLogs()
  statusInterval = window.setInterval(fetchStatus, 2000)
  logsInterval = window.setInterval(fetchLogs, 5000)
})

onUnmounted(() => {
  window.clearInterval(timeInterval)
  window.clearInterval(statusInterval)
  window.clearInterval(logsInterval)
})
</script>

<style scoped>
/* ===== 根容器 ===== */
.security-root {
  background: #0f172a;
  background-image:
    radial-gradient(ellipse at 20% 50%, rgba(99, 102, 241, 0.06) 0%, transparent 60%),
    radial-gradient(ellipse at 80% 20%, rgba(59, 130, 246, 0.04) 0%, transparent 50%);
  font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", "PingFang SC", "Hiragino Sans GB", "Microsoft YaHei", sans-serif;
  color: #e2e8f0;
}

/* ===== 顶部导航 ===== */
.nav-header {
  background: rgba(15, 23, 42, 0.85);
  backdrop-filter: blur(16px);
  border-bottom: 1px solid rgba(148, 163, 184, 0.1);
  padding: 0 24px;
  position: sticky;
  top: 0;
  z-index: 50;
}
.nav-inner {
  display: flex;
  align-items: center;
  justify-content: space-between;
  height: 60px;
}
.brand-icon {
  display: flex;
  align-items: center;
  justify-content: center;
  width: 40px;
  height: 40px;
  border-radius: 10px;
  background: linear-gradient(135deg, #6366f1, #3b82f6);
  color: white;
  box-shadow: 0 2px 8px rgba(99, 102, 241, 0.3);
}
.brand-title {
  font-size: 18px;
  font-weight: 700;
  color: #f1f5f9;
  letter-spacing: 0.02em;
}
.brand-sub {
  font-size: 11px;
  color: #64748b;
  letter-spacing: 0.03em;
  margin-top: -1px;
}
.nav-time {
  display: flex;
  align-items: center;
  font-size: 13px;
  color: #94a3b8;
  font-variant-numeric: tabular-nums;
}
.nav-btn {
  display: flex;
  align-items: center;
  padding: 7px 14px;
  font-size: 13px;
  color: #94a3b8;
  background: rgba(148, 163, 184, 0.06);
  border: 1px solid rgba(148, 163, 184, 0.12);
  border-radius: 8px;
  cursor: pointer;
  transition: all 0.2s;
}
.nav-btn:hover {
  color: #e2e8f0;
  background: rgba(99, 102, 241, 0.1);
  border-color: rgba(99, 102, 241, 0.3);
}
.nav-btn-danger {
  display: flex;
  align-items: center;
  padding: 7px 14px;
  font-size: 13px;
  color: #94a3b8;
  background: rgba(239, 68, 68, 0.06);
  border: 1px solid rgba(239, 68, 68, 0.15);
  border-radius: 8px;
  cursor: pointer;
  transition: all 0.2s;
}
.nav-btn-danger:hover {
  color: #fca5a5;
  background: rgba(239, 68, 68, 0.12);
  border-color: rgba(239, 68, 68, 0.3);
}

/* ===== 页面标题 ===== */
.page-title-bar {
  display: flex;
  align-items: center;
  justify-content: space-between;
}
.page-title {
  font-size: 24px;
  font-weight: 700;
  color: #f1f5f9;
}
.page-subtitle {
  font-size: 13px;
  color: #64748b;
  margin-top: 4px;
}

/* ===== 系统概览 ===== */
.overview-bar {
  display: flex;
  gap: 16px;
}
.status-card {
  flex: 1;
  background: rgba(30, 41, 59, 0.6);
  border: 1px solid rgba(148, 163, 184, 0.08);
  border-radius: 12px;
  padding: 20px;
  text-align: center;
  backdrop-filter: blur(8px);
}
.status-card h3 {
  font-size: 14px;
  color: #94a3b8;
  margin-bottom: 8px;
}
.status-dot {
  display: inline-block;
  width: 14px;
  height: 14px;
  border-radius: 50%;
}
.dot-normal {
  background-color: #34d399;
  box-shadow: 0 0 8px rgba(52, 211, 153, 0.5);
}
.dot-warning {
  background-color: #f59e0b;
  box-shadow: 0 0 8px rgba(245, 158, 11, 0.5);
}
.dot-alert {
  background-color: #ef4444;
  box-shadow: 0 0 8px rgba(239, 68, 68, 0.5);
  animation: pulse-dot 1.5s infinite;
}
.status-text {
  font-size: 16px;
  font-weight: 700;
  color: #f1f5f9;
}
.status-desc {
  font-size: 12px;
  color: #64748b;
  margin-top: 8px;
}

/* ===== 应用卡片网格 ===== */
.app-grid {
  display: grid;
  grid-template-columns: repeat(3, 1fr);
  gap: 16px;
}
.app-card {
  background: rgba(30, 41, 59, 0.6);
  border: 1px solid rgba(148, 163, 184, 0.08);
  border-radius: 12px;
  padding: 20px;
  backdrop-filter: blur(8px);
  transition: transform 0.3s, border-color 0.3s;
}
.app-card:hover {
  transform: translateY(-3px);
  border-color: rgba(99, 102, 241, 0.2);
}
.app-card-header {
  display: flex;
  align-items: center;
  gap: 12px;
  padding-bottom: 14px;
  border-bottom: 1px solid rgba(148, 163, 184, 0.08);
}
.app-card-header svg {
  color: #6366f1;
}
.app-card-header h2 {
  font-size: 18px;
  font-weight: 600;
  color: #e2e8f0;
}
.app-card-desc {
  font-size: 13px;
  color: #64748b;
  margin: 12px 0;
}
.data-row {
  display: flex;
  justify-content: space-between;
  align-items: center;
  padding: 10px 12px;
  background: rgba(15, 23, 42, 0.5);
  border-radius: 8px;
  margin-bottom: 8px;
}
.data-label {
  font-size: 13px;
  color: #94a3b8;
}
.data-value {
  font-size: 14px;
  font-weight: 600;
  color: #f1f5f9;
}
.text-danger { color: #ef4444; }
.text-warning { color: #f59e0b; }
.text-safe { color: #34d399; }
.text-unknown { color: #64748b; }

/* ===== 操作按钮 ===== */
.action-btn {
  padding: 8px 16px;
  font-size: 13px;
  font-weight: 600;
  color: #e2e8f0;
  background: rgba(148, 163, 184, 0.08);
  border: 1px solid rgba(148, 163, 184, 0.12);
  border-radius: 8px;
  cursor: pointer;
  transition: all 0.2s;
}
.action-btn:hover {
  background: rgba(148, 163, 184, 0.15);
}
.water-btn {
  border-left: 3px solid #3b82f6;
}
.intrusion-btn {
  border-left: 3px solid #8b5cf6;
}
.gas-btn {
  border-left: 3px solid #ef4444;
}
.btn-enable {
  border-left: 3px solid #34d399;
  color: #34d399;
}
.btn-enable:hover {
  background: rgba(52, 211, 153, 0.1);
  border-color: rgba(52, 211, 153, 0.3);
}
.btn-disable {
  border-left: 3px solid #ef4444;
  color: #fca5a5;
}
.btn-disable:hover {
  background: rgba(239, 68, 68, 0.1);
  border-color: rgba(239, 68, 68, 0.3);
}

/* ===== 联动服务状态 ===== */
.services-section {
  background: rgba(30, 41, 59, 0.6);
  border: 1px solid rgba(148, 163, 184, 0.08);
  border-radius: 12px;
  padding: 24px;
  backdrop-filter: blur(8px);
}
.section-title {
  display: flex;
  align-items: center;
  gap: 8px;
  font-size: 18px;
  font-weight: 600;
  color: #e2e8f0;
  margin-bottom: 8px;
}
.section-title svg {
  color: #6366f1;
}
.section-desc {
  font-size: 13px;
  color: #64748b;
  margin-bottom: 20px;
}
.services-grid {
  display: grid;
  grid-template-columns: repeat(4, 1fr);
  gap: 16px;
}
.service-card {
  background: rgba(15, 23, 42, 0.5);
  border: 1px solid rgba(148, 163, 184, 0.08);
  border-radius: 10px;
  padding: 18px;
  transition: all 0.3s;
}
.service-card.service-active {
  border-color: rgba(239, 68, 68, 0.4);
  box-shadow: 0 0 12px rgba(239, 68, 68, 0.1);
}
.service-header {
  display: flex;
  align-items: center;
  gap: 12px;
  margin-bottom: 16px;
  padding-bottom: 12px;
  border-bottom: 1px solid rgba(148, 163, 184, 0.08);
}
.service-header h4 {
  font-size: 14px;
  font-weight: 600;
  color: #e2e8f0;
  margin: 0;
}
.service-icon {
  width: 40px;
  height: 40px;
  border-radius: 10px;
  display: flex;
  align-items: center;
  justify-content: center;
  flex-shrink: 0;
}
.alarm-icon { background: rgba(239, 68, 68, 0.15); color: #ef4444; }
.center-icon { background: rgba(99, 102, 241, 0.15); color: #6366f1; }
.vent-icon { background: rgba(16, 185, 129, 0.15); color: #10b981; }
.water-icon { background: rgba(59, 130, 246, 0.15); color: #3b82f6; }
.service-status {
  font-size: 12px;
  padding: 2px 8px;
  border-radius: 4px;
}
.status-on {
  background: rgba(239, 68, 68, 0.2);
  color: #ef4444;
}
.status-off {
  background: rgba(100, 116, 139, 0.2);
  color: #94a3b8;
}
.service-body {
  display: flex;
  flex-direction: column;
  gap: 8px;
}
.service-detail {
  display: flex;
  justify-content: space-between;
  align-items: flex-start;
  gap: 8px;
}
.detail-label {
  font-size: 11px;
  color: #64748b;
  white-space: nowrap;
  min-width: 56px;
}
.detail-value {
  font-size: 12px;
  color: #cbd5e1;
  text-align: right;
}
.service-title {
  display: flex;
  align-items: center;
  gap: 8px;
  font-size: 16px;
  font-weight: 600;
  color: #e2e8f0;
  margin-bottom: 16px;
  padding-bottom: 12px;
  border-bottom: 1px solid rgba(148, 163, 184, 0.08);
}
.service-title svg {
  color: #6366f1;
}

/* ===== 硬件层 ===== */
.hardware-section {
  background: rgba(30, 41, 59, 0.6);
  border: 1px solid rgba(148, 163, 184, 0.08);
  border-radius: 12px;
  padding: 20px;
  backdrop-filter: blur(8px);
}
.hardware-desc {
  font-size: 13px;
  color: #64748b;
  margin-bottom: 16px;
}
.hardware-grid {
  display: grid;
  grid-template-columns: repeat(auto-fill, minmax(180px, 1fr));
  gap: 12px;
}
.hardware-item {
  display: flex;
  flex-direction: column;
  align-items: center;
  padding: 18px 12px;
  background: rgba(15, 23, 42, 0.5);
  border: 1px solid rgba(148, 163, 184, 0.06);
  border-radius: 10px;
  text-align: center;
  transition: all 0.3s;
}
.hardware-item:hover {
  transform: scale(1.03);
  border-color: rgba(99, 102, 241, 0.2);
}
.hardware-item svg {
  color: #6366f1;
  margin-bottom: 10px;
}
.hardware-item p {
  font-size: 14px;
  font-weight: 600;
  color: #e2e8f0;
  margin: 0;
}
.hardware-item small {
  font-size: 11px;
  color: #64748b;
  margin-top: 4px;
}
.hw-status {
  display: flex;
  align-items: center;
  gap: 6px;
  margin-top: 8px;
  font-size: 12px;
}
.hw-dot {
  width: 8px;
  height: 8px;
  border-radius: 50%;
}
.hw-dot-on {
  background: #34d399;
  box-shadow: 0 0 6px rgba(52, 211, 153, 0.5);
}
.hw-dot-off {
  background: #aaa;
}
.hw-btn {
  margin-top: 8px;
  padding: 4px 12px;
  font-size: 12px;
  font-weight: 600;
  color: #e2e8f0;
  background: rgba(148, 163, 184, 0.08);
  border: 1px solid rgba(148, 163, 184, 0.12);
  border-radius: 6px;
  cursor: pointer;
  transition: all 0.2s;
}
.hw-btn:hover {
  background: rgba(148, 163, 184, 0.15);
}
.hardware-offline {
  border-color: rgba(239, 68, 68, 0.3);
  background: rgba(239, 68, 68, 0.05);
}

/* ===== 日志区域 ===== */
.log-section {
  background: rgba(30, 41, 59, 0.6);
  border: 1px solid rgba(148, 163, 184, 0.08);
  border-radius: 12px;
  padding: 20px;
  backdrop-filter: blur(8px);
}
.log-section-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  margin-bottom: 16px;
}
.log-container {
  max-height: 320px;
  overflow-y: auto;
  background: rgba(15, 23, 42, 0.5);
  border-radius: 8px;
  padding: 12px;
}
.log-entry {
  padding: 10px 14px;
  margin-bottom: 8px;
  border-radius: 6px;
  background: rgba(30, 41, 59, 0.6);
  border-left: 3px solid rgba(99, 102, 241, 0.4);
}
.log-alarm { border-left-color: #ef4444; }
.log-warning { border-left-color: #f59e0b; }
.log-normal { border-left-color: #34d399; }
.log-time {
  font-size: 12px;
  color: #64748b;
  font-variant-numeric: tabular-nums;
}
.log-event {
  font-size: 13px;
  font-weight: 600;
  color: #e2e8f0;
  margin: 4px 0;
}
.log-details {
  font-size: 12px;
  color: #94a3b8;
}
.log-empty {
  text-align: center;
  color: #64748b;
  padding: 20px;
  font-size: 13px;
}

/* ===== 动画 ===== */
@keyframes pulse-dot {
  0% { box-shadow: 0 0 0 0 rgba(239, 68, 68, 0.6); }
  70% { box-shadow: 0 0 0 8px rgba(239, 68, 68, 0); }
  100% { box-shadow: 0 0 0 0 rgba(239, 68, 68, 0); }
}
@keyframes pulse-border {
  0% { box-shadow: 0 0 0 0 rgba(239, 68, 68, 0.5); }
  70% { box-shadow: 0 0 0 6px rgba(239, 68, 68, 0); }
  100% { box-shadow: 0 0 0 0 rgba(239, 68, 68, 0); }
}

/* ===== 响应式 ===== */
@media (max-width: 1024px) {
  .app-grid { grid-template-columns: 1fr; }
  .services-grid { grid-template-columns: repeat(2, 1fr); }
}
@media (max-width: 768px) {
  .overview-bar { flex-direction: column; }
  .hardware-grid { grid-template-columns: repeat(2, 1fr); }
}
</style>
