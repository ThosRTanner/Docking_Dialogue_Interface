#include "Dialogue_Interface.h"

#include "Plugin.h"

#include <ShlwApi.h>
#include <WinUser.h>
#include <comutil.h>

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
    TCHAR temp[MAX_PATH] = {0};
    ::GetModuleFileName(module_instance, &temp[0], MAX_PATH);
    return ::PathFindFileName(&temp[0]);
}

std::wstring get_dialogue_name(HWND dialog_handle)
{
    TCHAR temp[MAX_PATH] = {0};
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

HWND Dialogue_Interface::GetDlgItem(int item) const noexcept
{
    return ::GetDlgItem(dialogue_window_, item);
}

int Dialogue_Interface::message_box(wchar_t const *message, UINT type)
    const noexcept
{
    return ::MessageBox(
        dialogue_window_, message, dialogue_name_.c_str(), type
    );
}

std::optional<LONG_PTR> Dialogue_Interface::on_dialogue_message(
    UINT message, WPARAM wParam, LPARAM lParam
) noexcept(false)
{
    return std::nullopt;
}

std::optional<LONG_PTR> Dialogue_Interface::on_unhandled_dialogue_message(
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
        if (! retval)
        {
            retval =
                instance->on_unhandled_dialogue_message(message, wParam, lParam);
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
