# Notepad++ plugin with docking and modal dialogues - base library and demo

This is probably massively overengineered, but there's a lot of boilerplate in here. Heavy use is made of the Template Method design pattern, hopefully meaning there's only a few bits you'll need to implement yourself. Those bits you do need to implement are actually virtual private methods. Most of them have default empty methods, so you don't have to implement a lot of empty functions.

You will require a compiler that supports C++ 20 or later to use the library.

In order to build the demo project, you will need the latest Windows SDK (10.0.22621.0 at the time of writing), as prior to that, the windows headers use non-standard extensions (or you can set Project => Properties => Configuration Properties => C/C++ => Language => Disable Language Extensions to no)

## Project Layout

This includes the notepad++ template (from https://github.com/npp-plugins/plugintemplate) as a submodule so the include path is set up for that. Projects based off of this therefore don't *need* to include it, but should you have require it, there is a notepad++ directory which contains forwarding includes for all the headers.

You will need to add the path to the project in your includes setting.

It's worth using the "Forced Include File" setting in the C/C++ advanced settings pane of your project configuration, or you'll have to mark _all_ the C++ source files in this module as 'not using precompiled header files'.

## Creating the basic plugin

Take a copy of `dllmain.cpp` from the `Template` directory and modify it to use your header and class name (at around line 18).

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

## Giving notepad++ the name of your plugin

Notepad++ queries your plugin for its name by calling `getName`. `dllmain.cpp` transfers control to this function in your class:

`static wchar_t const *get_plugin_name() noexcept`

It's arguably unnecessary boilerplate, but it keeps all your plugin related code in one class and cuts down the amount of stuff you need to copy and modify.

## Creating the menu definition for your plugin

Notepad++ queries your DLL for a list of function pointers. Using these to call into your class is moderately complicated, as notepad++ doesn't allow you to pass a parameter to call your supplied function pointer with. However, there is a way of doing this. It relies on some boiler plate code.

(If you're interested in how this works, see http://p-nand-q.com/programming/cplusplus/using_member_functions_with_c_function_pointers.html )

To aid in implementing this, 3 macros have been provided, `DEFINE_PLUGIN_MENU_CALLBACKS`, `PLUGIN_MENU_MAKE_CALLBACK` and `PLUGIN_MENU_MAKE_SEPARATOR`.

The first thing you need to do is to set up the context map.

```cpp
DEFINE_PLUGIN_MENU_CALLBACKS(My_Plugin);
```

You also need to define an enumeration in your class for the menu entries, and then you can implement the `on_get_menu_entries` method in your class like this:

```cpp
std::vector<FuncItem> &My_Plugin::on_get_menu_entries()
{
    static std::vector<FuncItem> res = {
        PLUGIN_MENU_MAKE_CALLBACK(My_Plugin, Entry_0, L"Entry 0", callback_0),
        PLUGIN_MENU_MAKE_CALLBACK(My_Plugin, Entry_1, L"Entry 1", callback_1),
        //...
        //If you want to add separators, do so like this...
        PLUGIN_MENU_MAKE_SEPARATOR(My_Plugin, Entry_2),
        //
        PLUGIN_MENU_MAKE_CALLBACK(My_Plugin, Entry_Last, L"Entry the last", callback_last)
    };
    return res;
};
```

You can also set the `init2Check` flag (which causes the menu item to get a check mark against it), and shortcut key as arguments to the macro. For most situations, you'll probably want to add your own macro because typing all those `My_Plugin, `s can get tedious.

_Notes_:
1. Notepad++ expects at least one menu entry, and it will crash if you provide none.
1. The returned vector must not be destroyed till notepad++ exits. hence the use of `static`. You could achieve this in other ways of course.

## The `Plugin` base class

This class provides a lot of boilerplate code and 3 virtual functions, the first of which you must implement. Default null implementations are supplied for the other two.

### Public API

Note that these are public mainly so that dialogue classes can get hold of useful functionality.

1. `Plugin(NppData const &, std::wstring_view name)`

    This takes the pointer to the notepad++ data which your constructor is passed, and the name of your plugin. This should be the same as your `get_name()` method returns.

1. `HINSTANCE module() const noexcept`

    Gets your module handle.

1. `std::wstring get_name() const`

    This returns the name with which you constructed the plugin.

1. `std::filesystem::path get_module_path() const`

    This returns the path to the module .dll.

1. `HWND get_notepad_window() const noexcept`

    Get hold of the notepad++ window handle. You probably won't need to use this.

1. `LRESULT send_to_notepad(UINT message, WPARAM = 0, LPARAM = 0) const noexcept`

    Send a message to notepad++

   `LRESULT send_to_notepad(UINT message, WPARAM wParam, void const *buff) const noexcept`

    Same, but avoids messy reinterpret_casts round the windows API

1. `std::filesystem::path get_config_dir() const`

    Get the path to the notepad++ config directory.

1. `std::filesystem::path get_plugin_config_dir() const`

    Get a path to a directory with the name of your plugin in the notepad++ config_directory. The directory will be created if necessary.

1. `std::filesystem::path get_document_path() const`

    Get the current document path.

1. `std::filesystem::path get_document_path(uptr_t buffer_id) const`

    Get the path to the document in the specified buffer.

1. `HWND get_scintilla_window() const noexcept`

    Get the current scintilla window. You probably won't need to use this.

1. `LRESULT send_to_editor(UINT message, WPARAM = 0, LPARAM = 0) const noexcept`

    Send a message to the current editor window

   `LRESULT send_to_editor(UINT message, WPARAM wParam, void const *buff) const noexcept`

    Same, but avoids messy reinterpret_casts round the windows API

1. `std::string get_document_text() const`

    Get the contents of the current document

1. `std::string get_line_text(int line) const`

    Get the contents of the specified line in the current document

### (Private) Virtual methods

1. `virtual std::vector<FuncItem> &on_get_menu_entries() = 0`

    See above

2. `virtual void on_process_notification(SCNotification const *)`

    Implement this to handle notifications from scintilla if required.

3. `virtual LRESULT on_process_message(UINT message, WPARAM, LPARAM)`

    Implement this to handle messages from notepad++ if required.
   _Note_ I cannot find any examples of how to use these messages in notepad++.

### (Protected) Utility methods

1. `int message_box(std::wstring const &message, UINT type) const noexcept`

    This is a wrapper round `::MessageBox`, and throws up a message box using your plugin name as the title.

2. `template <typename Callbacks, typename Context, typename Class, typename Callback>
    FuncItem make_callback(
        int entry, wchar_t const *message, Callbacks &contexts, Context context,
        Class self, Callback callback, bool check = false, ShortcutKey *key = nullptr
    )`

    This is a utility function to aid setting up notepad++ menu definition. You are strongly advised to use the `PLUGIN_MENU_MAKE_CALLBACK` macro for this.

3. `template <typename Callbacks, typename Context, typename Class>
    FuncItem make_separator(int entry, Callbacks &contexts, Context context, Class self)`

    Simplified wrapper around `make_callback` for menu separators. You are strongly advised to use the `PLUGIN_MENU_MAKE_SEPARATOR` macro for this.

## Processing scintilla notifications

To be written

## Processing notepad++ messages

To be written

## Dialogue APIs

This repo contains base classes for docking dialogues (which can be docked to any side of the notepad++ window or be free floating), non-modal (or modeless) dialogues, and modal dialogues (the ones where you have to press OK or cancel before you can resume editing), which all share a certain amount of common code.

In order to implement your class, you subclass the appropriate `_xxx_Dialogue_Interface_` class and implement the `on_dialogue_message` virtual method in order to process messages.

*Important Note*
Your `on_dialogue_message` method will get called with the `WM_INITDIALOG` message. However, be aware that your constructor may not have had all the members initialised at the point, so be very careful what you do in response.

### The Dialogue_Interface base class

This is a base class to all of the dialogue classes, and so all the protected methods can be used.

#### Public API

1. `Dialogue_Interface(Plugin const *)`

    Stashes a pointer to the plugin class and gets hold of the current module name.

##### (Private) Virtual Methods you should implement

1. `virtual Message_Return on_dialogue_message(UINT message, WPARAM wParam, LPARAM lParam)`

    `Message_Return` is a typdef for `std::optional<INT_PTR>` but it's astonishingly easy to get that wrong if you look at stackoverflow, hence the typedef.

    Return `std::nullopt` (to return `FALSE` to windows dialog processing), or a value to be set with `::SetWindowLongPtr` (in which case `TRUE` will be returned to windows).
    Note that some messages require you to return `FALSE` (`std::nullopt`) even if you do handle them.

    `message`, `wParam` and `lParam` are the values passed to a `DLGPROC` function by windows,

### (Protected) Utility Methods

Most of these are wrappers round windows functions (or macros) of the same or similar name.

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

1. `HWND GetDlgItem(int, HWND = nullptr) const noexcept`

    Utility to get a dialogue item. Normally the item would be from your own window, in which case there's no need to pass a window handle, but if you need to get hold of something from another window, you can pass the window handle in.

1. `std::wstring get_window_text(int, HWND window = nullptr) const`

    A wrapper round `GetWindowText` which gets the text of the specified item in the window. Normally the item would be from your own window, in which case there's no need to pass a window handle, but if you need to get hold of something from another window, you can pass the window handle in.

1. `void SetFocus(int) const`

    Sets the focus to the given item.

1. `int message_box(std::wstring const &message, UINT type) const noexcept;`

    This is a wrapper round `::MessageBox`, and throws up a message box using your dialogue name as the title.

1. `void add_item_callback(int item, Item_Callback_Function callback_func)`
    This allows you to "subclass" (the phrase windows uses) a window element, to intercept events on it.

    `Item_Callback_Function` is defined as:

    `typedef std::optional<LRESULT> Item_Callback_Return`;
    `typedef std::function<Item_Callback_Return(HWND, UINT, WPARAM, LPARAM)> Item_Callback_Function`

    This behaves in much the same way as `on_dialogue_message`, in that you return `std::nullopt` if you've not handled the callback. Note that because it is a std::function, if you're using a class instance method, you will need to wrap it with std::bind and 4 placeholders...

### Creating a non modal (aka modeless) dialogue - the Non_Modal_Dialogue_Interface class

When you want to create a non-modal dialogue, you should create a subclass of the `Non_Modal_Dialogue_Interface` class. This contains the necessary calls to notepad++ to ensure it knows to send your dialogue keystrokes and such.

In your constructor, you must

1. Call the base class constructor with the ID of the dialogue (i.e. the ID from `resource.h`), and a pointer to your main plugin instance. Your dialogue will be created as part of this, and the base class will retain the window handle.

1. Do any window setup required (see the important note above about `WM_INITDIALOG` and do the setup in the constructor, not in the `on_dialog_message` callback)

1. Your class will start receiving messages to process via `on_dialogue_message` (if you have implemented it).

#### Public Methods

1. `Non_Modal_Dialogue_Interface(int dialogue_id, Plugin const *plugin, HWND parent = nullptr)`

    Constructor for the class. The dialogue ID is the appropriate identifier from `resource.h`. You can optionally pass the handle to a parent window.

### Creating docking dialogue - the Docking_Dialogue_Interface class

When you want to create a docking dialogue, you should create a subclass of the `Docking_Dialogue_Interface` class. A docking dialogue is pretty much a modeless dialogue, except it is able to be docked in the main notepad++ window (in the same way as e.g. the results of the find dialogue). Note that the parent window is always the notepad++ window so there's no parameter for that.

In your constructor, you must

1. Call the base class constructor with the ID of the dialogue (i.e. the ID from resource.h), and a pointer to your main plugin instance. Your dialogue will be created as part of this, and the `Docking_Dialogue_Interface` class will retain the window handle.

1. Do any window setup required (see the important note above about `WM_INITDIALOG` and do the setup in the constructor, not in the `on_dialog_message` callback)

1. Call `register_dialogue` in order to tell notepad++ that your new dialogue is ready to be used.

1. Your class will start receiving messages to process via `on_dialogue_message` (if you have implemented it).

*Important Note*:
1. The actual dialogue you create needs to have a caption bar with a title if you want Notepad++ to save the state between sessions.

#### Public Methods

1. `Docking_Dialogue_Interface(int dialogue_id, Plugin const *plugin)`

    Constructor for the class. The dialogue ID is the appropriate identifier from `resource.h`.

1. `void display() noexcept`

    Call this to display the dialogue. If you have special actions to be take on display, implement `void on_display() noexcept override`.

1. `void hide() noexcept`

    Call this to hide the dialogue. If you have special actions to be take on hiding, implement `void on_hide() noexcept override`.

1. `bool is_hidden() const noexcept`

    Returns `true` if the dialogue is currently hidden.

#### (Private) Virtual Methods

1. `virtual void on_display() noexcept`

    This is called whenever the dialogue is about to be displayed.

1. `virtual void on_hide() noexcept`

    This is called whenever the dialogue is about to be hidden.

#### (Protected) Utility Methods

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

### Modal dialogues - the Modal_Dialogue_Interface class

A modal dialogue doesn't return until you have clicked the OK or cancel button (or the close button at the top right). This means that when you create the dialogue, any subsequent code will not be executed, so your constructor needs to be implemented a little differently to that of a docking dialogue.

You should do all the work you *can* do before calling the `create_dialogue_window` method. Any further work needs to be done in the `on_dialogue_message` callback function.

The class provides default handlers for 'OK', 'Cancel' and 'Close' buttons. These return `Clicked_OK`, `Clicked_Cancel` and `Clicked_Close` to the API (which are NOT 0 or -1 - see below). Of course, you do not have to use the default handler, and may handle the events yourself in the `on_dialogue_message` function.

#### Public Methods

1. `Modal_Dialogue_Interface(Plugin const *)`

    The constructor. This just takes a pointer to the plugin class. It doesn't take a dialogue id, you need to pass that into `create_modal_dialogue`.

1. `INT_PTR get_result() const noexcept;`

    This returns the result passed to the `EndDialog` method. *Note* Avoid returning 0 or -1 via `EndDialog` as they are indistinguishable from the values returned when the windows `DialogBox` function gets an error.

#### (Protected) Utility Methods

1. `void create_modal_dialogue(int dialogID) noexcept;`

    Create the dialogue - this function will not return until the user has closed the dialogue, at which point the result may be checked by calling `get_result()`

1.  `BOOL EndDialog(INT_PTR retval) const noexcept;`

    This is a wrapper round `::EndDialog`. Try not to call it with 0 or -1 - see above. Be warned that calling this with 0 will give you a compiler error, because C++ can't tell the difference between 0 and `nullptr`. Use 0LL if you must return 0 (but see above on that approach).

 1. `BOOL EndDialog(void *retval) const noexcept;`

    Wrapper round `::EndDialog` that avoids reinterpret_cast. However, it is probably better to return `Clicked_OK` and provide another method in your class to return complex data.

1. `BOOL centre_dialogue() const noexcept;`

    Centre the dialogue on the Notepad++ window
