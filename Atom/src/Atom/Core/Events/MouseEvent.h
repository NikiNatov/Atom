#pragma once

#include "Event.h"
#include "Atom\Core\MouseCodes.h"
#include <sstream>

namespace Atom
{
	class MouseButtonEvent : public Event
	{
	public:
		inline MouseButton GetButton() const { return m_Button; }

		EVENT_CLASS_CATEGORY(EventCategoryInput | EventCategoryMouse)
	protected:
		MouseButtonEvent(MouseButton button)
			: m_Button(button)
		{
		}

		MouseButton m_Button;
	};

	class MouseButtonPressedEvent : public MouseButtonEvent
	{
	public:
		MouseButtonPressedEvent(MouseButton button)
			: MouseButtonEvent(button)
		{
		}

		String ToString() const override
		{
			std::stringstream ss;

			ss << "MouseButtonPressed: " << (s32)m_Button;

			return ss.str();
		}

		EVENT_CLASS_TYPE(MouseButtonPressed)
	};

	class MouseButtonReleasedEvent : public MouseButtonEvent
	{
	public:
		MouseButtonReleasedEvent(MouseButton button)
			: MouseButtonEvent(button)
		{
		}

		String ToString() const override
		{
			std::stringstream ss;

			ss << "MouseButtonReleased: " << (s32)m_Button;

			return ss.str();
		}

		EVENT_CLASS_TYPE(MouseButtonReleased)
	};

	class MouseMovedEvent : public Event
	{
	public:
		MouseMovedEvent(f32 xPos, f32 yPos)
			: m_XPos(xPos), m_YPos(yPos)
		{
		}

		inline f32 GetXPosition() const { return m_XPos; }
		inline f32 GetYPosition() const { return m_YPos; }

		String ToString() const override
		{
			std::stringstream ss;

			ss << "MouseMoved: " << m_XPos << ", " << m_YPos;

			return ss.str();
		}

		EVENT_CLASS_TYPE(MouseMoved)
		EVENT_CLASS_CATEGORY(EventCategoryInput | EventCategoryMouse)
	private:
		f32 m_XPos;
		f32 m_YPos;
	};

	class MouseScrolledEvent : public Event
	{
	public:
		MouseScrolledEvent(f32 xOffset, f32 yOffset)
			: m_XOffset(xOffset), m_YOffset(yOffset)
		{
		}

		inline f32 GetXOffset() const { return m_XOffset; }
		inline f32 GetYOffset() const { return m_YOffset; }

		String ToString() const override
		{
			std::stringstream ss;

			ss << "MouseScrolled: " << m_XOffset << ", " << m_YOffset;

			return ss.str();
		}

		EVENT_CLASS_TYPE(MouseScrolled)
		EVENT_CLASS_CATEGORY(EventCategoryInput | EventCategoryMouse)
	private:
		f32 m_XOffset;
		f32 m_YOffset;
	};
}