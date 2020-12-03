#pragma once

#include <nanovg.h>
#include <glm/glm.hpp>

namespace AF
{
	class Renderer
	{
	public:
		void BeginFrame(glm::vec2 size);
		void EndFrame();

		void BeginPath();
		void Fill();
		void FillColor(glm::vec4 color);
		void Rect(glm::vec2 position, glm::vec2 size);
		void FontFace(const char* face);
		void FontSize(float size);
		void TextAlign(int align);

		void VGRP_FillRect(glm::vec2 position, glm::vec2 size, glm::vec4 color);
		
		NVGcontext* m_Vg;
	};
}