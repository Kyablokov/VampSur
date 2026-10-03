#include "core/save.hpp"

#include <fstream>

#include <nlohmann/json.hpp>
#include <raylib.h>

using json = nlohmann::json;

namespace vk {

std::string getSavePath() {
    const char* candidates[] = {
        "assets/save.json",
        "../assets/save.json",
        "save.json",
    };

    for (const char* p : candidates) {
        std::string path = p;
        auto slash = path.find_last_of('/');
        std::string dir = (slash == std::string::npos) ? "." : path.substr(0, slash);

        // Проверяем существование директории
        std::ifstream test(dir + "/");
        if (!test.is_open()) continue;

        // Пробуем писать
        std::ofstream out(path, std::ios::app);
        if (out.is_open()) {
            out.close();
            return path;
        }
    }
    return "save.json";
}

SaveData loadSaveFile() {
    SaveData data;
    const std::string path = getSavePath();

    std::ifstream file(path);
    if (!file.is_open()) {
        TraceLog(LOG_INFO, "No save file at '%s', starting fresh", path.c_str());
        return data;
    }

    json j;
    try { file >> j; }
    catch (const std::exception& e) {
        TraceLog(LOG_WARNING, "Failed to parse save '%s': %s", path.c_str(), e.what());
        return data;
    }

    if (j.contains("best")) {
        const auto& b = j["best"];
        data.bestTime  = b.value("time",  data.bestTime);
        data.bestLevel = b.value("level", data.bestLevel);
        data.bestKills = b.value("kills", data.bestKills);
    }
    if (j.contains("last")) {
        const auto& l = j["last"];
        data.lastTime  = l.value("time",  data.lastTime);
        data.lastLevel = l.value("level", data.lastLevel);
        data.lastKills = l.value("kills", data.lastKills);
    }
    data.totalRuns = j.value("total_runs", data.totalRuns);

    TraceLog(LOG_INFO, "Save loaded from '%s' (runs=%d)", path.c_str(), data.totalRuns);
    return data;
}

void writeSaveFile(const SaveData& data) {
    json j;
    j["best"] = {
        { "time",  data.bestTime  },
        { "level", data.bestLevel },
        { "kills", data.bestKills },
    };
    j["last"] = {
        { "time",  data.lastTime  },
        { "level", data.lastLevel },
        { "kills", data.lastKills },
    };
    j["total_runs"] = data.totalRuns;

    const std::string path = getSavePath();
    std::ofstream out(path);
    if (!out.is_open()) {
        TraceLog(LOG_ERROR, "Cannot write save to '%s'", path.c_str());
        return;
    }
    out << j.dump(2) << std::endl;
    TraceLog(LOG_INFO, "Save written to '%s'", path.c_str());
}

} // namespace vk