#include "GlobalEventHook.hxx"

#include <Windows.h>

#include <QtWidgets/QtWidgets>

////////////////////////////////////////////////////////////////////////////////

HHOOK H_MOUSE_HOOK = NULL;

LRESULT CALLBACK HookedMouseProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    auto pMouseStruct = (MSLLHOOKSTRUCT*)lParam;

    const int x = pMouseStruct->pt.x;
    const int y = pMouseStruct->pt.y;

    switch( wParam )
    {
        case WM_LBUTTONDOWN:
            GetGlobalEventHook()->MouseButtonDown(x, y, 0);
        break;
        case WM_LBUTTONUP:
            GetGlobalEventHook()->MouseButtonUp(x, y, 0);
        break;
        case WM_MOUSEMOVE:
            GetGlobalEventHook()->MouseMove(x, y);
        break;
        case WM_MOUSEWHEEL:
        break;
        case WM_MOUSEHWHEEL:
        break;
        case WM_RBUTTONDOWN:
            GetGlobalEventHook()->MouseButtonDown(x, y, 0);
        break;
        case WM_RBUTTONUP:
            GetGlobalEventHook()->MouseButtonUp(x, y, 0);
        break;
    }

    return ::CallNextHookEx(H_MOUSE_HOOK, nCode, wParam, lParam);
}


void GlobalEventHook::HookMouse()
{
    // qDebug() << __CURRENT_FUNCTION_NAME__;

    H_MOUSE_HOOK = ::SetWindowsHookEx(WH_MOUSE_LL, HookedMouseProc, NULL, 0);
    if( H_MOUSE_HOOK == NULL) {
        qDebug() << "HookMouse failed";
    }
}


void GlobalEventHook::UnhookMouse()
{
    // qDebug() << __CURRENT_FUNCTION_NAME__;

    if( ::UnhookWindowsHookEx(H_MOUSE_HOOK) == 0 )
    {
        qDebug() << "UnhookMouse failed";
    }
}


////////////////////////////////////////////////////////////////////////////////

extern int main(int argc, char *argv[]);

// Convert a wchar_t to char string, equivalent to QString::toLocal8Bit()
// when passed CP_ACP.
static inline char *wideToMulti(int codePage, const wchar_t *aw)
{
    const int required = WideCharToMultiByte(codePage, 0, aw, -1, NULL, 0, NULL, NULL);
    char *result = new char[required];
    WideCharToMultiByte(codePage, 0, aw, -1, result, required, NULL, NULL);
    return result;
}


int APIENTRY WinMain(HINSTANCE, HINSTANCE, LPSTR /*cmdParamarg*/, int /*cmdShow*/)
{
    int argc;
    wchar_t **argvW = CommandLineToArgvW(GetCommandLineW(), &argc);
    if (!argvW)
        return -1;
    char **argv = new char *[argc + 1];
    for (int i = 0; i < argc; ++i)
        argv[i] = wideToMulti(CP_ACP, argvW[i]);
    argv[argc] = Q_NULLPTR;
    LocalFree(argvW);
    const int exitCode = main(argc, argv);
    for (int i = 0; i < argc && argv[i]; ++i)
        delete [] argv[i];
    delete [] argv;
    return exitCode;
}