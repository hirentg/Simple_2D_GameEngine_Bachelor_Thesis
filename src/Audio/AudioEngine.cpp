#define MINIAUDIO_IMPLEMENTATION
// Only enable the decoders we need; reduces binary size
#define MA_NO_FLAC
#define MA_NO_MP3
#include "miniaudio.h"
#ifdef PlaySound
#undef PlaySound
#endif

#include "AudioEngine.h"
#include <iostream>

AudioEngine::AudioEngine() {
    m_Engine = std::make_unique<ma_engine>();

    ma_engine_config config = ma_engine_config_init();
    ma_result result = ma_engine_init(&config, m_Engine.get());
    if (result != MA_SUCCESS) {
        std::cerr << "[AudioEngine] Failed to initialize miniaudio engine. "
                  << "Error code: " << result << std::endl;
        m_Engine.reset();
    }
}

AudioEngine::~AudioEngine() {
    StopAll();
    if (m_Engine) {
        ma_engine_uninit(m_Engine.get());
    }
}

uint32_t AudioEngine::PlaySound(const std::string& filepath, float volume,
                                 bool loop)
{
    if (!m_Engine) return 0;

    auto sound = std::make_unique<ma_sound>();
    ma_result result = ma_sound_init_from_file(
        m_Engine.get(), filepath.c_str(),
        MA_SOUND_FLAG_DECODE,  // decode at load time for low-latency playback
        nullptr, nullptr, sound.get());

    if (result != MA_SUCCESS) {
        std::cerr << "[AudioEngine] Failed to load sound: " << filepath
                  << " (error: " << result << ")" << std::endl;
        return 0;
    }

    ma_sound_set_volume(sound.get(), volume);
    ma_sound_set_looping(sound.get(), loop ? MA_TRUE : MA_FALSE);
    ma_sound_start(sound.get());

    uint32_t handle = m_NextHandle++;
    m_ActiveSounds[handle] = std::move(sound);
    return handle;
}

void AudioEngine::StopSound(uint32_t handle) {
    auto it = m_ActiveSounds.find(handle);
    if (it != m_ActiveSounds.end()) {
        ma_sound_stop(it->second.get());
        ma_sound_uninit(it->second.get());
        m_ActiveSounds.erase(it);
    }
}

void AudioEngine::StopAll() {
    for (auto& [handle, sound] : m_ActiveSounds) {
        ma_sound_stop(sound.get());
        ma_sound_uninit(sound.get());
    }
    m_ActiveSounds.clear();
}

void AudioEngine::SetMasterVolume(float volume) {
    if (m_Engine) {
        ma_engine_set_volume(m_Engine.get(), volume);
    }
}

void AudioEngine::CollectFinished(){for(auto it=m_ActiveSounds.begin();it!=m_ActiveSounds.end();){if(ma_sound_at_end(it->second.get())){ma_sound_uninit(it->second.get());it=m_ActiveSounds.erase(it);}else ++it;}}
