#include "ModManager.h"
#include "BlockRegistry.h"
#include <sol/sol.hpp>
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

void ModManager::loadBlocks(const std::string& modsDirectory) {
    sol::state lua;

    lua.open_libraries(sol::lib::base, sol::lib::package, sol::lib::string, sol::lib::math);

    // Check folder
    if (!fs::exists(modsDirectory)) {
        fs::create_directories(modsDirectory);
        return;
    }

    for (const auto& entry : fs::directory_iterator(modsDirectory)) {
        if (entry.is_regular_file() && entry.path().extension() == ".lua") {
            try {
				// Execute the Lua script
                lua.script_file(entry.path().string());

                sol::table blockTable = lua["block"];
                if (blockTable.valid()) {
                    BlockData newBlock;

					// Read properties
                    newBlock.id = blockTable["id"];
                    newBlock.name = blockTable["name"];
                    newBlock.isTransparent = blockTable.get_or("isTransparent", false);
                    newBlock.isSolid = blockTable.get_or("isSolid", true);
                    newBlock.lightEmission = blockTable.get_or("lightEmission", 0);

					// Extract texture indices
                    sol::table texTable = blockTable["textures"];
                    if (texTable.valid()) {
                        // Order: FRONT, BACK, LEFT, RIGHT, TOP, BOTTOM
                        newBlock.textureIndices[0] = texTable.get_or("front", 0);
                        newBlock.textureIndices[1] = texTable.get_or("back", 0);
                        newBlock.textureIndices[2] = texTable.get_or("left", 0);
                        newBlock.textureIndices[3] = texTable.get_or("right", 0);
                        newBlock.textureIndices[4] = texTable.get_or("top", 0);
                        newBlock.textureIndices[5] = texTable.get_or("bottom", 0);
                    }

					// Add to registry
                    BlockRegistry::get().registerBlock(newBlock);
                }
            }
            catch (const sol::error& e) {
                std::cerr << "[ModManager] Errore Lua in " << entry.path().filename() << ":\n" << e.what() << "\n";
            }
        }
    }
}