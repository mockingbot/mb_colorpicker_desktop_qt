#include "GlobalEventHook.hxx"
#include "ColorPickerHost.hxx"


GlobalEventHook* GLOBAL_EVENT_HOOK = nullptr;

void
BootGlobalEventHook()
{
    if( GLOBAL_EVENT_HOOK == nullptr ) {
        static GlobalEventHook hook;
        GLOBAL_EVENT_HOOK = &hook;
    }
}

GlobalEventHook*
GetGlobalEventHook()
{
    return GLOBAL_EVENT_HOOK;
}

GlobalEventHook::GlobalEventHook()
{
    HookMouse();
}

GlobalEventHook::~GlobalEventHook()
{
    UnhookMouse();
}

void
GlobalEventHook::MouseMove(const int x, const int y)
{
    // qDebug() << "GlobalEventHook::MouseMove" << x << y;
    return ColorPickerHost::TraceMouseMove(x, y);
}

void
GlobalEventHook::MouseButtonUp(const int x, const int y, const int mask)
{
    // qDebug() << "GlobalEventHook::MouseButtonUp" << x << y << mask;
    return ColorPickerHost::TraceMouseButtonUp(x, y, mask);
}

void
GlobalEventHook::MouseButtonDown(const int x, const int y, const int mask)
{
    // qDebug() << "GlobalEventHook::MouseButtonDown" << x << y << mask;
    return ColorPickerHost::TraceMouseButtonDown(x, y, mask);
}
