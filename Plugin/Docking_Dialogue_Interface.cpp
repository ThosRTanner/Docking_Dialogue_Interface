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

#include <comutil.h>
#include <ShlwApi.h>
#include <WinUser.h>

#include <stdexcept>
#include <system_error>

#ifdef _DEBUG
#pragma comment(lib, "comsuppwd.lib")
#else
#pragma comment(lib, "comsuppw.lib")
#endif
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "shlwapi.lib")

namespace
{
std::wstring get_module_name(HINSTANCE module_instance)
{
    TCHAR temp[MAX_PATH] = {0};
    ::GetModuleFileName(module_instance, &temp[0], MAX_PATH);
    return ::PathFindFileName(&temp[0]);
}

std::wstring get_plugin_name(HWND dialog_handle)
{
    TCHAR temp[MAX_PATH] = {0};
    ::GetWindowText(dialog_handle, &temp[0], MAX_PATH);
    return &temp[0];
}

}    // namespace

Docking_Dialogue_Interface::Docking_Dialogue_Interface(
    int dialogue_ID, Plugin const *plugin
) :
    plugin_(plugin),
    dialogue_window_(create_dialogue_window(dialogue_ID)),
    module_name_(get_module_name(plugin_->module())),
    plugin_name_(get_plugin_name(dialogue_window_))
{
    ::SetWindowLongPtr(
        dialogue_window_,
        GWLP_USERDATA,
#pragma warning(suppress : 26490)
        reinterpret_cast<LONG_PTR>(this)
    );

    send_dialogue_info(NPPM_MODELESSDIALOG, MODELESSDIALOGADD);
}

Docking_Dialogue_Interface::~Docking_Dialogue_Interface()
{
    // Stop process_dialogue_message from doing anything, since it calls a
    // virtual method which won't be there.
    ::SetWindowLongPtr(dialogue_window_, GWLP_USERDATA, NULL);
    send_dialogue_info(NPPM_MODELESSDIALOG, MODELESSDIALOGREMOVE);
    ::DestroyWindow(dialogue_window_);
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

    data.hClient = dialogue_window_;
    data.pszName = plugin_name_.c_str();
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
    data.pszModuleName = module_name_.c_str();

    plugin_->send_to_notepad(NPPM_DMMREGASDCKDLG, 0, &data);

    // Hide the dialogue by default. This allows Notepad++ to open it as
    // necessary at startup. If being opened in response to a menu
    // selection, just call display()
    hide();
}

void Docking_Dialogue_Interface::InvalidateRect(RECT const *rect) const noexcept
{
    ::InvalidateRect(dialogue_window_, rect, TRUE);
}

RECT Docking_Dialogue_Interface::getClientRect() const noexcept
{
    RECT rc;
    ::GetClientRect(dialogue_window_, &rc);
    return rc;
}

RECT Docking_Dialogue_Interface::getWindowRect() const noexcept
{
    RECT rc;
    ::GetWindowRect(dialogue_window_, &rc);
    return rc;
}

HWND Docking_Dialogue_Interface::GetDlgItem(int item) const noexcept
{
    return ::GetDlgItem(dialogue_window_, item);
}

void Docking_Dialogue_Interface::on_display() noexcept
{
}

void Docking_Dialogue_Interface::on_hide() noexcept
{
}

std::optional<LONG_PTR> Docking_Dialogue_Interface::on_dialogue_message(
    UINT message, UINT_PTR wParam, LONG_PTR lParam
)
{
    return std::nullopt;
}

void Docking_Dialogue_Interface::send_dialogue_info(
    int msg, int wParam
) noexcept
{
    plugin_->send_to_notepad(msg, wParam, dialogue_window_);
}

INT_PTR __stdcall Docking_Dialogue_Interface::process_dialogue_message(
    HWND window_handle, UINT message, WPARAM wParam, LPARAM lParam
) noexcept
{
#pragma warning(suppress : 26490)
    auto instance = reinterpret_cast<Docking_Dialogue_Interface *>(
        ::GetWindowLongPtr(window_handle, GWLP_USERDATA)
    );
    if (instance == nullptr)
    {
        return FALSE;
    }

    try
    {
        auto retval = instance->on_dialogue_message(message, wParam, lParam);
        if (! retval)
        {
            retval =
                instance->unhandled_dialogue_message(message, wParam, lParam);
        }
        if (retval)
        {
            SetWindowLongPtr(window_handle, DWLP_MSGRESULT, *retval);
        }
        return static_cast<bool>(retval);
    }
    catch (std::exception const &e)
    {
        try
        {
            ::MessageBox(
                window_handle,
                static_cast<wchar_t *>(static_cast<_bstr_t>(e.what())),
                instance->plugin_name_.c_str(),
                MB_OK | MB_ICONERROR
            );
        }
        catch (std::exception const &)
        {
            ::MessageBox(
                window_handle,
                L"Caught exception but cannot get reason",
                instance->plugin_name_.c_str(),
                MB_OK | MB_ICONERROR
            );
        }
        return FALSE;
    }
}

std::optional<LONG_PTR> Docking_Dialogue_Interface::unhandled_dialogue_message(
    UINT message, WPARAM, LPARAM lParam
) noexcept
{
    switch (message)
    {
        case WM_NOTIFY:
        {
#pragma warning(suppress : 26490)
            NMHDR const *pnmh = reinterpret_cast<LPNMHDR>(lParam);

            if (pnmh->hwndFrom == plugin_->get_notepad_window())
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
            ::RedrawWindow(dialogue_window_, nullptr, nullptr, RDW_INVALIDATE);
            return std::nullopt;

        default:
            break;
    }
    return std::nullopt;
}

HWND Docking_Dialogue_Interface::create_dialogue_window(int dialogID)
{
#pragma warning(suppress : 26490)
    HWND const dialogue_window{::CreateDialogParam(
        plugin_->module(),
        MAKEINTRESOURCE(dialogID),
        plugin_->get_notepad_window(),
        process_dialogue_message,
        reinterpret_cast<LPARAM>(this)
    )};
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
