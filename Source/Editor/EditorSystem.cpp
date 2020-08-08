/*
    Copyright (c) 2018-2020 Piotr Doan. All rights reserved.
*/

#include "Precompiled.hpp"
#include "Editor/EditorSystem.hpp"
#include <System/InputManager.hpp>
#include <Game/GameInstance.hpp>
using namespace Editor;

namespace
{
    // Callback function for setting clipboard text.
    void SetClipboardTextCallback(void* userData, const char* text)
    {
        ASSERT(userData != nullptr, "User data argument is nullptr!");
        glfwSetClipboardString((GLFWwindow*)userData, text);
    }

    // Callback function for getting clipboard text.
    const char* GetClipboardTextCallback(void* userData)
    {
        ASSERT(userData != nullptr, "User data argument is nullptr!");
        return glfwGetClipboardString((GLFWwindow*)userData);
    }
}

EditorSystem::EditorSystem()
{
    // Bind event receivers.
    m_receiverInputStateChanged.Bind<EditorSystem, &EditorSystem::OnInputStateChanged>(this);
    m_receiverTextInput.Bind<EditorSystem, &EditorSystem::OnTextInput>(this);
    m_receiverKeyboardKey.Bind<EditorSystem, &EditorSystem::OnKeyboardKey>(this);
    m_receiverMouseButton.Bind<EditorSystem, &EditorSystem::OnMouseButton>(this);
    m_receiverMouseScroll.Bind<EditorSystem, &EditorSystem::OnMouseScroll>(this);
    m_receiverCursorPosition.Bind<EditorSystem, &EditorSystem::OnCursorPosition>(this);
}

EditorSystem::~EditorSystem()
{
    if(m_interface)
    {
        ImGui::DestroyContext(m_interface);
        m_interface = nullptr;
    }
}

