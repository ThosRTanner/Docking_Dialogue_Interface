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

#include "Dialogue_Interface.h"

#include <basetsd.h>

#include <optional>

// Forward declarations from windows headers
typedef struct HICON__ *HICON;

// Forward declarations.
class Plugin;

/** This provides an abstraction for creating a docking dialogue. */
class Docking_Dialogue_Interface : public Dialogue_Interface
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

  private:
    /** Implement this if you have your own stuff to do when displaying */
    virtual void on_display() noexcept;

    /** Implement this if you have your own stuff to do when hiding */
    virtual void on_hide() noexcept;

    /** Utility wrapper round SendMessage to send pointers to our self */
    void send_dialogue_info(int msg, int wParam = 0) noexcept;

    /** Handler for unhandled messages */
    std::optional<LONG_PTR> on_unhandled_dialogue_message(
        UINT message, WPARAM wParam, LPARAM lParam
    ) noexcept override final;

    /** Called during construction to set up dialogue_window_ */
    HWND create_dialogue(int dialogID);

    /** Hide the other creation method */
    using Dialogue_Interface::create_modal_dialogue;

    int docked_pos_ = 0;
    bool is_floating_ = true;
    bool is_hidden_ = false;
};
