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
) noexcept
{
    //Sadly this is not trivial and should probably be forced to be implemented somehow.
    switch (message)
    {
        case WM_INITDIALOG:
        {
            centre_dialogue();
        }
        break;

        default:
            break;
    }

    return std::nullopt;
}
