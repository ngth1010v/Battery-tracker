#pragma once
#include <string>
#include <windows.h>

extern HANDLE g_configEvent;

class ConfigManager {
public:
    struct DataPack {
        // =====================
        // THRESHOLDS
        // =====================
        int LOW_THRESHOLD = 30;
        int WARNING_LOW_THRESHOLD = 40;
        int WARNING_HIGH_THRESHOLD = 80;
        int HIGH_THRESHOLD = 90;

        // =====================
        // TIMING
        // =====================
        int INTERVAL_CHECK = 1000;

        // =====================
        // NOTIFICATION
        // =====================
        bool ACTIVE_NOTIFICATION = true;
        int REPEAT_NOTIFICATION_AFTER_PERCENT = 5;

        // =====================
        // UI
        // =====================
        float POPUP_SCALE = 1.0f;
        bool ACTIVE_POPUP = true;
    };

public:
    static ConfigManager& Instance();

    const DataPack& get();
    void set(const DataPack& newData);

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