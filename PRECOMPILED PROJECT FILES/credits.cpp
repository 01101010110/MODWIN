#include "modwin.h"
#include <shellapi.h>
#include <math.h>

void DrawCreditsUI() {
    // --- MODWIN V7 Decal with Green Neon Effect ---
    // Calculate a pulsing green color
    float time = (float)ImGui::GetTime();
    float decalAlpha = 0.5f + 0.5f * sin(time * 2.0f); // Pulses between 0.0 and 1.0 (or adjust to 0.5-1.0 for subtle)
    // Clamp alpha to ensure visibility if preferred, e.g. 0.4f + 0.6f * ...
    if (decalAlpha < 0.4f) decalAlpha = 0.4f;

    ImVec4 decalColor = ImVec4(0.0f, 1.0f, 0.0f, decalAlpha); // Green

    const char* decal = R"(
||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
||                                                                      ||
||  1111   1111  000000   111111     00  00  00  111111   00       00   ||
||  11 11 11 11 00    00  111  111   00  00  00    11     0000     00   ||
||  11  111  11 00    00  111   111  00  00  00    11     00 00    00   ||
||  11   11  11 00    00  111   111  00  00  00    11     00  00   00   ||
||  11       11 00    00  111   111  00  00  00    11     00   00  00   ||
||  11       11  000000   11111111   0000000000  111111   00     0000   ||
||                                                                   V7 ||
||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
)";

    ImGui::PushStyleColor(ImGuiCol_Text, decalColor);
    ImGui::TextUnformatted(decal);
    ImGui::PopStyleColor();
    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Text("This project wouldn't be possible without the help and support from our XDA community");
    ImGui::SameLine();
    ImGui::Spacing();
    // Heart 
    float t = (float)ImGui::GetTime();
    float alpha = 0.6f + 0.4f * sin(t * 4.0f);
    ImVec4 heartCol = ImVec4(1.0f, 0.4f, 0.7f, alpha);
    ImGui::SetWindowFontScale(2.0f);
    ImGui::TextColored(heartCol, "<3 <3 <3 <3 <3 <3 <3");
    ImGui::SetWindowFontScale(1.0f); 
    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Text("The first thanks goes to"); ImGui::SameLine();
    DrawFireText("Persona78"); ImGui::SameLine();
    ImGui::Text(", from them we learned how to build the ISO in Oscdimg.exe originally.");
    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Text("Thanks also goes to"); ImGui::SameLine();

    // Name List
    DrawFireText("james28909"); ImGui::SameLine(); ImGui::Text(","); ImGui::SameLine();
    DrawFireText("P414DIN"); ImGui::SameLine(); ImGui::Text(","); ImGui::SameLine();
    DrawFireText("knigge111"); ImGui::SameLine(); ImGui::Text(","); ImGui::SameLine();
    DrawFireText("Indospot"); ImGui::SameLine(); ImGui::Text(","); ImGui::SameLine();
    DrawFireText("Xazac"); ImGui::SameLine(); ImGui::Text(","); ImGui::SameLine();
    DrawFireText("elong7681"); ImGui::SameLine(); ImGui::Text(", and"); ImGui::SameLine();
    DrawFireText("Ro Kappa");
    ImGui::Spacing();
    ImGui::Text("Due to their input, a lot of bugs were worked out and features added to MODWIN.");
    ImGui::Spacing();
    ImGui::Spacing();

    // Kerberos002
    ImGui::Spacing();
    ImGui::Text("Thanks to"); ImGui::SameLine(); DrawFireText("kerberos002"); ImGui::SameLine();
    ImGui::Text("for sharing mkisofs on sourceforge:");
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.6f, 1.0f, 1.0f));
    ImGui::Text("https://sourceforge.net/projects/cdrtfe/");
    if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(0)) ShellExecuteA(NULL, "open", "https://sourceforge.net/projects/cdrtfe/", NULL, NULL, SW_SHOWNORMAL);
    ImGui::PopStyleColor();
    ImGui::Spacing();

    // Asheroto
    ImGui::Spacing();
    ImGui::Text("Thanks to"); ImGui::SameLine(); DrawFireText("Asheroto"); ImGui::SameLine();
    ImGui::Text("for their gist code teaching us how to add the TPM bypass to the unattended file:");
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.6f, 1.0f, 1.0f));
    ImGui::Text("https://gist.github.com/asheroto/c4a9fb4e5e5bdad10bcb831e3a3daee6");
    if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(0)) ShellExecuteA(NULL, "open", "https://gist.github.com/asheroto/c4a9fb4e5e5bdad10bcb831e3a3daee6", NULL, NULL, SW_SHOWNORMAL);
    ImGui::PopStyleColor();
    ImGui::Spacing();

    // Microsoft
    ImGui::Spacing();
    ImGui::Text("Thanks to"); ImGui::SameLine(); DrawFireText("learn.microsoft.com"); ImGui::SameLine();
    ImGui::Text(", we now have a working database with knowledge of what all the apps, packages, and features do on our WIM.:");
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.6f, 1.0f, 1.0f));
    ImGui::Text("https://learn.microsoft.com/en-us/");
    if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(0)) ShellExecuteA(NULL, "open", "https://learn.microsoft.com/en-us//", NULL, NULL, SW_SHOWNORMAL);
    ImGui::PopStyleColor();
    ImGui::Spacing();

    // Ocornut 
    ImGui::Spacing();
    ImGui::Text("Thanks to"); ImGui::SameLine(); DrawFireText("Ocornut"); ImGui::SameLine();
    ImGui::Text(", creator of imgui");
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.6f, 1.0f, 1.0f));
    ImGui::Text("https://github.com/ocornut/imgui");
    if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(0)) ShellExecuteA(NULL, "open", "https://github.com/ocornut/imgui", NULL, NULL, SW_SHOWNORMAL);
    ImGui::PopStyleColor();

    // Me :>
    ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();
    ImGui::Text("Developed by"); ImGui::SameLine();
    DrawFireText("Jenneh");
}