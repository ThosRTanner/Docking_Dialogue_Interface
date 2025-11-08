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

#include <basetsd.h>

#include <windows.h>    // IWYU pragma: keep
// Windows.h is required for the following to compile:
#include <minwindef.h>
#include <windef.h>
#include <winuser.h>

#include <optional>

Modal_Dialogue_Interface::Modal_Dialogue_Interface(Plugin const &plugin) :
    Super(plugin),
    result_()
{
}

Modal_Dialogue_Interface::~Modal_Dialogue_Interface() = default;

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
    int const x_pos =
        ((rect_npp.right - rect_npp.left) - width) / 2 + rect_npp.left;
    int const y_pos =
        ((rect_npp.bottom - rect_npp.top) - height) / 2 + rect_npp.top;

    return ::MoveWindow(window(), x_pos, y_pos, width, height, TRUE);
}

Modal_Dialogue_Interface::Message_Return
Modal_Dialogue_Interface::on_unhandled_dialogue_message(
    UINT message, WPARAM wParam, LPARAM    // NOLINT
) noexcept
{
    // This provides default handlers for OK, cancel, and close clicks.
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

INT_PTR Modal_Dialogue_Interface::create_modal_dialogue(int dialogID) noexcept
{
    HWND focus = ::GetFocus();
    result_ = Super::create_modal_dialogue(dialogID);
    ::SetFocus(focus);
    return result_;
}
