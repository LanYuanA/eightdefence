const express = require('express');
const router = express.Router();
const AtomicServiceService = require('../services/AtomicServiceService');
const { authenticateToken, requireRole } = require('../middleware/auth');

// 获取所有原子服务
router.get('/', authenticateToken, async (req, res) => {
  try {
    const { type, category, status, page = 1, limit = 50 } = req.query;
    const result = await AtomicServiceService.getAll({ type, category, status, page, limit });
    res.json({ success: true, data: result });
  } catch (error) {
    res.status(500).json({ success: false, message: error.message });
  }
});

// 获取单个原子服务
router.get('/:id', authenticateToken, async (req, res) => {
  try {
    const service = await AtomicServiceService.getById(req.params.id);
    if (!service) {
      return res.status(404).json({ success: false, message: '原子服务不存在' });
    }
    res.json({ success: true, data: service });
  } catch (error) {
    res.status(500).json({ success: false, message: error.message });
  }
});

// 创建原子服务
router.post('/', authenticateToken, requireRole(['admin', 'operator']), async (req, res) => {
  try {
    const service = await AtomicServiceService.create(req.body);
    res.status(201).json({ success: true, data: service });
  } catch (error) {
    res.status(400).json({ success: false, message: error.message });
  }
});

// 更新原子服务
router.put('/:id', authenticateToken, requireRole(['admin', 'operator']), async (req, res) => {
  try {
    const service = await AtomicServiceService.update(req.params.id, req.body);
    res.json({ success: true, data: service });
  } catch (error) {
    res.status(400).json({ success: false, message: error.message });
  }
});

// 删除原子服务
router.delete('/:id', authenticateToken, requireRole(['admin']), async (req, res) => {
  try {
    await AtomicServiceService.delete(req.params.id);
    res.json({ success: true, message: '原子服务已删除' });
  } catch (error) {
    res.status(400).json({ success: false, message: error.message });
  }
});

// 按类型获取原子服务
router.get('/type/:type', authenticateToken, async (req, res) => {
  try {
    const services = await AtomicServiceService.getByType(req.params.type);
    res.json({ success: true, data: services });
  } catch (error) {
    res.status(500).json({ success: false, message: error.message });
  }
});

// 获取原子服务组合
router.get('/:id/compositions', authenticateToken, async (req, res) => {
  try {
    const compositions = await AtomicServiceService.getCompositions(req.params.id);
    res.json({ success: true, data: compositions });
  } catch (error) {
    res.status(500).json({ success: false, message: error.message });
  }
});

// 添加原子服务组合
router.post('/:id/compositions', authenticateToken, requireRole(['admin', 'operator']), async (req, res) => {
  try {
    const composition = await AtomicServiceService.addComposition(req.params.id, req.body);
    res.status(201).json({ success: true, data: composition });
  } catch (error) {
    res.status(400).json({ success: false, message: error.message });
  }
});

// 测试原子服务
router.post('/:id/test', authenticateToken, requireRole(['admin', 'operator']), async (req, res) => {
  try {
    const result = await AtomicServiceService.test(req.params.id, req.body);
    res.json({ success: true, data: result });
  } catch (error) {
    res.status(400).json({ success: false, message: error.message });
  }
});

module.exports = router;
