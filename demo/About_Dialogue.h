#pragma once
#include "Plugin/Modal_Dialogue_Interface.h"

class Plugin;

class About_Dialogue : public Modal_Dialogue_Interface
{
  public:
    About_Dialogue(Plugin const *);

    ~About_Dialogue();

  private:
    std::optional<LONG_PTR> on_dialogue_message(
        UINT message, WPARAM wParam, LPARAM lParam
    ) override;
};
