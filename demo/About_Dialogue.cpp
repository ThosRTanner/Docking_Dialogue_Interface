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

#include "About_Dialogue.h"

#include "resource.h"

// Need windows.h for winuser.h to compile
#include <windows.h>    // IWYU pragma: keep
#include <winuser.h>    // For WM_INITDIALOG
// And this is IWYU not being able to understand 'override'
#include <minwindef.h>

#include <optional>

About_Dialogue::About_Dialogue(Plugin const *plugin) : Super(plugin)
{
    create_modal_dialogue(IDD_ABOUT_DIALOG);
}

About_Dialogue::~About_Dialogue() = default;

About_Dialogue::Message_Return About_Dialogue::on_dialogue_message(
    UINT message, WPARAM, LPARAM
) noexcept
{
    switch (message)
    {
        case WM_INITDIALOG:
        {
            // Possibly this should be default behaviour?
            centre_dialogue();
        }
        break;

        default:
            break;
    }

    return std::nullopt;
}
