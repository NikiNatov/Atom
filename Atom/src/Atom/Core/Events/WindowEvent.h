#pragma once

#include "Event.h"
#include <sstream>

namespace Atom
{
	class WindowResizedEvent : public Event
	{
	public:
		WindowResizedEvent(u32 width, u32 height)
			: m_Width(width), m_Height(height)
		{}

		inline u32 GetWidth() const { return m_Width; }
		inline u32 GetHeight() const { return m_Height; }

		String ToString() const override
		{
			std::stringstream ss;

			ss << "WindowResized: " << m_Width << ", " << m_Height;

			return ss.str();
		}

		EVENT_CLASS_TYPE(WindowResized)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)
	private:
		u32 m_Width;
		u32 m_Height;
	};

	class WindowClosedEvent : public Event
	{
	public:
		WindowClosedEvent()
		{}

		EVENT_CLASS_TYPE(WindowClosed)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)
	};

	class WindowMovedEvent : public Event
	{
	public:

	private:
	};
}