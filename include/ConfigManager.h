#pragma once
#include <string>
#include <windows.h>

HANDLE g_configEvent = NULL;

class ConfigManager {
public:
    bool changed = false;

    struct DataPack {
        
        int LOW_THRESHOLD = 30;
        int WARNING_LOW_THRESHOLD = 40;
        int WARNING_HIGH_THRESHOLD = 80;
        int HIGH_THRESHOLD = 90;

        int INTERVAL_CHECK = 5000;
        int INTERVAL_CHECK_WHEN_WARNING = 1000;

        bool ACTIVE_NOTIFICATION = true;
        int REPEAT_NOTIFICATION_AFTER_PERCENT = 5;

        bool ACTIVE_POPUP = true;
        float POPUP_SCALE = 1.0f;

        bool DEBUGGING = true;
    };

public:
    static ConfigManager& Instance();

    const DataPack& get();
    void set(const DataPack& newData);
    static bool IsAnyActive(const ConfigManager::DataPack& cfg)
    {
        return cfg.ACTIVE_NOTIFICATION || cfg.ACTIVE_POPUP;
    }

private:
    ConfigManager();
    ~ConfigManager();

    void loadFromFile();
    void saveToFile();

    std::string getConfigPath();
    bool ensureFileExists();
    bool isFileModified();
    long long getFileTime();

    int readInt(const char* section, const char* key, int def);
    bool readBool(const char* section, const char* key, bool def);
    float readFloat(const char* section, const char* key, float def);


private:
    DataPack cache;
    std::string filePath;
    long long lastFileTime = 0;
};