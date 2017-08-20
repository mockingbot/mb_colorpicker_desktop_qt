#include "ColorPickerHost.hxx"

#include <atomic>

#include <iostream>

#include <AppKit/AppKit.h>
#include <CoreGraphics/CoreGraphics.h>

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
public:
    void stopUpdateTimer();
    void startUpdateTimer();
};

ScreenCaptureHost* capture_host = nullptr;

const uint32_t display_id_list_size = 16; // 16 display is enought
uint32_t display_count = 0;
CGDirectDisplayID display_id_list[display_id_list_size] = {};
CGColorSpaceRef display_color_space_list[display_id_list_size] = {};
CGRect display_bound_list[display_id_list_size] = {};
NSColorSpace* color_space_sRGB;

template<>
void Hack::BootProcessForTrackPictureSurroundCursor<Hack::OS::macOS>()
{
    // qDebug() << __CURRENT_FUNCTION_NAME__;
    if( capture_host != nullptr ){
        return;
    }
    capture_host = new ScreenCaptureHost;
    if( kCGErrorSuccess != CGGetActiveDisplayList(16, display_id_list, &display_count) ){
        qDebug() << "CGGetActiveDisplayList failed\n";
        throw std::runtime_error("CGGetActiveDisplayList Failed\n");
    }
    for(uint32_t idx=0; idx < display_count; ++idx){
        display_color_space_list[idx] = CGDisplayCopyColorSpace(display_id_list[idx]);
        display_bound_list[idx] = CGDisplayBounds(display_id_list[idx]);
        //qDebug() << display_bound_list[idx].origin.x ;
        //qDebug() << display_bound_list[idx].origin.y ;
        //qDebug() << display_bound_list[idx].size.width ;
        //qDebug() << display_bound_list[idx].size.height ;
    }
    color_space_sRGB = [NSColorSpace sRGBColorSpace];
}

template<>
void Hack::ShutdonwProcessForTrackPictureSurroundCursor<Hack::OS::macOS>()
{
    // qDebug() << __CURRENT_FUNCTION_NAME__;
    if( capture_host == nullptr ){
        return;
    }
    delete capture_host;
    for(uint32_t idx=0; idx < display_count; ++idx){
        CGColorSpaceRelease(display_color_space_list[idx]);
    }
}

template<>
void Hack::EnableProcessForTrackPictureSurroundCursor<Hack::OS::macOS>()
{
    capture_host->startUpdateTimer();
}

template<>
void Hack::DisableProcessForTrackPictureSurroundCursor<Hack::OS::macOS>()
{
    capture_host->stopUpdateTimer();
}

/////////////////////////////////////////////////////////////////////////////////
QColor FixColorSpace(const QColor& origin_pixel_color, CGColorSpaceRef origin_color_space)
{
    CGFloat color_values[] = {0/255.f, 0/255.f, 0/255.f, 1.0f};
    color_values[0] = origin_pixel_color.redF();
    color_values[1] = origin_pixel_color.greenF();
    color_values[2] = origin_pixel_color.blueF();

    auto tmp_color = CGColorCreate(origin_color_space, color_values);
    CGFloat fixed_red, fixed_blue, fixed_green;

    @autoreleasepool {
        NSColor* color = [NSColor colorWithCGColor: tmp_color];

        auto color_space_sRGB = [NSColorSpace sRGBColorSpace];
        auto fixed_color = [color colorUsingColorSpace: color_space_sRGB];

        fixed_red = [fixed_color redComponent];
        fixed_green = [fixed_color greenComponent];
        fixed_blue = [fixed_color blueComponent];
    }
    auto fixed_pixel_color = QColor::fromRgbF(fixed_red, fixed_green, fixed_blue);

    CGColorRelease(tmp_color);

    return fixed_pixel_color;
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
    CGPoint cursor_position;
    auto event = ::CGEventCreate(NULL);
    rect.origin = cursor_position = ::CGEventGetLocation(event);
    rect.origin.x = int(rect.origin.x) - CAPTURE_WIDTH/2;
    rect.origin.y = int(rect.origin.y) - CAPTURE_HIGHT/2;
    rect.size.width = CAPTURE_WIDTH;
    rect.size.height = CAPTURE_HIGHT;
    ::CFRelease(event);

    auto cg_image = ::CGWindowListCreateImageFromArray(rect, window_list_filtered, \
                                                    kCGWindowImageNominalResolution);

    auto ns_bitmap_image = [[NSBitmapImageRep alloc] initWithCGImage: cg_image];

    uint32_t display_id_idx = -1;
    for(uint32_t idx=0; idx < display_count; ++idx){
        const auto& rect = display_bound_list[idx];
        if( true == CGRectContainsPoint(rect, cursor_position) ){
            display_id_idx = idx;
            break;
        }
    }
    auto current_display_color_space = display_color_space_list[display_id_idx];

    // fix color space here
    for(int y = 0; y < CAPTURE_HIGHT; ++y)
    {
       for(int x = 0; x < CAPTURE_WIDTH; ++x)
       {
            auto color = [ns_bitmap_image colorAtX: x y: y];
            auto red   = [color redComponent  ];
            auto green = [color greenComponent];
            auto blue  = [color blueComponent ];
            auto origin_pixel_color = QColor::fromRgbF(red, green, blue);

            auto fixed_pixel_color = FixColorSpace(origin_pixel_color, \
                                                    current_display_color_space);
            CAPTURED_SURROUND_CURSOR_IMAGE_PTR->setPixelColor(x, y, fixed_pixel_color);
       }
    }

    [ns_bitmap_image release];

    ::CFRelease(window_list_filtered);
    ::CFRelease(window_list);
    ::CGImageRelease(cg_image);

    TRACK_CURSOR_PROCESS_START_STATE = true;
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

void
ScreenCaptureHost::startUpdateTimer()
{
    m_update_timer->start();
}

void
ScreenCaptureHost::stopUpdateTimer()
{
    m_update_timer->stop();
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

