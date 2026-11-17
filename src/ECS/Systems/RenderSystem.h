#pragma once

#include "../Registry.h"
#include "../Components.h"
#include "../../Renderer/Renderer2D.h"

class RenderSystem {
public:
    static void Update(Registry& registry) {
        auto view = registry.GetView<TransformComponent, SpriteComponent>();

        for (EntityID entity : view) {
            auto& transform = registry.GetComponent<TransformComponent>(entity);
            auto& sprite    = registry.GetComponent<SpriteComponent>(entity);

            if (sprite.texture) {
                Renderer2D::DrawQuad(
                    transform.position,
                    transform.scale,
                    transform.rotation,
                    sprite.texture,
                    sprite.color,
                    sprite.uvRect
                );
            } else {
                // No texture — draw a flat colored quad
                Renderer2D::DrawQuad(
                    transform.position,
                    transform.scale,
                    sprite.color
                );
            }
        }
    }
};
