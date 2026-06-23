const { sequelize } = require('../database/connection');
const { QueryTypes } = require('sequelize');

class DeviceService {
  // 获取所有设备
  async getAll({ layer, type, status, page, limit }) {
    let whereClause = 'WHERE 1=1';
    const params = [];

    if (layer) {
      whereClause += ' AND d.layer = ?';
      params.push(layer);
    }
    if (type) {
      whereClause += ' AND d.type = ?';
      params.push(type);
    }
    if (status) {
      whereClause += ' AND d.status = ?';
      params.push(status);
    }

    const offset = (page - 1) * limit;

    // 获取总数
    const countQuery = `SELECT COUNT(*) as total FROM devices d ${whereClause}`;
    const countResult = await sequelize.query(countQuery, {
      replacements: params,
      type: QueryTypes.SELECT
    });
    const total = countResult[0].total;

    // 获取分页数据
    const dataQuery = `
      SELECT d.*, u.username as created_by_name
      FROM devices d
      LEFT JOIN users u ON d.created_by = u.id
      ${whereClause}
      ORDER BY d.created_at DESC
      LIMIT ? OFFSET ?
    `;
    const devices = await sequelize.query(dataQuery, {
      replacements: [...params, parseInt(limit), offset],
      type: QueryTypes.SELECT
    });

    return {
      devices,
      pagination: {
        page: parseInt(page),
        limit: parseInt(limit),
        total,
        pages: Math.ceil(total / limit)
      }
    };
  }

  // 获取单个设备
  async getById(id) {
    const query = `
      SELECT d.*, u.username as created_by_name
      FROM devices d
      LEFT JOIN users u ON d.created_by = u.id
      WHERE d.id = ?
    `;
    const devices = await sequelize.query(query, {
      replacements: [id],
      type: QueryTypes.SELECT
    });
    return devices[0] || null;
  }

  // 创建设备
  async create(deviceData) {
    const { name, type, model, address, description, layer, layer_name, config, created_by } = deviceData;

    const query = `
      INSERT INTO devices (name, type, model, address, description, layer, layer_name, config, created_by)
      VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)
    `;
    const result = await sequelize.query(query, {
      replacements: [name, type, model, address, description, layer, layer_name, JSON.stringify(config), created_by],
      type: QueryTypes.INSERT
    });

    return this.getById(result[0]);
  }

  // 更新设备
  async update(id, deviceData) {
    const { name, type, model, address, description, layer, layer_name, config, status } = deviceData;

    const query = `
      UPDATE devices
      SET name = ?, type = ?, model = ?, address = ?, description = ?, layer = ?, layer_name = ?, config = ?, status = ?
      WHERE id = ?
    `;
    await sequelize.query(query, {
      replacements: [name, type, model, address, description, layer, layer_name, JSON.stringify(config), status, id],
      type: QueryTypes.UPDATE
    });

    return this.getById(id);
  }

  // 删除设备
  async delete(id) {
    const query = 'DELETE FROM devices WHERE id = ?';
    await sequelize.query(query, {
      replacements: [id],
      type: QueryTypes.DELETE
    });
    return true;
  }

  // 获取设备寄存器配置
  async getRegisters(deviceId) {
    const query = 'SELECT * FROM device_registers WHERE device_id = ? ORDER BY register_address';
    return sequelize.query(query, {
      replacements: [deviceId],
      type: QueryTypes.SELECT
    });
  }

  // 添加设备寄存器配置
  async addRegister(deviceId, registerData) {
    const { register_name, register_address, data_type, scale_factor, offset_value, unit, description, is_readonly } = registerData;

    const query = `
      INSERT INTO device_registers (device_id, register_name, register_address, data_type, scale_factor, offset_value, unit, description, is_readonly)
      VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)
    `;
    const result = await sequelize.query(query, {
      replacements: [deviceId, register_name, register_address, data_type, scale_factor, offset_value, unit, description, is_readonly],
      type: QueryTypes.INSERT
    });

    const selectQuery = 'SELECT * FROM device_registers WHERE id = ?';
    const registers = await sequelize.query(selectQuery, {
      replacements: [result[0]],
      type: QueryTypes.SELECT
    });
    return registers[0];
  }

  // 获取设备阈值配置
  async getThresholds(deviceId) {
    const query = `
      SELECT dt.*, dr.register_name, dr.register_address
      FROM device_thresholds dt
      JOIN device_registers dr ON dt.register_id = dr.id
      WHERE dt.device_id = ?
    `;
    return sequelize.query(query, {
      replacements: [deviceId],
      type: QueryTypes.SELECT
    });
  }

  // 添加设备阈值配置
  async addThreshold(deviceId, thresholdData) {
    const { register_id, min_value, max_value, alarm_level, description } = thresholdData;

    const query = `
      INSERT INTO device_thresholds (device_id, register_id, min_value, max_value, alarm_level, description)
      VALUES (?, ?, ?, ?, ?, ?)
    `;
    const result = await sequelize.query(query, {
      replacements: [deviceId, register_id, min_value, max_value, alarm_level, description],
      type: QueryTypes.INSERT
    });

    const selectQuery = 'SELECT * FROM device_thresholds WHERE id = ?';
    const thresholds = await sequelize.query(selectQuery, {
      replacements: [result[0]],
      type: QueryTypes.SELECT
    });
    return thresholds[0];
  }

  // 按层级获取设备
  async getByLayer(layer) {
    const query = 'SELECT * FROM devices WHERE layer = ? ORDER BY name';
    return sequelize.query(query, {
      replacements: [layer],
      type: QueryTypes.SELECT
    });
  }

  // 批量更新设备状态
  async batchUpdateStatus(deviceIds, status) {
    const query = 'UPDATE devices SET status = ? WHERE id IN (?)';
    await sequelize.query(query, {
      replacements: [status, deviceIds],
      type: QueryTypes.UPDATE
    });
    return true;
  }
}

module.exports = new DeviceService();
