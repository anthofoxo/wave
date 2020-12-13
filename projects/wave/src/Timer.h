#pragma once

namespace AF
{
	template<typename t_Type>
	class Timer final
	{
	public:
		Timer(t_Type duration, bool singleUse = false)
			: m_Duration(duration), m_Position(static_cast<t_Type>(0)), m_SingleUse(singleUse)
		{
		}

		bool Update(t_Type deltaTime)
		{
			m_Position += deltaTime;

			if (m_Position >= m_Duration)
			{
				if (m_SingleUse) m_Position = m_Duration;
				else m_Position -= m_Duration;

				return true;
			}

			return false;
		}

		t_Type PercentComplete()
		{
			return m_Position / m_Duration;
		}
	private:
		t_Type m_Duration;
		t_Type m_Position;
		bool m_SingleUse;
	};
}