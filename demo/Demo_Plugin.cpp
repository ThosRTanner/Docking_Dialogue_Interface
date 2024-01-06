// this file is part of notepad++
// Copyright (C)2022 Don HO <don.h@free.fr>
//
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

#include "Demo_Plugin.h"

#include "About_Dialogue.h"
#include "Goto_Dialogue.h"

#include "Plugin/Callback_Context.h"

#include "menuCmdID.h"

#include <memory>

typedef Callback_Context_Base<Demo_Plugin> Callbacks;

#define CALLBACK_ENTRY(N)                                                     \
    {                                                                         \
        Demo_Plugin::N,                                                       \
            std::make_shared<Callback_Context<Demo_Plugin, Demo_Plugin::N>>() \
    }

template <>
Callbacks::Contexts Callbacks::contexts = {
#pragma warning(push)
#pragma warning(disable : 26426)
    CALLBACK_ENTRY(Menu_Entry_Hello_File),
    CALLBACK_ENTRY(Menu_Entry_Hello_Message),
    CALLBACK_ENTRY(Menu_Entry_Separator_1),
    CALLBACK_ENTRY(Menu_Entry_Goto_Dialogue),
    CALLBACK_ENTRY(Menu_Entry_Separator_2),
    CALLBACK_ENTRY(Menu_Entry_About_Dialogue)
#pragma warning(pop)
};

wchar_t const *Demo_Plugin::get_plugin_name() noexcept
{
    return L"Demo Notepad++ plugin";
}

Demo_Plugin::Demo_Plugin(NppData const &data) : Plugin(data, get_plugin_name())
{
}

Demo_Plugin::~Demo_Plugin()
{
    // Remember to deallocate any assigned memory here.
}

#define MAKE_CALLBACK(entry, text, method) \
    make_callback(entry, text, Callbacks::contexts, this, &Demo_Plugin::method)

#define MAKE_SEPARATOR(entry) \
    make_separator(entry, Callbacks::contexts, this)

std::vector<FuncItem> &Demo_Plugin::on_get_menu_entries()
{
    static std::vector<FuncItem> res = {
        MAKE_CALLBACK(
            Menu_Entry_Hello_File, L"Hello Notepad++ File", hello_file
        ),
        MAKE_CALLBACK(
            Menu_Entry_Hello_Message, L"Hello Notepad++ Message", hello_message
        ),
        MAKE_SEPARATOR(Menu_Entry_Separator_1),
        MAKE_CALLBACK(
            Menu_Entry_Goto_Dialogue, L"Goto docked dialogue", goto_dialogue
        ),
        MAKE_SEPARATOR(Menu_Entry_Separator_2),
        MAKE_CALLBACK(
            Menu_Entry_About_Dialogue, L"About modal dialogue", about_dialogue
        )
    };
    return res;
}

#if 0
void Demo_Plugin::on_notification(
    SCNotification const *notification
) noexcept
{
    switch (notification->nmhdr.code)
    {
        default:
            return;
    }
}
#endif

#if 0
LRESULT Demo_Plugin::on_message(
    UINT /*message*/, WPARAM /*wParam*/, LPARAM /*lParam*/
) noexcept
{
    return TRUE;
}
#endif

void Demo_Plugin::hello_file() const noexcept
{
    // Open a new document
    send_to_notepad(NPPM_MENUCOMMAND, 0, IDM_FILE_NEW);

    // Say hello now :
    // Scintilla control has no Unicode mode, so we use (char *) here
    send_to_editor(SCI_SETTEXT, 0, "Hello, Notepad++!");
}

void Demo_Plugin::hello_message() const
{
    message_box(L"Hello, Notepad++!", MB_OK);
}

void Demo_Plugin::goto_dialogue()
{
    if (! goto_dialogue_)
    {
        goto_dialogue_ =
            std::make_unique<Goto_Dialogue>(Menu_Entry_Goto_Dialogue, this);
    }
    goto_dialogue_->display();
}

void Demo_Plugin::about_dialogue() const
{
    About_Dialogue dialogue(this);
    auto const res = dialogue.get_result();
    if (res == 0)
    {
        message_box(L"Cancelled", MB_ICONEXCLAMATION | MB_ABORTRETRYIGNORE);
    }
    else
    {
        message_box(L"OK", MB_OK);
    }
}
