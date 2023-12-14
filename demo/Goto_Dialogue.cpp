#include "Goto_Dialogue.h"

#include "Plugin/Plugin.h"

#include "resource.h"

#include <Windows.h>
#include <WinUser.h>

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
                        plugin()->send_to_editor(SCI_ENSUREVISIBLE, line);
                        plugin()->send_to_editor(SCI_GOTOLINE, line);
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
