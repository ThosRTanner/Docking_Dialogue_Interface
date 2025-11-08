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

#include "Min_Win_Defs.h"

// IWYU pragma: no_include <windef.h>

// Forward declarations.
class Plugin;

/** This provides the base abstraction for creating a non modal dialogue. */
class Non_Modal_Dialogue_Base : public Dialogue_Interface
{
    using Super = Dialogue_Interface;

  public:
    /** Create a docking dialogue.
     *
     * dialogue_id is the resource number of the dialogue
     */
    Non_Modal_Dialogue_Base(
        int dialogue_id, Plugin const &plugin, HWND parent = nullptr
    );

    Non_Modal_Dialogue_Base(Non_Modal_Dialogue_Base const &) = delete;
    Non_Modal_Dialogue_Base(Non_Modal_Dialogue_Base &&) = delete;
    Non_Modal_Dialogue_Base &operator=(Non_Modal_Dialogue_Base const &) =
        delete;
    Non_Modal_Dialogue_Base &operator=(Non_Modal_Dialogue_Base &&) = delete;

    ~Non_Modal_Dialogue_Base() override = 0;

  protected:
    /** Utility wrapper round SendMessage to send pointers to our self */
    void send_dialogue_info(int msg, int wParam = 0) noexcept;

  private:
    /** Implement this to handle other unhandled messages */
    virtual Message_Return on_unhandled_non_modal_dialogue_message(
        UINT message, WPARAM wParam, LPARAM lParam
    ) noexcept;

    /** Handler for unhandled messages */
    Message_Return on_unhandled_dialogue_message(
        UINT message, WPARAM wParam, LPARAM lParam
    ) noexcept final;

    /** Hide the other creation method */
    using Dialogue_Interface::create_modal_dialogue;
};
