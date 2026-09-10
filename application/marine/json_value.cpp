#include "json_value.hpp"

#include <cctype>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <stdexcept>

namespace marine {

JsonValue::JsonValue() : value_(nullptr) {}
JsonValue::JsonValue(std::nullptr_t) : value_(nullptr) {}
JsonValue::JsonValue(bool value) : value_(value) {}
JsonValue::JsonValue(double value) : value_(value) {}
JsonValue::JsonValue(int value) : value_(static_cast<double>(value)) {}
JsonValue::JsonValue(const char* value) : value_(std::string(value)) {}
JsonValue::JsonValue(std::string value) : value_(std::move(value)) {}
JsonValue::JsonValue(Array value) : value_(std::move(value)) {}
JsonValue::JsonValue(Object value) : value_(std::move(value)) {}
bool JsonValue::isNull() const { return std::holds_alternative<std::nullptr_t>(value_); }
bool JsonValue::isBool() const { return std::holds_alternative<bool>(value_); }
bool JsonValue::isNumber() const { return std::holds_alternative<double>(value_); }
bool JsonValue::isString() const { return std::holds_alternative<std::string>(value_); }
bool JsonValue::isArray() const { return std::holds_alternative<Array>(value_); }
bool JsonValue::isObject() const { return std::holds_alternative<Object>(value_); }
bool JsonValue::asBool() const { return std::get<bool>(value_); }
double JsonValue::asNumber() const { return std::get<double>(value_); }
const std::string& JsonValue::asString() const { return std::get<std::string>(value_); }
const JsonValue::Array& JsonValue::asArray() const { return std::get<Array>(value_); }
const JsonValue::Object& JsonValue::asObject() const { return std::get<Object>(value_); }
JsonValue::Array& JsonValue::asArray() { return std::get<Array>(value_); }
JsonValue::Object& JsonValue::asObject() { return std::get<Object>(value_); }
const JsonValue* JsonValue::get(const std::string& key) const {
    if (!isObject()) return nullptr;
    const auto iterator = asObject().find(key);
    return iterator == asObject().end() ? nullptr : &iterator->second;
}

namespace {
class Parser {
public:
    explicit Parser(const std::string& input) : input_(input) {}
    bool parse(JsonValue& value, std::string& error) {
        skipWhitespace();
        if (!parseValue(value, error)) return false;
        skipWhitespace();
        if (position_ != input_.size()) { error = "JSON 末尾存在额外内容。"; return false; }
        return true;
    }
private:
    const std::string& input_;
    size_t position_ = 0;

