#pragma once


class GlobalEventHook
{
    friend void BootGlobalEventHook();
private:
    GlobalEventHook();
    ~GlobalEventHook();
public:
    void MouseMove(const int x, const int y);
};

void BootGlobalEventHook();
GlobalEventHook* GetGlobalEventHook();

namespace OS
{
namespace Hack
{

void SetCursor();

void HookMouse();
void UnhookMouse();

}// namespace Hack
}// namespace OS