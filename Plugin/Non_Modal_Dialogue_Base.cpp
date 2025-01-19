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

#include "Non_Modal_Dialogue_Base.h"

#include "Plugin.h"

#include "notepad++/Notepad_plus_msgs.h"

// This is because IWYU doesn't understand overrides and appears to have got
// a very odd idea about using RECT
#include <minwindef.h>

#include <optional>

Non_Modal_Dialogue_Base::Non_Modal_Dialogue_Base(
    int dialogue_ID, Plugin const &plugin, HWND parent
) :
    Super(plugin)
{
    create_dialogue(dialogue_ID, parent);
    send_dialogue_info(NPPM_MODELESSDIALOG, MODELESSDIALOGADD);
}

Non_Modal_Dialogue_Base::~Non_Modal_Dialogue_Base()
{
    send_dialogue_info(NPPM_MODELESSDIALOG, MODELESSDIALOGREMOVE);
}

void Non_Modal_Dialogue_Base::send_dialogue_info(int msg, int wParam) noexcept
{
    plugin()->send_to_notepad(msg, wParam, window());
}

Non_Modal_Dialogue_Base::Message_Return
Non_Modal_Dialogue_Base::on_unhandled_non_modal_dialogue_message(
    UINT /*message*/, WPARAM, LPARAM
) noexcept
{
    return std::nullopt;
}

Non_Modal_Dialogue_Base::Message_Return
Non_Modal_Dialogue_Base::on_unhandled_dialogue_message(
    UINT message, WPARAM wParam, LPARAM lParam
) noexcept
{
    return on_unhandled_non_modal_dialogue_message(message, wParam, lParam);
}
