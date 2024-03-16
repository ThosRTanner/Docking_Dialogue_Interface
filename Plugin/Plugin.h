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

#pragma once

#include "PluginInterface.h"

#include <memory>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>

class SCNotification;

class Plugin
{
  public:
    Plugin(NppData const &, std::wstring_view name);

    virtual ~Plugin();

    Plugin(Plugin const &) = delete;
    Plugin(Plugin &&) = delete;
    Plugin &operator=(Plugin const &) = delete;
    Plugin &operator=(Plugin &&) = delete;

    /** Get the module handle */
    HINSTANCE module() const noexcept
    {
        return module_;
    }

    // Notepad++ wrappers

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

    /** Get the config directory */
    std::wstring get_config_dir() const;

    /** Get the current document path */
    std::wstring get_document_path() const;

    // Scintilla wrappers

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

    /** Get the contents of the current document */
    std::string get_document_text() const;

    /** Get the contents of the specified line in the current document */
    std::string get_line_text(int line) const;

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
