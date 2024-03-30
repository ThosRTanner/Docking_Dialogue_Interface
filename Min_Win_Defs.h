#pragma once
// Forward declarations from windows windef.h / minwindef.h (mostly) which
// fail to build if you add them as per suggestions...

#include <basetsd.h>

typedef struct tagRECT RECT;
typedef struct HWND__ *HWND;
typedef unsigned int UINT;
typedef UINT_PTR WPARAM;
typedef LONG_PTR LPARAM;
typedef LONG_PTR LRESULT;
typedef void *HANDLE;
typedef struct HICON__ *HICON;
typedef int BOOL;
