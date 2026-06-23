<template>
  <div class="device-node" :style="{ borderColor: data.color }">
    <!-- 输入端口 -->
    <div v-if="data.inputs && data.inputs.length > 0" class="node-inputs">
      <Handle
        v-for="(_input, index) in data.inputs"
        :key="`input-${index}`"
        type="target"
        :position="Position.Left"
        :id="`input-${index}`"
        :style="{ top: `${((index + 1) / (data.inputs.length + 1)) * 100}%` }"
      />
    </div>

    <!-- 节点内容 -->
    <div class="node-content" :style="{ background: data.color + '20' }">
      <span class="node-icon">{{ data.icon }}</span>
      <span class="node-label">{{ data.label }}</span>
    </div>

    <!-- 输出端口 -->
    <div v-if="data.outputs && data.outputs.length > 0" class="node-outputs">
      <Handle
        v-for="(_output, index) in data.outputs"
        :key="`output-${index}`"
        type="source"
        :position="Position.Right"
        :id="`output-${index}`"
        :style="{ top: `${((index + 1) / (data.outputs.length + 1)) * 100}%` }"
      />
    </div>
  </div>
</template>

<script setup lang="ts">
import { Handle, Position } from '@vue-flow/core'

defineProps<{
  data: {
    label: string
    icon: string
    color: string
    inputs: string[]
    outputs: string[]
    config?: any
  }
}>()
</script>

<style scoped>
.device-node {
  background: var(--bg-card);
  border: 2px solid;
  border-radius: var(--radius-md);
  min-width: 120px;
  position: relative;
}

.node-inputs,
.node-outputs {
  position: absolute;
  top: 0;
  bottom: 0;
  width: 10px;
}

.node-inputs {
  left: -10px;
}

.node-outputs {
  right: -10px;
}

.node-content {
  display: flex;
  align-items: center;
  gap: 8px;
  padding: 12px 16px;
  border-radius: var(--radius-sm);
}

.node-icon {
  font-size: 20px;
}

.node-label {
  font-size: 13px;
  font-weight: 600;
  color: var(--text-primary);
}
</style>
