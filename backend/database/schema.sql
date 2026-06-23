-- ============================================
-- 八防综合监控平台 - 数据库设计
-- ============================================

-- 创建数据库
CREATE DATABASE IF NOT EXISTS eightdefence_db
CHARACTER SET utf8mb4
COLLATE utf8mb4_unicode_ci;

USE eightdefence_db;

-- ============================================
-- 1. 设备管理相关表
-- ============================================

-- 设备表
CREATE TABLE devices (
    id INT PRIMARY KEY AUTO_INCREMENT,
    name VARCHAR(100) NOT NULL COMMENT '设备名称',
    type VARCHAR(50) NOT NULL COMMENT '设备类型',
    model VARCHAR(100) COMMENT '设备型号',
    address VARCHAR(20) COMMENT 'Modbus地址',
    description TEXT COMMENT '设备描述',
    layer ENUM('application', 'atomic_upper', 'atomic_lower', 'virtual', 'physical') NOT NULL COMMENT '所属层级',
    layer_name VARCHAR(50) COMMENT '层级名称',
    config JSON COMMENT '设备配置（寄存器映射、阈值等）',
    status ENUM('online', 'offline', 'error', 'maintenance') DEFAULT 'offline' COMMENT '设备状态',
    last_online_at TIMESTAMP NULL COMMENT '最后在线时间',
    created_by INT COMMENT '创建者ID',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    INDEX idx_layer (layer),
    INDEX idx_type (type),
    INDEX idx_status (status)
) COMMENT '设备表';

-- 设备寄存器配置表
CREATE TABLE device_registers (
    id INT PRIMARY KEY AUTO_INCREMENT,
    device_id INT NOT NULL,
    register_name VARCHAR(50) NOT NULL COMMENT '寄存器名称',
    register_address INT NOT NULL COMMENT '寄存器地址',
    data_type VARCHAR(20) NOT NULL COMMENT '数据类型（uint16, int16, float32等）',
    scale_factor DECIMAL(10, 4) DEFAULT 1.0 COMMENT '缩放系数',
    offset_value DECIMAL(10, 4) DEFAULT 0.0 COMMENT '偏移量',
    unit VARCHAR(20) COMMENT '单位',
    description TEXT COMMENT '描述',
    is_readonly BOOLEAN DEFAULT TRUE COMMENT '是否只读',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (device_id) REFERENCES devices(id) ON DELETE CASCADE,
    UNIQUE KEY uk_device_register (device_id, register_name)
) COMMENT '设备寄存器配置表';

-- 设备阈值配置表
CREATE TABLE device_thresholds (
    id INT PRIMARY KEY AUTO_INCREMENT,
    device_id INT NOT NULL,
    register_id INT NOT NULL,
    min_value DECIMAL(20, 4) COMMENT '最小阈值',
    max_value DECIMAL(20, 4) COMMENT '最大阈值',
    alarm_level ENUM('info', 'warning', 'critical') DEFAULT 'warning' COMMENT '报警级别',
    description TEXT COMMENT '描述',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (device_id) REFERENCES devices(id) ON DELETE CASCADE,
    FOREIGN KEY (register_id) REFERENCES device_registers(id) ON DELETE CASCADE
) COMMENT '设备阈值配置表';

-- ============================================
-- 2. 原子服务相关表
-- ============================================

-- 原子服务表
CREATE TABLE atomic_services (
    id INT PRIMARY KEY AUTO_INCREMENT,
    name VARCHAR(100) NOT NULL COMMENT '服务名称',
    type ENUM('upper', 'lower') NOT NULL COMMENT '服务类型（上层/下层）',
    category VARCHAR(50) COMMENT '服务分类',
    description TEXT COMMENT '服务描述',
    icon VARCHAR(50) COMMENT '图标',
    color VARCHAR(20) COMMENT '颜色',
    input_schema JSON COMMENT '输入参数schema',
    output_schema JSON COMMENT '输出参数schema',
    implementation JSON COMMENT '实现配置（函数、脚本等）',
    config JSON COMMENT '额外配置',
    is_system BOOLEAN DEFAULT FALSE COMMENT '是否系统内置',
    status ENUM('active', 'inactive', 'deprecated') DEFAULT 'active' COMMENT '状态',
    created_by INT COMMENT '创建者ID',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    INDEX idx_type (type),
    INDEX idx_category (category),
    INDEX idx_status (status)
) COMMENT '原子服务表';

-- 原子服务组合表（上层服务由下层服务组合而成）
CREATE TABLE atomic_service_compositions (
    id INT PRIMARY KEY AUTO_INCREMENT,
    parent_service_id INT NOT NULL COMMENT '上层服务ID',
    child_service_id INT NOT NULL COMMENT '下层服务ID',
    execution_order INT DEFAULT 0 COMMENT '执行顺序',
    config JSON COMMENT '组合配置',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (parent_service_id) REFERENCES atomic_services(id) ON DELETE CASCADE,
    FOREIGN KEY (child_service_id) REFERENCES atomic_services(id) ON DELETE CASCADE,
    UNIQUE KEY uk_composition (parent_service_id, child_service_id)
) COMMENT '原子服务组合表';

