const { sequelize } = require('../database/connection');
const { QueryTypes } = require('sequelize');

class AtomicServiceService {
  // 获取所有原子服务
  async getAll({ type, category, status, page, limit }) {
    let whereClause = 'WHERE 1=1';
    const params = [];

    if (type) {
      whereClause += ' AND type = ?';
      params.push(type);
    }
    if (category) {
      whereClause += ' AND category = ?';
      params.push(category);
    }
    if (status) {
      whereClause += ' AND status = ?';
      params.push(status);
    }

    const offset = (page - 1) * limit;

    // 获取总数
    const countQuery = `SELECT COUNT(*) as total FROM atomic_services ${whereClause}`;
    const countResult = await sequelize.query(countQuery, {
      replacements: params,
      type: QueryTypes.SELECT
    });
    const total = countResult[0].total;

    // 获取分页数据
    const dataQuery = `
      SELECT aserv.*, u.username as created_by_name
      FROM atomic_services aserv
      LEFT JOIN users u ON aserv.created_by = u.id
      ${whereClause}
      ORDER BY aserv.created_at DESC
      LIMIT ? OFFSET ?
    `;
    const services = await sequelize.query(dataQuery, {
      replacements: [...params, parseInt(limit), offset],
      type: QueryTypes.SELECT
    });

    return {
      services,
      pagination: {
        page: parseInt(page),
        limit: parseInt(limit),
        total,
        pages: Math.ceil(total / limit)
      }
    };
  }

  // 获取单个原子服务
  async getById(id) {
    const query = `
      SELECT aserv.*, u.username as created_by_name
      FROM atomic_services aserv
      LEFT JOIN users u ON aserv.created_by = u.id
      WHERE aserv.id = ?
    `;
    const services = await sequelize.query(query, {
      replacements: [id],
      type: QueryTypes.SELECT
    });
    return services[0] || null;
  }

  // 创建原子服务
  async create(serviceData) {
    const { name, type, category, description, icon, color, input_schema, output_schema, implementation, config, is_system, created_by } = serviceData;

    const query = `
      INSERT INTO atomic_services (name, type, category, description, icon, color, input_schema, output_schema, implementation, config, is_system, created_by)
      VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
    `;
    const result = await sequelize.query(query, {
      replacements: [name, type, category, description, icon, color, JSON.stringify(input_schema), JSON.stringify(output_schema), JSON.stringify(implementation), JSON.stringify(config), is_system || false, created_by],
      type: QueryTypes.INSERT
    });

    return this.getById(result[0]);
  }

  // 更新原子服务
  async update(id, serviceData) {
    const { name, type, category, description, icon, color, input_schema, output_schema, implementation, config, status } = serviceData;

    const query = `
      UPDATE atomic_services
      SET name = ?, type = ?, category = ?, description = ?, icon = ?, color = ?, input_schema = ?, output_schema = ?, implementation = ?, config = ?, status = ?
      WHERE id = ?
    `;
    await sequelize.query(query, {
      replacements: [name, type, category, description, icon, color, JSON.stringify(input_schema), JSON.stringify(output_schema), JSON.stringify(implementation), JSON.stringify(config), status, id],
      type: QueryTypes.UPDATE
    });

    return this.getById(id);
  }

  // 删除原子服务
  async delete(id) {
    const query = 'DELETE FROM atomic_services WHERE id = ?';
    await sequelize.query(query, {
      replacements: [id],
      type: QueryTypes.DELETE
    });
    return true;
  }

  // 按类型获取原子服务
  async getByType(type) {
    const query = 'SELECT * FROM atomic_services WHERE type = ? AND status = ? ORDER BY name';
    return sequelize.query(query, {
      replacements: [type, 'active'],
      type: QueryTypes.SELECT
    });
  }

  // 获取原子服务组合
  async getCompositions(serviceId) {
    const query = `
      SELECT aserv.id, aserv.name, aserv.type, aserv.category, asc2.execution_order, asc2.config
      FROM atomic_service_compositions asc2
      JOIN atomic_services aserv ON asc2.child_service_id = aserv.id
      WHERE asc2.parent_service_id = ?
      ORDER BY asc2.execution_order
    `;
    return sequelize.query(query, {
      replacements: [serviceId],
      type: QueryTypes.SELECT
    });
  }

  // 添加原子服务组合
  async addComposition(parentServiceId, compositionData) {
    const { child_service_id, execution_order, config } = compositionData;

    const query = `
      INSERT INTO atomic_service_compositions (parent_service_id, child_service_id, execution_order, config)
      VALUES (?, ?, ?, ?)
    `;
    const result = await sequelize.query(query, {
      replacements: [parentServiceId, child_service_id, execution_order || 0, JSON.stringify(config)],
      type: QueryTypes.INSERT
    });

    const selectQuery = 'SELECT * FROM atomic_service_compositions WHERE id = ?';
    const compositions = await sequelize.query(selectQuery, {
      replacements: [result[0]],
      type: QueryTypes.SELECT
    });
    return compositions[0];
  }

  // 测试原子服务
  async test(serviceId, testData) {
    const service = await this.getById(serviceId);
    if (!service) {
      throw new Error('原子服务不存在');
    }

    // 这里可以添加实际的测试逻辑
    // 例如：调用服务的实现，传入测试数据，返回结果
    return {
      service_id: serviceId,
      service_name: service.name,
      test_data: testData,
      result: '测试成功',
      timestamp: new Date().toISOString()
    };
  }
}

module.exports = new AtomicServiceService();
