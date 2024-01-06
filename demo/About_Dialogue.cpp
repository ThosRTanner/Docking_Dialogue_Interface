#include "About_Dialogue.h"

#include "resource.h"

#include <windows.h>
//#include <WinUser.h>

About_Dialogue::About_Dialogue(Plugin const *plugin) :
    Modal_Dialogue_Interface(plugin)
{
    create_modal_dialogue(IDD_ABOUT_DIALOG);
}

About_Dialogue::~About_Dialogue() = default;

std::optional<LONG_PTR> About_Dialogue::on_dialogue_message(
    UINT message, WPARAM wParam, LPARAM lParam
)
{
    //Sadly this is not trivial and should probably be forced to be implemented somehow.
    //Although we can always do the cancel button in the default.
    switch (message)
    {
        case WM_INITDIALOG:
        {
            centre_dialogue();
        }
        break;

        case WM_COMMAND:
            if (HIWORD(wParam) == BN_CLICKED)
            {
                switch (LOWORD(wParam))
                {
                    case IDOK:
                    {
                        EndDialog(1);
                        return TRUE;
                    }

                    case IDCANCEL:
                    {
                        EndDialog(0LL);
                        return TRUE;
                    }
                }
            }
            break;

        case WM_SYSCOMMAND:
            if (wParam == SC_CLOSE)
            {
                // cancel
                EndDialog(0LL);
                return TRUE;
            }
            break;

        default:
            break;
    }

    return std::nullopt;
}
