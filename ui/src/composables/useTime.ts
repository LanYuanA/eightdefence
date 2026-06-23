import { ref, onMounted, onUnmounted } from 'vue'

export interface UseTimeOptions {
  format?: 'full' | 'date' | 'time' | 'datetime'
  locale?: string
  updateInterval?: number
}

export function useTime(options: UseTimeOptions = {}) {
  const {
    format = 'full',
    locale = 'zh-CN',
    updateInterval = 1000
  } = options

  const currentTime = ref('')
  const currentDate = ref('')
  const currentWeekday = ref('')
  let intervalId: ReturnType<typeof setInterval> | null = null

  function updateTime() {
    const now = new Date()

    switch (format) {
      case 'full':
        currentTime.value = now.toLocaleString(locale, {
          year: 'numeric',
          month: '2-digit',
          day: '2-digit',
          hour: '2-digit',
          minute: '2-digit',
          second: '2-digit',
          hour12: false
        })
        break

      case 'date':
        currentTime.value = now.toLocaleDateString(locale, {
          year: 'numeric',
          month: '2-digit',
          day: '2-digit'
        })
        break

      case 'time':
        currentTime.value = now.toLocaleTimeString(locale, {
          hour: '2-digit',
          minute: '2-digit',
          second: '2-digit',
          hour12: false
        })
        break

      case 'datetime':
        currentTime.value = now.toLocaleString(locale, {
          month: '2-digit',
          day: '2-digit',
          hour: '2-digit',
          minute: '2-digit',
          hour12: false
        })
        break
    }

    // 单独设置日期和星期
    currentDate.value = now.toLocaleDateString(locale, {
      year: 'numeric',
      month: 'long',
      day: 'numeric'
    })

    currentWeekday.value = now.toLocaleDateString(locale, {
      weekday: 'long'
    })
  }

  onMounted(() => {
    updateTime()
    intervalId = setInterval(updateTime, updateInterval)
  })

  onUnmounted(() => {
    if (intervalId) {
      clearInterval(intervalId)
      intervalId = null
    }
  })

  return {
    currentTime,
    currentDate,
    currentWeekday
  }
}
