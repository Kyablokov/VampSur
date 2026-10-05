#pragma once

#include <string>
#include <unordered_map>
#include <raylib.h>

namespace vk {

struct AudioConfig;

// Обёртка над raylib audio. Хранит звуки по id, умеет их проигрывать
// с питч-варированием и throttling'ом.
class Audio {
public:
    Audio() = default;
    ~Audio();

    // Инициализация: открывает аудио-устройство, грузит файлы.
    // Должна вызываться после InitWindow.
    void init(const AudioConfig& cfg);

    // Закрытие. Должно вызываться ДО CloseWindow.
    void shutdown();
    // в audio.hpp
    void update();

    // Проигрывает звук по id. pitch — множитель (1.0 = без изменений).
    // Учитывает master/sfx volume и throttling для hit/kill.
    void play(const std::string& id, float pitch = 1.0f);

    // Управление музыкой.
    void playMusic(const std::string& path, bool loop = true);
    void stopMusic();
    void setMusicPaused(bool paused);
    bool isMusicPlaying() const;

    // Громкость
    void setMasterVolume(float v);
    float masterVolume() const { return masterVolume_; }

private:
    struct SoundSlot {
        Sound       sound{};
        float       lastPlayedAt = -1000.0f;   // для throttling
        float       minInterval  = 0.0f;        // 0 = без throttle
        bool        loaded       = false;
    };

    void loadSound(const std::string& id, const char* path, float minInterval = 0.0f);

    std::unordered_map<std::string, SoundSlot> sounds_;
    Music  music_{};
    bool   musicLoaded_    = false;
    bool   audioReady_     = false;
    float  masterVolume_   = 1.0f;
    float  sfxVolume_      = 1.0f;
    float  musicVolume_    = 0.35f;
};

}