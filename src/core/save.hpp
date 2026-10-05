#pragma once

#include <string>
#include <unordered_map>

#include "core/config.hpp"

namespace vk {

struct SaveData {
    // Рекорды
    float bestTime  = 0.0f;
    int   bestLevel = 1;
    int   bestKills = 0;

    // Последний прогон
    float lastTime  = 0.0f;
    int   lastLevel = 1;
    int   lastKills = 0;

    // Сколько всего было прогонов
    int   totalRuns = 0;

    // Флаг: этот прогон побил рекорд (заполняется в finalizeRun)
    bool newBestTime  = false;
    bool newBestLevel = false;
    bool newBestKills = false;

    int gold = 0;
    std::unordered_map<std::string, int> permanentBonuses;   // id -> level

    // helper: сколько золота всего вложено (для отображения)
    int totalPerkLevels() const {
        int sum = 0;
        for (auto& [id, lvl] : permanentBonuses) sum += lvl;
        return sum;
    }
};

// Возвращает путь к save.json (не создаёт файл).
std::string getSavePath();

// Загружает сохранение. Если файла нет — вернёт дефолтное.
SaveData loadSaveFile();

// Записывает сохранение. Молча логирует ошибку, если не удалось.
void writeSaveFile(const SaveData& data);

// Обнуляет переданную структуру и пишет её в файл.
void clearSaveFile();

// Стоимость следующего уровня перка (level — текущий)
int perkCost(const ShopItemConfig& item, int currentLevel);
}