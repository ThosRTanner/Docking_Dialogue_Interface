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

#include <basetsd.h>

#include <optional>
#include <string>

// Forward declarations from windows headers
typedef struct tagRECT RECT;
typedef struct HWND__ *HWND;
typedef struct HICON__ *HICON;
typedef unsigned int UINT;
typedef UINT_PTR WPARAM;
typedef LONG_PTR LPARAM;

// Forward declarations.
class Plugin;

/** This is designed as a base class, you cannot instantiate this.
 *
 * It is recommended that you use private or protected inheritance.
 */
class Docking_Dialogue_Interface
{
  public:
    /** Create a docking dialogue.
     *
     * dialogue_id is the resource number of the dialogue
     */
    Docking_Dialogue_Interface(int dialogue_id, Plugin const *plugin);

    Docking_Dialogue_Interface(Docking_Dialogue_Interface const &) = delete;
    Docking_Dialogue_Interface(Docking_Dialogue_Interface &&) = delete;
    Docking_Dialogue_Interface &operator=(Docking_Dialogue_Interface const &) =
        delete;
    Docking_Dialogue_Interface &operator=(Docking_Dialogue_Interface &&) =
        delete;

    virtual ~Docking_Dialogue_Interface() = 0;

    /** Call this to display the dialogue. */
    void display() noexcept;

    /** Call this to hide the dialogue. */
    void hide() noexcept;

    /** Find out if the dialogue has been hidden. */
    bool is_hidden() const noexcept
    {
        return is_hidden_;
    }

  protected:
    /** Where to place dialogue initially */
    enum class Position
    {
        Dock_Left,
        Dock_Right,
        Dock_Top,
        Dock_Bottom,
        Floating
    };

    /** Register dialogue with Notepad++.
     *
     * I'm not a fan of 2-phase initialisation, but this bit has to be done
     * after the dialogue is actually created, or things go wrong as notepad++
     * will start sending messages to the dialogue, and the instance will not
     * yet have been initialised.
     *
     * menu_index is the ID used to communicate with notepad++ (i.e. the menu
     * entry).
     *
     * extra is extra text to display on dialogue title.
     */
    void register_dialogue(
        int menu_index, Position, HICON = nullptr,
        wchar_t const *extra = nullptr
    ) noexcept;

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

    /** Requests a redraw */
    void InvalidateRect(RECT const *rect = nullptr) const noexcept;

    /** Utility to get the current client rectangle */
    RECT getClientRect() const noexcept;

    /** Utility to get the current window rectangle */
    RECT getWindowRect() const noexcept;

    /** Utility to get a dialogue item */
    HWND GetDlgItem(int) const noexcept;

  private:
    /** Implement this if you have your own stuff to do when displaying */
    virtual void on_display() noexcept;

    /** Implement this if you have your own stuff to do when hiding */
    virtual void on_hide() noexcept;

    /** Implement this to handle messages.
     *
     * Return std::nullopt to (to return FALSE to windows dialog processing), or
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
    virtual std::optional<LONG_PTR> on_dialogue_message(
        UINT message, UINT_PTR wParam, LONG_PTR lParam
    ) noexcept(false);

    /** Utility wrapper round SendMessage to send pointers to our self */
    void send_dialogue_info(int msg, int wParam = 0) noexcept;

    /** Callback handler for messages */
    static INT_PTR __stdcall process_dialogue_message(
        HWND, UINT message, WPARAM, LPARAM
    ) noexcept;

    /** Handler for unhandled messages */
    std::optional<LONG_PTR> unhandled_dialogue_message(
        UINT message, UINT_PTR wParam, LONG_PTR lParam
    ) noexcept;

    /** Called during construction to set up dialogue_window_ */
    HWND create_dialogue_window(int dialogID);

    Plugin const *plugin_;
    HWND dialogue_window_;
    int docked_pos_ = 0;
    bool is_floating_ = true;
    bool is_hidden_ = false;
    std::wstring module_name_;
    std::wstring plugin_name_;
};
