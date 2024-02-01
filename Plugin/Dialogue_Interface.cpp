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

#include "Dialogue_Interface.h"

#include "Plugin.h"

#include <ShlwApi.h>
#include <WinUser.h>
#include <comutil.h>
#include <errhandlingapi.h>
#include <libloaderapi.h>

#include <cstdio>
#include <exception>
#include <stdexcept>
#include <string>
#include <system_error>
#include <vector>

#ifdef _DEBUG
#pragma comment(lib, "comsuppwd.lib")
#else
#pragma comment(lib, "comsuppw.lib")
#endif
#pragma comment(lib, "shlwapi.lib")

namespace
{

std::wstring get_module_name(HINSTANCE module_instance)
{
    wchar_t temp[MAX_PATH] = {0};
    ::GetModuleFileName(module_instance, &temp[0], MAX_PATH);
    return ::PathFindFileName(&temp[0]);
}

std::wstring get_dialogue_name(HWND dialog_handle)
{
    wchar_t temp[MAX_PATH] = {0};
    ::GetWindowText(dialog_handle, &temp[0], MAX_PATH);
    return &temp[0];
}

}    // namespace

Dialogue_Interface::Dialogue_Interface(Plugin const *plugin) :
    plugin_(plugin),
    module_name_(get_module_name(plugin_->module())),
    dialogue_window_()
{
}

Dialogue_Interface::~Dialogue_Interface()
{
    // Stop process_dialogue_message from doing anything, since it calls a
    // virtual method which won't be there.
    ::SetWindowLongPtr(dialogue_window_, GWLP_USERDATA, NULL);
    ::DestroyWindow(dialogue_window_);
}

void Dialogue_Interface::InvalidateRect(RECT const *rect) const noexcept
{
    ::InvalidateRect(dialogue_window_, rect, TRUE);
}

RECT Dialogue_Interface::getClientRect() const noexcept
{
    RECT rc;
    ::GetClientRect(dialogue_window_, &rc);
    return rc;
}

RECT Dialogue_Interface::getWindowRect() const noexcept
{
    RECT rc;
    ::GetWindowRect(dialogue_window_, &rc);
    return rc;
}

RECT Dialogue_Interface::getParentRect() const noexcept
{
    RECT rc;
    ::GetWindowRect(plugin_->get_notepad_window(), &rc);
    return rc;
}

HWND Dialogue_Interface::GetDlgItem(int item, HWND window) const noexcept
{
    return ::GetDlgItem(window == nullptr ? dialogue_window_ : window, item);
}

std::wstring Dialogue_Interface::get_window_text(int item, HWND window) const
{
    auto handle = GetDlgItem(item, window);
    int length = GetWindowTextLength(handle) + 1;
    std::vector<wchar_t> buffer;
    buffer.resize(length);
    length = GetWindowText(handle, &*buffer.begin(), length);
    return std::wstring(&*buffer.begin(), length);
}

void Dialogue_Interface::SetFocus(int item) const noexcept
{
    ::SetFocus(GetDlgItem(item));
}

int Dialogue_Interface::message_box(wchar_t const *message, UINT type)
    const noexcept
{
    return ::MessageBox(
        dialogue_window_, message, dialogue_name_.c_str(), type
    );
}

