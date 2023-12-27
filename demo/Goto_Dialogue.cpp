#include "Goto_Dialogue.h"

#include "Plugin/Plugin.h"

#include "resource.h"

#include <Windows.h>
#include <WinUser.h>

#include <sstream>

Goto_Dialogue::Goto_Dialogue(int menu_entry, Plugin const *plugin) :
    Docking_Dialogue_Interface(IDD_GOTO_DIALOGUE, plugin)
{
    register_dialogue(
        menu_entry,
        Position::Dock_Bottom,
        static_cast<HICON>(::LoadImage(
            plugin->module(),
            MAKEINTRESOURCE(IDI_ICON1),
            IMAGE_ICON,
            0,
            0,
            LR_LOADMAP3DCOLORS | LR_LOADTRANSPARENT
        )),
        L"Some extra text"
    );
}

std::optional<LONG_PTR> Goto_Dialogue::on_dialogue_message(
    UINT message, UINT_PTR wParam, LONG_PTR lParam
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
                                msg.str(), MB_ICONEXCLAMATION | MB_OKCANCEL
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
