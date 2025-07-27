#pragma once

#include <string>

#include "//third_party/imgui/imgui.h"

namespace imgui_util {

// Helper to display a little (?) mark which shows a tooltip when hovered.
inline void ImguiHelpMarker(const std::string &msg) {
  ImGui::TextDisabled("(?)");
  if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort)) {
    ImGui::BeginTooltip();
    ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
    ImGui::TextUnformatted(msg.c_str());
    ImGui::PopTextWrapPos();
    ImGui::EndTooltip();
  }
}

} // namespace imgui_util