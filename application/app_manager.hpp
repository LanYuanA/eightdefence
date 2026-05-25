/**
 * @file app_manager.hpp
 * @brief 应用管理器 - 统一管理多个应用的生命周期和路由
 *
 * AppManager 负责:
 *   - 注册和管理多个应用实例
 *   - 根据请求路径分发到对应应用
 *   - 管理应用的生命周期 (init/start/stop)
 *   - 提供应用列表查询接口
 *   - 提供应用首页路由入口
 */

#ifndef APP_MANAGER_HPP
#define APP_MANAGER_HPP

#include "app_base.hpp"
#include <vector>
#include <memory>
#include <string>
#include <mutex>

class AppManager {
public:
    static AppManager& instance() {
        static AppManager s_instance;
        return s_instance;
    }

    /**
     * @brief 注册一个应用
     * @param app 应用实例 (所有权转移给 AppManager)
     */
    void registerApp(std::shared_ptr<AppBase> app) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_apps.push_back(std::move(app));
    }

    /**
     * @brief 初始化所有已注册的应用
     * @return 0=全部成功, 非0=有失败
     */
    int initAll() {
        std::lock_guard<std::mutex> lock(m_mutex);
        int failures = 0;
        for (auto& app : m_apps) {
            if (app->init() != 0) {
                fprintf(stderr, "[AppManager] 应用 '%s' 初始化失败!\n",
                        app->getName().c_str());
                failures++;
            }
        }
        return failures;
    }

    /**
     * @brief 启动所有已注册的应用
     */
    void startAll() {
        std::lock_guard<std::mutex> lock(m_mutex);
        for (auto& app : m_apps) {
            if (app->start() != 0) {
                fprintf(stderr, "[AppManager] 应用 '%s' 启动失败!\n",
                        app->getName().c_str());
            }
        }
    }

    /**
     * @brief 停止所有应用
     */
    void stopAll() {
        std::lock_guard<std::mutex> lock(m_mutex);
        for (auto& app : m_apps) {
            app->stop();
        }
    }

    /**
     * @brief 处理 HTTP 请求 - 路由分发
     *
     * 优先级:
     *   1. 精确匹配应用路由前缀 -> 转发给对应应用
     *   2. /api/apps -> 返回应用列表
     *   3. /app/<name> -> 返回应用首页
     *   4. 返回 nullptr 表示未匹配
     *
     * @param method HTTP 方法
     * @param path 完整请求路径
     * @param queryString 查询字符串
     * @param body 请求体
     * @return 响应, 或 nullptr 表示未匹配
     */
    std::unique_ptr<HttpResponse> routeRequest(
        const std::string& method,
        const std::string& path,
        const std::string& queryString = "",
        const std::string& body = "")
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        // 系统级 API: 应用列表
        if (path == "/api/apps") {
            return std::make_unique<HttpResponse>(
                HttpResponse::json(getAppsJson()));
        }

        // 尝试匹配应用路由
        for (auto& app : m_apps) {
            if (app->matchRoute(path)) {
                HttpRequest req;
                req.method = method;
                req.fullPath = path;
                req.path = app->stripPrefix(path);
                req.queryString = queryString;
                req.body = body;

                // API 请求
                if (req.path.find("/api/") == 0 || req.path == "/api") {
                    auto resp = app->handleApi(req);
                    return std::make_unique<HttpResponse>(std::move(resp));
                }

                // 静态文件 / 首页请求
                std::string indexPage = app->getIndexPage();
                if (!indexPage.empty()) {
                    HttpResponse resp;
                    resp.statusCode = 200;
                    resp.statusText = "OK";
                    resp.contentType = "text/html; charset=utf-8";
                    // 读取文件内容将在 web_server 层处理
                    // 这里返回文件路径信息
                    resp.body = "__FILE__:" + indexPage;
                    return std::make_unique<HttpResponse>(std::move(resp));
                }

                return std::make_unique<HttpResponse>(
                    HttpResponse::error(404, "No index page for app: " + app->getName()));
            }
        }

        return nullptr; // 未匹配任何应用
    }

    /**
     * @brief 根据名称查找应用
     */
    std::shared_ptr<AppBase> findApp(const std::string& name) {
        std::lock_guard<std::mutex> lock(m_mutex);
        for (auto& app : m_apps) {
            if (app->getName() == name) return app;
        }
        return nullptr;
    }

    /**
     * @brief 获取所有应用列表
     */
    std::vector<std::shared_ptr<AppBase>> getAllApps() {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_apps;
    }

    /**
     * @brief 获取已注册应用数量
     */
    size_t getAppCount() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_apps.size();
    }

private:
    AppManager() = default;
    ~AppManager() = default;
    AppManager(const AppManager&) = delete;
    AppManager& operator=(const AppManager&) = delete;

    std::string getAppsJson() const {
        std::string json = "[";
        for (size_t i = 0; i < m_apps.size(); i++) {
            if (i > 0) json += ",";
            json += m_apps[i]->getInfoJson();
        }
        json += "]";
        return json;
    }

    mutable std::mutex m_mutex;
    std::vector<std::shared_ptr<AppBase>> m_apps;
};

#endif // APP_MANAGER_HPP