#pragma once
#include <string>

class Notification {
public:
    static void Init();
    static void Show(const std::string& title,
                     const std::string& message);
};