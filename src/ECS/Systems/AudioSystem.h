#pragma once

#include "../Registry.h"
#include "../Components.h"
#include "../../Audio/AudioEngine.h"

class AudioSystem {
public:
    static void Update(Registry& registry, AudioEngine& audioEngine) {
        auto view = registry.GetView<AudioSourceComponent>();

        for (EntityID entity : view) {
            auto& audio = registry.GetComponent<AudioSourceComponent>(entity);

            // Auto-play on first frame if requested
            if (audio.playOnStart && !audio.isPlaying) {
                audio.soundHandle = audioEngine.PlaySound(
                    audio.clipPath, audio.volume, audio.loop);
                audio.isPlaying = true;
                audio.playOnStart = false;  // only trigger once
            }
        }
    }

    // Trigger collision sounds: play a one-shot SFX for entities involved
    // in collisions that have an AudioSourceComponent
    static void OnCollisions(
        Registry& registry, AudioEngine& audioEngine,
        const std::vector<std::pair<EntityID, EntityID>>& collisions)
    {
        for (const auto& [entityA, entityB] : collisions) {
            // Check if either entity has an audio component
            if (registry.HasComponent<AudioSourceComponent>(entityA)) {
                auto& audio = registry.GetComponent<AudioSourceComponent>(entityA);
                if (!audio.clipPath.empty() && !audio.loop) {
                    audioEngine.PlaySound(audio.clipPath, audio.volume, false);
                }
            }
            if (registry.HasComponent<AudioSourceComponent>(entityB)) {
                auto& audio = registry.GetComponent<AudioSourceComponent>(entityB);
                if (!audio.clipPath.empty() && !audio.loop) {
                    audioEngine.PlaySound(audio.clipPath, audio.volume, false);
                }
            }
        }
    }
};
