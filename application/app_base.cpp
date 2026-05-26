/**
 * @file app_base.cpp
 * @brief 应用程序基类实现
 */

#include "app_base.hpp"
#include "../core/app_logger.hpp"

AppBase::~AppBase() {
    if (m_logger) { delete m_logger; m_logger = nullptr; }
}

int AppBase::initLogger(const std::string& logDir) {
    if (m_logger) return 0;  // 已初始化
    m_logger = new AppLogger(m_name, logDir);
    return m_logger->init();
}
