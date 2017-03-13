#include "ColorPickerCanvas.hxx"

#include <AppKit/AppKit.h>


template<>
void Hack::MakeWindowOverMenubar<Hack::OS::macOS>(WId window_id)
{
    NSView* nativeView = reinterpret_cast<NSView*>(window_id);
    NSWindow* nativeWindow = [nativeView window];

    [nativeWindow setLevel:NSMainMenuWindowLevel+1];
}

template<>
void Hack::GetCurrentCursorPosition<Hack::OS::macOS>(int* x, int* y)
{
    NSPoint mouseLoc = [NSEvent mouseLocation];
    *x = mouseLoc.x;
    *y = mouseLoc.y;
}

bool TRACK_CURSOR_PROCESS_START_STATE = false;

template<>
bool Hack::IsTrackCursorProcessStarted<Hack::OS::macOS>()
{
    return TRACK_CURSOR_PROCESS_START_STATE;
}

// const int CAPTURE_WIDTH = 220;
// const int CAPTURE_HIGHT = 220;
const int CAPTURE_WIDTH = 18;
const int CAPTURE_HIGHT = 18;

QImage init_captured_surround_picture_boot()
{
    // QPixmap pixmap(CAPTURE_WIDTH, CAPTURE_HIGHT);
    QImage image(CAPTURE_WIDTH, CAPTURE_HIGHT, QImage::Format_ARGB32);
    // image.fill(Qt::transparent);
    image.fill(Qt::red);
    return image;
}

// QImage CAPTURED_SURROUND_PICTURE(CAPTURE_WIDTH, CAPTURE_HIGHT, QImage::Format_RGB32);
QImage CAPTURED_SURROUND_PICTURE = init_captured_surround_picture_boot();

template<>
const QImage& Hack::GetPictureSurroundedCurrentCursor<Hack::OS::macOS>()
{
    return CAPTURED_SURROUND_PICTURE;
}


template<>
void Hack::HideCursor<Hack::OS::macOS>()
{
    // TODO:
}

