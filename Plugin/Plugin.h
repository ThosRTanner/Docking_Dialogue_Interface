#pragma once

#include "PluginInterface.h"

#include <vector>

class Plugin
{
  public:
    Plugin(NppData const &) noexcept;

    virtual ~Plugin();

    Plugin(Plugin const &) = delete;
    Plugin(Plugin &&) = delete;
    Plugin &operator=(Plugin const &) = delete;
    Plugin &operator=(Plugin &&) = delete;

    /** Get the notepad++ window */
    HWND get_notepad_window() const noexcept
    {
        return npp_data_._nppHandle;
    }

    /** Send a message to notepad++ */
    LRESULT send_to_notepad(UINT message, WPARAM = 0, LPARAM = 0)
        const noexcept;

    /** Send a message which includes a buffer */
    LRESULT send_to_notepad(UINT message, WPARAM wParam, void const *buff)
        const noexcept
    {
#pragma warning(suppress : 26490)
        return send_to_notepad(message, wParam, reinterpret_cast<LPARAM>(buff));
    }

    /** Get the current scintilla window */
    HWND get_scintilla_window() const noexcept;

    /** Send a message to the current editor window */
    LRESULT send_to_editor(UINT message, WPARAM = 0, LPARAM = 0) const noexcept;

    LRESULT send_to_editor(UINT message, WPARAM wParam, void const *buff)
        const noexcept
    {
#pragma warning(suppress : 26490)
        return send_to_editor(message, wParam, reinterpret_cast<LPARAM>(buff));
    }

    /** Get the module handle */
    HINSTANCE module() const noexcept;

  protected:
    /** Utility function to aid setting up notepad++ menu definition */
    template <typename Callbacks, typename Class, typename Callback>
    FuncItem make_callback(
        int entry, wchar_t const *message, Callbacks contexts, Class self,
        Callback callback, bool check = false, ShortcutKey *key = nullptr
    )
    {
        FuncItem item;
        item._cmdID = entry;
        lstrcpyn(&item._itemName[0], message, sizeof(item._itemName));
        item._pFunc = contexts[entry]->reserve(self, callback);
        item._init2Check = check;
        item._pShKey = key;
        return item;
    }

  private:
    // Classes should override these methods.

    /** get the list of menu items (from getFuncsArray)
     *
     * Note that you must return at least one item, or notepad++ will fail.
     */
    virtual std::vector<FuncItem> &on_get_menu_entries() noexcept(false) = 0;

    /** Process scintilla notifications (from beNotified) */
    virtual void on_notification(SCNotification const *) noexcept(false);

    /** Process notepad++ messages (from messageProc) */
    virtual LRESULT on_message(UINT message, WPARAM, LPARAM) noexcept(false);

    /** These 3 are APIs that notepad++ expects to exist
     *
     * We use them to bounce into a class method.
     */
    static FuncItem *getFuncsArray(int *);
    static void beNotified(SCNotification *);
    static LRESULT messageProc(UINT, WPARAM, LPARAM);

    HINSTANCE module_;
    NppData npp_data_;
};
