/*
    Copyright (c) 2018 Piotr Doan. All rights reserved.
*/

#include "Precompiled.hpp"
#include "Renderer/StateRenderer.hpp"
#include "Game/Components/TransformComponent.hpp"
#include "Game/Components/CameraComponent.hpp"
#include "Game/Components/SpriteComponent.hpp"
#include "Game/GameState.hpp"
#include "Engine.hpp"
using namespace Renderer;

StateRenderer::DrawParams::DrawParams() :
    gameState(nullptr),
    cameraName("Camera"),
    viewportRect(0.0f, 0.0f, 0.0f, 0.0f),
    timeAlpha(1.0f)
{
}

StateRenderer::StateRenderer() :
    m_engine(nullptr),
    m_initialized(false)
{
}

StateRenderer::~StateRenderer()
{
}

StateRenderer::StateRenderer(StateRenderer&& other)
{
    *this = std::move(other);
}

StateRenderer& StateRenderer::operator=(StateRenderer&& other)
{
    std::swap(m_engine, other.m_engine);
    std::swap(m_initialized, other.m_initialized);

    return *this;
}

bool StateRenderer::Initialize(Engine::Root* engine)
{
    LOG() << "Initializing state renderer..." << LOG_INDENT();

    // Make sure class instance has not been already initialized.
    ASSERT(!m_initialized, "State renderer has already been initialized!");

    // Reset class instance if initialization fails.
    SCOPE_GUARD_IF(!m_initialized, *this = StateRenderer());

    // Validate and save engine reference.
    if(engine == nullptr)
    {
        LOG_ERROR() << "Invalid argument - \"engine\" is nullptr!";
        return false;
    }

    m_engine = engine;

    // Success!
    return m_initialized = true;
}

void StateRenderer::Draw(const DrawParams& drawParams)
{
    ASSERT(m_initialized, "State renderer has not been initialized yet!");

    // Checks if game state is null.
    if(drawParams.gameState == nullptr)
    {
        LOG_WARNING() << "Attempted to draw null game state!";
        return;
    }

    // Push the render state.
    auto& renderState = m_engine->renderContext.PushState();
    SCOPE_GUARD(m_engine->renderContext.PopState());

    // Setup the drawing viewport.
    renderState.Viewport(
        drawParams.viewportRect.x,
        drawParams.viewportRect.y,
        drawParams.viewportRect.z,
        drawParams.viewportRect.w
    );

    // Get game state and its systems.
    auto& entitySystem = drawParams.gameState->entitySystem;
    auto& componentSystem = drawParams.gameState->componentSystem;
    auto& identitySystem = drawParams.gameState->identitySystem;

    // Base camera transform.
    glm::mat4 cameraTransform(1.0f);

    // Retrieve transform from camera entity.
    Game::EntityHandle cameraEntity = identitySystem.GetEntityByName(drawParams.cameraName);

    if(entitySystem.IsHandleValid(cameraEntity))
    {
        auto cameraComponent = componentSystem.Lookup<Game::CameraComponent>(cameraEntity);

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
            LOG_WARNING() << "Could not retrieve camera component from \"" << drawParams.cameraName << "\" entity.";
        }
    }
    else
    {
        LOG_WARNING() << "Could not retrieve \"" << drawParams.cameraName << "\" camera entity.";
    }

    // Clear the frame buffer.
    m_engine->renderContext.GetState().Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Create a list of sprites that will be drawn.
    Graphics::SpriteDrawList spriteDrawList;

    // Get all sprite components.
    for(auto& spriteComponent : componentSystem.GetPool<Game::SpriteComponent>())
    {
        // Get the transform component.
        Game::TransformComponent* transformComponent = spriteComponent.GetTransformComponent();
        ASSERT(transformComponent != nullptr, "Required transform component is missing!");

        // Add a sprite to the draw list.
        Graphics::Sprite sprite;
        sprite.info.texture = spriteComponent.GetTextureView().GetTexturePtr();
        sprite.info.transparent = spriteComponent.IsTransparent();
        sprite.info.filtered = spriteComponent.IsFiltered();
        sprite.data.transform = transformComponent->CalculateMatrix();
        sprite.data.rectangle = spriteComponent.GetRectangle();
        sprite.data.coords = spriteComponent.GetTextureView().GetTextureRect();
        sprite.data.color = spriteComponent.GetColor();

        spriteDrawList.AddSprite(sprite);
    }

    // Sort the sprite draw list.
    spriteDrawList.SortSprites();

    // Draw sprite components.
    m_engine->spriteRenderer.DrawSprites(spriteDrawList, cameraTransform);
}