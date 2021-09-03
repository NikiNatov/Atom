#pragma once

#include "Event.h"
#include "Atom\Core\KeyCodes.h"
#include <sstream>

namespace Atom
{
	class KeyEvent : public Event
	{
	public:
		inline Key GetKeyCode() const { return m_KeyCode; }

		EVENT_CLASS_CATEGORY(EventCategoryKeyboard | EventCategoryInput)
	protected:
		KeyEvent(Key keycode)
			: m_KeyCode(keycode)
		{
		}

		Key m_KeyCode;
	};

	class KeyPressedEvent : public KeyEvent
	{
	public:
		KeyPressedEvent(Key keycode, s32 repeatCount)
			: KeyEvent(keycode), m_RepeatCount(repeatCount)
		{
		}

		virtual String ToString() const override
		{
			std::stringstream ss;

			ss << "KeyPressed: " << (s32)m_KeyCode << "(" << m_RepeatCount << ")";

			return ss.str();
		}

		inline s32 GetRepeatCount() const { return m_RepeatCount; }

		EVENT_CLASS_TYPE(KeyPressed)

	private:
		s32 m_RepeatCount;
	};


	class KeyReleasedEvent : public KeyEvent
	{
	public:
		KeyReleasedEvent(Key keycode)
			: KeyEvent(keycode)
		{
		}

		virtual String ToString() const override
		{
			std::stringstream ss;

			ss << "KeyReleased: " << (s32)m_KeyCode;

			return ss.str();
		}

		EVENT_CLASS_TYPE(KeyReleased)

	};

	class KeyTypedEvent : public KeyEvent
	{
	public:
		KeyTypedEvent(Key keycode)
			: KeyEvent(keycode)
		{
		}

		String ToString() const override
		{
			std::stringstream ss;

			ss << "KeyTyped: " << (s32)m_KeyCode;

			return ss.str();
		}

		EVENT_CLASS_TYPE(KeyTyped)
	};
}