EditorSystem::CreateResult EditorSystem::Create(const CreateFromParams& params)
{
    LOG("Creating editor system...");
    LOG_SCOPED_INDENT();

    // Validate arguments.
    CHECK_ARGUMENT_OR_RETURN(params.services != nullptr, Common::Failure(CreateErrors::InvalidArgument));

    // Acquire engine services.
    System::Window* window = params.services->GetWindow();
    System::InputManager* inputManager = params.services->GetInputManager();

    // Create instance.
    auto instance = std::unique_ptr<EditorSystem>(new EditorSystem());

    // Save window reference.
    instance->m_window = window;

    // Create ImGui context.
    instance->m_interface = ImGui::CreateContext();

    if(instance->m_interface == nullptr)
    {
        LOG_ERROR("Failed to initialize user interface context!");
        return Common::Failure(CreateErrors::FailedContextCreation);
    }

    // Setup user interface.
    ImGuiIO& io = ImGui::GetIO();

    // Disable writing of INI config in the working directory.
    // This file would hold the layout of windows, but we plan
    // on doing it differently and reading it elsewhere.
    io.IniFilename = nullptr;

    // Setup interface input.
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
    io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;

    io.KeyMap[ImGuiKey_Tab] = System::KeyboardKeys::KeyTab;
    io.KeyMap[ImGuiKey_LeftArrow] = System::KeyboardKeys::KeyLeft;
    io.KeyMap[ImGuiKey_RightArrow] = System::KeyboardKeys::KeyRight;
    io.KeyMap[ImGuiKey_UpArrow] = System::KeyboardKeys::KeyUp;
    io.KeyMap[ImGuiKey_DownArrow] = System::KeyboardKeys::KeyDown;
    io.KeyMap[ImGuiKey_PageUp] = System::KeyboardKeys::KeyPageUp;
    io.KeyMap[ImGuiKey_PageDown] = System::KeyboardKeys::KeyPageDown;
    io.KeyMap[ImGuiKey_Home] = System::KeyboardKeys::KeyHome;
    io.KeyMap[ImGuiKey_End] = System::KeyboardKeys::KeyEnd;
    io.KeyMap[ImGuiKey_Insert] = System::KeyboardKeys::KeyInsert;
    io.KeyMap[ImGuiKey_Delete] = System::KeyboardKeys::KeyDelete;
    io.KeyMap[ImGuiKey_Backspace] = System::KeyboardKeys::KeyBackspace;
    io.KeyMap[ImGuiKey_Space] = System::KeyboardKeys::KeySpace;
    io.KeyMap[ImGuiKey_Enter] = System::KeyboardKeys::KeyEnter;
    io.KeyMap[ImGuiKey_Escape] = System::KeyboardKeys::KeyEscape;
    io.KeyMap[ImGuiKey_A] = System::KeyboardKeys::KeyA;
    io.KeyMap[ImGuiKey_C] = System::KeyboardKeys::KeyC;
    io.KeyMap[ImGuiKey_V] = System::KeyboardKeys::KeyV;
    io.KeyMap[ImGuiKey_X] = System::KeyboardKeys::KeyX;
    io.KeyMap[ImGuiKey_Y] = System::KeyboardKeys::KeyY;
    io.KeyMap[ImGuiKey_Z] = System::KeyboardKeys::KeyZ;

    io.SetClipboardTextFn = SetClipboardTextCallback;
    io.GetClipboardTextFn = GetClipboardTextCallback;
    io.ClipboardUserData = window->GetPrivateHandle();

    // Subscribe to input events.
    // Call on input state changed method once after subscribing.
    bool subscriptionResult = inputManager->events.inputStateChanged.Subscribe(instance->m_receiverInputStateChanged);

    if(!subscriptionResult)
    {
        LOG_ERROR("Failed to subscribe to event receivers!");
        return Common::Failure(CreateErrors::FailedEventSubscription);
    }

    instance->OnInputStateChanged(inputManager->GetInputState().get());

    // Create editor renderer.
    EditorRenderer::CreateFromParams editorRendererParams;
    editorRendererParams.services = params.services;

    instance->m_editorRenderer = EditorRenderer::Create(editorRendererParams).UnwrapOr(nullptr);
    if(instance->m_editorRenderer == nullptr)
    {
        LOG_ERROR("Could not create editor renderer!");
        return Common::Failure(CreateErrors::FailedSubsystemCreation);
    }

    // Create editor shell.
    EditorShell::CreateFromParams editorShellParams;
    editorShellParams.services = params.services;

    instance->m_editorShell = EditorShell::Create(editorShellParams).UnwrapOr(nullptr);
    if(instance->m_editorShell == nullptr)
    {
        LOG_ERROR("Could not create editor shell!");
        return Common::Failure(CreateErrors::FailedSubsystemCreation);
    }

    // Success!
    return Common::Success(std::move(instance));
}

void EditorSystem::Update(float timeDelta)
{
    // Set context as current.
    ImGui::SetCurrentContext(m_interface);
    ImGuiIO& io = ImGui::GetIO();

    // Set current delta time.
    io.DeltaTime = timeDelta;

    // Set current display size.
    io.DisplaySize.x = (float)m_window->GetWidth();
    io.DisplaySize.y = (float)m_window->GetHeight();

    // Start new interface frame.
    ImGui::NewFrame();

    // Update editor shell.
    m_editorShell->Update(timeDelta);
}

void EditorSystem::Draw()
{
    // Ends current interface frame.
    ImGui::EndFrame();

    // Set context and draw the editor interface.
    ImGui::SetCurrentContext(m_interface);
    m_editorRenderer->Draw();
}

void EditorSystem::OnInputStateChanged(System::InputState* inputState)
{
    if(inputState == nullptr)
        return;

    // We insert receivers in front of dispatcher queue as we want to have priority for input events.
    Event::SubscriptionPolicy subscriptionPolicy = Event::SubscriptionPolicy::ReplaceSubscription;
    Event::PriorityPolicy priorityPolicy = Event::PriorityPolicy::InsertFront;

    inputState->events.keyboardKey.Subscribe(m_receiverKeyboardKey, subscriptionPolicy, priorityPolicy);
    inputState->events.textInput.Subscribe(m_receiverTextInput, subscriptionPolicy, priorityPolicy);
    inputState->events.mouseButton.Subscribe(m_receiverMouseButton, subscriptionPolicy, priorityPolicy);
    inputState->events.mouseScroll.Subscribe(m_receiverMouseScroll, subscriptionPolicy, priorityPolicy);
    inputState->events.cursorPosition.Subscribe(m_receiverCursorPosition, subscriptionPolicy, priorityPolicy);
}

