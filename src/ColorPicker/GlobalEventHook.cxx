#include "GlobalEventHook.hxx"
#include "ColorPickerCanvas.hxx"


GlobalEventHook* GLOBAL_EVENT_HOOK = nullptr;

void
BootGlobalEventHook()
{
    if( GLOBAL_EVENT_HOOK == nullptr ) {
        GLOBAL_EVENT_HOOK = new GlobalEventHook();
    }
}

GlobalEventHook*
GetGlobalEventHook()
{
    return GLOBAL_EVENT_HOOK;
}

GlobalEventHook::GlobalEventHook()
{
    OS::Hack::HookMouse();
}

GlobalEventHook::~GlobalEventHook()
{
    OS::Hack::UnhookMouse();
}

void
GlobalEventHook::MouseMove(const int x, const int y)
{
    // qDebug() << "GlobalEventHook::MouseMove" << x << y;
    return ColorPickerHost::TraceMouseMove(x, y);
}
