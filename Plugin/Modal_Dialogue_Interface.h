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

#include <intsafe.h>

// Forward declarations.
class Plugin;

/** This provides an abstraction for creating a docking dialogue. */
class Modal_Dialogue_Interface : public Dialogue_Interface
{
    typedef Dialogue_Interface Super;

  public:
    /** Create a modal dialogue. */
    Modal_Dialogue_Interface(Plugin const &plugin);

    Modal_Dialogue_Interface(Modal_Dialogue_Interface const &) = delete;
    Modal_Dialogue_Interface(Modal_Dialogue_Interface &&) = delete;
    Modal_Dialogue_Interface &operator=(Modal_Dialogue_Interface const &) =
        delete;
    Modal_Dialogue_Interface &operator=(Modal_Dialogue_Interface &&) = delete;

    virtual ~Modal_Dialogue_Interface() = 0;

    /** Special return values for get_result if the default handler is used. */
    enum Return_Values
    {
        Clicked_OK = -2,
        Clicked_Cancel = -3,
        Clicked_Close = -4
    };

    auto get_result() const noexcept
    {
        return result_;
    }

  protected:
    /** You need to call this from your constructor */
    INT_PTR create_modal_dialogue(int dialogID) noexcept final;

    /** Wrapper round ::EndDialog
     *
     * Note: You should avoid returning 0 or -1 via this as windows returns 0 or
     * -1 when the ::DialogBox call fails.
     *
     * Moreover, due to the vagaries of C++, if you want to return 0, you must
     * return 0LL or sim, as 0 can be a null pointer or an int...
     */
    BOOL EndDialog(INT_PTR retval) const noexcept;

    /** Wrapper round ::EndDialog that takes a pointer */
    BOOL EndDialog(void *retval) const noexcept
    {
#pragma warning(suppress : 26490)
        return EndDialog(reinterpret_cast<INT_PTR>(retval));
    }

    /** Centre the dialogue on the Notepad++ window */
    BOOL centre_dialogue() const noexcept;

  private:
    /** Handler for unhandled messages */
    Message_Return on_unhandled_dialogue_message(
        UINT message, WPARAM wParam, LPARAM lParam
    ) noexcept override final;

    /** Hide the other creation method */
    using Dialogue_Interface::create_dialogue;

    INT_PTR result_;
};
