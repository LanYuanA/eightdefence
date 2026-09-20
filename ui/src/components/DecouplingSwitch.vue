<script setup lang="ts">
defineProps<{ disabled?: boolean }>()
</script>

<template>
  <Teleport to="body">
    <nav class="decoupling-switch" aria-label="解耦演示切换" :inert="disabled">
      <RouterLink to="/software-defined-platform">软软解耦</RouterLink>
      <span>·</span>
      <RouterLink to="/hardware-decoupling">软硬解耦</RouterLink>
    </nav>
  </Teleport>
</template>

<style scoped>
.decoupling-switch { position: fixed; right: 8px; bottom: 4px; z-index: 50; display: flex; align-items: center; gap: 5px; padding: 3px 7px; border: 1px solid #36cee536; border-radius: 5px; background: #00142cdd; font: 11px/1.4 'Microsoft YaHei', sans-serif; }
a { color: #a3c5d6; text-decoration: none; }
a.router-link-active { color: #51e9ef; }
a:hover, a:focus-visible { color: white; }
span { color: #527887; }
nav[inert] { opacity: .45; }
</style>
