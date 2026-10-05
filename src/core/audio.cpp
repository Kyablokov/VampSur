#include "core/audio.hpp"

#include <cmath>
#include <raylib.h>

#include "core/config.hpp"

namespace vk {

namespace {
// Пути ищем относительно CWD и относительно ASSETS_DIR — как config.
std::string findAudioPath(const char* rel) {
    std::string name = rel;
    const std::string candidates[] = {
        "assets/audio/" + name,
        "../assets/audio/" + name,
    };
    for (const auto& c : candidates) {
        if (FileExists(c.c_str())) return c;
    }
#ifdef ASSETS_DIR
    std::string p = std::string(ASSETS_DIR) + "/audio/" + name;
    if (FileExists(p.c_str())) return p;
#endif
    return "";
}
} // namespace

Audio::~Audio() {
    shutdown();
}

void Audio::init(const AudioConfig& cfg) {
    InitAudioDevice();
    if (!IsAudioDeviceReady()) {
        TraceLog(LOG_WARNING, "Audio device not ready, running silent");
        return;
    }
    audioReady_   = true;
    masterVolume_ = cfg.masterVolume;
    sfxVolume_    = cfg.sfxVolume;
    musicVolume_  = cfg.musicVolume;

    loadSound("shoot",      "sfx/shoot.wav");
    loadSound("hit",        "sfx/hit.wav",       cfg.hitSfxMinInterval);
    loadSound("kill",       "sfx/kill.wav",      cfg.killSfxMinInterval);
    loadSound("pickup",     "sfx/pickup.wav");
    loadSound("levelup",    "sfx/levelup.wav");
    loadSound("player_hit", "sfx/player_hit.wav");
    loadSound("boss_death", "sfx/boss_death.wav");

    for (auto& [id, slot] : sounds_) {
        if (slot.loaded) {
            SetSoundVolume(slot.sound, masterVolume_ * sfxVolume_);
        }
    }
}

void Audio::shutdown() {
    if (!audioReady_) return;
    for (auto& [id, slot] : sounds_) {
        if (slot.loaded) UnloadSound(slot.sound);
    }
    sounds_.clear();
    if (musicLoaded_) {
        UnloadMusicStream(music_);
        musicLoaded_ = false;
    }
    CloseAudioDevice();
    audioReady_ = false;
}

void Audio::loadSound(const std::string& id, const char* path, float minInterval) {
    const std::string full = findAudioPath(path);
    SoundSlot slot;
    slot.minInterval = minInterval;

    if (full.empty()) {
        TraceLog(LOG_WARNING, "Audio file not found: %s (id=%s)", path, id.c_str());
        sounds_[id] = slot;   // loaded=false
        return;
    }
    slot.sound  = LoadSound(full.c_str());
    slot.loaded = IsSoundValid(slot.sound);
    if (!slot.loaded) {
        TraceLog(LOG_WARNING, "Failed to load sound: %s", full.c_str());
    }
    sounds_[id] = slot;
}

void Audio::play(const std::string& id, float pitch) {
    if (!audioReady_) return;

    auto it = sounds_.find(id);
    if (it == sounds_.end()) return;

    SoundSlot& slot = it->second;
    if (!slot.loaded) return;

    if (slot.minInterval > 0.0f) {
        const float now = static_cast<float>(GetTime());
        if (now - slot.lastPlayedAt < slot.minInterval) return;
        slot.lastPlayedAt = now;
    }

    SetSoundPitch(slot.sound, pitch);
    PlaySound(slot.sound);
}

void Audio::playMusic(const std::string& path, bool loop) {
    if (!audioReady_) return;
    if (musicLoaded_) {
        StopMusicStream(music_);
        UnloadMusicStream(music_);
        musicLoaded_ = false;
    }

    const std::string full = findAudioPath(path.c_str());
    if (full.empty()) {
        TraceLog(LOG_INFO, "No music file at %s, running silent", path.c_str());
        return;
    }

    music_ = LoadMusicStream(full.c_str());
    musicLoaded_ = IsMusicValid(music_);
    if (musicLoaded_) {
        music_.looping = loop;
        SetMusicVolume(music_, masterVolume_ * musicVolume_);
        PlayMusicStream(music_);
    }
}

void Audio::stopMusic() {
    if (!audioReady_ || !musicLoaded_) return;
    StopMusicStream(music_);
}

void Audio::setMusicPaused(bool paused) {
    if (!audioReady_ || !musicLoaded_) return;
    if (paused) PauseMusicStream(music_);
    else        ResumeMusicStream(music_);
}

bool Audio::isMusicPlaying() const {
    return musicLoaded_ && IsMusicStreamPlaying(music_);
}

void Audio::setMasterVolume(float v) {
    masterVolume_ = v;
    for (auto& [id, slot] : sounds_) {
        if (slot.loaded) SetSoundVolume(slot.sound, masterVolume_ * sfxVolume_);
    }
    if (musicLoaded_) SetMusicVolume(music_, masterVolume_ * musicVolume_);
}

// в audio.cpp
void Audio::update() {
    if (audioReady_ && musicLoaded_) UpdateMusicStream(music_);
}

} // namespace vk