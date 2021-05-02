/*
    Copyright (c) 2018-2021 Piotr Doan. All rights reserved.
    Software distributed under the permissive MIT License.
*/

#include "Graphics/Precompiled.hpp"
#include "Graphics/RenderContext.hpp"
#include <Core/ServiceStorage.hpp>
#include <System/Window.hpp>
using namespace Graphics;

RenderContext::RenderContext() = default;
RenderContext::~RenderContext() = default;

RenderContext::CreateResult RenderContext::Create()
{
    LOG("Creating rendering context...");
    LOG_SCOPED_INDENT();

    auto instance = std::unique_ptr<RenderContext>(new RenderContext());
    return Common::Success(std::move(instance));
}

bool RenderContext::OnAttach(const Core::ServiceStorage* serviceStorage)
{
    m_window = serviceStorage->Locate<System::Window>();
    if(!m_window)
    {
        LOG_ERROR("Failed to locate window service!");
        return false;
    }

    // Save initial render state.
    // Window has to set its OpenGL context as current for this to succeed.
    // Here we assume that at this point OpenGL context is in pristine state.
    // Maybe default render state should be collected immediately when context is created?
    m_window->MakeContextCurrent();
    m_currentState.Save();

    return true;
}

void RenderContext::MakeCurrent()
{
    m_window->MakeContextCurrent();
}

RenderState& RenderContext::PushState()
{
    // Push copy of current state.
    m_pushedStates.push(m_currentState);

    // Return current state for convenience.
    return m_currentState;
}

RenderState& RenderContext::GetState()
{
    return m_currentState;
}

void RenderContext::PopState()
{
    ASSERT(!m_pushedStates.empty(), "Trying to pop non existing render state!");

    // Apply changes from the stack below.
    m_currentState.Apply(m_pushedStates.top());

    // Remove applied state.
    m_pushedStates.pop();
}
