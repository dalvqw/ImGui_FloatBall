#ifndef DRAW_H
#define DRAW_H

#define STB_IMAGE_IMPLEMENTATION
#include "ImGui/stb_image.h"
#include "ImGui/imgui.h"
#include <map>
#include <vector>
#include <assert.h>
#include <windows.h>

#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE 0x812F
#endif

void	PlotVar(const char* label, float value, float scale_min = FLT_MAX, float scale_max = FLT_MAX, size_t buffer_size = 120);

// Simple helper function to load an image into a OpenGL texture with common settings
bool LoadTextureFromFile(const char* filename, GLuint* out_texture, int* out_width, int* out_height)
{
    // Load from file
    int image_width = 0;
    int image_height = 0;
    unsigned char* image_data = stbi_load(filename, &image_width, &image_height, NULL, 4);
    if (image_data == NULL)
        return false;

    // Create a OpenGL texture identifier
    GLuint image_texture;
    glGenTextures(1, &image_texture);
    glBindTexture(GL_TEXTURE_2D, image_texture);

    // Setup filtering parameters for display
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // This is required on WebGL for non power-of-two textures
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // Same

    // Upload pixels into texture
#if defined(GL_UNPACK_ROW_LENGTH) && !defined(__EMSCRIPTEN__)
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#endif
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_width, image_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
    stbi_image_free(image_data);

    *out_texture = image_texture;
    *out_width = image_width;
    *out_height = image_height;

    return true;
}

struct PlotVarData
{
	ImGuiID        ID;
	std::vector<float>  Data;
	int            DataInsertIdx;
	int            LastFrame;

	PlotVarData() : ID(0), DataInsertIdx(0), LastFrame(-1) {}
};

typedef std::map<ImGuiID, PlotVarData> PlotVarsMap;
static PlotVarsMap	g_PlotVarsMap;

// Plot value over time
// Call with 'value == FLT_MAX' to draw without adding new value to the buffer
void PlotVar(const char* label, float value, float scale_min, float scale_max, size_t buffer_size)
{
	assert(label);
	if (buffer_size == 0)
		buffer_size = 120;

	ImGui::PushID(label);
	ImGuiID id = ImGui::GetID("");

	// Lookup O(log N)
	PlotVarData& pvd = g_PlotVarsMap[id];

	// Setup
	if (pvd.Data.capacity() != buffer_size)
	{
		pvd.Data.resize(buffer_size);
		memset(&pvd.Data[0], 0, sizeof(float) * buffer_size);
		pvd.DataInsertIdx = 0;
		pvd.LastFrame = -1;
	}

	// Insert (avoid unnecessary modulo operator)
	if (pvd.DataInsertIdx == buffer_size)
		pvd.DataInsertIdx = 0;
	int display_idx = pvd.DataInsertIdx;
	if (value != FLT_MAX)
		pvd.Data[pvd.DataInsertIdx++] = value;

	// Draw
	int current_frame = ImGui::GetFrameCount();
	if (pvd.LastFrame != current_frame)
	{
		ImGui::PlotLines("##plot", &pvd.Data[0], buffer_size, pvd.DataInsertIdx, NULL, scale_min, scale_max, ImVec2(0, 40));
		ImGui::SameLine();
		ImGui::Text("%s\n%-3.4f", label, pvd.Data[display_idx]);	// Display last value in buffer
		pvd.LastFrame = current_frame;
	}

	ImGui::PopID();
}

void PlotVarFlushOldEntries()
{
	int current_frame = ImGui::GetFrameCount();
	for (PlotVarsMap::iterator it = g_PlotVarsMap.begin(); it != g_PlotVarsMap.end(); )
	{
		PlotVarData& pvd = it->second;
		if (pvd.LastFrame < current_frame - max(400, (int)pvd.Data.size()))
			it = g_PlotVarsMap.erase(it);
		else
			++it;
	}
}

#endif
