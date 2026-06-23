import { ref, onUnmounted } from 'vue'

export interface UseAnimationOptions {
  duration?: number
  easing?: (t: number) => number
  onUpdate?: (value: number) => void
  onComplete?: () => void
}

// 常用缓动函数
export const easings = {
  linear: (t: number) => t,
  easeInQuad: (t: number) => t * t,
  easeOutQuad: (t: number) => t * (2 - t),
  easeInOutQuad: (t: number) => t < 0.5 ? 2 * t * t : -1 + (4 - 2 * t) * t,
  easeInCubic: (t: number) => t * t * t,
  easeOutCubic: (t: number) => (--t) * t * t + 1,
  easeInOutCubic: (t: number) => t < 0.5 ? 4 * t * t * t : (t - 1) * (2 * t - 2) * (2 * t - 2) + 1,
  easeOutElastic: (t: number) => {
    const p = 0.3
    return Math.pow(2, -10 * t) * Math.sin((t - p / 4) * (2 * Math.PI) / p) + 1
  },
  easeOutBounce: (t: number) => {
    if (t < 1 / 2.75) {
      return 7.5625 * t * t
    } else if (t < 2 / 2.75) {
      return 7.5625 * (t -= 1.5 / 2.75) * t + 0.75
    } else if (t < 2.5 / 2.75) {
      return 7.5625 * (t -= 2.25 / 2.75) * t + 0.9375
    } else {
      return 7.5625 * (t -= 2.625 / 2.75) * t + 0.984375
    }
  }
}

export function useAnimation() {
  let animationFrameId: number | null = null
  let startTime: number | null = null
  let isRunning = ref(false)

  // 停止动画
  function stop() {
    if (animationFrameId) {
      cancelAnimationFrame(animationFrameId)
      animationFrameId = null
    }
    startTime = null
    isRunning.value = false
  }

  // 执行动画
  function animate(options: UseAnimationOptions): Promise<void> {
    const {
      duration = 300,
      easing = easings.easeOutCubic,
      onUpdate,
      onComplete
    } = options

    // 如果已有动画在运行，先停止
    stop()

    return new Promise((resolve) => {
      isRunning.value = true
      startTime = performance.now()

      function tick(currentTime: number) {
        if (!startTime) {
          stop()
          resolve()
          return
        }

        const elapsed = currentTime - startTime
        const progress = Math.min(elapsed / duration, 1)
        const easedProgress = easing(progress)

        onUpdate?.(easedProgress)

        if (progress < 1) {
          animationFrameId = requestAnimationFrame(tick)
        } else {
          stop()
          onComplete?.()
          resolve()
        }
      }

      animationFrameId = requestAnimationFrame(tick)
    })
  }

  // 数值动画
  function animateValue(
    from: number,
    to: number,
    options: Omit<UseAnimationOptions, 'onUpdate'> & {
      onUpdate?: (value: number) => void
    } = {}
  ): Promise<void> {
    const { onUpdate, ...rest } = options

    return animate({
      ...rest,
      onUpdate: (progress) => {
        const value = from + (to - from) * progress
        onUpdate?.(value)
      }
    })
  }

  // 延迟
  function delay(ms: number): Promise<void> {
    return new Promise(resolve => setTimeout(resolve, ms))
  }

  // 序列动画
  async function sequence(animations: (() => Promise<void>)[]): Promise<void> {
    for (const anim of animations) {
      await anim()
    }
  }

  // 并行动画
  async function parallel(animations: (() => Promise<void>)[]): Promise<void> {
    await Promise.all(animations.map(anim => anim()))
  }

  // 交错动画
  function stagger(
    items: any[],
    animationFn: (item: any, index: number) => Promise<void>,
    staggerDelay: number = 50
  ): Promise<void> {
    return new Promise(async (resolve) => {
      const promises = items.map((item, index) =>
        delay(staggerDelay * index).then(() => animationFn(item, index))
      )
      await Promise.all(promises)
      resolve()
    })
  }

  // 组件销毁时清理
  onUnmounted(() => {
    stop()
  })

  return {
    isRunning,
    animate,
    animateValue,
    delay,
    sequence,
    parallel,
    stagger,
    stop,
    easings
  }
}
