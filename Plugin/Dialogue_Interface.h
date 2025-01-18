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

#include "Min_Win_Defs.h"

#include <intsafe.h>

#include <functional>
#include <list>
#include <optional>
#include <string>
#include <unordered_map>

// Forward references
class Plugin;

class Dialogue_Interface
{
  public:
    Dialogue_Interface(Plugin const *plugin);

    Dialogue_Interface(Dialogue_Interface const &) = delete;
    Dialogue_Interface(Dialogue_Interface &&) = delete;
    Dialogue_Interface &operator=(Dialogue_Interface const &) = delete;
    Dialogue_Interface &operator=(Dialogue_Interface &&) = delete;

    virtual ~Dialogue_Interface();

  protected:
    /** Get hold of plugin object for useful boilerplate */
    Plugin const *plugin() const noexcept
    {
        return plugin_;
    }

    /** Get hold of the current dialogue window handle */
    HWND window() const noexcept
    {
        return dialogue_window_;
    }

    wchar_t const *module_name() const noexcept
    {
        return module_name_.c_str();
    }

    wchar_t const *dialogue_name() const noexcept
    {
        return dialogue_name_.c_str();
    }

    /** Requests a redraw */
    void InvalidateRect(RECT const *rect = nullptr) const noexcept;

    /** Utility to get the current client rectangle */
    RECT getClientRect() const noexcept;

    /** Utility to get the current window rectangle */
    RECT getWindowRect() const noexcept;

    /** Utility to get the parent window rectangle */
    RECT getParentRect() const noexcept;

    /** Utility to get a dialogue item */
    HWND GetDlgItem(int, HWND window = nullptr) const noexcept;

    /** Get text of item */
    std::wstring get_window_text(int, HWND window = nullptr) const;

    /** Set the focus to the given item */
    void SetFocus(int) const noexcept;

    /** Throw up a message box
     *
     * The title will be the same as the docking dialogue title.
     */
    int message_box(wchar_t const *message, UINT type) const noexcept;

    /** Creates a non-modal dialogue.
     *
     * Note: This is virtual so that the xxxx_Dialogue_Interface classes can
     * hide it.
     */
    virtual HWND create_dialogue(int dialogue, HWND parent);

    /** Create a modal dialogue.
     *
     * Note: This is virtual so that the xxxx_Dialogue_Interface classes can
     * hide it.
     */
    virtual INT_PTR create_modal_dialogue(int dialogue) noexcept;

    typedef std::optional<LRESULT> Item_Callback_Return;
    typedef std::function<Item_Callback_Return(HWND, UINT, WPARAM, LPARAM)>
        Item_Callback_Function;

    /** Allows you to subclass a window element, to intercept events on it */
    void add_item_callback(int item, Item_Callback_Function callback_func);

    /** Return type for dialogue message callbacks */
    typedef std::optional<INT_PTR> Message_Return;

  private:
    /** Implement this to handle messages.
     *
     * Return std::nullopt to return FALSE to windows dialog processing, or
     * a value to be set with SetWindowLongPtr (in which case TRUE will be
     * returned). Note that some messages require you to return FALSE
     * (std::nullopt) even if you do handle them.
     *
     * If you don't handle the message, you MUST call the base class version of
     * this.
     *
     * message, wParam and lParam are the values passed to
     * process_dialogue_message by windows
     */
    virtual Message_Return on_dialogue_message(
        UINT message, WPARAM wParam, LPARAM lParam
    ) noexcept(false);

    /** Handler for unhandled messages */
    virtual Message_Return on_unhandled_dialogue_message(
        UINT message, WPARAM wParam, LPARAM lParam
    ) noexcept;

    /** Callback handler for messages */
    static INT_PTR __stdcall process_dialogue_message(
        HWND, UINT message, WPARAM, LPARAM
    ) noexcept;

    static LRESULT __stdcall process_subclassed_message(
        HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam
    ) noexcept;

    Plugin const *plugin_;
    std::wstring module_name_;
    HWND dialogue_window_;
    std::wstring dialogue_name_;

    struct Callback_Info
    {
        LONG_PTR old_proc;
        Item_Callback_Function callback_func;
    };

    std::unordered_map<HANDLE, std::list<Callback_Info>> callbacks_;
};
