#include "ColorPickerHost.hxx"

#include <atomic>

#include <iostream>

#include <AppKit/AppKit.h>

#include <QtMacExtras/QtMacExtras>


template<>
void Hack::MakeWindowOverMenubar<Hack::OS::macOS>(WId window)
{
    auto nativeWindow = [reinterpret_cast<NSView*>(window) window];
    [nativeWindow setLevel:NSMainMenuWindowLevel+1];
}

/////////////////////////////////////////////////////////////////////////////////

template<>
void Hack::SetWindowFocus<Hack::OS::macOS>(WId window)
{
    [[NSApplication sharedApplication] activateIgnoringOtherApps: YES];
}

/////////////////////////////////////////////////////////////////////////////////

template<>
void Hack::GetCurrentCursorPosition<Hack::OS::macOS>(int* x, int* y)
{
    auto event = ::CGEventCreate(NULL);
    auto point = ::CGEventGetLocation(event);
    *x = point.x;
    *y = point.y;
    CFRelease(event);
}

/////////////////////////////////////////////////////////////////////////////////
std::atomic<uint32_t> EXCLUDE_WINDOW_COUNT{0};
CGWindowID EXCLUDE_WINDOW_LIST[20] = {0};

template<>
void Hack::ExcluedWindowFromPictureTraceProcess<Hack::OS::macOS>(WId window)
{
    // don't add same window in the list, your will get fucked
    auto nativeWindow = [reinterpret_cast<NSView*>(window) window];
    auto windowID = [nativeWindow windowNumber];

    EXCLUDE_WINDOW_LIST[EXCLUDE_WINDOW_COUNT] = windowID;
    EXCLUDE_WINDOW_COUNT++;
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

QImage init_captured_surround_cursor_image()
{
    QImage image(CAPTURE_WIDTH, CAPTURE_HIGHT, QImage::Format_ARGB32);
    // image.fill(Qt::transparent);
    image.fill(Qt::red);
    return image;
}

QImage* CAPTURED_SURROUND_CURSOR_IMAGE_PTR;

template<>
void Hack::GetPictureSurroundedCurrentCursor<Hack::OS::macOS>(QImage** ptr)
{
    static auto captured_surround_cursor_image = init_captured_surround_cursor_image();
    *ptr = &captured_surround_cursor_image;
    CAPTURED_SURROUND_CURSOR_IMAGE_PTR = &captured_surround_cursor_image;
}


/////////////////////////////////////////////////////////////////////////////////

class ScreenCaptureHost: public QObject
{
public:
    ScreenCaptureHost();
    ~ScreenCaptureHost();
private:
    QTimer* m_update_timer;
};

ScreenCaptureHost* capture_host = nullptr;


template<>
void Hack::BootProcessForTrackPictureSurroundCursor<Hack::OS::macOS>()
{
    // qDebug() << __CURRENT_FUNCTION_NAME__;
    if( capture_host == nullptr ){
        capture_host = new ScreenCaptureHost;
    }
}

template<>
void Hack::ShutdonwProcessForTrackPictureSurroundCursor<Hack::OS::macOS>()
{
    // qDebug() << __CURRENT_FUNCTION_NAME__;
    if( capture_host != nullptr ){
        delete capture_host;
    }
}

/////////////////////////////////////////////////////////////////////////////////
void CaptureImageSurroundCursor()
{
    auto window_list = ::CGWindowListCreate(kCGWindowListOptionOnScreenOnly, \
                                            kCGNullWindowID);

    auto window_list_size = ::CFArrayGetCount(window_list);

    auto window_list_filtered = ::CFArrayCreateMutableCopy(kCFAllocatorDefault, \
                                                  window_list_size, window_list);

    for (int idx = window_list_size - 1; idx >= 0; --idx)
    {
        auto window = (CGWindowID)(uintptr_t)::CFArrayGetValueAtIndex(window_list, idx);

        if( EXCLUDE_WINDOW_COUNT == 0 ){
            break;
        }

        for (int item_idx = EXCLUDE_WINDOW_COUNT-1; item_idx >= 0; --item_idx)
        {
            if( EXCLUDE_WINDOW_LIST[item_idx] == window )
            {
                ::CFArrayRemoveValueAtIndex(window_list_filtered, idx);
            }
        }
    }

    NSRect rect;
    auto event = ::CGEventCreate(NULL);
    rect.origin = ::CGEventGetLocation(event);
    rect.origin.x = int(rect.origin.x) - CAPTURE_WIDTH/2;
    rect.origin.y = int(rect.origin.y) - CAPTURE_HIGHT/2;
    rect.size.width = CAPTURE_WIDTH;
    rect.size.height = CAPTURE_HIGHT;
    ::CFRelease(event);

    auto image = ::CGWindowListCreateImageFromArray(rect, window_list_filtered, \
                                                    kCGWindowImageNominalResolution);

    (*CAPTURED_SURROUND_CURSOR_IMAGE_PTR) = QtMac::fromCGImageRef(image).toImage();

    TRACK_CURSOR_PROCESS_START_STATE = true;

    ::CGImageRelease(image);
    ::CFRelease(window_list_filtered);
    ::CFRelease(window_list);
}

ScreenCaptureHost::ScreenCaptureHost()
    :QObject(nullptr)
    //
    ,m_update_timer(new QTimer(this))
{
    // qDebug() << __CURRENT_FUNCTION_NAME__;
    connect(m_update_timer, &QTimer::timeout, CaptureImageSurroundCursor);

    m_update_timer->setSingleShot(false);
    m_update_timer->setInterval(50); // 20 hz
    m_update_timer->start();
}

ScreenCaptureHost::~ScreenCaptureHost()
{
    // qDebug() << __CURRENT_FUNCTION_NAME__;
}

/////////////////////////////////////////////////////////////////////////////////

template<>
void Hack::HideCursor<Hack::OS::macOS>()
{
    [NSCursor hide];
}


template<>
void Hack::ShowCursor<Hack::OS::macOS>()
{
    [NSCursor unhide];
}


template<>
bool Hack::WhetherOneInstanceStarted<Hack::OS::macOS>()
{
    if ([[NSRunningApplication runningApplicationsWithBundleIdentifier:
                            [[NSBundle mainBundle] bundleIdentifier]] count] > 1) {
        return true;
    }

    return false;
}

