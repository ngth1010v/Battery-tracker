#include "ConfigManager.h"
#include <fstream>
#include <windows.h>
#include <shlwapi.h>
#include <string>

#pragma comment(lib, "Shlwapi.lib")

ConfigManager& ConfigManager::Instance() {
    static ConfigManager inst;
    return inst;
}

ConfigManager::ConfigManager() {
    filePath = getConfigPath();
    ensureFileExists();
    loadFromFile();
    lastFileTime = getFileTime();
}

ConfigManager::~ConfigManager() {}

const ConfigManager::DataPack& ConfigManager::get() {
    if (isFileModified()) {
        loadFromFile();
        lastFileTime = getFileTime();
    }
    return cache;
}

void ConfigManager::set(const DataPack& newData) {
    cache = newData;
    saveToFile();
    lastFileTime = getFileTime();

    if (g_configEvent)
        SetEvent(g_configEvent);
}

std::string ConfigManager::getConfigPath() {
    char path[MAX_PATH];
    GetModuleFileNameA(NULL, path, MAX_PATH);

    std::string p(path);
    size_t pos = p.find_last_of("\\/");
    return p.substr(0, pos + 1) + "config.ini";
}

bool ConfigManager::ensureFileExists() {
    std::ifstream f(filePath);
    if (f.good()) return true;

    std::ofstream out(filePath);
    out << "[threshold]\nlow=30\nwarning_low=40\nwarning_high=80\nhigh=90\n";
    out << "[timing]\ninterval_check=1000\ninterval_warning=1000\n";
    out << "[feature]\nrepeat_notify=5\n";
    out << "[ui]\npopup_scale=1.0\n";
    out << "[debug]\nenable=0\n";

    return true;
}

long long ConfigManager::getFileTime() {
    WIN32_FILE_ATTRIBUTE_DATA info;
    if (!GetFileAttributesExA(filePath.c_str(), GetFileExInfoStandard, &info))
        return 0;

    ULARGE_INTEGER t;
    t.LowPart = info.ftLastWriteTime.dwLowDateTime;
    t.HighPart = info.ftLastWriteTime.dwHighDateTime;

    return (long long)t.QuadPart;
}

bool ConfigManager::isFileModified() {
    return getFileTime() != lastFileTime;
}

void ConfigManager::loadFromFile() {
    DataPack def;

    cache.LOW_THRESHOLD = readInt("threshold", "low", def.LOW_THRESHOLD);
    cache.WARNING_LOW_THRESHOLD = readInt("threshold", "warning_low", def.WARNING_LOW_THRESHOLD);
    cache.WARNING_HIGH_THRESHOLD = readInt("threshold", "warning_high", def.WARNING_HIGH_THRESHOLD);
    cache.HIGH_THRESHOLD = readInt("threshold", "high", def.HIGH_THRESHOLD);

    cache.INTERVAL_CHECK = readInt("timing", "interval_check", def.INTERVAL_CHECK);

    cache.REPEAT_NOTIFICATION_AFTER_PERCENT =
        readInt("feature", "repeat_notify", def.REPEAT_NOTIFICATION_AFTER_PERCENT);

    cache.POPUP_SCALE = readFloat("ui", "popup_scale", def.POPUP_SCALE);

    cache.DEBUGGING = readBool("debug", "enable", def.DEBUGGING);
}

void ConfigManager::saveToFile() {
    WritePrivateProfileStringA("threshold", "low", std::to_string(cache.LOW_THRESHOLD).c_str(), filePath.c_str());
    WritePrivateProfileStringA("threshold", "warning_low", std::to_string(cache.WARNING_LOW_THRESHOLD).c_str(), filePath.c_str());
    WritePrivateProfileStringA("threshold", "warning_high", std::to_string(cache.WARNING_HIGH_THRESHOLD).c_str(), filePath.c_str());
    WritePrivateProfileStringA("threshold", "high", std::to_string(cache.HIGH_THRESHOLD).c_str(), filePath.c_str());

    WritePrivateProfileStringA("timing", "interval_check", std::to_string(cache.INTERVAL_CHECK).c_str(), filePath.c_str());

    WritePrivateProfileStringA("feature", "repeat_notify", std::to_string(cache.REPEAT_NOTIFICATION_AFTER_PERCENT).c_str(), filePath.c_str());

    WritePrivateProfileStringA("ui", "popup_scale", std::to_string(cache.POPUP_SCALE).c_str(), filePath.c_str());
    WritePrivateProfileStringA("debug", "enable", cache.DEBUGGING ? "1" : "0", filePath.c_str());
}

int ConfigManager::readInt(const char* section, const char* key, int def) {
    return GetPrivateProfileIntA(section, key, def, filePath.c_str());
}

bool ConfigManager::readBool(const char* section, const char* key, bool def) {
    return GetPrivateProfileIntA(section, key, def ? 1 : 0, filePath.c_str()) != 0;
}

float ConfigManager::readFloat(const char* section, const char* key, float def) {
    char buf[32];
    GetPrivateProfileStringA(section, key, std::to_string(def).c_str(), buf, 32, filePath.c_str());
    return std::stof(buf);
}