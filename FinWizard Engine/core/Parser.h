#pragma once

#include "Task.h"
#include <string>

#include <nlohmann/json.hpp>

class ConfigParser {
public:

std::optional<Task> parse(const std::string& path);
}
