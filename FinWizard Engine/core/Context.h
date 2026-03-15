#pragma once
#include "Value.h"

#include <unordered_map>
#include <string>
#include <optional>

class Context {
public:
    Context ctx;

    // Основное хранилище
    std::unordered_map<std::string, Value> data;

    // Хелперы чтобы плагины могли удобно работать

    // string
    void set_string(const std::string& key, const std::string& val);
    std::optional<std::string> get_string(const std::string& key) const;

    // int64
    void set_int64(const std::string& key, long long val);
    std::optional<long long> get_int64(const std::string& key) const;

    // double
    void set_double(const std::string& key, double val);
    std::optional<double> get_double(const std::string& key) const;

    // binary
    void set_binary(const std::string& key, const std::vector<char>& val);
    std::optional<std::vector<char>> get_binary(const std::string& key) const;

    // есть ли ключ?
    bool has(const std::string& key) const;

    // очистить контекст
    void clear();

    // для отладки: вывести весь контекст
    std::string to_string() const;
};
