#include "Value.h"

#include <sstream>
#include <iomanip>

// Безопасные геттеры
std::optional<std::string> ValueWrapper::get_string() const{
    if (auto* p = std::get_if<std::string>(&value)) return *p;
    return std::nullopt;
}

std::optional<long long> ValueWrapper::get_int64() const{
    if (auto* p = std::get_if<long long>(&value)) return *p;
    return std::nullopt;
}

std::optional<double> ValueWrapper::get_double() const {
    if (auto* p = std::get_if<double>(&value)) return *p;
    return std::nullopt;
}

std::optional<std::vector<char>> ValueWrapper::get_binary() const {
    if (auto* p = std::get_if<std::vector<char>>(&value)) return *p;
    return std::nullopt;
}

// Геттеры с дефолтом
std::string ValueWrapper::as_string(const std::string& default_val) const {
    if (auto opt = get_string()) return *opt;
    return default_val;
}

long long ValueWrapper::as_int64(long long default_val) const {
    if (auto opt = get_int64()) return *opt;
    return default_val;
}

double ValueWrapper::as_double(double default_val) const {
    if (auto opt = get_double()) return *opt;
    return default_val;
}

std::string ValueWrapper::to_string() const {
    return std::visit([](auto** arg) -> std::string {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, std::monostate>) return "null";
        else if constexpr (std::is_same_v<T, std::string>) return "\"" + arg + "\"";
        else if constexpr (std::is_same_v<T, long long>) return std::to_string(arg);
        else if constexpr (std::is_same_v<T, double>) return std::to_string(arg);
        else if constexpr (std::is_same_v<T, std::vector<char>>) {
            return "[binary " + std::to_string(arg.size()) + " bytes]";
        }
    }, value);
}

// Сравнение
bool ValueWrapper::operator==(const ValueWrapper& other) const {
    return value == other.value;
}

// Вывод в поток (для отладки)
std::ostream& operator<<(std::ostream& os, const ValueWrapper& v) {
    os << v.to_string();
    return os;
}
