<template>
  <div class="node-config" v-if="node">
    <div class="config-header">
      <div class="config-title">
        <span class="config-icon">{{ node.data.icon }}</span>
        <span>{{ node.data.label }}</span>
      </div>
      <button class="config-close" @click="$emit('close')">×</button>
    </div>

    <div class="config-content">
      <!-- 基本信息 -->
      <div class="config-section">
        <div class="section-title">基本信息</div>
        <div class="config-item">
          <label class="config-label">节点ID</label>
          <div class="config-value">{{ node.id }}</div>
        </div>
        <div class="config-item">
          <label class="config-label">节点类型</label>
          <div class="config-value">{{ node.type }}</div>
        </div>
      </div>

      <!-- 输入端口 -->
      <div v-if="node.data.inputs && node.data.inputs.length > 0" class="config-section">
        <div class="section-title">输入端口</div>
        <div v-for="(input, index) in node.data.inputs" :key="`input-${index}`" class="config-item">
          <label class="config-label">{{ input }}</label>
          <div class="config-value port-value">
            <span class="port-dot target"></span>
            输入 {{ index + 1 }}
          </div>
        </div>
      </div>

      <!-- 输出端口 -->
      <div v-if="node.data.outputs && node.data.outputs.length > 0" class="config-section">
        <div class="section-title">输出端口</div>
        <div v-for="(output, index) in node.data.outputs" :key="`output-${index}`" class="config-item">
          <label class="config-label">{{ output }}</label>
          <div class="config-value port-value">
            <span class="port-dot source"></span>
            输出 {{ index + 1 }}
          </div>
        </div>
      </div>

      <!-- 节点配置 -->
      <div class="config-section">
        <div class="section-title">节点配置</div>
        <div class="config-item">
          <label class="config-label">自定义名称</label>
          <input
            v-model="customLabel"
            type="text"
            class="config-input"
            placeholder="输入自定义名称"
            @change="updateLabel"
          />
        </div>
        <div class="config-item">
          <label class="config-label">备注</label>
          <textarea
            v-model="remark"
            class="config-textarea"
            placeholder="输入备注信息"
            rows="3"
          />
        </div>
      </div>

      <!-- 操作按钮 -->
      <div class="config-actions">
        <CyberButton variant="primary" size="sm" @click="saveConfig">保存配置</CyberButton>
        <CyberButton variant="danger" size="sm" @click="deleteNode">删除节点</CyberButton>
      </div>
    </div>
  </div>
</template>

<script setup lang="ts">
import { ref, watch } from 'vue'
import CyberButton from '../CyberButton.vue'

interface FlowNode {
  id: string
  type: string
  position: { x: number; y: number }
  data: {
    label: string
    icon: string
    color: string
    inputs: string[]
    outputs: string[]
    config?: any
  }
}

const props = defineProps<{
  node: FlowNode | null
}>()

const emit = defineEmits<{
  close: []
  save: [node: FlowNode]
  delete: [nodeId: string]
}>()

const customLabel = ref('')
const remark = ref('')

// 监听节点变化
watch(() => props.node, (newNode) => {
  if (newNode) {
    customLabel.value = newNode.data.label || ''
    remark.value = newNode.data.config?.remark || ''
  }
}, { immediate: true })

// 更新标签
function updateLabel() {
  if (props.node) {
    props.node.data.label = customLabel.value
  }
}

// 保存配置
function saveConfig() {
  if (props.node) {
    props.node.data.config = {
      ...props.node.data.config,
      remark: remark.value
    }
    emit('save', props.node)
  }
}

// 删除节点
function deleteNode() {
  if (props.node) {
    emit('delete', props.node.id)
  }
}
</script>

<style scoped>
.node-config {
  width: 300px;
  background: var(--bg-card);
  border-left: 1px solid var(--border-primary);
  display: flex;
  flex-direction: column;
  height: 100%;
}

.config-header {
  display: flex;
  align-items: center;
  justify-content: space-between;
  padding: var(--spacing-md);
  border-bottom: 1px solid var(--border-primary);
}

.config-title {
  display: flex;
  align-items: center;
  gap: 8px;
  font-size: 14px;
  font-weight: 600;
  color: var(--text-primary);
}

.config-icon {
  font-size: 20px;
}

.config-close {
  width: 28px;
  height: 28px;
  display: flex;
  align-items: center;
  justify-content: center;
  background: transparent;
  border: none;
  color: var(--text-secondary);
  font-size: 18px;
  cursor: pointer;
  border-radius: var(--radius-sm);
  transition: all var(--duration-fast);
}

.config-close:hover {
  background: rgba(255, 255, 255, 0.1);
  color: var(--text-primary);
}

.config-content {
  flex: 1;
  overflow-y: auto;
  padding: var(--spacing-md);
}

.config-section {
  margin-bottom: var(--spacing-lg);
}

.section-title {
  font-size: 12px;
  font-weight: 600;
  color: var(--text-secondary);
  text-transform: uppercase;
  letter-spacing: 0.5px;
  margin-bottom: var(--spacing-sm);
  padding-bottom: var(--spacing-xs);
  border-bottom: 1px solid var(--border-primary);
}

.config-item {
  display: flex;
  flex-direction: column;
  gap: 4px;
  margin-bottom: var(--spacing-sm);
}

.config-label {
  font-size: 12px;
  color: var(--text-tertiary);
}

.config-value {
  font-size: 13px;
  color: var(--text-primary);
  font-family: var(--font-mono);
}

.port-value {
  display: flex;
  align-items: center;
  gap: 6px;
}

.port-dot {
  width: 8px;
  height: 8px;
  border-radius: 50%;
}

.port-dot.target {
  background: var(--accent-primary);
}

.port-dot.source {
  background: var(--status-success);
}

.config-input {
  width: 100%;
  padding: 8px 10px;
  background: var(--bg-primary);
  border: 1px solid var(--border-primary);
  border-radius: var(--radius-sm);
  color: var(--text-primary);
  font-size: 13px;
  outline: none;
  transition: border-color var(--duration-fast);
}

.config-input:focus {
  border-color: var(--accent-primary);
}

.config-textarea {
  width: 100%;
  padding: 8px 10px;
  background: var(--bg-primary);
  border: 1px solid var(--border-primary);
  border-radius: var(--radius-sm);
  color: var(--text-primary);
  font-size: 13px;
  outline: none;
  resize: vertical;
  transition: border-color var(--duration-fast);
}

.config-textarea:focus {
  border-color: var(--accent-primary);
}

.config-actions {
  display: flex;
  gap: var(--spacing-sm);
  padding-top: var(--spacing-md);
  border-top: 1px solid var(--border-primary);
}
</style>
