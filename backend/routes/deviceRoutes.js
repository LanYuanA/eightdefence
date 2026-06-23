const express = require('express');
const router = express.Router();
const DeviceService = require('../services/DeviceService');
const { authenticateToken, requireRole } = require('../middleware/auth');

// 获取所有设备
router.get('/', authenticateToken, async (req, res) => {
  try {
    const { layer, type, status, page = 1, limit = 50 } = req.query;
    const result = await DeviceService.getAll({ layer, type, status, page, limit });
    res.json({ success: true, data: result });
  } catch (error) {
    res.status(500).json({ success: false, message: error.message });
  }
});

// 获取单个设备
router.get('/:id', authenticateToken, async (req, res) => {
  try {
    const device = await DeviceService.getById(req.params.id);
    if (!device) {
      return res.status(404).json({ success: false, message: '设备不存在' });
    }
    res.json({ success: true, data: device });
  } catch (error) {
    res.status(500).json({ success: false, message: error.message });
  }
});

// 创建设备
router.post('/', authenticateToken, requireRole(['admin', 'operator']), async (req, res) => {
  try {
    const device = await DeviceService.create(req.body);
    res.status(201).json({ success: true, data: device });
  } catch (error) {
    res.status(400).json({ success: false, message: error.message });
  }
});

// 更新设备
router.put('/:id', authenticateToken, requireRole(['admin', 'operator']), async (req, res) => {
  try {
    const device = await DeviceService.update(req.params.id, req.body);
    res.json({ success: true, data: device });
  } catch (error) {
    res.status(400).json({ success: false, message: error.message });
  }
});

// 删除设备
router.delete('/:id', authenticateToken, requireRole(['admin']), async (req, res) => {
  try {
    await DeviceService.delete(req.params.id);
    res.json({ success: true, message: '设备已删除' });
  } catch (error) {
    res.status(400).json({ success: false, message: error.message });
  }
});

// 获取设备寄存器配置
router.get('/:id/registers', authenticateToken, async (req, res) => {
  try {
    const registers = await DeviceService.getRegisters(req.params.id);
    res.json({ success: true, data: registers });
  } catch (error) {
    res.status(500).json({ success: false, message: error.message });
  }
});

// 添加设备寄存器配置
router.post('/:id/registers', authenticateToken, requireRole(['admin', 'operator']), async (req, res) => {
  try {
    const register = await DeviceService.addRegister(req.params.id, req.body);
    res.status(201).json({ success: true, data: register });
  } catch (error) {
    res.status(400).json({ success: false, message: error.message });
  }
});

// 获取设备阈值配置
router.get('/:id/thresholds', authenticateToken, async (req, res) => {
  try {
    const thresholds = await DeviceService.getThresholds(req.params.id);
    res.json({ success: true, data: thresholds });
  } catch (error) {
    res.status(500).json({ success: false, message: error.message });
  }
});

// 添加设备阈值配置
router.post('/:id/thresholds', authenticateToken, requireRole(['admin', 'operator']), async (req, res) => {
  try {
    const threshold = await DeviceService.addThreshold(req.params.id, req.body);
    res.status(201).json({ success: true, data: threshold });
  } catch (error) {
    res.status(400).json({ success: false, message: error.message });
  }
});

// 按层级获取设备
router.get('/layer/:layer', authenticateToken, async (req, res) => {
  try {
    const devices = await DeviceService.getByLayer(req.params.layer);
    res.json({ success: true, data: devices });
  } catch (error) {
    res.status(500).json({ success: false, message: error.message });
  }
});

// 批量更新设备状态
router.post('/batch/status', authenticateToken, requireRole(['admin', 'operator']), async (req, res) => {
  try {
    const { deviceIds, status } = req.body;
    await DeviceService.batchUpdateStatus(deviceIds, status);
    res.json({ success: true, message: '批量更新成功' });
  } catch (error) {
    res.status(400).json({ success: false, message: error.message });
  }
});

module.exports = router;
