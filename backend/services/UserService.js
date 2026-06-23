const { sequelize } = require('../database/connection');
const { QueryTypes } = require('sequelize');
const bcrypt = require('bcryptjs');

class UserService {
  // 获取所有用户
  async getAll({ role, status, page, limit }) {
    let whereClause = 'WHERE 1=1';
    const params = [];

    if (role) {
      whereClause += ' AND role = ?';
      params.push(role);
    }
    if (status) {
      whereClause += ' AND status = ?';
      params.push(status);
    }

    const offset = (page - 1) * limit;

    // 获取总数
    const countQuery = `SELECT COUNT(*) as total FROM users ${whereClause}`;
    const countResult = await sequelize.query(countQuery, {
      replacements: params,
      type: QueryTypes.SELECT
    });
    const total = countResult[0].total;

    // 获取分页数据（不包含密码）
    const dataQuery = `
      SELECT id, username, email, phone, real_name, role, permissions, avatar, last_login_at, status, created_at, updated_at
      FROM users
      ${whereClause}
      ORDER BY created_at DESC
      LIMIT ? OFFSET ?
    `;
    const users = await sequelize.query(dataQuery, {
      replacements: [...params, parseInt(limit), offset],
      type: QueryTypes.SELECT
    });

    return {
      users,
      pagination: {
        page: parseInt(page),
        limit: parseInt(limit),
        total,
        pages: Math.ceil(total / limit)
      }
    };
  }

  // 获取单个用户
  async getById(id) {
    const query = `
      SELECT id, username, email, phone, real_name, role, permissions, avatar, last_login_at, status, created_at, updated_at
      FROM users
      WHERE id = ?
    `;
    const users = await sequelize.query(query, {
      replacements: [id],
      type: QueryTypes.SELECT
    });
    return users[0] || null;
  }

  // 创建用户
  async create(userData) {
    const { username, password, email, phone, real_name, role, permissions, avatar } = userData;

    // 检查用户名是否已存在
    const existingUser = await this.getByUsername(username);
    if (existingUser) {
      throw new Error('用户名已存在');
    }

    // 加密密码
    const salt = await bcrypt.genSalt(10);
    const passwordHash = await bcrypt.hash(password, salt);

    const query = `
      INSERT INTO users (username, password_hash, email, phone, real_name, role, permissions, avatar)
      VALUES (?, ?, ?, ?, ?, ?, ?, ?)
    `;
    const result = await sequelize.query(query, {
      replacements: [username, passwordHash, email, phone, real_name, role || 'viewer', JSON.stringify(permissions), avatar],
      type: QueryTypes.INSERT
    });

    return this.getById(result[0]);
  }

  // 更新用户
  async update(id, userData) {
    const { email, phone, real_name, role, permissions, avatar, status } = userData;

    const query = `
      UPDATE users
      SET email = ?, phone = ?, real_name = ?, role = ?, permissions = ?, avatar = ?, status = ?
      WHERE id = ?
    `;
    await sequelize.query(query, {
      replacements: [email, phone, real_name, role, JSON.stringify(permissions), avatar, status, id],
      type: QueryTypes.UPDATE
    });

    return this.getById(id);
  }

  // 删除用户
  async delete(id) {
    const query = 'DELETE FROM users WHERE id = ?';
    await sequelize.query(query, {
      replacements: [id],
      type: QueryTypes.DELETE
    });
    return true;
  }

  // 根据用户名获取用户
  async getByUsername(username) {
    const query = 'SELECT * FROM users WHERE username = ?';
    const users = await sequelize.query(query, {
      replacements: [username],
      type: QueryTypes.SELECT
    });
    return users[0] || null;
  }

  // 重置密码
  async resetPassword(id, newPassword) {
    const salt = await bcrypt.genSalt(10);
    const passwordHash = await bcrypt.hash(newPassword, salt);

    const query = 'UPDATE users SET password_hash = ? WHERE id = ?';
    await sequelize.query(query, {
      replacements: [passwordHash, id],
      type: QueryTypes.UPDATE
    });
    return true;
  }

  // 获取用户操作日志
  async getOperationLogs(userId, { page, limit }) {
    const offset = (page - 1) * limit;

    const query = `
      SELECT *
      FROM user_operation_logs
      WHERE user_id = ?
      ORDER BY created_at DESC
      LIMIT ? OFFSET ?
    `;
    return sequelize.query(query, {
      replacements: [userId, parseInt(limit), offset],
      type: QueryTypes.SELECT
    });
  }
}

module.exports = new UserService();
