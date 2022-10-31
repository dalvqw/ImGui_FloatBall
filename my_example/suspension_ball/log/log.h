#pragma once

#include <cstdint>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <map>

#include "../imgui/imgui.h"
#include "../imgui/imgui_internal.h"

extern float accent_colour[4];

namespace login {
    bool selected(const char* label, bool tab = false);
}

