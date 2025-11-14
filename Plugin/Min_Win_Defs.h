#pragma once
// Forward declarations from windows windef.h / minwindef.h (mostly) which
// fail to build if you add them as per suggestions...

#ifdef _WINDOWS_
#include <basetsd.h>      // IWYU pragma: export
#include <minwindef.h>    // IWYU pragma: export
#include <windef.h>       // IWYU pragma: export
#include <winnt.h>        // IWYU pragma: export
#else
// NOLINTBEGIN
#include <intsafe.h>    // IWYU pragma: export

typedef struct tagRECT RECT;
typedef struct HWND__ *HWND;
typedef UINT_PTR WPARAM;
typedef LONG_PTR LPARAM;
typedef LONG_PTR LRESULT;
typedef void *HANDLE;
typedef struct HICON__ *HICON;
typedef int BOOL;
// NOLINTEND
#endif
