#include "Plugin.h"

#include "libloaderapi.h"

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

HINSTANCE Plugin::module() const noexcept
{
    return module_;
}

int Plugin::message_box(std::wstring const & message, UINT type) const noexcept
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
#pragma comment(linker, "/EXPORT:getFuncsArray=" __FUNCDNAME__)
        auto &res = plugin->on_get_menu_entries();
        *nbF = static_cast<int>(res.size());
        return &res[0];
    }

    void Plugin::beNotified(SCNotification *notification)
    {
#pragma comment(linker, "/EXPORT:beNotified=" __FUNCDNAME__)
        plugin->on_notification(notification);
    }

    LRESULT Plugin::messageProc(UINT message, WPARAM wParam, LPARAM lParam)
    {
#pragma comment(linker, "/EXPORT:messageProc=" __FUNCDNAME__)
        return plugin->on_message(message, wParam, lParam);
    }
}