-- ============================================
-- 3. 资源池相关表
-- ============================================

-- 资源池表
CREATE TABLE resource_pool (
    id INT PRIMARY KEY AUTO_INCREMENT,
    name VARCHAR(100) NOT NULL COMMENT '资源名称',
    type ENUM('device', 'data', 'service', 'ui_component') NOT NULL COMMENT '资源类型',
    resource_id INT NOT NULL COMMENT '资源ID',
    resource_data JSON COMMENT '资源数据',
    category VARCHAR(50) COMMENT '资源分类',
    tags JSON COMMENT '标签',
    description TEXT COMMENT '描述',
    is_active BOOLEAN DEFAULT TRUE COMMENT '是否启用',
    created_by INT COMMENT '创建者ID',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    INDEX idx_type (type),
    INDEX idx_category (category),
    INDEX idx_active (is_active)
) COMMENT '资源池表';

-- 资源绑定表（资源与UI组件的绑定关系）
CREATE TABLE resource_bindings (
    id INT PRIMARY KEY AUTO_INCREMENT,
    resource_id INT NOT NULL COMMENT '资源ID',
    component_id VARCHAR(100) NOT NULL COMMENT '组件ID',
    component_type VARCHAR(50) COMMENT '组件类型',
    binding_config JSON COMMENT '绑定配置',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (resource_id) REFERENCES resource_pool(id) ON DELETE CASCADE,
    INDEX idx_component (component_id)
) COMMENT '资源绑定表';

-- ============================================
-- 4. 流程编排相关表
-- ============================================

-- 流程定义表
CREATE TABLE workflows (
    id INT PRIMARY KEY AUTO_INCREMENT,
    name VARCHAR(100) NOT NULL COMMENT '流程名称',
    description TEXT COMMENT '流程描述',
    category VARCHAR(50) COMMENT '流程分类',
    nodes JSON COMMENT '节点定义',
    edges JSON COMMENT '连线定义',
    config JSON COMMENT '流程配置',
    variables JSON COMMENT '流程变量',
    status ENUM('draft', 'active', 'inactive', 'archived') DEFAULT 'draft' COMMENT '状态',
    version INT DEFAULT 1 COMMENT '版本号',
    created_by INT COMMENT '创建者ID',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    INDEX idx_status (status),
    INDEX idx_category (category)
) COMMENT '流程定义表';

-- 流程执行历史表
CREATE TABLE workflow_executions (
    id BIGINT PRIMARY KEY AUTO_INCREMENT,
    workflow_id INT NOT NULL,
    trigger_type VARCHAR(50) COMMENT '触发类型（manual, auto, schedule）',
    trigger_data JSON COMMENT '触发数据',
    status ENUM('running', 'completed', 'failed', 'cancelled') DEFAULT 'running',
    result JSON COMMENT '执行结果',
    error_message TEXT COMMENT '错误信息',
    started_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    completed_at TIMESTAMP NULL,
    FOREIGN KEY (workflow_id) REFERENCES workflows(id) ON DELETE CASCADE,
    INDEX idx_workflow (workflow_id),
    INDEX idx_status (status),
    INDEX idx_started (started_at)
) COMMENT '流程执行历史表';

-- ============================================
-- 5. 用户和权限相关表
-- ============================================

-- 用户表
CREATE TABLE users (
    id INT PRIMARY KEY AUTO_INCREMENT,
    username VARCHAR(50) UNIQUE NOT NULL COMMENT '用户名',
    password_hash VARCHAR(255) NOT NULL COMMENT '密码哈希',
    email VARCHAR(100) COMMENT '邮箱',
    phone VARCHAR(20) COMMENT '手机号',
    real_name VARCHAR(50) COMMENT '真实姓名',
    role ENUM('admin', 'operator', 'viewer') DEFAULT 'viewer' COMMENT '角色',
    permissions JSON COMMENT '权限配置',
    avatar VARCHAR(255) COMMENT '头像',
    last_login_at TIMESTAMP NULL COMMENT '最后登录时间',
    status ENUM('active', 'inactive', 'locked') DEFAULT 'active' COMMENT '状态',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    INDEX idx_role (role),
    INDEX idx_status (status)
) COMMENT '用户表';

-- 用户操作日志表
CREATE TABLE user_operation_logs (
    id BIGINT PRIMARY KEY AUTO_INCREMENT,
    user_id INT NOT NULL,
    operation VARCHAR(50) NOT NULL COMMENT '操作类型',
    resource_type VARCHAR(50) COMMENT '资源类型',
    resource_id INT COMMENT '资源ID',
    detail JSON COMMENT '操作详情',
    ip_address VARCHAR(45) COMMENT 'IP地址',
    user_agent VARCHAR(500) COMMENT '用户代理',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE,
    INDEX idx_user (user_id),
    INDEX idx_operation (operation),
    INDEX idx_created (created_at)
) COMMENT '用户操作日志表';

