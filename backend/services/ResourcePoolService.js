const { sequelize } = require('../database/connection');
const { QueryTypes } = require('sequelize');

class ResourcePoolService {
  // 获取所有资源
  async getAll({ type, category, is_active, page, limit }) {
    let whereClause = 'WHERE 1=1';
    const params = [];

    if (type) {
      whereClause += ' AND rp.type = ?';
      params.push(type);
    }
    if (category) {
      whereClause += ' AND rp.category = ?';
      params.push(category);
    }
    if (is_active !== undefined) {
      whereClause += ' AND rp.is_active = ?';
      params.push(is_active === 'true');
    }

    const offset = (page - 1) * limit;

    // 获取总数
    const countQuery = `SELECT COUNT(*) as total FROM resource_pool rp ${whereClause}`;
    const countResult = await sequelize.query(countQuery, {
      replacements: params,
      type: QueryTypes.SELECT
    });
    const total = countResult[0].total;

    // 获取分页数据
    const dataQuery = `
      SELECT rp.*, u.username as created_by_name
      FROM resource_pool rp
      LEFT JOIN users u ON rp.created_by = u.id
      ${whereClause}
      ORDER BY rp.created_at DESC
      LIMIT ? OFFSET ?
    `;
    const resources = await sequelize.query(dataQuery, {
      replacements: [...params, parseInt(limit), offset],
      type: QueryTypes.SELECT
    });

    return {
      resources,
      pagination: {
        page: parseInt(page),
        limit: parseInt(limit),
        total,
        pages: Math.ceil(total / limit)
      }
    };
  }

  // 获取单个资源
  async getById(id) {
    const query = `
      SELECT rp.*, u.username as created_by_name
      FROM resource_pool rp
      LEFT JOIN users u ON rp.created_by = u.id
      WHERE rp.id = ?
    `;
    const resources = await sequelize.query(query, {
      replacements: [id],
      type: QueryTypes.SELECT
    });
    return resources[0] || null;
  }

  // 创建资源
  async create(resourceData) {
    const { name, type, resource_id, resource_data, category, tags, description, is_active, created_by } = resourceData;

    const query = `
      INSERT INTO resource_pool (name, type, resource_id, resource_data, category, tags, description, is_active, created_by)
      VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)
    `;
    const result = await sequelize.query(query, {
      replacements: [name, type, resource_id, JSON.stringify(resource_data), category, JSON.stringify(tags), description, is_active !== false, created_by],
      type: QueryTypes.INSERT
    });

    return this.getById(result[0]);
  }

  // 更新资源
  async update(id, resourceData) {
    const { name, type, resource_id, resource_data, category, tags, description, is_active } = resourceData;

    const query = `
      UPDATE resource_pool
      SET name = ?, type = ?, resource_id = ?, resource_data = ?, category = ?, tags = ?, description = ?, is_active = ?
      WHERE id = ?
    `;
    await sequelize.query(query, {
      replacements: [name, type, resource_id, JSON.stringify(resource_data), category, JSON.stringify(tags), description, is_active, id],
      type: QueryTypes.UPDATE
    });

    return this.getById(id);
  }

  // 删除资源
  async delete(id) {
    const query = 'DELETE FROM resource_pool WHERE id = ?';
    await sequelize.query(query, {
      replacements: [id],
      type: QueryTypes.DELETE
    });
    return true;
  }

  // 按类型获取资源
  async getByType(type) {
    const query = 'SELECT * FROM resource_pool WHERE type = ? AND is_active = TRUE ORDER BY name';
    return sequelize.query(query, {
      replacements: [type],
      type: QueryTypes.SELECT
    });
  }

  // 获取资源绑定
  async getBindings(resourceId) {
    const query = 'SELECT * FROM resource_bindings WHERE resource_id = ?';
    return sequelize.query(query, {
      replacements: [resourceId],
      type: QueryTypes.SELECT
    });
  }

  // 添加资源绑定
  async addBinding(resourceId, bindingData) {
    const { component_id, component_type, binding_config } = bindingData;

    const query = `
      INSERT INTO resource_bindings (resource_id, component_id, component_type, binding_config)
      VALUES (?, ?, ?, ?)
    `;
    const result = await sequelize.query(query, {
      replacements: [resourceId, component_id, component_type, JSON.stringify(binding_config)],
      type: QueryTypes.INSERT
    });

    const selectQuery = 'SELECT * FROM resource_bindings WHERE id = ?';
    const bindings = await sequelize.query(selectQuery, {
      replacements: [result[0]],
      type: QueryTypes.SELECT
    });
    return bindings[0];
  }

  // 删除资源绑定
  async removeBinding(bindingId) {
    const query = 'DELETE FROM resource_bindings WHERE id = ?';
    await sequelize.query(query, {
      replacements: [bindingId],
      type: QueryTypes.DELETE
    });
    return true;
  }

  // 搜索资源
  async search({ q, type, category }) {
    let whereClause = 'WHERE is_active = TRUE';
    const params = [];

    if (q) {
      whereClause += ' AND (name LIKE ? OR description LIKE ?)';
      params.push(`%${q}%`, `%${q}%`);
    }
    if (type) {
      whereClause += ' AND type = ?';
      params.push(type);
    }
    if (category) {
      whereClause += ' AND category = ?';
      params.push(category);
    }

    const query = `SELECT * FROM resource_pool ${whereClause} ORDER BY name LIMIT 100`;
    return sequelize.query(query, {
      replacements: params,
      type: QueryTypes.SELECT
    });
  }
}

module.exports = new ResourcePoolService();
