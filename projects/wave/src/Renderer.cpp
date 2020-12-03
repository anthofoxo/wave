#include "Renderer.h"

namespace AF
{
	void Renderer::BeginFrame(glm::vec2 size)
	{
		nvgBeginFrame(m_Vg, size.x, size.y, 1.0f);
	}

	void Renderer::BeginPath()
	{
		nvgBeginPath(m_Vg);
	}

	void Renderer::EndFrame()
	{
		nvgEndFrame(m_Vg);
	}

	void Renderer::Fill()
	{
		nvgFill(m_Vg);
	}

	void Renderer::FillColor(glm::vec4 color)
	{
		nvgFillColor(m_Vg, *(NVGcolor*) &color);
	}

	void Renderer::Rect(glm::vec2 position, glm::vec2 size)
	{
		nvgRect(m_Vg, position.x, position.y, size.x, size.y);
	}

	void Renderer::FontFace(const char* face)
	{
		nvgFontFace(m_Vg, face);
	}

	void Renderer::FontSize(float size)
	{
		nvgFontSize(m_Vg, size);
	}

	void Renderer::TextAlign(int align)
	{
		nvgTextAlign(m_Vg, align);
	}

	//
	//
	//

	void Renderer::VGRP_FillRect(glm::vec2 position, glm::vec2 size, glm::vec4 color)
	{
		BeginPath();
		Rect(position, size);
		FillColor(color);
		Fill();
	}
}