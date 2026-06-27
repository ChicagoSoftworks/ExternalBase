#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>
#include <cstdlib>
#include <cctype>
// Chicago Custom Json Library
namespace json {

struct value {
    enum Type { Null, Number, String, Object, Array };
    Type type = Null;
    int64_t num = 0;
    std::string str;
    std::unordered_map<std::string, value> obj;
    std::vector<value> arr;

    value() : type(Null) {}
    value(Type t) : type(t) {}
    value(int64_t n) : type(Number), num(n) {}
    value(const std::string& s) : type(String), str(s) {}

    bool is_null() const { return type == Null; }
    bool is_obj() const { return type == Object; }
    bool is_str() const { return type == String; }
    bool is_num() const { return type == Number; }
    bool is_arr() const { return type == Array; }

    const value& operator[](const char* k) const {
        static value null_val;
        if (type != Object) return null_val;
        auto it = obj.find(k);
        return it != obj.end() ? it->second : null_val;
    }

    value& operator[](const char* k) {
        static value null_val;
        if (type != Object) return null_val;
        return obj[k];
    }

    const value& operator[](size_t i) const {
        static value null_val;
        if (type != Array || i >= arr.size()) return null_val;
        return arr[i];
    }

    int64_t as_int64() const { return num; }
    std::string as_str() const { return str; }
    size_t size() const { return type == Array ? arr.size() : 0; }
};

inline std::string trim(const std::string& s, size_t& pos) {
    while (pos < s.size() && (s[pos] == ' ' || s[pos] == '\t' || s[pos] == '\n' || s[pos] == '\r')) pos++;
    return {};
}

inline value parse_value(const std::string& s, size_t& pos);

inline std::string parse_string(const std::string& s, size_t& pos) {
    std::string result;
    if (pos >= s.size() || s[pos] != '"') return result;
    pos++;
    while (pos < s.size() && s[pos] != '"') {
        if (s[pos] == '\\') {
            pos++;
            switch (s[pos]) {
                case '"': result += '"'; break;
                case '\\': result += '\\'; break;
                case '/': result += '/'; break;
                case 'n': result += '\n'; break;
                case 't': result += '\t'; break;
                case 'r': result += '\r'; break;
                default: result += s[pos]; break;
            }
            pos++;
        } else {
            result += s[pos++];
        }
    }
    if (pos < s.size()) pos++;
    return result;
}

inline value parse_object(const std::string& s, size_t& pos) {
    value v(value::Object);
    if (pos >= s.size() || s[pos] != '{') return v;
    pos++;
    while (pos < s.size()) {
        trim(s, pos);
        if (pos >= s.size()) break;
        if (s[pos] == '}') { pos++; return v; }
        if (s[pos] != '"') break;
        std::string key = parse_string(s, pos);
        trim(s, pos);
        if (pos < s.size() && s[pos] == ':') pos++;
        trim(s, pos);
        v.obj[key] = parse_value(s, pos);
        trim(s, pos);
        if (pos < s.size() && s[pos] == ',') pos++;
    }
    return v;
}

inline value parse_array(const std::string& s, size_t& pos) {
    value v(value::Array);
    if (pos >= s.size() || s[pos] != '[') return v;
    pos++;
    while (pos < s.size()) {
        trim(s, pos);
        if (pos >= s.size()) break;
        if (s[pos] == ']') { pos++; return v; }
        v.arr.push_back(parse_value(s, pos));
        trim(s, pos);
        if (pos < s.size() && s[pos] == ',') pos++;
    }
    return v;
}

inline value parse_number(const std::string& s, size_t& pos) {
    size_t start = pos;
    bool neg = false;
    if (pos < s.size() && s[pos] == '-') { neg = true; pos++; }
    while (pos < s.size() && isdigit((unsigned char)s[pos])) pos++;
    if (pos < s.size() && s[pos] == '.') {
        pos++;
        while (pos < s.size() && isdigit((unsigned char)s[pos])) pos++;
    }
    double d = strtod(s.c_str() + start, nullptr);
    return value((int64_t)d);
}

inline value parse_value(const std::string& s, size_t& pos) {
    trim(s, pos);
    if (pos >= s.size()) return {};
    switch (s[pos]) {
        case '{': return parse_object(s, pos);
        case '[': return parse_array(s, pos);
        case '"': return value(parse_string(s, pos));
        case 't': pos += 4; return value((int64_t)1);
        case 'f': pos += 5; return value((int64_t)0);
        case 'n': pos += 4; return {};
        default: return parse_number(s, pos);
    }
}

inline value parse(const std::string& s) {
    size_t pos = 0;
    return parse_value(s, pos);
}

}
