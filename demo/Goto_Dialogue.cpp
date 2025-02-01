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

#include "Goto_Dialogue.h"

#include "resource.h"

#include "Plugin/Plugin.h"

#include "notepad++/Scintilla.h"

#include <windows.h>    // IWYU pragma: keep

#include <minwindef.h>
#include <windef.h>
#include <winuser.h>

#include <optional>
#include <sstream>

Goto_Dialogue::Goto_Dialogue(int menu_entry, Plugin const &plugin) :
    Super(IDD_GOTO_DIALOGUE, plugin)
{
    register_dialogue(
        menu_entry,
        Position::Dock_Bottom,
        static_cast<HICON>(::LoadImage(
            plugin.module(),
            MAKEINTRESOURCE(IDI_ICON1),
            IMAGE_ICON,
            0,
            0,
            LR_LOADMAP3DCOLORS | LR_LOADTRANSPARENT
        )),
        L"Some extra text"
    );
}

Goto_Dialogue::~Goto_Dialogue() = default;

Goto_Dialogue::Message_Return Goto_Dialogue::on_dialogue_message(
    UINT message, WPARAM wParam, LPARAM
)
{
    switch (message)
    {
        case WM_COMMAND:
        {
            switch (wParam)
            {
                case IDC_GO_BUTTON:
                {
                    BOOL successful;
                    int const line =
                        ::GetDlgItemInt(
                            window(), IDC_LINE_NUMBER, &successful, FALSE
                        )
                        - 1;
                    if (successful)
                    {
                        LRESULT const lines =
                            plugin()->send_to_editor(SCI_GETLINECOUNT);
                        if (line < 0 || line >= lines)
                        {
                            std::wstringstream msg;
                            msg << "Line must be between 1 and " << lines;
                            message_box(
                                msg.str().c_str(),
                                MB_ICONEXCLAMATION | MB_OKCANCEL
                            );
                        }
                        else
                        {
                            plugin()->send_to_editor(SCI_ENSUREVISIBLE, line);
                            plugin()->send_to_editor(SCI_GOTOLINE, line);
                        }
                    }
                    else
                    {
                        message_box(
                            L"Not a valid line number", MB_ICONERROR | MB_OK
                        );
                    }
                    return TRUE;
                }

                default:
                    break;
            }
            break;
        }

        default:
            break;
    }
    return std::nullopt;
}

void Goto_Dialogue::on_display() noexcept
{
    // Empty function just so you can put a breakpoint on it!
}
