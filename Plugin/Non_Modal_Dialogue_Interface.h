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

#include "Non_Modal_Dialogue_Base.h"

#include "Min_Win_Defs.h"

// This doesn't actually provide anything useful, but IWYU can't currently
// work out that functions declared as override don't need to a separate header
#include <intsafe.h>

// Forward references
class Plugin;

/** This provides an abstraction for creating a non modal non docking dialogue.
 *
 */
class Non_Modal_Dialogue_Interface : public Non_Modal_Dialogue_Base
{
    typedef Non_Modal_Dialogue_Base Super;

  public:
    /** Create a docking dialogue.
     *
     * dialogue_id is the resource number of the dialogue
     */
    Non_Modal_Dialogue_Interface(
        int dialogue_id, Plugin const *plugin, HWND parent = nullptr
    );

    Non_Modal_Dialogue_Interface(Non_Modal_Dialogue_Interface const &) = delete;
    Non_Modal_Dialogue_Interface(Non_Modal_Dialogue_Interface &&) = delete;
    Non_Modal_Dialogue_Interface &
    operator=(Non_Modal_Dialogue_Interface const &) = delete;
    Non_Modal_Dialogue_Interface &operator=(Non_Modal_Dialogue_Interface &&) =
        delete;

    virtual ~Non_Modal_Dialogue_Interface() = 0;

  private:
    /** Handler for unhandled messages
     *
     * Needs to exist so it can be final.
     */
    Message_Return on_unhandled_non_modal_dialogue_message(
        UINT message, WPARAM wParam, LPARAM lParam
    ) noexcept override final;
};
