// context.cpp
#include "Context.h"

// Хелперы

// string
void ctx::set_string(const std::string& key, const std::string& val) {
    data[key] = val;
}

std::optional<std::string> ctx::get_string(const std::string& key) const {
    auto it = data.find(key);
    if (it == data.end()) {
        return std::nullopt;
    }
    // Ключ есть — теперь проверяем, что значение именно строка
    // std::get_if — безопасный способ достать указатель на тип std::string
    if (const std::string* p = std::get_if<std::string>(&(it->second))) {
        return *p;
    }
    return std::nullopt;
}

// int64
void ctx::set_int64(const std::string& key, long long val) {
    data[key] = val;
}

std::optional<long long> ctx::get_int64(const std::string& key) const {
    auto it = data.find(key);
    if (it == data.end()) {
        return std::nullopt;
    }

    if (const long long *p = std::get_if<long long>(&(it->second))) {
        return *p;
    }
    return std::nullopt;
}

// double
void ctx::set_double(const std::string& key, double val) {
    data[key] = val;
}

std::optional<double> ctx::get_double(const std::string& key) const {
    auto it = data.find(key);
    if (it == data.end()) {
        return std::nullopt;
    }

    if (const double *p = std::get_if<double>(&(it->second))) {
        return *p;
    }
    return std::nullopt;
}

// binary
void ctx::set_binary(const std::string& key, const std::vector<char>& val) {
    data[key] = val;
}

std::optional<std::vector<char>> ctx::get_binary(const std::string& key) const {
    auto it = data.find(key);
    if (it == data.end()) {
        return std::nullopt;
    }

    if (const std::vector<char> *p = std::get_if<std::vector<char>>(&(it->second))) {
        return *p;
    }
    return std::nullopt;
}

// есть ли ключ?
bool has(const std::string& key) const {
    return data.find(key) != data.end();
}

// очистить контекст
void clear() {
    data.clear();
}

std::string to_string() const {
    std::ostringstream oss;     // для сборки строки

    if (data.empty()) {
            return "Context пустой";
        }

    oss << "Context содержит " << data.size() << " значений:\n";
    for (const auto& pair : data) {
        oss << "  " << pair.first << " = " << pair.second.to_string() << "\n";
    }
    return oss.str();
}


