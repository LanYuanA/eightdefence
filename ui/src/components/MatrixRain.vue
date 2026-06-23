<template>
  <div class="matrix-container" ref="containerRef">
    <canvas ref="canvasRef" class="matrix-canvas" />
    <div class="matrix-overlay">
      <slot />
    </div>
  </div>
</template>

<script setup lang="ts">
import { ref, onMounted, onUnmounted } from 'vue'

const props = withDefaults(defineProps<{
  fontSize?: number
  color?: string
  speed?: number
  density?: number
  opacity?: number
}>(), {
  fontSize: 14,
  color: '#00ff41',
  speed: 33,
  density: 0.975,
  opacity: 0.05
})

const containerRef = ref<HTMLElement | null>(null)
const canvasRef = ref<HTMLCanvasElement | null>(null)
let animationId: number | null = null

// 矩阵字符集
const matrixChars = 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789@#$%^&*()_+-=[]{}|;:,.<>?~`'

onMounted(() => {
  if (!canvasRef.value || !containerRef.value) return

  const canvas = canvasRef.value
  const ctx = canvas.getContext('2d')
  if (!ctx) return

  // 设置画布大小
  function resize() {
    if (!containerRef.value) return
    canvas.width = containerRef.value.offsetWidth
    canvas.height = containerRef.value.offsetHeight
  }
  resize()
  window.addEventListener('resize', resize)

  // 初始化列
  const columns = Math.floor(canvas.width / props.fontSize)
  const drops: number[] = []
  for (let i = 0; i < columns; i++) {
    drops[i] = Math.random() * -100
  }

  // 绘制函数
  function draw() {
    if (!ctx) return

    // 半透明覆盖，产生拖尾效果
    ctx.fillStyle = `rgba(0, 0, 0, ${props.opacity})`
    ctx.fillRect(0, 0, canvas.width, canvas.height)

    // 设置文字样式
    ctx.fillStyle = props.color
    ctx.font = `${props.fontSize}px monospace`
    ctx.shadowColor = props.color
    ctx.shadowBlur = 5

    // 绘制字符
    for (let i = 0; i < drops.length; i++) {
      // 随机字符
      const char = matrixChars[Math.floor(Math.random() * matrixChars.length)]

      // 绘制字符
      ctx.fillText(char, i * props.fontSize, drops[i] * props.fontSize)

      // 随机重置
      if (drops[i] * props.fontSize > canvas.height && Math.random() > props.density) {
        drops[i] = 0
      }

      drops[i]++
    }

    ctx.shadowBlur = 0
    animationId = requestAnimationFrame(draw)
  }

  // 控制帧率
  let lastTime = 0
  function animate(currentTime: number) {
    if (currentTime - lastTime >= props.speed) {
      draw()
      lastTime = currentTime
    }
    animationId = requestAnimationFrame(animate)
  }

  animate(0)
})

onUnmounted(() => {
  if (animationId) {
    cancelAnimationFrame(animationId)
  }
})
</script>

<style scoped>
.matrix-container {
  position: relative;
  width: 100%;
  height: 100%;
  overflow: hidden;
  background: #000;
}

.matrix-canvas {
  position: absolute;
  top: 0;
  left: 0;
  width: 100%;
  height: 100%;
}

.matrix-overlay {
  position: relative;
  z-index: 1;
  width: 100%;
  height: 100%;
}
</style>
