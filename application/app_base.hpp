/**
 * @file app_base.hpp
 * @brief 应用程序基类 - 多应用框架的核心抽象
 *
 * 所有上层应用(安防、消防、环控等)都继承此基类。
 * 每个应用拥有独立的:
 *   - 名称和路由前缀
 *   - 前端静态文件目录
 *   - API 处理逻辑
 *   - 独立的业务线程(可选)
 *
 * 应用通过 AppManager 统一管理生命周期。
 */

#ifndef APP_BASE_HPP
#define APP_BASE_HPP

#include <string>
#include <vector>
#include <functional>
#include <atomic>
#include <map>

/**
 * @brief HTTP 请求信息
 */
struct HttpRequest {
    std::string method;       // GET, POST, PUT, DELETE
    std::string path;         // 请求路径 (不含路由前缀)
    std::string fullPath;     // 完整路径
    std::string queryString;  // 查询字符串
    std::string body;         // 请求体 (POST/PUT)
    std::map<std::string, std::string> headers;

    // 从查询字符串中提取参数
    std::string getParam(const std::string& key) const {
        size_t pos = queryString.find(key + "=");
        if (pos == std::string::npos) return "";
        pos += key.length() + 1;
        size_t end = queryString.find('&', pos);
        if (end == std::string::npos) end = queryString.length();
        return queryString.substr(pos, end - pos);
    }
};

/**
 * @brief HTTP 响应信息
 */
struct HttpResponse {
    int statusCode = 200;
    std::string statusText = "OK";
    std::string contentType = "text/plain";
    std::string body;
    std::map<std::string, std::string> headers;

    static HttpResponse json(const std::string& jsonBody, int code = 200) {
        HttpResponse resp;
        resp.statusCode = code;
        resp.statusText = (code == 200) ? "OK" : "Error";
        resp.contentType = "application/json; charset=utf-8";
        resp.body = jsonBody;
        return resp;
    }

    static HttpResponse html(const std::string& htmlBody, int code = 200) {
        HttpResponse resp;
        resp.statusCode = code;
        resp.statusText = (code == 200) ? "OK" : "Error";
        resp.contentType = "text/html; charset=utf-8";
        resp.body = htmlBody;
        return resp;
    }

    static HttpResponse error(int code, const std::string& message) {
        HttpResponse resp;
        resp.statusCode = code;
        resp.statusText = message;
        resp.contentType = "application/json; charset=utf-8";
        resp.body = "{\"error\":\"" + message + "\"}";
        return resp;
    }
};

/**
 * @brief API 路由处理器类型
 */
using ApiHandler = std::function<HttpResponse(const HttpRequest&)>;

/**
 * @brief 应用程序基类
 *
 * 生命周期: init() -> start() -> [运行中] -> stop()
 */
class AppBase {
public:
    AppBase(const std::string& name,
            const std::string& routePrefix,
            const std::string& description)
        : m_name(name)
        , m_routePrefix(routePrefix)
        , m_description(description)
        , m_running(false)
    {
    }

    virtual ~AppBase() = default;

    /* ============================================================
     * 属性访问
     * ============================================================ */
    const std::string& getName() const { return m_name; }
    const std::string& getRoutePrefix() const { return m_routePrefix; }
    const std::string& getDescription() const { return m_description; }
    bool isRunning() const { return m_running.load(); }

    /**
     * @brief 获取前端静态文件目录
     * @return 静态文件根目录路径, 空字符串表示无前端
     */
    virtual std::string getStaticDir() const { return ""; }

    /**
     * @brief 获取应用首页文件路径 (用于浏览器直接访问)
     * @return 首页HTML文件路径, 空字符串表示无首页
     */
    virtual std::string getIndexPage() const { return ""; }

    /* ============================================================
     * 生命周期方法
     * ============================================================ */

    /**
     * @brief 初始化应用 (加载配置、初始化资源)
     * @return 0=成功, 非0=失败
     */
    virtual int init() { return 0; }

    /**
     * @brief 启动应用 (启动业务线程等)
     * @return 0=成功, 非0=失败
     */
    virtual int start() {
        m_running = true;
        return 0;
    }

    /**
     * @brief 停止应用
     */
    virtual void stop() {
        m_running = false;
    }

    /* ============================================================
     * HTTP 请求处理
     * ============================================================ */

    /**
     * @brief 处理 API 请求
     * @param request HTTP 请求 (路径已去除应用前缀)
     * @return HTTP 响应
     */
    virtual HttpResponse handleApi(const HttpRequest& request) {
        // 查找注册的路由处理器
        for (const auto& route : m_apiRoutes) {
            if (request.path == route.first) {
                return route.second(request);
            }
        }
        return HttpResponse::error(404, "API not found: " + request.path);
    }

    /**
     * @brief 检查请求路径是否属于此应用
     * @param path 完整请求路径
     * @return true 如果此路径属于此应用
     */
    bool matchRoute(const std::string& path) const {
        return path.find(m_routePrefix) == 0;
    }

    /**
     * @brief 从完整路径中提取应用相对路径
     * @param fullPath 完整请求路径
     * @return 相对于应用路由前缀的路径
     */
    std::string stripPrefix(const std::string& fullPath) const {
        if (fullPath.find(m_routePrefix) == 0) {
            std::string sub = fullPath.substr(m_routePrefix.length());
            if (sub.empty() || sub[0] != '/') sub = "/" + sub;
            return sub;
        }
        return fullPath;
    }

    /**
     * @brief 获取应用信息 (JSON 格式)
     */
    virtual std::string getInfoJson() const {
        char buf[512];
        snprintf(buf, sizeof(buf),
            "{\"name\":\"%s\",\"route\":\"%s\",\"description\":\"%s\",\"running\":%s}",
            m_name.c_str(), m_routePrefix.c_str(), m_description.c_str(),
            m_running.load() ? "true" : "false");
        return std::string(buf);
    }

protected:
    /**
     * @brief 注册 API 路由
     * @param path API 路径 (相对于应用前缀)
     * @param handler 处理函数
     */
    void registerApi(const std::string& path, ApiHandler handler) {
        m_apiRoutes.emplace_back(path, std::move(handler));
    }

    std::string m_name;           // 应用名称
    std::string m_routePrefix;    // 路由前缀, 如 "/security"
    std::string m_description;    // 应用描述
    std::atomic<bool> m_running;  // 运行状态

    // API 路由表
    std::vector<std::pair<std::string, ApiHandler>> m_apiRoutes;
};

#endif // APP_BASE_HPP