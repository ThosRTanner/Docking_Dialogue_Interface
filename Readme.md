# Notepad++ plugin with docking and modal dialogues - base library and demo.

This is probably massively overengineered, but there's a lot of boilerplate in here. Heavy use is made of the Template Method design pattern, hopefully meaning there's only a few bits you'll need to implement yourself. Those bits you do need to implement are actually virtual private methods. Most of them have default empty methods, so you don't have to implement a lot of empty functions.

You will require a compiler that supports C++ 17 or later to use the library.

In order to build the demo project, you will need the latest Windows SDK (10.0.22621.0 at the time of writing), as prior to that, the windows headers use non-standard extensions (or you can set Project => Properties => Configuration Properties => C/C++ => Language => Disable Language Extensions to no) 

**Important note**
This is a work in progress. Clone at your own risk.

## Project Layout

This comes WITHOUT a copy of the notepad++ here template (from https://github.com/npp-plugins/plugintemplate) because ultimately that would just be confusing. However, it assumes that the headers from there are available in your `-I` path. In this demo, it was cloned into the same parent directory as this project, so the project settings have `../plugintemplate/src` in the C++ include setting.

## Creating the basic plugin

Take a copy of `dllmain.cpp` from the `Plugin` directory and modify it to use your header and class name (at around line 18).

Create your plugin by making a class which inherits from the `Plugin` class.

You will need to implement one static method, `get_plugin_name` and one pure virtual function, `on_get_menu_entries` (a pure virtual function required by the `Plugin` base class).

You may wish to implement a DllMain function, but that is not necessary for a notepad++ plugin, and is therefore outside the scope of this document.

The overall life story of your plugin is:

1. Notepad++ calls `getName`. This causes the main code to call your `get_plugin_name` static function. This is used to display the name of your plugin in the plugins menu. It is unclear whether or not notepad++ copies this data but it's probably safest to return a static string.
1. Notepad++ calls the `setInfo` function, and the main code will construct an instance of your class, passing a reference to the supplied `NppData` object.
1. Notepad++ calls the `getFuncsArray` function. The main code will then call the `on_get_menu_entries` method of your class (see below)
1. Messages and notifications are sent to your plugin via the `on_message` and `on_notification` methods.
   1. You should probably ignore the `NPPM_SHUTDOWN` message. Among other things it is not safe to delete the class object until Notepad++ exits because it still accesses the memory allocated for the module name in `TbData` after it sends the message, and it makes more sense to clean up resources in your destructor.
1. Notepad++ exits.
   1. The runtime library deletes the instance of your class, and so your destructor is called.

## Giving notepad++ the name of your plugin.

Notepad++ queries your plugin for its name by calling `getName`. dllmain.cpp transfers control to this function in your class:

`static wchar_t const *get_plugin_name() noexcept`

It's arguably unnecessary boilerplate, but it keeps all your plugin related code in one class and cuts down the amount of stuff you need to copy and modify.

## Creating the menu definition for your plugin.

Notepad++ queries your DLL for a list of function pointers. Using these to call into your class is moderately complicated, as notepad++ doesn't allow you to pass a parameter to call your supplied function pointer with. However, there is a way of doing this. It relies on some boiler plate code.

(If you're interested in how this works, see http://p-nand-q.com/programming/cplusplus/using_member_functions_with_c_function_pointers.html )

You'll need to provide a table like this:

```
typedef Callback_Context_Base<Demo_Plugin> Callbacks;

#define CALLBACK_ENTRY(N) { (N), std::make_shared<Callback_Context<My_Plugin, (N)>>() }

template <> Callbacks::Contexts Callbacks::contexts = {
    CALLBACK_ENTRY(0),
    CALLBACK_ENTRY(1),
    ...
    CALLBACK_ENTRY(last)
};
```

and provide this function in your class (you may want to use macros here, considering the level of boilerplate):

```
std::vector<FuncItem> &My_Plugin::on_get_menu_entries()
{
    static std::vector<FuncItem> res = {
        make_callback(
            0, L"Entry 0", Callbacks::contexts, this, &My_Plugin::callback_0
        ),
        make_callback(
            1, L"Entry 1", Callbacks::contexts, this, &My_Plugin::callback_1
        ),
        //...
        make_callback(
            last, L"Entry the last", Callbacks::contexts, this, &My_Plugin::callback_last
        )
    };
    return res;
};
```

_Note_: Notepad++ expects at least one menu entry, and it will crash if you provide none.

## The `Plugin` base class
This class provides a lot of boilerplate code and 3 virtual functions, the first of which you must implement. Default null implementations are supplied for the other two.

### (Private) Virtual methods

1. `virtual std::vector<FuncItem> &on_get_menu_entries() = 0`

   See above

2. `virtual void on_process_notification(SCNotification const *)`

   Implement this to handle notifications from scintilla if required.

3. `virtual LRESULT on_process_message(UINT message, WPARAM, LPARAM)`

   Implement this to handle messages from notepad++ if required.
   _Note_ I cannot find any examples of how to use these messages in notepad++.

### (Protected) Utility methods.

1. `HWND get_notepad_window() const noexcept`

   Get hold of the notepad++ window handle. You probably won't need to use this.

2. `LRESULT send_to_notepad(UINT message, WPARAM = 0, LPARAM = 0) const noexcept`

   Send a message to notepad++

   `LRESULT send_to_notepad(UINT message, WPARAM wParam, void const *buff) const noexcept`

   Same, but avoids messy reinterpret_casts round the windows API

3. `HWND get_scintilla_window() const noexcept`

   Get the current scintilla window. You probably won't need to use this.

4. `LRESULT send_to_editor(UINT message, WPARAM = 0, LPARAM = 0) const noexcept`

   Send a message to the current editor window

   `LRESULT send_to_editor(UINT message, WPARAM wParam, void const *buff) const noexcept`

   Same, but avoids messy reinterpret_casts round the windows API

5. `HINSTANCE module() const noexcept`

   Gets your module handle.

6. `int message_box(std::wstring const &message, UINT type) const noexcept`

    This is a wrapper round `::MessageBox`, and throws up a message box using your plugin name as the title.

## Processing scintilla notifications
To be written

## Processing notepad++ messages
To be written

## Dialogue API

This repo contains base classes for both docking dialogues (which can be docked to any side of the notepad++ window or be free floating) and model dialogues (the ones where you have to press OK or cancel before you can resume editing), which share a certain amount of common code.

In order to implement your class, you subclass the appropriate _xxx_Dialogue_Interface_ class and implement the `on_dialogue_message` virtual method in order to process messages.

*Important Note*
Your `on_dialogue_message` method will get called with the `WM_INITDIALOG` message. However, be aware that your constructor may not have had all the members initialised at the point, so be very careful what you do in response.

### The Dialogue_Interface class
document the constructor here

##### (Private) Virtual methods you should implement

1. `virtual std::optional<LONG_PTR> on_dialogue_message(UINT message, WPARAM wParam, LPARAM lParam)`

     Return `std::nullopt` (to return `FALSE` to windows dialog processing), or a value to be set with `::SetWindowLongPtr` (in which case `TRUE` will be returned to windows).
     Note that some messages require you to return `FALSE` (`std::nullopt`) even if you do handle them.

     `message`, `wParam` and `lParam` are the values passed to a `DLGPROC` function by windows,

### Utility (protected) methods you can call

1. `Plugin const *plugin() const noexcept`

    Get hold of plugin object for useful boilerplate

1. `HWND window() const noexcept`

    Get hold of the current dialogue window handle

1. `void InvalidateRect(RECT const *rect = nullptr) const noexcept`

    Requests a redraw by invalidating the specified rectangle or the whole dialogue area.

1. `RECT getClientRect() const noexcept`

    Utility to get the current client rectangle

1. `RECT getWindowRect() const noexcept`

    Utility to get the current window rectangle
    
1. `HWND GetDlgItem(int) const noexcept`

    Utility to get a dialogue item

1. `int message_box(std::wstring const &message, UINT type) const noexcept;`

    This is a wrapper round `::MessageBox`, and throws up a message box using your dialogue name as the title.

### Creating a docking dialogue
When you want to create a docking dialogue, you should create a subclass of the `Docking_Dialogue_Interface` class.

In your constructor, you must

1. Call the base class constructor with the ID of the dialogue (i.e. the ID from resource.h), and a pointer to your main plugin instance. Your dialogue will be created as part of this, and the `Docking_Dialogue_Interface` class will retain the window handle.

2. Do any window setup required (see the important note above about `WM_INITDIALOG` and do the setup in the constructor, not in the `on_dialog_message` callback)

3. Call `register_dialogue` in order to tell notepad++ that your new dialogue is ready to be used.

4. Your class will start receiving messages to process via `on_dialogue_message` (if you have implemented it).

*Important Notes*:
1. The actual dialogue you create needs to have a caption bar with a title if you want Notepad++ to save the state between sessions.


#### Docking_Dialog_Interface class

This handles the API with notepad++ and provides some default behaviour and some utility methods.

##### Public methods

1. `Docking_Dialogue_Interface(int dialogue_id, Plugin const *plugin)`

   Constructor for the class.

1. `void display() noexcept`

    Call this to display the dialogue. If you have special actions to be take on display, implement `void on_display() noexcept override`

1. `void hide() noexcept`

    Call this to hide the dialogue. If you have special actions to be take on hiding, implement `void on_hide() noexcept override`

1. `bool is_hidden() const noexcept`

    Returns `true` if the dialogue is currently hidden.

##### (Private) Virtual methods

1. `virtual void on_display() noexcept`

    This is called whenever the dialogue is about to be displayed.

1. `virtual void on_hide() noexcept`

    This is called whenever the dialogue is about to be hidden.

##### (Protected) Utility Methods

1. `void register_dialogue(int menu_index, Position position, HICON icon = nullptr, wchar_t const *extra = nullptr) noexcept`

    `menu_index` is the menu entry number which causes this dialogue to be displayed. I strongly recommend you use an `enum` here so you can tie it up with the entries in the `FuncItem` table. Note that Notepad++ appears to use this value when saving window states and you can get some quite unexpected results if the numbers don't match (or change)!

    `position` defines where the dialogue will be placed the first time it is displayed by Notepad++. On subsequent runs, this value will be ignored. It may be one of

        Dock_Left,
        Dock_Right,
        Dock_Top,
        Dock_Bottom,
        Floating

    `icon`, if supplied, must be an icon which you have loaded. It will be displayed on the tab bar in the docking dialogue window.

    `extra` is extra text to display in the title bar if required.

## Modal dialogues

A modal dialogue doesn't return until you have clicked the OK or cancel button (or the close button at the top right). This means that when you create the dialogue, any subsequent code will not be executed so the your constructor needs to be implemented a little differently to that of a docking dialogue.

You should do all the work you *can* do before calling the `create_dialogue_window` method. Any further work needs to be done in the `on_dialogue_message` callback function.

### Modal_Dialogue_Interface class


#### Public methods

1. `INT_PTR get_result() const noexcept;`

    This returns the result passed to the `EndDialog` method. *Note* Avoid returning 0 or -1 via `EndDialog` as they are indistinguishable from the values returned when the windows `DialogBox` function gets an error.

#### Protected (utility) methods

1. `void create_modal_dialogue(int dialogID) noexcept;`

    Create the dialogue - this function will not return until the user has closed the dialogue, at which point the result may be checked by calling `get_result()`

1.  `BOOL EndDialog(INT_PTR retval) const noexcept;`

    This is a wrapper round `::EndDialog`. Try not to call it with 0 or -1 - see above. Also, calling ith 0 will give you a compiler error, because C++ can't tell the difference between 0 and `nullptr`. Use 0LL if you must return 0.
 
 1. `BOOL EndDialog(void *retval) const noexcept;`
 
    Wrapper round `::EndDialog` that avoids reinterpret_cast.

1. `BOOL centre_dialogue() const noexcept;`

    Centre the dialogue on the Notepad++ window
