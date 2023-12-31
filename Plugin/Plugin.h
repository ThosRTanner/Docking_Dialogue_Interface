#pragma once

#include "PluginInterface.h"

#include <memory>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>

class Plugin
{
  public:
    Plugin(NppData const &, std::wstring_view name);

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
    template <
        typename Callbacks, typename Context, typename Class, typename Callback>
    FuncItem make_callback(
        int entry, wchar_t const *message, Callbacks &contexts, Context context,
        Class self, Callback callback, bool checked = false,
        ShortcutKey *key = nullptr
    )
    {
        contexts[entry] = std::make_unique<Context>(context);
        //In C++20 this could be made a little easier to read.
        FuncItem item;
        std::ignore = lstrcpyn(
            &item._itemName[0],
            message,
            sizeof(item._itemName) / sizeof(wchar_t)
        );
        item._pFunc = contexts[entry]->reserve(self, callback);
        item._cmdID = entry;
        item._init2Check = checked;
        item._pShKey = key;
        return item;
    }

    /** Wrapper around make_callback for separators */
    template <typename Callbacks, typename Context, typename Class>
    FuncItem make_separator(
        int entry, Callbacks &contexts, Context context, Class self
    )
    {
        return make_callback(entry, L"---", contexts, context, self, nullptr);
    }

    /** Throw up a message box
     *
     * The title will be the string passed in the class constructor
     *
     * This would take a wstring_view but there's no guarantee that that is null
     * terminated.
     */
    int message_box(std::wstring const &message, UINT type) const noexcept;

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
    std::wstring name_;
};

#define DEFINE_PLUGIN_MENU_CALLBACKS(class)         \
    typedef Callback_Context_Base<class> Callbacks; \
    template <>                                     \
    Callbacks::Contexts Callbacks::contexts = {}

// Warning - this won't work if you have /Zc:preprocessor set
// Could try __VA_OPT__ and C++20

#define PLUGIN_MENU_MAKE_CALLBACK(class, entry, text, method, ...) \
    make_callback(                                                 \
        entry,                                                     \
        text,                                                      \
        Callbacks::contexts,                                       \
        Callback_Context<class, entry>(),                          \
        this,                                                      \
        &class::method,                                            \
        __VA_ARGS__                                                \
    )

#define PLUGIN_MENU_MAKE_SEPARATOR(class, entry)                           \
    make_separator(                                                        \
        entry, Callbacks::contexts, Callback_Context<class, entry>(), this \
    )
