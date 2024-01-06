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

#include "Docking_Dialogue_Interface.h"

#include "Plugin.h"

#include "DockingFeature/Docking.h"
#include "DockingFeature/dockingResource.h"
#include "Notepad_plus_msgs.h"

#include <WinUser.h>

#include <stdexcept>
#include <system_error>

namespace
{

std::wstring get_dialogue_name(HWND dialog_handle)
{
    TCHAR temp[MAX_PATH] = {0};
    ::GetWindowText(dialog_handle, &temp[0], MAX_PATH);
    return &temp[0];
}

}    // namespace

Docking_Dialogue_Interface::Docking_Dialogue_Interface(
    int dialogue_ID, Plugin const *plugin
) :
    Dialogue_Interface(plugin)
{
    create_dialogue(dialogue_ID);
    send_dialogue_info(NPPM_MODELESSDIALOG, MODELESSDIALOGADD);
}

Docking_Dialogue_Interface::~Docking_Dialogue_Interface()
{
    send_dialogue_info(NPPM_MODELESSDIALOG, MODELESSDIALOGREMOVE);
}

void Docking_Dialogue_Interface::display() noexcept
{
    on_display();
    is_hidden_ = false;
    send_dialogue_info(NPPM_DMMSHOW);
}

void Docking_Dialogue_Interface::hide() noexcept
{
    on_hide();
    is_hidden_ = true;
    send_dialogue_info(NPPM_DMMHIDE);
}

/** Register dialogue with Notepad++.
 *
 * I'm not a fan of 2-phase initialisation, but this bit has to be done
 * after the dialogue is actually created, or things go wrong
 *
 * menu_index is the ID used to communicate with notepad++ (i.e. the menu
 * entry) extra is extra text to display on dialogue title.
 */

void Docking_Dialogue_Interface::register_dialogue(
    int menu_index, Position pos, HICON icon, wchar_t const *extra
) noexcept
{
    tTbData data{};

    data.hClient = window();
    data.pszName = dialogue_name();
    data.dlgID = menu_index;

    data.uMask = pos == Position::Floating ? DWS_DF_FLOATING
                                           : static_cast<int>(pos) << 28;
    if (icon != nullptr)
    {
        data.uMask |= DWS_ICONTAB;
        data.hIconTab = icon;
    }
    if (extra != nullptr)
    {
        data.uMask |= DWS_ADDINFO;
        data.pszAddInfo = extra;
    }
    data.pszModuleName = module_name();

    plugin()->send_to_notepad(NPPM_DMMREGASDCKDLG, 0, &data);

    // Hide the dialogue by default. This allows Notepad++ to open it as
    // necessary at startup. If being opened in response to a menu
    // selection, just call display()
    hide();
}

void Docking_Dialogue_Interface::on_display() noexcept
{
}

void Docking_Dialogue_Interface::on_hide() noexcept
{
}

void Docking_Dialogue_Interface::send_dialogue_info(
    int msg, int wParam
) noexcept
{
    plugin()->send_to_notepad(msg, wParam, window());
}

std::optional<LONG_PTR> Docking_Dialogue_Interface::on_unhandled_dialogue_message(
    UINT message, WPARAM, LPARAM lParam
) noexcept
{
    switch (message)
    {
        case WM_NOTIFY:
        {
#pragma warning(suppress : 26490)
            NMHDR const *pnmh = reinterpret_cast<LPNMHDR>(lParam);

            if (pnmh->hwndFrom == plugin()->get_notepad_window())
            {
                switch (LOWORD(pnmh->code))
                {
                    case DMN_CLOSE:
                        is_hidden_ = true;
                        // This must not say it has handled things.
                        return std::nullopt;

                    case DMN_DOCK:
                        docked_pos_ = HIWORD(pnmh->code);
                        is_floating_ = false;
                        return TRUE;

                    case DMN_FLOAT:
                        is_floating_ = true;
                        return TRUE;

                    // These are defined in DockingResource.h but I've not
                    // managed to trigger them.
                    case DMN_SWITCHIN:
                    case DMN_SWITCHOFF:
                    case DMN_FLOATDROPPED:
                    default:
                        break;
                }
            }
            break;
        }

        case WM_PAINT:
            // This must not say it has handled things.
            ::RedrawWindow(window(), nullptr, nullptr, RDW_INVALIDATE);
            return std::nullopt;

        default:
            break;
    }
    return std::nullopt;
}

HWND Docking_Dialogue_Interface::create_dialogue(int dialogID)
{
    HWND const dialogue_window{Dialogue_Interface::create_dialogue(dialogID)};
    if (dialogue_window == nullptr)
    {
        char buff[2048];
        auto const err = GetLastError();
        try
        {
            std::snprintf(
                &buff[0],
                sizeof(buff),
                "Could not create dialogue: %s",
                std::generic_category().message(err).c_str()
            );
        }
        catch (std::exception const &e)
        {
#pragma warning(suppress : 26447)    // MS Bug with e.what() decl
            std::snprintf(
                &buff[0],
                sizeof(buff),
                "Could not create dialogue: Error code %08x then got %s",
                err,
                e.what()
            );
        }

        throw std::runtime_error(&buff[0]);
    }
    return dialogue_window;
}
