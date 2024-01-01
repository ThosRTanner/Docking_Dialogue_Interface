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
typedef int BOOL;

// Forward declarations.
class Plugin;

/** This provides an abstraction for creating a docking dialogue. */
class Modal_Dialogue_Interface : public Dialogue_Interface
{
  public:
    /** Create a modal dialogue. */
    Modal_Dialogue_Interface(Plugin const *plugin);

    Modal_Dialogue_Interface(Modal_Dialogue_Interface const &) = delete;
    Modal_Dialogue_Interface(Modal_Dialogue_Interface &&) = delete;
    Modal_Dialogue_Interface &operator=(Modal_Dialogue_Interface const &) =
        delete;
    Modal_Dialogue_Interface &operator=(Modal_Dialogue_Interface &&) = delete;

    virtual ~Modal_Dialogue_Interface() = 0;

    auto get_result() const noexcept
    {
        return result_;
    }

  protected:
    /** You need to call this from your constructor */
    void create_dialogue_window(int dialogID) noexcept;

    /** Wrapper round ::EndDialog */
    BOOL EndDialog(INT_PTR retval) const noexcept;

    /** Wrapper round ::EndDialog that takes a pointer
     *
     * Note: Due to the vagaries of the language, C++ compilers can't
     * differentiate between 0 and nullptr. Pass 0LL if you really need to
     * pass 0, or pass nullptr.
     */
    BOOL EndDialog(void *retval) const noexcept
    {
#pragma warning(suppress : 26490)
        return EndDialog(reinterpret_cast<INT_PTR>(retval));
    }

    /** Centre the dialogue on the Notepad++ window */
    BOOL centre_dialogue() const noexcept;

  private:
    /** Handler for unhandled messages */
    std::optional<LONG_PTR> on_unhandled_dialogue_message(
        UINT message, WPARAM wParam, LPARAM lParam
    ) noexcept override final;

    INT_PTR result_;
};
