#pragma once

namespace AF
{
	class Timer final
	{
	public:
		Timer(double duration, bool singleUse = false);

		bool Update(double deltaTime);

		double PercentComplete();
	private:
		double m_Duration;
		double m_Position;
		bool m_SingleUse;
	};
}