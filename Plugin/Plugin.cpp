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

#include "Plugin.h"

#include "Notepad_plus_msgs.h"
#include "Scintilla.h"

#include <libloaderapi.h>
#include <windows.h> // IWYU pragma: keep

#include <winuser.h>

#include <memory>
#include <string>
#include <vector>

static Plugin *plugin;

Plugin::Plugin(NppData const &data, std::wstring_view name) :
    npp_data_(data),
    name_(name)
{
    GetModuleHandleEx(
        GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS
            | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
        L"",
        &module_
    );
    plugin = this;
}

Plugin::~Plugin() = default;

LRESULT Plugin::send_to_notepad(UINT message, WPARAM wParam, LPARAM lParam)
    const noexcept
{
    return ::SendMessage(npp_data_._nppHandle, message, wParam, lParam);
}

std::wstring Plugin::get_config_dir() const
{
    auto const len = send_to_notepad(NPPM_GETPLUGINSCONFIGDIR, 0, nullptr);
#if __cplusplus >= 202002L
    auto buff{std::make_unique_for_overwrite<wchar_t[]>(len + 1)};
#else
#pragma warning(suppress : 26409 26414)
    std::unique_ptr<wchar_t[]> buff{new wchar_t[len + 1]};
#endif
    send_to_notepad(
        NPPM_GETPLUGINSCONFIGDIR, len * sizeof(wchar_t), buff.get()
    );
    return std::wstring(buff.get(), len);
}

std::wstring Plugin::get_document_path() const
{
#if __cplusplus >= 202002L
    auto buff{std::make_unique_for_overwrite<wchar_t[]>(MAX_PATH)};
#else
#pragma warning(suppress : 26409 26414)
    std::unique_ptr<wchar_t[]> buff{new wchar_t[MAX_PATH]};
#endif
    send_to_notepad(NPPM_GETFULLCURRENTPATH, 0, buff.get());
    return std::wstring(buff.get());
}

HWND Plugin::get_scintilla_window() const noexcept
{
    LRESULT const view = send_to_notepad(NPPM_GETCURRENTVIEW);
    return view == 0 ? npp_data_._scintillaMainHandle
                     : npp_data_._scintillaSecondHandle;
}

LRESULT Plugin::send_to_editor(UINT Msg, WPARAM wParam, LPARAM lParam)
    const noexcept
{
    return SendMessage(get_scintilla_window(), Msg, wParam, lParam);
}

std::string Plugin::get_document_text() const
{
    LRESULT const length = send_to_editor(SCI_GETLENGTH);
#if __cplusplus >= 202002L
    auto buff{std::make_unique_for_overwrite<char[]>(length + 1)};
#else
#pragma warning(suppress : 26409 26414)
    std::unique_ptr<char[]> buff{new char[length + 1]};
#endif
    send_to_editor(SCI_GETTEXT, length, buff.get());
    return std::string(buff.get(), length);
}

std::string Plugin::get_line_text(int line) const
{
    LRESULT const length = send_to_editor(SCI_LINELENGTH, line);
#if __cplusplus >= 202002L
    auto buff{std::make_unique_for_overwrite<char[]>(length + 1)};
#else
#pragma warning(suppress : 26409 26414)
    std::unique_ptr<char[]> buff{new char[length + 1]};
#endif
    send_to_editor(SCI_GETLINE, line, buff.get());
    return std::string(buff.get(), length);
}

int Plugin::message_box(std::wstring const &message, UINT type) const noexcept
{
    return ::MessageBox(
        get_notepad_window(), message.c_str(), name_.c_str(), type
    );
}

void Plugin::on_notification(SCNotification const *)
{
}

LRESULT Plugin::on_message(UINT message, WPARAM, LPARAM)
{
    return TRUE;
}

// I should arguably use __declspec(dllexport) here, but it puts the classname
// in the exported name, despite being in an extern "C" block, so I have to do
// the hard work myself
extern "C"
{
    FuncItem *Plugin::getFuncsArray(int *nbF)
    {
#ifdef __FUNCDNAME__
#pragma comment(linker, "/EXPORT:getFuncsArray=" __FUNCDNAME__)
#endif
        auto &res = plugin->on_get_menu_entries();
        *nbF = static_cast<int>(res.size());
        return &*res.begin();
    }

    void Plugin::beNotified(SCNotification *notification)
    {
#ifdef __FUNCDNAME__
#pragma comment(linker, "/EXPORT:beNotified=" __FUNCDNAME__)
#endif
        plugin->on_notification(notification);
    }

    LRESULT Plugin::messageProc(UINT message, WPARAM wParam, LPARAM lParam)
    {
#ifdef __FUNCDNAME__
#pragma comment(linker, "/EXPORT:messageProc=" __FUNCDNAME__)
#endif
        return plugin->on_message(message, wParam, lParam);
    }
}
