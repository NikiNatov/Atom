#pragma once
// Include this header in ScriptEngine.cpp to create the embedded module

#include "Atom/Scene/Components.h"
#include "Atom/Core/Input.h"
#include "Atom/Core/Timer.h"

#include "Atom/Scripting/ScriptWrappers/Scene/EntityWrapper.h"

#include <glm/glm.hpp>
#include <pybind11/embed.h>

namespace Atom
{
    namespace py = pybind11;
    namespace wrappers = Atom::ScriptWrappers;

    PYBIND11_EMBEDDED_MODULE(Atom, m)
    {
        // --------------------------------------------------- Math ------------------------------------------------------------
        py::class_<glm::vec2>(m, "Vec2")
            .def(py::init<>())
            .def(py::init<f32>())
            .def(py::init<f32, f32>())
            .def(py::init<const glm::vec2&>())
            .def_readwrite("x", &glm::vec2::x)
            .def_readwrite("y", &glm::vec2::y);

        py::class_<glm::vec3>(m, "Vec3")
            .def(py::init<>())
            .def(py::init<f32>())
            .def(py::init<f32, f32, f32>())
            .def(py::init<const glm::vec3&>())
            .def_readwrite("x", &glm::vec3::x)
            .def_readwrite("y", &glm::vec3::y)
            .def_readwrite("z", &glm::vec3::z);

        py::class_<glm::vec4>(m, "Vec4")
            .def(py::init<>())
            .def(py::init<f32>())
            .def(py::init<f32, f32, f32, f32>())
            .def(py::init<const glm::vec4&>())
            .def_readwrite("x", &glm::vec4::x)
            .def_readwrite("y", &glm::vec4::y)
            .def_readwrite("z", &glm::vec4::z)
            .def_readwrite("w", &glm::vec4::y);


        // --------------------------------------------------- Core ------------------------------------------------------------
        py::class_<Atom::Logger>(m, "Log")
            .def_static("trace", [](const char* message) { ATOM_TRACE(message); })
            .def_static("info", [](const char* message) { ATOM_INFO(message); })
            .def_static("warning", [](const char* message) { ATOM_WARNING(message); })
            .def_static("error", [](const char* message) { ATOM_ERROR(message); });

        py::enum_<Atom::Key>(m, "Key")
            .value("Backspace", Atom::Key::Backspace)
            .value("Tab", Atom::Key::Tab)
            .value("Clear", Atom::Key::Clear)
            .value("Enter", Atom::Key::Enter)
            .value("LShift", Atom::Key::LShift)
            .value("RShift", Atom::Key::RShift)
            .value("LCtrl", Atom::Key::LCtrl)
            .value("RCtrl", Atom::Key::RCtrl)
            .value("LAlt", Atom::Key::LAlt)
            .value("RAlt", Atom::Key::RAlt)
            .value("Pause", Atom::Key::Pause)
            .value("CapsLock", Atom::Key::CapsLock)
            .value("Esc", Atom::Key::Esc)
            .value("Space", Atom::Key::Space)
            .value("PageUp", Atom::Key::PageUp)
            .value("PageDown", Atom::Key::PageDown)
            .value("End", Atom::Key::End)
            .value("Home", Atom::Key::Home)
            .value("LeftArrow", Atom::Key::LeftArrow)
            .value("RightArrow", Atom::Key::RightArrow)
            .value("UpArrow", Atom::Key::UpArrow)
            .value("DownArrow", Atom::Key::DownArrow)
            .value("Select", Atom::Key::Select)
            .value("Print", Atom::Key::Print)
            .value("Execute", Atom::Key::Execute)
            .value("PrintScreen", Atom::Key::PrintScreen)
            .value("Ins", Atom::Key::Ins)
            .value("Del", Atom::Key::Del)
            .value("Help", Atom::Key::Help)
            .value("Key0", Atom::Key::Key0)
            .value("Key1", Atom::Key::Key1)
            .value("Key2", Atom::Key::Key2)
            .value("Key3", Atom::Key::Key3)
            .value("Key4", Atom::Key::Key4)
            .value("Key5", Atom::Key::Key5)
            .value("Key6", Atom::Key::Key6)
            .value("Key7", Atom::Key::Key7)
            .value("Key8", Atom::Key::Key8)
            .value("Key9", Atom::Key::Key9)
            .value("A", Atom::Key::A)
            .value("B", Atom::Key::B)
            .value("C", Atom::Key::C)
            .value("D", Atom::Key::D)
            .value("E", Atom::Key::E)
            .value("F", Atom::Key::F)
            .value("G", Atom::Key::G)
            .value("H", Atom::Key::H)
            .value("I", Atom::Key::I)
            .value("J", Atom::Key::J)
            .value("K", Atom::Key::K)
            .value("L", Atom::Key::L)
            .value("M", Atom::Key::M)
            .value("N", Atom::Key::N)
            .value("O", Atom::Key::O)
            .value("P", Atom::Key::P)
            .value("Q", Atom::Key::Q)
            .value("R", Atom::Key::R)
            .value("S", Atom::Key::S)
            .value("T", Atom::Key::T)
            .value("U", Atom::Key::U)
            .value("V", Atom::Key::V)
            .value("W", Atom::Key::W)
            .value("X", Atom::Key::X)
            .value("Y", Atom::Key::Y)
            .value("Z", Atom::Key::Z)
            .value("LWinKey", Atom::Key::LWinKey)
            .value("RWinKey", Atom::Key::RWinKey)
            .value("NumPad0", Atom::Key::NumPad0)
            .value("NumPad1", Atom::Key::NumPad1)
            .value("NumPad2", Atom::Key::NumPad2)
            .value("NumPad3", Atom::Key::NumPad3)
            .value("NumPad4", Atom::Key::NumPad4)
            .value("NumPad5", Atom::Key::NumPad5)
            .value("NumPad6", Atom::Key::NumPad6)
            .value("NumPad7", Atom::Key::NumPad7)
            .value("NumPad8", Atom::Key::NumPad8)
            .value("NumPad9", Atom::Key::NumPad9)
            .value("F1", Atom::Key::F1)
            .value("F2", Atom::Key::F2)
            .value("F3", Atom::Key::F3)
            .value("F4", Atom::Key::F4)
            .value("F5", Atom::Key::F5)
            .value("F6", Atom::Key::F6)
            .value("F7", Atom::Key::F7)
            .value("F8", Atom::Key::F8)
            .value("F9", Atom::Key::F9)
            .value("F10", Atom::Key::F10)
            .value("F11", Atom::Key::F11)
            .value("F12", Atom::Key::F12)
            .value("NumLock", Atom::Key::NumLock);

        py::enum_<Atom::MouseButton>(m, "MouseButton")
            .value("Left", Atom::MouseButton::Left)
            .value("Right", Atom::MouseButton::Right)
            .value("Middle", Atom::MouseButton::Middle)
            .value("X1", Atom::MouseButton::X1)
            .value("X2", Atom::MouseButton::X2);

        py::class_<Atom::Input>(m, "Input")
            .def_static("is_key_pressed", [](Atom::Key key) { return Atom::Input::IsKeyPressed(key); })
            .def_static("is_mouse_button_pressed", [](Atom::MouseButton button) { return Atom::Input::IsMouseButtonPressed(button); })
            .def_static("get_mouse_position", []() { return Atom::Input::GetMousePosition(); })
            .def_static("set_mouse_cursor", [](bool enabled) { Atom::Input::SetMouseCursor(enabled); })
            .def_static("set_mouse_position", [](const glm::vec2& position) { Atom::Input::SetMousePosition(position); })
            .def_static("is_cursor_enabled", []() { return Atom::Input::IsCursorEnabled(); });

        py::class_<Atom::Timestep>(m, "Timestep")
            .def(py::init<>())
            .def("get_seconds", &Atom::Timestep::GetSeconds)
            .def("get_milliseconds", &Atom::Timestep::GetMilliseconds);

        py::class_<Atom::Timer>(m, "Timer")
            .def(py::init<>())
            .def("reset", &Atom::Timer::Reset)
            .def("stop", &Atom::Timer::Stop)
            .def("get_elapsed_time", &Atom::Timer::GetElapsedTime);

        // --------------------------------------------------- Scene ------------------------------------------------------------
        py::class_<Atom::SceneCamera>(m, "SceneCamera")
            .def(py::init<>())
            .def("set_viewport_size", &Atom::SceneCamera::SetViewportSize)
            .def_property("projection_type", &Atom::SceneCamera::GetProjectionType, &Atom::SceneCamera::SetProjectionType)
            .def_property("perspective_fov", &Atom::SceneCamera::GetPerspectiveFOV, &Atom::SceneCamera::SetPerspectiveFOV)
            .def_property("perspective_near", &Atom::SceneCamera::GetPerspectiveNear, &Atom::SceneCamera::SetPerspectiveNear)
            .def_property("perspective_far", &Atom::SceneCamera::GetPerspectiveFar, &Atom::SceneCamera::SetPerspectiveFar)
            .def_property("ortho_size", &Atom::SceneCamera::GetOrthographicSize, &Atom::SceneCamera::SetOrthographicSize)
            .def_property("ortho_near", &Atom::SceneCamera::GetOrthographicNear, &Atom::SceneCamera::SetOrthographicNear)
            .def_property("ortho_far", &Atom::SceneCamera::GetOrthographicFar, &Atom::SceneCamera::SetOrthographicFar);

        py::class_<Atom::TransformComponent>(m, "TransformComponent")
            .def(py::init<>())
            .def_readwrite("translation", &Atom::TransformComponent::Translation)
            .def_readwrite("rotation", &Atom::TransformComponent::Translation)
            .def_readwrite("scale", &Atom::TransformComponent::Scale);

        py::class_<Atom::CameraComponent>(m, "CameraComponent")
            .def(py::init<>())
            .def_readwrite("camera", &Atom::CameraComponent::Camera)
            .def_readwrite("fixed_aspect_ratio", &Atom::CameraComponent::FixedAspectRatio)
            .def_readwrite("is_primary_camera", &Atom::CameraComponent::Primary);

        py::class_<Atom::DirectionalLightComponent>(m, "DirectionalLightComponent")
            .def(py::init<>())
            .def_readwrite("color", &Atom::DirectionalLightComponent::Color)
            .def_readwrite("intensity", &Atom::DirectionalLightComponent::Intensity);

        py::class_<Atom::PointLightComponent>(m, "PointLightComponent")
            .def(py::init<>())
            .def_readwrite("color", &Atom::PointLightComponent::Color)
            .def_readwrite("intensity", &Atom::PointLightComponent::Intensity)
            .def_readwrite("attenuation_factors", &Atom::PointLightComponent::AttenuationFactors);

        py::class_<Atom::SpotLightComponent>(m, "SpotLightComponent")
            .def(py::init<>())
            .def_readwrite("color", &Atom::SpotLightComponent::Color)
            .def_readwrite("intensity", &Atom::SpotLightComponent::Intensity)
            .def_readwrite("direction", &Atom::SpotLightComponent::Direction)
            .def_readwrite("cone_angle", &Atom::SpotLightComponent::ConeAngle)
            .def_readwrite("attenuation_factors", &Atom::SpotLightComponent::AttenuationFactors);

        py::class_<wrappers::Entity>(m, "Entity")
            .def(py::init<>())
            .def(py::init<u64>())
            .def("has_camera_component", &wrappers::Entity::HasComponent<CameraComponent>)
            .def("has_dir_light_component", &wrappers::Entity::HasComponent<DirectionalLightComponent>)
            .def("has_point_light_component", &wrappers::Entity::HasComponent<PointLightComponent>)
            .def("has_spot_light_component", &wrappers::Entity::HasComponent<SpotLightComponent>)
            .def("get_camera_component", &wrappers::Entity::GetComponent<CameraComponent>, py::return_value_policy::reference)
            .def("get_dir_light_component", &wrappers::Entity::GetComponent<DirectionalLightComponent>, py::return_value_policy::reference)
            .def("get_point_light_component", &wrappers::Entity::GetComponent<PointLightComponent>, py::return_value_policy::reference)
            .def("get_spot_light_component", &wrappers::Entity::GetComponent<SpotLightComponent>, py::return_value_policy::reference)
            .def("get_script", &wrappers::Entity::GetScriptInstance)
            .def("is_valid", &wrappers::Entity::IsValid)
            .def_static("find_entity_by_name", &wrappers::Entity::FindEntityByName)
            .def_property_readonly("id", &wrappers::Entity::GetUUID)
            .def_property("tag", &wrappers::Entity::GetTag, &wrappers::Entity::SetTag)
            .def_property("translation", &wrappers::Entity::GetTranslation, &wrappers::Entity::SetTranslation)
            .def_property("euler_angles", &wrappers::Entity::GetRotation, &wrappers::Entity::SetRotation)
            .def_property("scale", &wrappers::Entity::GetScale, &wrappers::Entity::SetScale);
    }
}