#pragma once

#include <string>
#include <vector>

class ModManager {
public:
    // Load all .lua files
    static void loadBlocks(const std::string& modsDirectory);
};