bool EditorSystem::OnTextInput(const System::InputEvents::TextInput& event)
{
    // Set context as current.
    ImGui::SetCurrentContext(m_interface);
    ImGuiIO& io = ImGui::GetIO();

    // Convert character from UTF-32 to UTF-8 encoding.
    // We will need an array for four UTF-8 characters and a null terminator.
    char utf8Character[5] = { 0 };

    ASSERT(utf8::internal::is_code_point_valid(event.utf32Character), "Invalid UTF-32 encoding!");
    utf8::unchecked::utf32to8(&event.utf32Character, &event.utf32Character + 1, &utf8Character[0]);

    // Add text input character.
    io.AddInputCharactersUTF8(&utf8Character[0]);

    // Prevent input from passing through.
    return io.WantCaptureKeyboard;
}

bool EditorSystem::OnKeyboardKey(const System::InputEvents::KeyboardKey& event)
{
    // Set context as current.
    ImGui::SetCurrentContext(m_interface);
    ImGuiIO& io = ImGui::GetIO();

    // Make sure that the array is of an expected size.
    const size_t MaxKeyboardKeyCount = Common::StaticArraySize(io.KeysDown);
    ASSERT(MaxKeyboardKeyCount >= System::KeyboardKeys::Count, "Insufficient ImGUI keyboard state array size!");

    // We can only handle a specific number of keys.
    if(event.key < 0 || event.key >= MaxKeyboardKeyCount)
        return false;

    // Change key state.
    io.KeysDown[event.key] = (event.state == System::InputStates::Pressed);

    // Change states of key modifiers.
    io.KeyAlt = event.modifiers & System::KeyboardModifiers::Alt;
    io.KeyCtrl = event.modifiers & System::KeyboardModifiers::Ctrl;
    io.KeyShift = event.modifiers & System::KeyboardModifiers::Shift;
    io.KeySuper = event.modifiers & System::KeyboardModifiers::Super;

    // Prevent input from passing through.
    return io.WantCaptureKeyboard;
}

bool EditorSystem::OnMouseButton(const System::InputEvents::MouseButton& event)
{
    // Set context as current.
    ImGui::SetCurrentContext(m_interface);
    ImGuiIO& io = ImGui::GetIO();

    // Determine number of supported mouse buttons.
    const std::size_t SupportedMouseButtonCount = std::min(
        Common::StaticArraySize(io.MouseDown),
        (std::size_t)System::MouseButtons::Count
    );

    // We can only handle specific number of mouse buttons.
    if(event.button < System::MouseButtons::Button1)
        return false;

    if(event.button >= System::MouseButtons::Button1 + SupportedMouseButtonCount)
        return false;

    // Set mouse button state.
    const unsigned int MouseButtonIndex = event.button - System::MouseButtons::Button1;
    io.MouseDown[MouseButtonIndex] = (event.state == System::InputStates::Pressed);

    // Prevent input from passing through.
    return io.WantCaptureMouse;
}

bool EditorSystem::OnMouseScroll(const System::InputEvents::MouseScroll& event)
{
    // Set context as current.
    ImGui::SetCurrentContext(m_interface);
    ImGuiIO& io = ImGui::GetIO();

    // Set mouse wheel offset.
    io.MouseWheel = (float)event.offset;

    // Prevent input from passing through.
    return io.WantCaptureMouse;
}

void EditorSystem::OnCursorPosition(const System::InputEvents::CursorPosition& event)
{
    // Set context as current.
    ImGui::SetCurrentContext(m_interface);
    ImGuiIO& io = ImGui::GetIO();

    // Set cursor position.
    io.MousePos.x = (float)event.x;
    io.MousePos.y = (float)event.y;
}
