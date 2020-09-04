/*
    Copyright (c) 2018-2020 Piotr Doan. All rights reserved.
*/

#include "Renderer/Precompiled.hpp"
#include "Renderer/GameRenderer.hpp"
#include <Graphics/RenderContext.hpp>
#include <Graphics/Sprite/SpriteRenderer.hpp>
#include <Game/Components/TransformComponent.hpp>
#include <Game/Components/CameraComponent.hpp>
#include <Game/Components/SpriteComponent.hpp>
#include <Game/Components/SpriteAnimationComponent.hpp>
#include <Game/GameInstance.hpp>
using namespace Renderer;

GameRenderer::GameRenderer() = default;
GameRenderer::~GameRenderer() = default;

GameRenderer::CreateResult GameRenderer::Create(const CreateFromParams& params)
{
    LOG("Creating game renderer...");
    LOG_SCOPED_INDENT();

    // Validate arguments
    CHECK_ARGUMENT_OR_RETURN(params.services != nullptr, Common::Failure(CreateErrors::InvalidArgument));

    // Create instance.
    auto instance = std::unique_ptr<GameRenderer>(new GameRenderer());

    // Save system references.
    instance->m_renderContext = params.services->GetRenderContext();
    instance->m_spriteRenderer = params.services->GetSpriteRenderer();

    // Success!
    return Common::Success(std::move(instance));
}

void GameRenderer::Draw(const DrawParams& drawParams)
{
    // Clear frame buffer.
    m_renderContext->GetState().Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Checks if game instance is null.
    if(!drawParams.gameInstance)
    {
        LOG_WARNING("Attempted to draw null game instance!");
        return;
    }

    // Make sure time alpha is normalized.
    ASSERT(drawParams.timeAlpha >= 0.0f && drawParams.timeAlpha <= 1.0f, "Time alpha is not normalized!");

    // Get game instance systems.
    auto& entitySystem = drawParams.gameInstance->entitySystem;
    auto& componentSystem = drawParams.gameInstance->componentSystem;
    auto& identitySystem = drawParams.gameInstance->identitySystem;

    // Update sprite components for rendering.
    for(auto& spriteAnimationComponent : componentSystem->GetPool<Game::SpriteAnimationComponent>())
    {
        // Update sprite texture view using currently playing animation.
        if(spriteAnimationComponent.IsPlaying())
        {
            Game::SpriteComponent* spriteComponent = spriteAnimationComponent.GetSpriteComponent();

            auto spriteAnimation = spriteAnimationComponent.GetSpriteAnimation();
            float animationTime = spriteAnimationComponent.CalculateAnimationTime(drawParams.timeAlpha);

            ASSERT(spriteAnimation, "Sprite animation is null despite being played!");
            spriteComponent->SetTextureView(spriteAnimation->GetFrameByTime(animationTime).textureView);
        }
    }

    // Push render state.
    auto& renderState = m_renderContext->PushState();
    SCOPE_GUARD(m_renderContext->PopState());

    // Setup drawing viewport.
    renderState.Viewport(
        drawParams.viewportRect.x,
        drawParams.viewportRect.y,
        drawParams.viewportRect.z,
        drawParams.viewportRect.w
    );

    // Base camera transform.
    glm::mat4 cameraTransform(1.0f);

    // Retrieve transform from camera entity.
    auto cameraEntityResult = identitySystem->GetEntityByName(drawParams.cameraName);

    if(cameraEntityResult.IsSuccess())
    {
        auto cameraComponent = componentSystem->Lookup<Game::CameraComponent>(cameraEntityResult.Unwrap());

        if(cameraComponent != nullptr)
        {
            // Calculate viewport size.
            glm::ivec2 viewportSize;
            viewportSize.x = drawParams.viewportRect.z - drawParams.viewportRect.x;
            viewportSize.y = drawParams.viewportRect.w - drawParams.viewportRect.y;

            // Calculate camera transform.
            cameraTransform = cameraComponent->CalculateTransform(viewportSize);
        }
        else
        {
            LOG_WARNING("Could not retrieve camera component from \"{}\" entity.", drawParams.cameraName);
        }
    }
    else
    {
        LOG_WARNING("Could not retrieve \"{}\" camera entity.", drawParams.cameraName);
    }

    // Create list of sprites that will be drawn.
    Graphics::SpriteDrawList spriteDrawList;

    // Get all sprite components.
    for(auto& spriteComponent : componentSystem->GetPool<Game::SpriteComponent>())
    {
        // Get transform component.
        Game::TransformComponent* transformComponent = spriteComponent.GetTransformComponent();
        ASSERT(transformComponent != nullptr, "Required transform component is missing!");

        // Add sprite to the draw list.
        Graphics::Sprite sprite;
        sprite.info.texture = spriteComponent.GetTextureView().GetTexturePtr();
        sprite.info.transparent = spriteComponent.IsTransparent();
        sprite.info.filtered = spriteComponent.IsFiltered();
        sprite.data.transform = transformComponent->CalculateMatrix(drawParams.timeAlpha);
        sprite.data.rectangle = spriteComponent.GetRectangle();
        sprite.data.coords = spriteComponent.GetTextureView().GetTextureRect();
        sprite.data.color = spriteComponent.GetColor();

        spriteDrawList.AddSprite(sprite);
    }

    // Sort sprite draw list.
    spriteDrawList.SortSprites();

    // Draw sprite components.
    m_spriteRenderer->DrawSprites(spriteDrawList, cameraTransform);
}
