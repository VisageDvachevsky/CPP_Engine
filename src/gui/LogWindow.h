#pragma once

class LogWindow {
public:
    LogWindow() = default;
    ~LogWindow() = default;
    
    void show();

private:
    bool m_autoScroll = true;
    int m_selectedLevel = 0; // 0=All, 1=Info+, 2=Warn+, 3=Error only
};