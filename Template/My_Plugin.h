// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#pragma once

// Note: This header is purely here for dllmain not to produce odd warnings if
// looked at in visual studio. Do NOT copy it.

#include "Plugin/Plugin.h"

class My_Plugin : public Plugin
{
  public:
    /** Return the plugin name */
    static wchar_t const *get_plugin_name() noexcept;

    /** The object is created when notepad++ initialises your plugin */
    explicit My_Plugin(NppData const &data) : Plugin(data, L"")
    {
    }

    ~My_Plugin() override = default;

    My_Plugin(My_Plugin const &) = delete;
    My_Plugin(My_Plugin &&) = delete;
    My_Plugin &operator=(My_Plugin const &) = delete;
    My_Plugin &operator=(My_Plugin &&) = delete;
};
