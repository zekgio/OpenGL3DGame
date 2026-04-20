#pragma once
#include "BlockData.h"
#include "Constants.h"
#include <array>
#include <unordered_map>
#include <iostream>

class BlockRegistry {
private:
    std::array<BlockData, 256> blocks;
    std::unordered_map<std::string, uint8_t> nameToIdMap;

    BlockRegistry() {
        // L'Aria è sempre l'ID 0
        BlockData air;
        air.id = Constants::BlockType::AIR;
        air.name = "air";
        air.isTransparent = true;
        air.isSolid = false;
        registerBlock(air);
    }

public:
    // Singleton pattern per accesso globale rapido
    static BlockRegistry& get() {
        static BlockRegistry instance;
        return instance;
    }

    // Registra un blocco nel dizionario
    void registerBlock(const BlockData& block) {
        blocks[block.id] = block;
        nameToIdMap[block.name] = block.id;
    }

    // Lettura rapida tramite ID (Usata nel loop di rendering di Chunk.cpp)
    inline const BlockData& getBlock(uint8_t id) const {
        return blocks[id];
    }

    // Lettura tramite nome (Utile per comandi /give o logica Lua)
    uint8_t getIdFromName(const std::string& name) const {
        auto it = nameToIdMap.find(name);
        if (it != nameToIdMap.end()) return it->second;
        return 0; // Ritorna Aria se non trovato
    }
};