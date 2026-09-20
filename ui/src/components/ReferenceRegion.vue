<script setup lang="ts">
import reference from '../assets/software-platform-reference.png'

// Original artwork at native resolution, reused as individual component skins.
defineProps<{ x: number; y: number; width: number; height: number }>()
</script>

<template>
  <span aria-hidden="true" :style="{
    display: 'block', flex: 'none', width: `${width}px`, height: `${height}px`,
    backgroundImage: `url(${reference})`, backgroundSize: '1678px 937px',
    backgroundPosition: `-${x}px -${y}px`, backgroundRepeat: 'no-repeat',
    pointerEvents: 'none',
  }"></span>
</template>
