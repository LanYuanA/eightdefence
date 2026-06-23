const { sequelize } = require('../database/connection');
const { QueryTypes } = require('sequelize');

class WorkflowService {
  // 获取所有流程
  async getAll({ status, category, page, limit }) {
    let whereClause = 'WHERE 1=1';
    const params = [];

    if (status) {
      whereClause += ' AND status = ?';
      params.push(status);
    }
    if (category) {
      whereClause += ' AND category = ?';
      params.push(category);
    }

    const offset = (page - 1) * limit;

    // 获取总数
    const countQuery = `SELECT COUNT(*) as total FROM workflows ${whereClause}`;
    const countResult = await sequelize.query(countQuery, {
      replacements: params,
      type: QueryTypes.SELECT
    });
    const total = countResult[0].total;

    // 获取分页数据
    const dataQuery = `
      SELECT w.*, u.username as created_by_name
      FROM workflows w
      LEFT JOIN users u ON w.created_by = u.id
      ${whereClause}
      ORDER BY w.updated_at DESC
      LIMIT ? OFFSET ?
    `;
    const workflows = await sequelize.query(dataQuery, {
      replacements: [...params, parseInt(limit), offset],
      type: QueryTypes.SELECT
    });

    return {
      workflows,
      pagination: {
        page: parseInt(page),
        limit: parseInt(limit),
        total,
        pages: Math.ceil(total / limit)
      }
    };
  }

  // 获取单个流程
  async getById(id) {
    const query = `
      SELECT w.*, u.username as created_by_name
      FROM workflows w
      LEFT JOIN users u ON w.created_by = u.id
      WHERE w.id = ?
    `;
    const workflows = await sequelize.query(query, {
      replacements: [id],
      type: QueryTypes.SELECT
    });
    return workflows[0] || null;
  }

  // 创建流程
  async create(workflowData) {
    const { name, description, category, nodes, edges, config, variables, created_by } = workflowData;

    const query = `
      INSERT INTO workflows (name, description, category, nodes, edges, config, variables, created_by)
      VALUES (?, ?, ?, ?, ?, ?, ?, ?)
    `;
    const result = await sequelize.query(query, {
      replacements: [name, description, category, JSON.stringify(nodes), JSON.stringify(edges), JSON.stringify(config), JSON.stringify(variables), created_by],
      type: QueryTypes.INSERT
    });

    return this.getById(result[0]);
  }

  // 更新流程
  async update(id, workflowData) {
    const { name, description, category, nodes, edges, config, variables, status } = workflowData;

    const query = `
      UPDATE workflows
      SET name = ?, description = ?, category = ?, nodes = ?, edges = ?, config = ?, variables = ?, status = ?, version = version + 1
      WHERE id = ?
    `;
    await sequelize.query(query, {
      replacements: [name, description, category, JSON.stringify(nodes), JSON.stringify(edges), JSON.stringify(config), JSON.stringify(variables), status, id],
      type: QueryTypes.UPDATE
    });

    return this.getById(id);
  }

  // 删除流程
  async delete(id) {
    const query = 'DELETE FROM workflows WHERE id = ?';
    await sequelize.query(query, {
      replacements: [id],
      type: QueryTypes.DELETE
    });
    return true;
  }

  // 执行流程
  async execute(workflowId, triggerData) {
    const workflow = await this.getById(workflowId);
    if (!workflow) {
      throw new Error('流程不存在');
    }

    if (workflow.status !== 'active') {
      throw new Error('流程未激活');
    }

    // 创建执行记录
    const executionQuery = `
      INSERT INTO workflow_executions (workflow_id, trigger_type, trigger_data, status)
      VALUES (?, ?, ?, 'running')
    `;
    const executionResult = await sequelize.query(executionQuery, {
      replacements: [workflowId, triggerData.trigger_type || 'manual', JSON.stringify(triggerData)],
      type: QueryTypes.INSERT
    });

    const executionId = executionResult[0];

    // 这里可以添加实际的流程执行逻辑
    // 例如：解析nodes和edges，按顺序执行各个节点

    // 模拟执行完成
    const updateQuery = `
      UPDATE workflow_executions
      SET status = 'completed', result = ?, completed_at = NOW()
      WHERE id = ?
    `;
    await sequelize.query(updateQuery, {
      replacements: [JSON.stringify({ message: '执行成功' }), executionId],
      type: QueryTypes.UPDATE
    });

    const selectQuery = 'SELECT * FROM workflow_executions WHERE id = ?';
    const executions = await sequelize.query(selectQuery, {
      replacements: [executionId],
      type: QueryTypes.SELECT
    });
    return executions[0];
  }

  // 获取流程执行历史
  async getExecutions(workflowId, { page, limit }) {
    const offset = (page - 1) * limit;

    const query = `
      SELECT *
      FROM workflow_executions
      WHERE workflow_id = ?
      ORDER BY started_at DESC
      LIMIT ? OFFSET ?
    `;
    return sequelize.query(query, {
      replacements: [workflowId, parseInt(limit), offset],
      type: QueryTypes.SELECT
    });
  }

  // 激活/停用流程
  async toggleStatus(workflowId) {
    const workflow = await this.getById(workflowId);
    if (!workflow) {
      throw new Error('流程不存在');
    }

    const newStatus = workflow.status === 'active' ? 'inactive' : 'active';

    const query = 'UPDATE workflows SET status = ? WHERE id = ?';
    await sequelize.query(query, {
      replacements: [newStatus, workflowId],
      type: QueryTypes.UPDATE
    });

    return this.getById(workflowId);
  }

  // 复制流程
  async duplicate(workflowId) {
    const workflow = await this.getById(workflowId);
    if (!workflow) {
      throw new Error('流程不存在');
    }

    const newWorkflowData = {
      name: `${workflow.name} (副本)`,
      description: workflow.description,
      category: workflow.category,
      nodes: workflow.nodes,
      edges: workflow.edges,
      config: workflow.config,
      variables: workflow.variables,
      created_by: workflow.created_by
    };

    return this.create(newWorkflowData);
  }
}

module.exports = new WorkflowService();
