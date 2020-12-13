#include "Timer.h"

namespace AF
{
	Timer::Timer(double duration, bool singleUse)
		: m_Duration(duration), m_Position(0.0), m_SingleUse(singleUse)
	{
	}

	bool Timer::Update(double deltaTime)
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

	double Timer::PercentComplete()
	{
		return m_Position / m_Duration;
	}
}