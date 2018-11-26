/*
    Copyright (c) 2018 Piotr Doan. All rights reserved.
*/

#include "Precompiled.hpp"
#include "Editor/GameStateEditor.hpp"
#include "Game/GameState.hpp"
using namespace Editor;

GameStateEditor::GameStateEditor() :
    m_gameState(nullptr),
    m_updateRateSlider(0.0f),
    m_initialized(false)
{
}

GameStateEditor::~GameStateEditor()
{
}

GameStateEditor::GameStateEditor(GameStateEditor&& other)
{
    *this = std::move(other);
}

GameStateEditor& GameStateEditor::operator=(GameStateEditor&& other)
{
    std::swap(m_gameState, other.m_gameState);
    std::swap(m_receiverDestruction, other.m_receiverDestruction);
    std::swap(m_updateRateSlider, other.m_updateRateSlider);
    std::swap(m_initialized, other.m_initialized);

    return *this;
}

bool GameStateEditor::Initialize()
{
    LOG() << "Initializing game state editor..." << LOG_INDENT();

    // Make sure class instance has not been initialized yet.
    ASSERT(!m_initialized, "Game state editor instance has already been initialized!");

    // Setup receiver to clear game state when it gets destructed.
    m_receiverDestruction.Bind([this]()
    {
        m_gameState = nullptr;
    });

    // Success!
    return m_initialized = true;
}

void GameStateEditor::Update(float timeDelta)
{
    ASSERT(m_initialized, "Game state editor has not been initialized yet!");

    // Do not create a window if game state reference is not set.
    if(!m_gameState)
        return;

    // Show game state window.
    ImGui::SetNextWindowSizeConstraints(ImVec2(200.0f, 0.0f), ImVec2(FLT_MAX, FLT_MAX));

    if(ImGui::Begin("Game State", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        if(ImGui::CollapsingHeader("Core"))
        {
            if(ImGui::TreeNode("Update Timer"))
            {
                // Show update controls.
                float currentUpdateTime = m_gameState->GetUpdateTime();
                float currentUpdateRate = 1.0f / currentUpdateTime;
                ImGui::BulletText("Update time: %fs (%.1f update rate)", currentUpdateTime, currentUpdateRate);

                ImGui::SliderFloat("##UpdateRateSlider", &m_updateRateSlider, 1.0f, 100.0f, "%.1f updates per second", 2.0f);
                ImGui::SameLine();

                if(ImGui::Button("Apply"))
                {
                    float newUpdateTime = 1.0f / m_updateRateSlider;
                    m_gameState->SetUpdateTime(newUpdateTime);
                }

                // Show update histogram.
                ImGui::BulletText("Update histogram:");
                ImGui::PlotHistogram("##UpdateHistogram", nullptr, nullptr, 0);

                ImGui::TreePop();
            }
        }
    }
    ImGui::End();
}

void GameStateEditor::SetGameState(Game::GameState* gameState)
{
    ASSERT(m_initialized, "Game state editor has not been initialized yet!");

    if(gameState)
    {
        // Replace with new game state reference.
        m_gameState = gameState;
        m_receiverDestruction.Subscribe(gameState->events.instanceDestructed);

        // Update initial widget values.
        m_updateRateSlider = 1.0f / m_gameState->GetUpdateTime();
    }
    else
    {
        // Remove game state reference.
        m_gameState = nullptr;
        m_receiverDestruction.Unsubscribe();
    }
}

Game::GameState* GameStateEditor::GetGameState() const
{
    return m_gameState;
}