    void skipWhitespace() { while (position_ < input_.size() && std::isspace(static_cast<unsigned char>(input_[position_]))) ++position_; }
    bool consume(char expected) { if (position_ < input_.size() && input_[position_] == expected) { ++position_; return true; } return false; }
    bool parseValue(JsonValue& value, std::string& error) {
        if (position_ >= input_.size()) { error = "JSON 值不完整。"; return false; }
        const char token = input_[position_];
        if (token == '{') return parseObject(value, error);
        if (token == '[') return parseArray(value, error);
        if (token == '"') { std::string text; if (!parseString(text, error)) return false; value = JsonValue(std::move(text)); return true; }
        if (token == '-' || std::isdigit(static_cast<unsigned char>(token))) return parseNumber(value, error);
        if (input_.compare(position_, 4, "true") == 0) { position_ += 4; value = JsonValue(true); return true; }
        if (input_.compare(position_, 5, "false") == 0) { position_ += 5; value = JsonValue(false); return true; }
        if (input_.compare(position_, 4, "null") == 0) { position_ += 4; value = JsonValue(nullptr); return true; }
        error = "JSON 值格式错误。"; return false;
    }
    bool parseObject(JsonValue& value, std::string& error) {
        consume('{'); skipWhitespace(); JsonValue::Object object;
        if (consume('}')) { value = JsonValue(std::move(object)); return true; }
        while (true) {
            skipWhitespace(); std::string key;
            if (!parseString(key, error)) return false;
            skipWhitespace();
            if (!consume(':')) { error = "JSON 对象缺少冒号。"; return false; }
            skipWhitespace(); JsonValue field;
            if (!parseValue(field, error)) return false;
            if (!object.emplace(key, std::move(field)).second) { error = "JSON 对象包含重复字段。"; return false; }
            skipWhitespace();
            if (consume('}')) { value = JsonValue(std::move(object)); return true; }
            if (!consume(',')) { error = "JSON 对象缺少逗号。"; return false; }
        }
    }
    bool parseArray(JsonValue& value, std::string& error) {
        consume('['); skipWhitespace(); JsonValue::Array array;
        if (consume(']')) { value = JsonValue(std::move(array)); return true; }
        while (true) {
            skipWhitespace(); JsonValue entry;
            if (!parseValue(entry, error)) return false;
            array.push_back(std::move(entry)); skipWhitespace();
            if (consume(']')) { value = JsonValue(std::move(array)); return true; }
            if (!consume(',')) { error = "JSON 数组缺少逗号。"; return false; }
        }
    }
    bool parseString(std::string& value, std::string& error) {
        if (!consume('"')) { error = "JSON 字符串应以引号开始。"; return false; }
        value.clear();
        while (position_ < input_.size()) {
            unsigned char c = static_cast<unsigned char>(input_[position_++]);
            if (c == '"') return true;
            if (c < 0x20) { error = "JSON 字符串包含控制字符。"; return false; }
            if (c != '\\') { value.push_back(static_cast<char>(c)); continue; }
            if (position_ >= input_.size()) { error = "JSON 转义不完整。"; return false; }
            const char escaped = input_[position_++];
            switch (escaped) {
                case '"': value.push_back('"'); break; case '\\': value.push_back('\\'); break; case '/': value.push_back('/'); break;
                case 'b': value.push_back('\b'); break; case 'f': value.push_back('\f'); break; case 'n': value.push_back('\n'); break;
                case 'r': value.push_back('\r'); break; case 't': value.push_back('\t'); break;
                case 'u': error = "当前 JSON 接口不接受 Unicode 转义，请使用 UTF-8。"; return false;
                default: error = "JSON 转义字符无效。"; return false;
            }
        }
        error = "JSON 字符串未闭合。"; return false;
    }
    bool parseNumber(JsonValue& value, std::string& error) {
        const size_t start = position_;
        if (input_[position_] == '-') ++position_;
        if (position_ >= input_.size()) { error = "JSON 数字不完整。"; return false; }
        if (input_[position_] == '0') ++position_;
        else if (std::isdigit(static_cast<unsigned char>(input_[position_]))) while (position_ < input_.size() && std::isdigit(static_cast<unsigned char>(input_[position_]))) ++position_;
        else { error = "JSON 数字格式错误。"; return false; }
        if (position_ < input_.size() && input_[position_] == '.') { ++position_; size_t fraction = position_; while (position_ < input_.size() && std::isdigit(static_cast<unsigned char>(input_[position_]))) ++position_; if (fraction == position_) { error = "JSON 小数不完整。"; return false; } }
        if (position_ < input_.size() && (input_[position_] == 'e' || input_[position_] == 'E')) { ++position_; if (position_ < input_.size() && (input_[position_] == '+' || input_[position_] == '-')) ++position_; size_t exponent = position_; while (position_ < input_.size() && std::isdigit(static_cast<unsigned char>(input_[position_]))) ++position_; if (exponent == position_) { error = "JSON 指数不完整。"; return false; } }
        try { double parsed = std::stod(input_.substr(start, position_ - start)); if (!std::isfinite(parsed)) throw std::out_of_range("not finite"); value = JsonValue(parsed); return true; }
        catch (...) { error = "JSON 数字超出范围。"; return false; }
    }
};

std::string escape(const std::string& input) {
    std::ostringstream output;
    for (const unsigned char c : input) {
        switch (c) { case '"': output << "\\\\\""; break; case '\\': output << "\\\\\\\\"; break; case '\b': output << "\\\\b"; break; case '\f': output << "\\\\f"; break; case '\n': output << "\\\\n"; break; case '\r': output << "\\\\r"; break; case '\t': output << "\\\\t"; break; default: output << static_cast<char>(c); }
    }
    return output.str();
}
}

bool parseJson(const std::string& text, JsonValue& value, std::string& error) { return Parser(text).parse(value, error); }
std::string toJson(const JsonValue& value) {
    if (value.isNull()) return "null";
    if (value.isBool()) return value.asBool() ? "true" : "false";
    if (value.isNumber()) { std::ostringstream output; output << std::setprecision(15) << value.asNumber(); return output.str(); }
    if (value.isString()) return '"' + escape(value.asString()) + '"';
    if (value.isArray()) { std::string output = "["; bool first = true; for (const auto& entry : value.asArray()) { if (!first) output += ','; output += toJson(entry); first = false; } return output + ']'; }
    std::string output = "{"; bool first = true; for (const auto& entry : value.asObject()) { if (!first) output += ','; output += '"' + escape(entry.first) + "\":" + toJson(entry.second); first = false; } return output + '}';
}

} // namespace marine
