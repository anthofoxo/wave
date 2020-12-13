#include "Debugger.h"

#include <sstream>

#include <nanovg.h>

#include "Application.h"
#include "Log.h"

namespace AF
{
	void DebuggerSection::Update()
	{
	}

	namespace Debugger
	{
		std::vector<DebuggerSection> s_Sections;
		bool s_Enabled = true;

		void Update()
		{
			if (!s_Enabled) return;

			constexpr float margin = 8.0f;

			auto* app = AF::GetApplication();

			std::stringstream ss;

			for (auto& section : s_Sections)
			{
				section.Update();

				ss << section.m_Title << "\n-----\n";
				
				for (auto& [key, value] : section.m_Content)
					ss << fmt::format("{}: {}\n", key, value);
			}

			std::string string = ss.str();

			nvgTextAlign(app->m_Renderer.m_Vg, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
			nvgFontSize(app->m_Renderer.m_Vg, 12.0f);
			app->m_Renderer.FillColor({ 1.0f, 1.0f, 1.0f, 1.0f });

			nvgTextBox(app->m_Renderer.m_Vg, margin, margin, app->m_Size.x - margin * 2.0f, string.c_str(), nullptr);
		}
	}
}