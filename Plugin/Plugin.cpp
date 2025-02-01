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

#include "notepad++/Notepad_plus_msgs.h"
#include "notepad++/Scintilla.h"

#include <libloaderapi.h>

#include <windows.h>    // IWYU pragma: keep

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

std::filesystem::path Plugin::get_config_dir() const
{
    auto const len = send_to_notepad(NPPM_GETPLUGINSCONFIGDIR, 0, nullptr);
    auto buff{std::make_unique_for_overwrite<wchar_t[]>(len + 1)};
    wchar_t const *const dir = buff.get();
    send_to_notepad(NPPM_GETPLUGINSCONFIGDIR, len * sizeof(wchar_t), dir);
    return std::wstring(dir, len);
}

std::filesystem::path Plugin::get_plugin_config_dir() const
{
    auto cfg_dir = get_config_dir();
    cfg_dir.append(name_);
    std::filesystem::create_directories(cfg_dir);
    return cfg_dir;
}

std::filesystem::path Plugin::get_document_path() const
{
    return get_document_path(send_to_notepad(NPPM_GETCURRENTBUFFERID));
}

std::filesystem::path Plugin::get_document_path(uptr_t buffer_id) const
{
    auto const len =
        send_to_notepad(NPPM_GETFULLPATHFROMBUFFERID, buffer_id, nullptr);
    auto buff{std::make_unique_for_overwrite<wchar_t[]>(len + 1)};
    send_to_notepad(NPPM_GETFULLPATHFROMBUFFERID, buffer_id, buff.get());
    return std::wstring(buff.get(), len);
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
    auto const length = send_to_editor(SCI_GETLENGTH);
    auto buff{std::make_unique_for_overwrite<char[]>(length + 1)};
    send_to_editor(SCI_GETTEXT, length, buff.get());
    return std::string(buff.get(), length);
}

std::string Plugin::get_line_text(int line) const
{
    auto const length = send_to_editor(SCI_LINELENGTH, line);
    auto buff{std::make_unique_for_overwrite<char[]>(length + 1)};
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

LRESULT Plugin::on_message(UINT /*message*/, WPARAM, LPARAM)
{
    return TRUE;
}

// I should arguably use __declspec(dllexport) here, but it puts the classname
// in the exported name, despite being in an extern "C" block, so I have to do
// the hard work myself
extern "C"
{
#pragma warning(suppress : 26429)
    FuncItem *Plugin::getFuncsArray(int *nbF)
    {
#ifdef __FUNCDNAME__
#pragma comment(linker, "/EXPORT:getFuncsArray=" __FUNCDNAME__)
#endif
        auto &res = plugin->on_get_menu_entries();
        *nbF = static_cast<int>(res.size());
        return &*res.begin();
    }

    void Plugin::beNotified(SCNotification const *notification)
    {
#ifdef __FUNCDNAME__
#pragma comment(linker, "/EXPORT:beNotified=" __FUNCDNAME__)
#endif
        plugin->on_notification(notification);
    }

    LRESULT
    Plugin::messageProc(UINT message, WPARAM wParam, LPARAM lParam)
    {
#ifdef __FUNCDNAME__
#pragma comment(linker, "/EXPORT:messageProc=" __FUNCDNAME__)
#endif
        return plugin->on_message(message, wParam, lParam);
    }

    /** This must be defined and must always return TRUE
     *
     * It dates from when notepad++ had a unicode and a non unicode version
     */
    __declspec(dllexport) BOOL isUnicode()
    {
        return TRUE;
    }
}
