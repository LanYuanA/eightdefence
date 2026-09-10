#ifndef MARINE_JSON_VALUE_HPP
#define MARINE_JSON_VALUE_HPP

#include <map>
#include <string>
#include <variant>
#include <vector>

namespace marine {

class JsonValue {
public:
    using Array = std::vector<JsonValue>;
    using Object = std::map<std::string, JsonValue>;
    using Storage = std::variant<std::nullptr_t, bool, double, std::string, Array, Object>;

    JsonValue();
    JsonValue(std::nullptr_t);
    JsonValue(bool value);
    JsonValue(double value);
    JsonValue(int value);
    JsonValue(const char* value);
    JsonValue(std::string value);
    JsonValue(Array value);
    JsonValue(Object value);

    bool isNull() const;
    bool isBool() const;
    bool isNumber() const;
    bool isString() const;
    bool isArray() const;
    bool isObject() const;
    bool asBool() const;
    double asNumber() const;
    const std::string& asString() const;
    const Array& asArray() const;
    const Object& asObject() const;
    Array& asArray();
    Object& asObject();
    const JsonValue* get(const std::string& key) const;

private:
    Storage value_;
};

bool parseJson(const std::string& text, JsonValue& value, std::string& error);
std::string toJson(const JsonValue& value);

} // namespace marine

#endif
