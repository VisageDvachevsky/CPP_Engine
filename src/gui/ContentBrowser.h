#pragma once

#include <string>

class ContentBrowser {
public:
    ContentBrowser();
    ~ContentBrowser() = default;
    
    void show();

private:
    void showDirectoryTree();
    void showFileGrid();
    void showPathBar();
    
    std::string m_currentPath;
};