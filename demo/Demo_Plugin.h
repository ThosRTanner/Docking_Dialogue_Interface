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

#include "Plugin/Plugin.h"

#include "PluginInterface.h"

#include <memory>

class Goto_Dialogue;

class Demo_Plugin : public Plugin
{
  public:
    /** Return the plugin name */
    static wchar_t const *get_plugin_name() noexcept;

    /** The object is created when notepad++ initialises your plugin */
    Demo_Plugin(NppData const &);

    ~Demo_Plugin();

    Demo_Plugin(Demo_Plugin const &) = delete;
    Demo_Plugin(Demo_Plugin &&) = delete;
    Demo_Plugin &operator=(Demo_Plugin const &) = delete;
    Demo_Plugin &operator=(Demo_Plugin &&) = delete;

    enum Menu_Entries
    {
        Menu_Entry_Hello_File,
        Menu_Entry_Hello_Message,
        Menu_Entry_Goto_Dialogue
    };

  private:
    std::vector<FuncItem> &on_get_menu_entries() override;

#if 0
    void on_notification(SCNotification const *) noexcept override;

    LRESULT on_message(UINT, WPARAM, LPARAM) noexcept override;
#endif

    //
    // Your plugin command functions
    //

    void hello_file() const noexcept;
    void hello_message() const;
    void goto_dialogue();

    std::unique_ptr<Goto_Dialogue> goto_dialogue_;
};
