# Change log

## 2025-08-04

Update to latest plugintemplate version.

## 2025-03-08

1. Changes to the `Plugin` class:
   1. Add an override to `make_callback` to allow passing a `std::wstring` as the menu text.
   1. Make `make_callback` take `const` pointers to the shortcut key definitions.

## 2025-02-23

1. Changes to the `Plugin` class:
   1. Addded `get_name` which returns the name with which you registered.
   1. Added `get_module_path` which returns the path to the module .dll.

## 2025-02-01

### Important notes - Changes to API

1. This now requires you to build with a C++ 20 compatible compiler.
1. The API has changed everywhere so that dialogue constructors need to be passed a const ref to the `Plugin` class, rather than a const pointer.

### Other

1. Added the "notepad++" directory for including notepad++ headers more easily. You only need to include the path to this repo in your include files, and not an additional path to the notepad++ headers.
1. Now builds (mostly) clean with -W4
1. It is no longer necessary to supply a stub `isUnicode` function - it is supplied in the `Plugin` class.
1. Changes to the `Plugin` class:
   1. `get_document_path` can now be supplied with a scintilla buffer ID. This will get the document path associated with the specified buffer.
   1. A new function `get_plugin_config_dir` has been added which will, if necessary, create a directory inside the Notepad++ config directory with the name of your plugin.

## 2024-02-29 ish

Initial release...
