const express = require('express');
const router = express.Router();
const UserService = require('../services/UserService');
const { authenticateToken, requireRole } = require('../middleware/auth');

// 获取所有用户
router.get('/', authenticateToken, requireRole(['admin']), async (req, res) => {
  try {
    const { role, status, page = 1, limit = 50 } = req.query;
    const result = await UserService.getAll({ role, status, page, limit });
    res.json({ success: true, data: result });
  } catch (error) {
    res.status(500).json({ success: false, message: error.message });
  }
});

// 获取单个用户
router.get('/:id', authenticateToken, requireRole(['admin']), async (req, res) => {
  try {
    const user = await UserService.getById(req.params.id);
    if (!user) {
      return res.status(404).json({ success: false, message: '用户不存在' });
    }
    res.json({ success: true, data: user });
  } catch (error) {
    res.status(500).json({ success: false, message: error.message });
  }
});

// 创建用户
router.post('/', authenticateToken, requireRole(['admin']), async (req, res) => {
  try {
    const user = await UserService.create(req.body);
    res.status(201).json({ success: true, data: user });
  } catch (error) {
    res.status(400).json({ success: false, message: error.message });
  }
});

// 更新用户
router.put('/:id', authenticateToken, requireRole(['admin']), async (req, res) => {
  try {
    const user = await UserService.update(req.params.id, req.body);
    res.json({ success: true, data: user });
  } catch (error) {
    res.status(400).json({ success: false, message: error.message });
  }
});

// 删除用户
router.delete('/:id', authenticateToken, requireRole(['admin']), async (req, res) => {
  try {
    await UserService.delete(req.params.id);
    res.json({ success: true, message: '用户已删除' });
  } catch (error) {
    res.status(400).json({ success: false, message: error.message });
  }
});

// 重置用户密码
router.post('/:id/reset-password', authenticateToken, requireRole(['admin']), async (req, res) => {
  try {
    const { newPassword } = req.body;
    await UserService.resetPassword(req.params.id, newPassword);
    res.json({ success: true, message: '密码已重置' });
  } catch (error) {
    res.status(400).json({ success: false, message: error.message });
  }
});

// 获取用户操作日志
router.get('/:id/logs', authenticateToken, requireRole(['admin']), async (req, res) => {
  try {
    const { page = 1, limit = 50 } = req.query;
    const logs = await UserService.getOperationLogs(req.params.id, { page, limit });
    res.json({ success: true, data: logs });
  } catch (error) {
    res.status(500).json({ success: false, message: error.message });
  }
});

module.exports = router;
