#include "GlobalEventHook.hxx"

#include <AppKit/AppKit.h>

#include <QtWidgets/QtWidgets>

////////////////////////////////////////////////////////////////////////////////

id H_MOUSE_HOOK = 0;

CGEventRef MouseEventCallback(CGEventTapProxy proxy, CGEventType type, \
                                            CGEventRef event, void * refcon)
{
    auto mouseLocation = CGEventGetLocation(event);

    auto x = (int)mouseLocation.x;
    auto y = (int)mouseLocation.y;

    GetGlobalEventHook()->MouseMove(x, y);

    return event;
}

CFMachPortRef MOUSE_EVENT_TAP;
CFRunLoopSourceRef EVENT_TAP_SRC_REF;

void OS::Hack::HookMouse()
{
    // We only want one kind of event at the moment: The mouse has moved
    auto emask = CGEventMaskBit(kCGEventMouseMoved);

    /*
     https://developer.apple.com/reference/coregraphics/1454426-cgeventtapcreate
     */

    MOUSE_EVENT_TAP = CGEventTapCreate (
        kCGSessionEventTap,
        kCGTailAppendEventTap,
        kCGEventTapOptionListenOnly,
        emask,
        &MouseEventCallback,
        NULL
    );

    EVENT_TAP_SRC_REF = CFMachPortCreateRunLoopSource(
        kCFAllocatorDefault,
        MOUSE_EVENT_TAP,
        0
    );

    CFRunLoopAddSource(
        CFRunLoopGetCurrent(),
        EVENT_TAP_SRC_REF,
        kCFRunLoopDefaultMode
    );
}

void OS::Hack::UnhookMouse()
{
    CFRunLoopRemoveSource(
        CFRunLoopGetCurrent(),
        EVENT_TAP_SRC_REF,
        kCFRunLoopDefaultMode
    );

    CFRelease(MOUSE_EVENT_TAP);
}

