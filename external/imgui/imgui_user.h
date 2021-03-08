#include <string>
#include <vector>

namespace ImGui
{

bool Combo(const char* label, int* currIndex, std::vector<std::string>& values);
bool ListBox(const char* label, int* currIndex, std::vector<std::string>& values);
void AddUnderLine(ImColor col_);
bool TextButtonColored(ImVec4 color, const char* name_);

} // namespace ImGui
