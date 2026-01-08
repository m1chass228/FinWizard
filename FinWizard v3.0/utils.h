#ifndef UTILS_H
#define UTILS_H
#pragma once
#endif // UTILS_H

#include <string>
#include <cstdlib>
#include <filesystem>
#include <vector>
#include <QDebug>
#include <QString>
#include <QProcess>
#include <iostream>

namespace fs = std::filesystem;

#ifdef _WIN32
#include <windows.h>
#include <shlobj.h>
#endif

extern std::string base_folder;
extern fs::path dir_on_desktop;
std::string get_desktop_path();
std::vector<fs::path> xlsx_in_path(const fs::path& folder);
void rm_rf_xlsx(const fs::path& folder);



