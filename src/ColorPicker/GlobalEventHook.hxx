#pragma once


class GlobalEventHook
{
    friend void BootGlobalEventHook();
private:
    GlobalEventHook();
    ~GlobalEventHook();
public:
    void MouseMove(const int x, const int y);
    void MouseButtonUp(const int x, const int y, const int mask);
    void MouseButtonDown(const int x, const int y, const int mask);
public:
    static void HookMouse();
    static void UnhookMouse();
    static void HookKeyboard();
    static void UnhookKeyboard();
};

void BootGlobalEventHook();
GlobalEventHook* GetGlobalEventHook();
