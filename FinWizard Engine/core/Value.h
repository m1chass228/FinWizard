#pragma once
#include <variant>
#include <string>
#include <vector>
#include <optional>
#include <ostream> // для operator<<

/**
 * Универсальный тип значения для контекста.
 * Поддерживает: null, строка, целое число, дробное число, бинарные данные.
 */

using Value = std::variant<
    std::monostate,           // null / отсутствие значения
    std::string,              // текст, пути к файлам, JSON-строки
    long long,                // целые числа (64-бит)
    double,                   // дробные числа
    std::vector<char>         // бинарные данные (файлы, буферы)
>;

/**
 * Удобные методы для работы с Value
 */
class ValueWrapper {
public:
    explicit ValueWrapper(const Value& v = std::monostate{}) : value(v) {}

    // Проверки типа
    bool is_null()    const { return std::holds_alternative<std::monostate>(value); }
    bool is_string()  const { return std::holds_alternative<std::string>(value); }
    bool is_int64()   const { return std::holds_alternative<long long>(value); }
    bool is_double()  const { return std::holds_alternative<double>(value); }
    bool is_binary()  const { return std::holds_alternative<std::vector<char>>(value); }

    // Безопасные геттеры (возвращают optional)
    std::optional<std::string>      get_string()  const;
    std::optional<long long>        get_int64()   const;
    std::optional<double>           get_double()  const;
    std::optional<std::vector<char>> get_binary() const;

    // Геттеры с дефолтным значением
    std::string      as_string(const std::string& default_val = "") const;
    long long        as_int64(long long default_val = 0) const;
    double           as_double(double default_val = 0.0) const;

    // Для отладки и логов
    std::string to_string() const;

    // Сравнение
    bool operator==(const ValueWrapper& other) const;

    // Вывод в поток
    friend std::ostream& operator<<(std::ostream& os, const ValueWrapper& v);

private:
    Value value;
};

// Для удобства — можно использовать ValueWrapper вместо голого variant
using Value = ValueWrapper;
