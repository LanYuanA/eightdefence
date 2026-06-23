const express = require('express');
const router = express.Router();
const ResourcePoolService = require('../services/ResourcePoolService');
const { authenticateToken, requireRole } = require('../middleware/auth');

// 获取所有资源
router.get('/', authenticateToken, async (req, res) => {
  try {
    const { type, category, is_active, page = 1, limit = 50 } = req.query;
    const result = await ResourcePoolService.getAll({ type, category, is_active, page, limit });
    res.json({ success: true, data: result });
  } catch (error) {
    res.status(500).json({ success: false, message: error.message });
  }
});

// 获取单个资源
router.get('/:id', authenticateToken, async (req, res) => {
  try {
    const resource = await ResourcePoolService.getById(req.params.id);
    if (!resource) {
      return res.status(404).json({ success: false, message: '资源不存在' });
    }
    res.json({ success: true, data: resource });
  } catch (error) {
    res.status(500).json({ success: false, message: error.message });
  }
});

// 创建资源
router.post('/', authenticateToken, requireRole(['admin', 'operator']), async (req, res) => {
  try {
    const resource = await ResourcePoolService.create(req.body);
    res.status(201).json({ success: true, data: resource });
  } catch (error) {
    res.status(400).json({ success: false, message: error.message });
  }
});

// 更新资源
router.put('/:id', authenticateToken, requireRole(['admin', 'operator']), async (req, res) => {
  try {
    const resource = await ResourcePoolService.update(req.params.id, req.body);
    res.json({ success: true, data: resource });
  } catch (error) {
    res.status(400).json({ success: false, message: error.message });
  }
});

// 删除资源
router.delete('/:id', authenticateToken, requireRole(['admin']), async (req, res) => {
  try {
    await ResourcePoolService.delete(req.params.id);
    res.json({ success: true, message: '资源已删除' });
  } catch (error) {
    res.status(400).json({ success: false, message: error.message });
  }
});

// 按类型获取资源
router.get('/type/:type', authenticateToken, async (req, res) => {
  try {
    const resources = await ResourcePoolService.getByType(req.params.type);
    res.json({ success: true, data: resources });
  } catch (error) {
    res.status(500).json({ success: false, message: error.message });
  }
});

// 获取资源绑定
router.get('/:id/bindings', authenticateToken, async (req, res) => {
  try {
    const bindings = await ResourcePoolService.getBindings(req.params.id);
    res.json({ success: true, data: bindings });
  } catch (error) {
    res.status(500).json({ success: false, message: error.message });
  }
});

// 添加资源绑定
router.post('/:id/bindings', authenticateToken, requireRole(['admin', 'operator']), async (req, res) => {
  try {
    const binding = await ResourcePoolService.addBinding(req.params.id, req.body);
    res.status(201).json({ success: true, data: binding });
  } catch (error) {
    res.status(400).json({ success: false, message: error.message });
  }
});

// 删除资源绑定
router.delete('/bindings/:bindingId', authenticateToken, requireRole(['admin', 'operator']), async (req, res) => {
  try {
    await ResourcePoolService.removeBinding(req.params.bindingId);
    res.json({ success: true, message: '绑定已删除' });
  } catch (error) {
    res.status(400).json({ success: false, message: error.message });
  }
});

// 搜索资源
router.search('/search', authenticateToken, async (req, res) => {
  try {
    const { q, type, category } = req.query;
    const resources = await ResourcePoolService.search({ q, type, category });
    res.json({ success: true, data: resources });
  } catch (error) {
    res.status(500).json({ success: false, message: error.message });
  }
});

module.exports = router;
