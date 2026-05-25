# UI 前端开发指南

## 1. 技术栈概述

本项目前端使用 Vue 3 + TypeScript 构建，主要技术栈：

| 技术 | 版本 | 用途 |
|------|------|------|
| Vue 3 | ^3.5 | 核心框架（Composition API + `<script setup>`） |
| TypeScript | ~6.0 | 类型安全 |
| Vite | ^8.0 | 构建工具 + 开发服务器 |
| Vue Router | ^5.0 | 路由管理 |
| Tailwind CSS | ^4.3 | 原子化 CSS 样式 |
| Element Plus | ^2.14 | UI 组件库（Message 提示等） |
| ECharts / vue-echarts | ^6.1 / ^8.0 | 图表可视化 |
| Axios | ^1.16 | HTTP 请求 |

## 2. 项目目录结构

```
ui/
├── index.html                # 入口 HTML
├── package.json              # 依赖配置
├── vite.config.ts            # Vite 构建配置
├── tsconfig.json             # TypeScript 配置
├── public/                   # 静态资源（直接复制到构建产物）
│   ├── favicon.svg
│   └── icons.svg
└── src/
    ├── main.ts               # 应用入口（挂载 Vue 实例）
    ├── style.css             # 全局样式
    ├── App.vue               # 根组件
    ├── router/
    │   └── index.ts          # 路由配置
    ├── api/                  # API 请求封装
    ├── assets/               # 需要构建处理的资源（图片等）
    ├── components/           # 公共组件
    ├── store/                # 状态管理（如有）
    └── views/                # 页面级组件
        ├── Login.vue         # 登录页
        ├── Dashboard.vue     # 主监控面板
        ├── LogViewer.vue     # 系统日志查看页
        └── Devices.vue       # 设备详情管理页
```

## 3. 环境搭建与编译

### 3.1 安装依赖

```bash
cd ui
npm install
```

> **注意**: 需要 Node.js >= 20.19 或 >= 22.12。可通过 `node -v` 检查版本。

### 3.2 开发模式（热重载）

```bash
cd ui
npm run dev
```

启动后访问 `http://localhost:5173`（默认 Vite 端口），Vite 会自动将 `/api/*` 请求代理到后端 `http://localhost:8080`。

开发时需要同时运行后端 C++ 服务，API 才能正常工作。

### 3.3 生产构建

```bash
cd ui
npm run build
```

此命令会：
1. 运行 `vue-tsc -b` 进行 TypeScript 类型检查
2. 运行 `vite build` 生成优化后的静态文件

构建产物输出到 `ui/dist/` 目录。

### 3.4 部署到后端

构建完成后，将产物复制到项目根目录供 C++ Web 服务器使用：

```bash
cp ui/dist/index.html dashboard.html
cp -r ui/dist/assets public/assets/
```

后端 HTTP 服务器（端口 8080）会自动提供：
- `dashboard.html` 作为所有非 API 路由的入口
- `public/assets/` 目录下的 JS/CSS 静态文件

## 4. 核心概念速成（Vue 3 + Composition API）

### 4.1 单文件组件 (SFC) 结构

每个 `.vue` 文件由三部分组成：

```vue
<template>
  <!-- HTML 模板，使用 Vue 指令绑定数据 -->
  <div>{{ message }}</div>
  <button @click="handleClick">点击</button>
</template>

<script setup lang="ts">
// TypeScript 逻辑代码
// 使用 <script setup> 语法，顶层变量/函数自动暴露给模板
import { ref, onMounted } from 'vue'

const message = ref('Hello')  // 响应式变量

function handleClick() {
  message.value = 'Clicked!'  // 修改值需要 .value
}

onMounted(() => {
  // 组件挂载后执行（类似 componentDidMount）
  console.log('组件已加载')
})
</script>

<style scoped>
/* scoped 表示样式只作用于当前组件 */
.my-class { color: red; }
</style>
```

### 4.2 响应式系统

```typescript
import { ref, reactive, computed } from 'vue'

// ref: 基本类型的响应式变量
const count = ref(0)
console.log(count.value)  // 读取: .value
count.value++              // 修改: .value

// reactive: 对象类型的响应式变量
const state = reactive({ name: 'test', age: 18 })
console.log(state.name)    // 读取: 直接访问
state.age = 20             // 修改: 直接赋值

// computed: 计算属性（自动追踪依赖，缓存结果）
const doubleCount = computed(() => count.value * 2)
```

### 4.3 生命周期

