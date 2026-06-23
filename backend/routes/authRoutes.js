const express = require('express');
const router = express.Router();
const bcrypt = require('bcryptjs');
const { sequelize } = require('../database/connection');
const { QueryTypes } = require('sequelize');
const { generateToken } = require('../middleware/auth');

// 用户登录
router.post('/login', async (req, res) => {
  try {
    const { username, password } = req.body;

    if (!username || !password) {
      return res.status(400).json({
        success: false,
        message: '用户名和密码不能为空'
      });
    }

    // 查找用户
    const query = 'SELECT * FROM users WHERE username = ? AND status = ?';
    const users = await sequelize.query(query, {
      replacements: [username, 'active'],
      type: QueryTypes.SELECT
    });

    if (users.length === 0) {
      return res.status(401).json({
        success: false,
        message: '用户名或密码错误'
      });
    }

    const user = users[0];

    // 验证密码
    const isValidPassword = await bcrypt.compare(password, user.password_hash);
    if (!isValidPassword) {
      return res.status(401).json({
        success: false,
        message: '用户名或密码错误'
      });
    }

    // 更新最后登录时间
    const updateQuery = 'UPDATE users SET last_login_at = NOW() WHERE id = ?';
    await sequelize.query(updateQuery, {
      replacements: [user.id],
      type: QueryTypes.UPDATE
    });

    // 生成令牌
    const token = generateToken(user);

    res.json({
      success: true,
      data: {
        token,
        user: {
          id: user.id,
          username: user.username,
          email: user.email,
          real_name: user.real_name,
          role: user.role,
          avatar: user.avatar
        }
      }
    });
  } catch (error) {
    console.error('登录错误:', error);
    res.status(500).json({
      success: false,
      message: '服务器内部错误'
    });
  }
});

// 获取当前用户信息
router.get('/me', async (req, res) => {
  try {
    const authHeader = req.headers['authorization'];
    const token = authHeader && authHeader.split(' ')[1];

    if (!token) {
      return res.status(401).json({
        success: false,
        message: '未提供认证令牌'
      });
    }

    const { verifyToken } = require('../middleware/auth');
    const decoded = verifyToken(token);

    if (!decoded) {
      return res.status(403).json({
        success: false,
        message: '令牌无效'
      });
    }

    const query = 'SELECT id, username, email, real_name, role, avatar, created_at FROM users WHERE id = ?';
    const users = await sequelize.query(query, {
      replacements: [decoded.id],
      type: QueryTypes.SELECT
    });

    if (users.length === 0) {
      return res.status(404).json({
        success: false,
        message: '用户不存在'
      });
    }

    res.json({
      success: true,
      data: users[0]
    });
  } catch (error) {
    res.status(500).json({
      success: false,
      message: '服务器内部错误'
    });
  }
});

// 修改密码
router.post('/change-password', async (req, res) => {
  try {
    const authHeader = req.headers['authorization'];
    const token = authHeader && authHeader.split(' ')[1];

    if (!token) {
      return res.status(401).json({
        success: false,
        message: '未提供认证令牌'
      });
    }

    const { verifyToken } = require('../middleware/auth');
    const decoded = verifyToken(token);

    if (!decoded) {
      return res.status(403).json({
        success: false,
        message: '令牌无效'
      });
    }

    const { oldPassword, newPassword } = req.body;

    if (!oldPassword || !newPassword) {
      return res.status(400).json({
        success: false,
        message: '旧密码和新密码不能为空'
      });
    }

    // 获取用户当前密码
    const query = 'SELECT password_hash FROM users WHERE id = ?';
    const users = await sequelize.query(query, {
      replacements: [decoded.id],
      type: QueryTypes.SELECT
    });

    if (users.length === 0) {
      return res.status(404).json({
        success: false,
        message: '用户不存在'
      });
    }

    // 验证旧密码
    const isValidPassword = await bcrypt.compare(oldPassword, users[0].password_hash);
    if (!isValidPassword) {
      return res.status(401).json({
        success: false,
        message: '旧密码错误'
      });
    }

    // 加密新密码
    const salt = await bcrypt.genSalt(10);
    const newPasswordHash = await bcrypt.hash(newPassword, salt);

    // 更新密码
    const updateQuery = 'UPDATE users SET password_hash = ? WHERE id = ?';
    await sequelize.query(updateQuery, {
      replacements: [newPasswordHash, decoded.id],
      type: QueryTypes.UPDATE
    });

    res.json({
      success: true,
      message: '密码修改成功'
    });
  } catch (error) {
    res.status(500).json({
      success: false,
      message: '服务器内部错误'
    });
  }
});

module.exports = router;
