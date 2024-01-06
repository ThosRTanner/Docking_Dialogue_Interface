// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#include "Modal_Dialogue_Interface.h"

#include "Plugin/Plugin.h"

#include <WinUser.h>

Modal_Dialogue_Interface::Modal_Dialogue_Interface(Plugin const *plugin) :
    Dialogue_Interface(plugin),
    result_()
{
}

Modal_Dialogue_Interface::~Modal_Dialogue_Interface()
{
}

BOOL Modal_Dialogue_Interface::EndDialog(INT_PTR retval) const noexcept
{
    return ::EndDialog(window(), retval);
}

BOOL Modal_Dialogue_Interface::centre_dialogue() const noexcept
{
    RECT const rect = getWindowRect();
    int const width = rect.right - rect.left;
    int const height = rect.bottom - rect.top;

    RECT const rect_npp = getParentRect();
    int const x = ((rect_npp.right - rect_npp.left) - width) / 2 + rect_npp.left;
    int const y = ((rect_npp.bottom - rect_npp.top) - height) / 2 + rect_npp.top;

    return ::MoveWindow(window(), x, y, width, height, TRUE);
}

std::optional<LONG_PTR> Modal_Dialogue_Interface::on_unhandled_dialogue_message(
    UINT message, WPARAM wParam, LPARAM lParam
) noexcept
{
    //This provides default handlers for OK, cancel, and close clicks.
    switch (message)
    {
        case WM_COMMAND:
            if (HIWORD(wParam) == BN_CLICKED)
            {
                switch (LOWORD(wParam))
                {
                    case IDOK:
                    {
                        EndDialog(Clicked_OK);
                        return TRUE;
                    }

                    case IDCANCEL:
                    {
                        EndDialog(Clicked_Cancel);
                        return TRUE;
                    }

                    default:
                        break;
                }
            }
            break;

        case WM_SYSCOMMAND:
            if (wParam == SC_CLOSE)
            {
                EndDialog(Clicked_Close);
                return TRUE;
            }
            break;

        default:
            break;
    }
    return std::nullopt;
}

void Modal_Dialogue_Interface::create_modal_dialogue(int dialogID) noexcept
{
    HWND focus = ::GetFocus();
    result_ = Dialogue_Interface::create_modal_dialogue(dialogID);
    ::SetFocus(focus);
}
