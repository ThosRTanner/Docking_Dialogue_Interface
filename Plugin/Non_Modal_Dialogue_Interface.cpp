// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#include "Non_Modal_Dialogue_Interface.h"

#include "Plugin.h"

Non_Modal_Dialogue_Interface::Non_Modal_Dialogue_Interface(
    int dialogue_ID, Plugin const *plugin, HWND parent
) :
    Non_Modal_Dialogue_Base(dialogue_ID, plugin, parent)
{
}

Non_Modal_Dialogue_Interface::~Non_Modal_Dialogue_Interface()
{
}

std::optional<LONG_PTR>
Non_Modal_Dialogue_Interface::on_unhandled_non_modal_dialogue_message(
    UINT message, WPARAM wParam, LPARAM lParam
) noexcept
{
    return std::nullopt;
}