-- ============================================
-- 6. 设备数据历史表
-- ============================================

-- 设备数据历史表
CREATE TABLE device_data_history (
    id BIGINT PRIMARY KEY AUTO_INCREMENT,
    device_id INT NOT NULL,
    register_name VARCHAR(50) NOT NULL COMMENT '寄存器名称',
    value DECIMAL(20, 4) COMMENT '数据值',
    quality ENUM('good', 'bad', 'uncertain') DEFAULT 'good' COMMENT '数据质量',
    timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (device_id) REFERENCES devices(id) ON DELETE CASCADE,
    INDEX idx_device_register (device_id, register_name),
    INDEX idx_timestamp (timestamp)
) COMMENT '设备数据历史表';

-- 设备报警历史表
CREATE TABLE device_alarm_history (
    id BIGINT PRIMARY KEY AUTO_INCREMENT,
    device_id INT NOT NULL,
    register_name VARCHAR(50) COMMENT '寄存器名称',
    alarm_type VARCHAR(50) NOT NULL COMMENT '报警类型',
    alarm_level ENUM('info', 'warning', 'critical') NOT NULL COMMENT '报警级别',
    alarm_value DECIMAL(20, 4) COMMENT '报警值',
    threshold_min DECIMAL(20, 4) COMMENT '最小阈值',
    threshold_max DECIMAL(20, 4) COMMENT '最大阈值',
    description TEXT COMMENT '报警描述',
    status ENUM('active', 'acknowledged', 'resolved') DEFAULT 'active' COMMENT '状态',
    acknowledged_by INT COMMENT '确认人ID',
    acknowledged_at TIMESTAMP NULL COMMENT '确认时间',
    resolved_at TIMESTAMP NULL COMMENT '解决时间',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (device_id) REFERENCES devices(id) ON DELETE CASCADE,
    INDEX idx_device (device_id),
    INDEX idx_level (alarm_level),
    INDEX idx_status (status),
    INDEX idx_created (created_at)
) COMMENT '设备报警历史表';

-- ============================================
-- 7. 系统配置表
-- ============================================

-- 系统配置表
CREATE TABLE system_configs (
    id INT PRIMARY KEY AUTO_INCREMENT,
    config_key VARCHAR(100) UNIQUE NOT NULL COMMENT '配置键',
    config_value TEXT COMMENT '配置值',
    config_type VARCHAR(20) DEFAULT 'string' COMMENT '配置类型（string, number, json, boolean）',
    description TEXT COMMENT '描述',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP
) COMMENT '系统配置表';

-- ============================================
-- 8. 初始化数据
-- ============================================

-- 插入默认管理员用户（密码: admin123）
INSERT INTO users (username, password_hash, email, real_name, role, status) VALUES
('admin', '$2b$10$xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx', 'admin@eightdefence.com', '系统管理员', 'admin', 'active');

-- 插入默认系统配置
INSERT INTO system_configs (config_key, config_value, config_type, description) VALUES
('system_name', '八防综合监控平台', 'string', '系统名称'),
('data_retention_days', '90', 'number', '数据保留天数'),
('alarm_notification_enabled', 'true', 'boolean', '是否启用报警通知'),
('polling_interval', '2000', 'number', '轮询间隔（毫秒）'),
('device_timeout', '60', 'number', '设备超时时间（秒）');

-- 插入默认原子服务（下层基础服务）
INSERT INTO atomic_services (name, type, category, description, is_system, status) VALUES
('读取设备数据', 'lower', 'data', '从设备读取数据', TRUE, 'active'),
('写入设备数据', 'lower', 'control', '向设备写入数据', TRUE, 'active'),
('设备状态检测', 'lower', 'monitor', '检测设备在线状态', TRUE, 'active'),
('数据阈值判断', 'lower', 'condition', '判断数据是否超阈值', TRUE, 'active'),
('发送通知', 'lower', 'action', '发送报警通知', TRUE, 'active'),
('记录日志', 'lower', 'action', '记录操作日志', TRUE, 'active');

-- 插入默认原子服务（上层组合服务）
INSERT INTO atomic_services (name, type, category, description, is_system, status) VALUES
('环境监测服务', 'upper', 'environment', '环境数据监测和报警', TRUE, 'active'),
('安防监控服务', 'upper', 'security', '安防设备监控和联动', TRUE, 'active'),
('消防预警服务', 'upper', 'fire', '消防设备监控和应急', TRUE, 'active'),
('设备联动服务', 'upper', 'automation', '设备联动控制', TRUE, 'active');
