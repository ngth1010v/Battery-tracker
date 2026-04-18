#include "ConfigManager.h"
#include <fstream>
#include <windows.h>
#include <string>

#pragma comment(lib, "Shlwapi.lib")

// ======================================================
// SINGLETON
// ======================================================
ConfigManager& ConfigManager::Instance() {
    static ConfigManager inst;
    return inst;
}

// ======================================================
// INIT
// ======================================================
ConfigManager::ConfigManager() {
    filePath = getConfigPath();
    ensureFileExists();
    loadFromFile();
    lastFileTime = getFileTime();
}

ConfigManager::~ConfigManager() {}

// ======================================================
// GET / SET
// ======================================================
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

// ======================================================
// PATH
// ======================================================
std::string ConfigManager::getConfigPath() {
    char path[MAX_PATH];
    GetModuleFileNameA(NULL, path, MAX_PATH);

    std::string p(path);
    size_t pos = p.find_last_of("\\/");
    return p.substr(0, pos + 1) + "config.ini";
}

// ======================================================
// FILE CREATE (NEW FORMAT)
// ======================================================
bool ConfigManager::ensureFileExists() {
    std::ifstream f(filePath);
    if (f.good()) return true;

    std::ofstream out(filePath);

    out <<
        "; =====================\n"
        "; BATTERY TRACKER CONFIG\n"
        "; =====================\n\n"

        "[threshold]\n"
        "LOW_THRESHOLD=30\n"
        "WARNING_LOW_THRESHOLD=40\n"
        "WARNING_HIGH_THRESHOLD=80\n"
        "HIGH_THRESHOLD=90\n\n"

        "[timing]\n"
        "INTERVAL_CHECK=500\n\n"

        "[notification]\n"
        "ACTIVE_NOTIFICATION=1\n"
        "REPEAT_NOTIFICATION_AFTER_PERCENT=5\n\n"

        "[ui]\n"
        "POPUP_SCALE=1.0\n"
        "ACTIVE_POPUP=1\n";

    return true;
}

// ======================================================
// FILE TIME CHECK
// ======================================================
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

// ======================================================
// LOAD CONFIG
// ======================================================
void ConfigManager::loadFromFile() {
    DataPack def;

    cache.LOW_THRESHOLD =
        readInt("threshold", "LOW_THRESHOLD", def.LOW_THRESHOLD);

    cache.WARNING_LOW_THRESHOLD =
        readInt("threshold", "WARNING_LOW_THRESHOLD", def.WARNING_LOW_THRESHOLD);

    cache.WARNING_HIGH_THRESHOLD =
        readInt("threshold", "WARNING_HIGH_THRESHOLD", def.WARNING_HIGH_THRESHOLD);

    cache.HIGH_THRESHOLD =
        readInt("threshold", "HIGH_THRESHOLD", def.HIGH_THRESHOLD);

    cache.INTERVAL_CHECK =
        readInt("timing", "INTERVAL_CHECK", def.INTERVAL_CHECK);

    cache.ACTIVE_NOTIFICATION =
        readBool("notification", "ACTIVE_NOTIFICATION", def.ACTIVE_NOTIFICATION);

    cache.REPEAT_NOTIFICATION_AFTER_PERCENT =
        readInt("notification", "REPEAT_NOTIFICATION_AFTER_PERCENT",
                 def.REPEAT_NOTIFICATION_AFTER_PERCENT);

    cache.POPUP_SCALE =
        readFloat("ui", "POPUP_SCALE", def.POPUP_SCALE);

    cache.ACTIVE_POPUP =
        readBool("ui", "ACTIVE_POPUP", def.ACTIVE_POPUP);
}

// ======================================================
// SAVE CONFIG
// ======================================================
void ConfigManager::saveToFile() {
    WritePrivateProfileStringA("threshold", "LOW_THRESHOLD",
        std::to_string(cache.LOW_THRESHOLD).c_str(), filePath.c_str());

    WritePrivateProfileStringA("threshold", "WARNING_LOW_THRESHOLD",
        std::to_string(cache.WARNING_LOW_THRESHOLD).c_str(), filePath.c_str());

    WritePrivateProfileStringA("threshold", "WARNING_HIGH_THRESHOLD",
        std::to_string(cache.WARNING_HIGH_THRESHOLD).c_str(), filePath.c_str());

    WritePrivateProfileStringA("threshold", "HIGH_THRESHOLD",
        std::to_string(cache.HIGH_THRESHOLD).c_str(), filePath.c_str());

    WritePrivateProfileStringA("timing", "INTERVAL_CHECK",
        std::to_string(cache.INTERVAL_CHECK).c_str(), filePath.c_str());

    WritePrivateProfileStringA("notification", "ACTIVE_NOTIFICATION",
        cache.ACTIVE_NOTIFICATION ? "1" : "0", filePath.c_str());

    WritePrivateProfileStringA("notification", "REPEAT_NOTIFICATION_AFTER_PERCENT",
        std::to_string(cache.REPEAT_NOTIFICATION_AFTER_PERCENT).c_str(), filePath.c_str());

    WritePrivateProfileStringA("ui", "POPUP_SCALE",
        std::to_string(cache.POPUP_SCALE).c_str(), filePath.c_str());

    WritePrivateProfileStringA("ui", "ACTIVE_POPUP",
        cache.ACTIVE_POPUP ? "1" : "0", filePath.c_str());
}

// ======================================================
// READ HELPERS
// ======================================================
int ConfigManager::readInt(const char* section, const char* key, int def) {
    return GetPrivateProfileIntA(section, key, def, filePath.c_str());
}

bool ConfigManager::readBool(const char* section, const char* key, bool def) {
    return GetPrivateProfileIntA(section, key, def ? 1 : 0, filePath.c_str()) != 0;
}

float ConfigManager::readFloat(const char* section, const char* key, float def) {
    char buf[32];
    GetPrivateProfileStringA(section, key, std::to_string(def).c_str(),
        buf, 32, filePath.c_str());
    return std::stof(buf);
}