```typescript
import { onMounted, onUnmounted } from 'vue'

onMounted(() => {
  // 组件挂载到 DOM 后执行
  // 常用于: 初始化数据、启动定时器、注册事件监听
})

onUnmounted(() => {
  // 组件销毁前执行
  // 常用于: 清理定时器、取消事件监听
})
```

### 4.4 模板指令

```vue
<template>
  <!-- 条件渲染 -->
  <div v-if="isVisible">显示</div>
  <div v-else>隐藏</div>

  <!-- 列表渲染 -->
  <div v-for="(item, index) in items" :key="item.id">
    {{ index }}: {{ item.name }}
  </div>

  <!-- 属性绑定 -->
  <div :class="{ active: isActive }" :style="{ color: textColor }">
  <button @click="handleClick">点击</button>
  <input v-model="inputText" />

  <!-- 双向绑定 -->
  <select v-model="selectedOption">
    <option value="a">A</option>
    <option value="b">B</option>
  </select>
</template>
```

### 4.5 组件间通信

```vue
<!-- 父组件 -->
<template>
  <ChildComponent :title="parentTitle" @update="handleUpdate" />
</template>

<!-- 子组件 ChildComponent.vue -->
<script setup lang="ts">
// 接收 props
const props = defineProps<{ title: string }>()

// 发送事件
const emit = defineEmits<{ update: [value: string] }>()
emit('update', 'new value')
</script>
```

## 5. 添加新页面的步骤

### 5.1 创建页面组件

在 `ui/src/views/` 下创建新文件，例如 `NewPage.vue`：

```vue
<template>
  <div class="cyber-container flex flex-col min-h-screen text-slate-100">
    <!-- Header -->
    <header class="cyber-header flex items-center justify-between px-8 py-4">
      <button class="cyber-btn text-xs" @click="$router.push('/')">◀ BACK</button>
      <h1 class="text-2xl text-cyan-400">新页面标题</h1>
    </header>

    <!-- Content -->
    <main class="flex-1 p-6">
      <!-- 页面内容 -->
    </main>
  </div>
</template>

<script setup lang="ts">
import { ref, onMounted, onUnmounted } from 'vue'
import axios from 'axios'

// 你的数据和逻辑
const data = ref<any>(null)
let refreshInterval = 0

async function fetchData() {
  try {
    const res = await axios.get('/api/your-endpoint')
    data.value = res.data
  } catch (e) {
    console.error('请求失败:', e)
  }
}

onMounted(() => {
  fetchData()
  refreshInterval = window.setInterval(fetchData, 5000)
})

onUnmounted(() => {
  window.clearInterval(refreshInterval)
})
</script>
```

### 5.2 注册路由

编辑 `ui/src/router/index.ts`：

```typescript
import NewPage from '../views/NewPage.vue'

const router = createRouter({
  // ...existing config...
  routes: [
    // ...existing routes...
    {
      path: '/new-page',        // URL 路径
      name: 'newPage',          // 路由名称
      component: NewPage        // 页面组件
    }
  ]
})
```

### 5.3 添加导航入口

在 `Dashboard.vue` 的 header 中添加按钮：

```vue
<button class="cyber-btn" @click="$router.push('/new-page')">新页面</button>
```

### 5.4 添加后端 API（如需要）

编辑 `application/web_server.cpp`，在请求处理的 if-else 链中添加：

```cpp
} else if (req.find("GET /api/your-endpoint") == 0) {
    char json[1024];
    snprintf(json, sizeof(json), "{\"key\":\"value\"}");
    send_response(new_socket, "200 OK", "application/json; charset=utf-8", json, strlen(json));
}
```

### 5.5 构建部署

```bash
cd ui && npm run build
cp dist/index.html ../dashboard.html
cp -r dist/assets ../public/assets/
```

## 6. 路由配置说明

当前路由表 (`ui/src/router/index.ts`)：

| 路径 | 页面 | 说明 |
|------|------|------|
| `/login` | Login.vue | 登录页 |
| `/` | Dashboard.vue | 主监控面板 |
| `/logs` | LogViewer.vue | 系统日志查看 |
| `/devices` | Devices.vue | 设备详情管理 |

路由守卫：未登录时自动跳转到 `/login`，已登录时访问 `/login` 自动跳转到 `/`。

## 7. API 接口列表

前端通过 Axios 调用后端 API，所有接口以 `/api/` 开头：

