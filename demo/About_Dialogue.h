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
#include "Plugin/Modal_Dialogue_Interface.h"

// These 2 don't actually provide anything useful, but IWYU can't currently
// work out that functions declared as override don't need to a separate header
#include "Plugin/Min_Win_Defs.h"

#include <intsafe.h>

// Forward references
class Plugin;

class About_Dialogue : public Modal_Dialogue_Interface
{
    typedef Modal_Dialogue_Interface Super;

  public:
    About_Dialogue(Plugin const *);

    ~About_Dialogue();

  private:
    Message_Return on_dialogue_message(
        UINT message, WPARAM wParam, LPARAM lParam
    ) noexcept override;
};
