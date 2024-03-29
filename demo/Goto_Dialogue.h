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

#include "Plugin/Docking_Dialogue_Interface.h"

class Plugin;

class Goto_Dialogue : public Docking_Dialogue_Interface
{
  public:
    Goto_Dialogue(int menu_entry, Plugin const *plugin);

    ~Goto_Dialogue();

  private:
    Message_Return on_dialogue_message(
        UINT message, WPARAM wParam, LPARAM lParam
    ) override;

    void on_display() noexcept override;
};
