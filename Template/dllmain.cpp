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

// Modify these 2 lines as appropriate for your plugin.
#include "My_Plugin.h"
typedef My_Plugin Npp_Plugin;

#include "PluginInterface.h"

#include <minwindef.h>

#include <memory>

std::unique_ptr<Npp_Plugin> plugin;

extern "C" __declspec(dllexport) wchar_t const *getName()
{
    return Npp_Plugin::get_plugin_name();
}

extern "C" __declspec(dllexport) void setInfo(NppData data)
{
    plugin = std::make_unique<Npp_Plugin>(data);
}

/** This must be defined and must always return TRUE
 *
 * It dates from when notepad++ had a unicode and a non unicode version
 */
extern "C" __declspec(dllexport) BOOL isUnicode()
{
    return TRUE;
}
