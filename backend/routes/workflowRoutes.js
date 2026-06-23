const express = require('express');
const router = express.Router();
const WorkflowService = require('../services/WorkflowService');
const { authenticateToken, requireRole } = require('../middleware/auth');

// 获取所有流程
router.get('/', authenticateToken, async (req, res) => {
  try {
    const { status, category, page = 1, limit = 50 } = req.query;
    const result = await WorkflowService.getAll({ status, category, page, limit });
    res.json({ success: true, data: result });
  } catch (error) {
    res.status(500).json({ success: false, message: error.message });
  }
});

// 获取单个流程
router.get('/:id', authenticateToken, async (req, res) => {
  try {
    const workflow = await WorkflowService.getById(req.params.id);
    if (!workflow) {
      return res.status(404).json({ success: false, message: '流程不存在' });
    }
    res.json({ success: true, data: workflow });
  } catch (error) {
    res.status(500).json({ success: false, message: error.message });
  }
});

// 创建流程
router.post('/', authenticateToken, requireRole(['admin', 'operator']), async (req, res) => {
  try {
    const workflow = await WorkflowService.create(req.body);
    res.status(201).json({ success: true, data: workflow });
  } catch (error) {
    res.status(400).json({ success: false, message: error.message });
  }
});

// 更新流程
router.put('/:id', authenticateToken, requireRole(['admin', 'operator']), async (req, res) => {
  try {
    const workflow = await WorkflowService.update(req.params.id, req.body);
    res.json({ success: true, data: workflow });
  } catch (error) {
    res.status(400).json({ success: false, message: error.message });
  }
});

// 删除流程
router.delete('/:id', authenticateToken, requireRole(['admin']), async (req, res) => {
  try {
    await WorkflowService.delete(req.params.id);
    res.json({ success: true, message: '流程已删除' });
  } catch (error) {
    res.status(400).json({ success: false, message: error.message });
  }
});

// 执行流程
router.post('/:id/execute', authenticateToken, requireRole(['admin', 'operator']), async (req, res) => {
  try {
    const execution = await WorkflowService.execute(req.params.id, req.body);
    res.json({ success: true, data: execution });
  } catch (error) {
    res.status(400).json({ success: false, message: error.message });
  }
});

// 获取流程执行历史
router.get('/:id/executions', authenticateToken, async (req, res) => {
  try {
    const { page = 1, limit = 20 } = req.query;
    const executions = await WorkflowService.getExecutions(req.params.id, { page, limit });
    res.json({ success: true, data: executions });
  } catch (error) {
    res.status(500).json({ success: false, message: error.message });
  }
});

// 激活/停用流程
router.post('/:id/toggle', authenticateToken, requireRole(['admin', 'operator']), async (req, res) => {
  try {
    const workflow = await WorkflowService.toggleStatus(req.params.id);
    res.json({ success: true, data: workflow });
  } catch (error) {
    res.status(400).json({ success: false, message: error.message });
  }
});

// 复制流程
router.post('/:id/duplicate', authenticateToken, requireRole(['admin', 'operator']), async (req, res) => {
  try {
    const newWorkflow = await WorkflowService.duplicate(req.params.id);
    res.status(201).json({ success: true, data: newWorkflow });
  } catch (error) {
    res.status(400).json({ success: false, message: error.message });
  }
});

module.exports = router;