| 接口 | 方法 | 说明 |
|------|------|------|
| `/api/data` | GET | 获取所有传感器实时数据 |
| `/api/control?device=xx&action=xx&val=xx` | GET | 发送设备控制指令 |
| `/api/devices` | GET | 获取所有设备详细信息（含类别、描述） |
| `/api/bus/stats` | GET | 获取 Modbus 总线统计信息 |
| `/api/cmd/stats` | GET | 获取命令队列统计信息 |
| `/api/logs/list` | GET | 获取所有日志文件列表 |
| `/api/logs/latest` | GET | 获取最新日志文件内容 |
| `/api/logs/{filename}` | GET | 获取指定日志文件内容 |

## 8. 样式系统

### 8.1 Tailwind CSS

项目使用 Tailwind CSS v4 原子化样式，直接在 class 中使用：

```vue
<div class="flex items-center gap-4 p-6 bg-slate-900 text-cyan-300">
  <span class="text-lg font-bold">内容</span>
</div>
```

常用类速查：
- 布局: `flex`, `grid`, `items-center`, `justify-between`, `gap-4`
- 间距: `p-4`(padding), `m-4`(margin), `px-8`(左右), `py-4`(上下)
- 颜色: `text-cyan-300`, `bg-slate-900`, `border-cyan-500/30`
- 尺寸: `w-full`, `h-screen`, `max-h-[500px]`
- 响应式: `lg:col-span-3`（lg 断点以上生效）

### 8.2 Cyber 风格自定义类

项目定义了一套赛博朋克风格的自定义 CSS 类（在各页面的 `<style scoped>` 中）：

| 类名 | 用途 |
|------|------|
| `.cyber-container` | 页面根容器（深色背景 + 渐变） |
| `.cyber-header` | 顶部导航栏（毛玻璃效果） |
| `.cyber-card` | 卡片容器（发光边框 + 角标） |
| `.cyber-btn` | 赛博风格按钮（切角 + 发光） |
| `.cyber-btn-danger` | 红色危险按钮 |
| `.section-title` | 区域标题（发光文字 + 底部线条） |
| `.sensor-box` | 传感器数据盒子 |
| `.alert-item` | 报警项 |

## 9. 常见开发场景

### 9.1 发起 API 请求

```typescript
import axios from 'axios'

// GET 请求
const res = await axios.get('/api/data')
console.log(res.data.temperature)

// 带参数的 GET 请求
const res2 = await axios.get('/api/control', {
  params: { device: 'ac', action: 'cool_on' }
})

// 错误处理
try {
  const res = await axios.get('/api/xxx')
} catch (e) {
  console.error('请求失败:', e)
}
```

### 9.2 定时刷新数据

```typescript
import { onMounted, onUnmounted } from 'vue'

let timer = 0

onMounted(() => {
  fetchData()  // 立即获取一次
  timer = window.setInterval(fetchData, 2000)  // 每2秒刷新
})

onUnmounted(() => {
  window.clearInterval(timer)  // 组件销毁时清理
})
```

### 9.3 使用 Element Plus 消息提示

```typescript
import { ElMessage } from 'element-plus'

ElMessage.success('操作成功')
ElMessage.warning('警告信息')
ElMessage.error('操作失败')
ElMessage.info('提示信息')
```

### 9.4 使用 ECharts 图表

```vue
<template>
  <v-chart class="w-full h-[400px]" :option="chartOption" autoresize />
</template>

<script setup lang="ts">
import VChart from 'vue-echarts'
import { use } from 'echarts/core'
import { CanvasRenderer } from 'echarts/renderers'
import { LineChart } from 'echarts/charts'
import { GridComponent, TooltipComponent } from 'echarts/components'

use([CanvasRenderer, LineChart, GridComponent, TooltipComponent])

const chartOption = {
  xAxis: { type: 'category', data: ['Mon', 'Tue', 'Wed'] },
  yAxis: { type: 'value' },
  series: [{ type: 'line', data: [120, 200, 150] }]
}
</script>
```

## 10. Vite 开发代理配置

`ui/vite.config.ts` 中配置了开发代理：

```typescript
server: {
  proxy: {
    '/api': {
      target: 'http://localhost:8080',  // 后端 C++ 服务器
      changeOrigin: true
    }
  }
}
```

这意味着开发模式下，前端 `http://localhost:5173/api/data` 会被代理到 `http://localhost:8080/api/data`。

## 11. 调试技巧

1. **浏览器开发者工具**: F12 打开，Network 面板查看 API 请求和响应
2. **Vue DevTools**: 安装浏览器扩展，可查看组件树、响应式数据、路由状态
3. **TypeScript 错误**: 运行 `npx vue-tsc -b` 可单独检查类型错误，无需完整构建
4. **热重载**: `npm run dev` 后修改代码会自动刷新浏览器
5. **Console 日志**: `console.log()` 输出可在浏览器控制台查看