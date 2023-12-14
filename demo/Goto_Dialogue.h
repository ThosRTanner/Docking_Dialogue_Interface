#pragma once

#include "Plugin/Docking_Dialogue_Interface.h"

class Plugin;

class Goto_Dialogue : public Docking_Dialogue_Interface
{
  public:
    Goto_Dialogue(int menu_entry, Plugin const *plugin);

  private:
    std::optional<LONG_PTR> on_dialogue_message(
        UINT message, UINT_PTR wParam, LONG_PTR lParam
    ) override;

    void on_display() noexcept override
    {
    }
};