HWND Dialogue_Interface::create_dialogue(int dialogue, HWND parent) noexcept(
    false
)
{
#pragma warning(suppress : 26490)
    auto dialogue_window = ::CreateDialogParam(
        plugin()->module(),
        MAKEINTRESOURCE(dialogue),
        parent == nullptr ? plugin()->get_notepad_window() : parent,
        process_dialogue_message,
        reinterpret_cast<LPARAM>(this)
    );
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

INT_PTR Dialogue_Interface::create_modal_dialogue(int dialogue) noexcept
{
#pragma warning(suppress : 26490)
    return ::DialogBoxParam(
        plugin()->module(),
        MAKEINTRESOURCE(dialogue),
        plugin()->get_notepad_window(),
        process_dialogue_message,
        reinterpret_cast<LPARAM>(this)
    );
}

void Dialogue_Interface::add_item_callback(
    int item, Item_Callback_Function callback_func
)
{
    HWND handle = GetDlgItem(item);
    if (callbacks_[handle].empty())
    {
        ::SetWindowLongPtr(
            handle,
            GWLP_USERDATA,
#pragma warning(suppress : 26490)
            reinterpret_cast<LONG_PTR>(this)
        );
    }
#pragma warning(suppress : 26490)
    auto const old_proc = SetWindowLongPtr(
        handle,
        GWLP_WNDPROC,
        reinterpret_cast<LONG_PTR>(process_subclassed_message)
    );
    Callback_Info info{old_proc, callback_func};
    callbacks_[handle].push_back(info);
}

std::optional<INT_PTR> Dialogue_Interface::on_dialogue_message(
    UINT message, WPARAM wParam, LPARAM lParam
) noexcept(false)
{
    return std::nullopt;
}

std::optional<INT_PTR> Dialogue_Interface::on_unhandled_dialogue_message(
    UINT message, WPARAM wParam, LPARAM lParam
) noexcept
{
    return std::nullopt;
}

INT_PTR __stdcall Dialogue_Interface::process_dialogue_message(
    HWND window_handle, UINT message, WPARAM wParam, LPARAM lParam
) noexcept
{
    if (message == WM_INITDIALOG)
    {
#pragma warning(suppress : 26490)
        auto const instance = reinterpret_cast<Dialogue_Interface *>(lParam);
        instance->dialogue_window_ = window_handle;
        try
        {
            instance->dialogue_name_ = get_dialogue_name(window_handle);
        }
        catch (std::exception const &)
        {
            ::MessageBox(
                window_handle,
                L"Failed to store dialogue name",
                instance->module_name_.c_str(),
                MB_OK | MB_ICONERROR
            );
        }

        ::SetWindowLongPtr(
            window_handle,
            GWLP_USERDATA,
#pragma warning(suppress : 26472)
            static_cast<LONG_PTR>(lParam)
        );
    }
#pragma warning(suppress : 26490)
    auto const instance = reinterpret_cast<Dialogue_Interface *>(
        ::GetWindowLongPtr(window_handle, GWLP_USERDATA)
    );
    if (instance == nullptr)
    {
        return FALSE;
    }

    try
    {
        auto retval = instance->on_dialogue_message(message, wParam, lParam);
        if (not retval.has_value())
        {
            retval = instance->on_unhandled_dialogue_message(
                message, wParam, lParam
            );
        }
        if (retval.has_value())
        {
            SetWindowLongPtr(window_handle, DWLP_MSGRESULT, retval.value());
        }
        return retval.has_value();
    }
    catch (std::exception const &e)
    {
        try
        {
            instance->message_box(
                static_cast<wchar_t *>(static_cast<_bstr_t>(e.what())),
                MB_OK | MB_ICONERROR
            );
        }
        catch (std::exception const &)
        {
            instance->message_box(
                L"Caught exception but cannot get reason", MB_OK | MB_ICONERROR
            );
        }
        return FALSE;
    }
}

LRESULT CALLBACK Dialogue_Interface::process_subclassed_message(
    HWND handle, UINT message, WPARAM wParam, LPARAM lParam
) noexcept
{
#pragma warning(suppress : 26490)
    auto const instance = reinterpret_cast<Dialogue_Interface *>(
        ::GetWindowLongPtr(handle, GWLP_USERDATA)
    );
    try
    {
        Callback_Info info = instance->callbacks_.at(handle).back();
        auto res = (info.callback_func)(handle, message, wParam, lParam);
        if (res.has_value())
        {
            return res.value();
        }
#pragma warning(suppress : 26490)
        return (*reinterpret_cast<WNDPROC>(info.old_proc))(handle, message, wParam, lParam);
    }
    catch (std::exception const &e)
    {
        try
        {
            instance->message_box(
                static_cast<wchar_t *>(static_cast<_bstr_t>(e.what())),
                MB_OK | MB_ICONERROR
            );
        }
        catch (std::exception const &)
        {
            instance->message_box(
                L"Caught exception but cannot get reason",
                MB_OK | MB_ICONERROR
            );
        }
        return FALSE;
    }
}
