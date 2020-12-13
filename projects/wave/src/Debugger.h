#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <utility>

namespace AF
{
	struct DebuggerSection
	{
		DebuggerSection() = default;
		virtual ~DebuggerSection() = default;

		std::string_view m_Title;
		std::vector<std::pair<std::string, std::string>> m_Content;

		virtual void Update();
	};

	namespace Debugger
	{
		extern std::vector<DebuggerSection> s_Sections;
		extern bool s_Enabled;

		void Update();
	}
}