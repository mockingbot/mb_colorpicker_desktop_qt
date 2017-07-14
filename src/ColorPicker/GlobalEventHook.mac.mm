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

    switch(type)
    {
        case kCGEventMouseMoved:
            GetGlobalEventHook()->MouseMove(x, y);
        break;
        case kCGEventLeftMouseDown:
            GetGlobalEventHook()->MouseButtonDown(x, y, 0);
        break;
        case kCGEventLeftMouseUp:
            GetGlobalEventHook()->MouseButtonUp(x, y, 0);
        break;
        case kCGEventRightMouseDown:
            GetGlobalEventHook()->MouseButtonDown(x, y, 0);
        break;
        case kCGEventRightMouseUp:
            GetGlobalEventHook()->MouseButtonUp(x, y, 0);
        break;
        default:
            qDebug() << "Get Unknow CGEventType\n";
            throw std::runtime_error(std::string(__CURRENT_FUNCTION_NAME__)+" CGEventType Unknow");
        break;
    }

    return event;
}

CFMachPortRef MOUSE_EVENT_TAP;
CFRunLoopSourceRef EVENT_TAP_SRC_REF;

void GlobalEventHook::HookMouse()
{
    CGEventMask emask = {0};
    emask |= CGEventMaskBit(kCGEventRightMouseDown);
    emask |= CGEventMaskBit(kCGEventRightMouseUp);
    emask |= CGEventMaskBit(kCGEventLeftMouseDown);
    emask |= CGEventMaskBit(kCGEventLeftMouseUp);
    emask |= CGEventMaskBit(kCGEventMouseMoved);

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

void GlobalEventHook::UnhookMouse()
{
    CFRunLoopRemoveSource(
        CFRunLoopGetCurrent(),
        EVENT_TAP_SRC_REF,
        kCFRunLoopDefaultMode
    );

    CFRelease(MOUSE_EVENT_TAP);
}

