#include "ColorPickerCanvas.hxx"

#include <memory>
#include <fstream>
#include <iostream>
#include <functional>
#include <unordered_map>

#include <mutex>
#include <atomic>

#include <Windows.h>
#include <Magnification.h>

#pragma comment(lib, "MAGNIFICATION.lib")



/////////////////////////////////////////////////////////////////////////////////

template<>
void Hack::SetWindowFocus<Hack::OS::Current>(WId window)
{
    ::SetFocus(HWND(window));
}

/////////////////////////////////////////////////////////////////////////////////

template<>
void Hack::GetCurrentCursorPosition<Hack::OS::Current>(int* x, int* y)
{
    POINT cursor_pos;
    ::GetCursorPos(&cursor_pos);
    *x =  cursor_pos.x;
    *y =  cursor_pos.y;
}

/////////////////////////////////////////////////////////////////////////////////

// the magnification api can only exclude some window, 20 is enough for us
std::atomic<uint32_t> EXCLUDE_WINDOW_COUNT{0};
HWND EXCLUDE_WINDOW_LIST[20] = {0};

template<>
void Hack::ExcluedWindowFromPictureTraceProcess<Hack::OS::Windows>(WId window)
{
    EXCLUDE_WINDOW_LIST[EXCLUDE_WINDOW_COUNT] = HWND(window);
    EXCLUDE_WINDOW_COUNT++;
}

bool TRACK_CURSOR_PROCESS_START_STATE = false;

template<>
bool Hack::IsTrackCursorProcessStarted<Hack::OS::Windows>()
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
void Hack::GetPictureSurroundedCurrentCursor<Hack::OS::Windows>(QImage** ptr)
{
    static auto captured_surround_cursor_image = init_captured_surround_cursor_image();
    *ptr = &captured_surround_cursor_image;
    CAPTURED_SURROUND_CURSOR_IMAGE_PTR = &captured_surround_cursor_image;
}

/////////////////////////////////////////////////////////////////////////////////

class EventFilter: public QObject, public QAbstractNativeEventFilter
{
public:
    EventFilter()
    {
        // printf("%s\n", __FUNCTION__);
    }
public:
    ~EventFilter()
    {
        // printf("%s\n", __FUNCTION__);
    }
public:
    bool nativeEventFilter(const QByteArray&, void*, long*) final override;
};

HWND HWND_QT = 0;
HWND HWND_HOST = 0;
HWND HWND_MAGNIFIER = 0;

QTimer* MAGNIFIER_UPDATE_TIMER = nullptr;

bool
EventFilter::nativeEventFilter(const QByteArray& eventType, void* message, long* result)
{
    if( eventType == "windows_dispatcher_MSG"){
        return false;
    }

    auto msg = reinterpret_cast<MSG*>(message);

    // std::cout << msg->hwnd << '\t'
    //           << msg->message << '\t'
    //           << (void*)msg->wParam << '\t'
    //           << (void*)msg->lParam << '\t'
    //           << '\n';

    if( (HWND_HOST == 0) && (msg->hwnd !=  HWND_QT) ){
        *result = ::DefWindowProc(msg->hwnd, msg->message, msg->wParam, msg->lParam);
        return true;
    }

    if( msg->hwnd == HWND_HOST ){
        *result = ::DefWindowProc(msg->hwnd, msg->message, msg->wParam, msg->lParam);
        return true;
    }

    return false;
}

BOOL WINAPI TheMagnifierCallback(HWND,void*,MAGIMAGEHEADER,void*,MAGIMAGEHEADER,RECT,RECT,HRGN);

template<>
void Hack::BootProcessForTrackPictureSurroundCursor<Hack::OS::Windows>()
{
    // printf("%s\n", __FUNCTION__);

    if (FALSE == ::MagInitialize())
    {
        printf("::MagInitialize Failed\n");
        throw std::runtime_error("::MagInitialize Failed");
    }
}

template<>
void Hack::ShutdonwProcessForTrackPictureSurroundCursor<Hack::OS::Windows>()
{
    // printf("%s\n", __FUNCTION__);

    MAGNIFIER_UPDATE_TIMER->stop();
    // ::SendMessage(HWND_HOST, WM_DESTROY, 0, 0);

    // if (FALSE == ::MagUninitialize())
    // {
    //     printf("::MagUninitialize Failed %d\n", ::GetLastError());
    //     throw std::runtime_error("::MagUninitialize Failed");
    // }
}

template<>
void Hack::BootMagnificationHost<Hack::OS::Windows>(WId winId)
{
    // printf("%s\n", __FUNCTION__);

    HWND_QT = HWND(winId);

    // std::cout << HWND_QT << " << HWND_QT \n";

    auto event_filter = new EventFilter;
    qApp->installNativeEventFilter(event_filter);

    auto hInstance = ::GetModuleHandle(nullptr);

    std::wstring window_class_name(256, 0);

    if( 0 == ::GetClassName(HWND_QT,
                            const_cast<wchar_t*>(window_class_name.c_str()),
                            int(window_class_name.size()) )
        )
    {
        printf("::GetClassName Failed %d\n", ::GetLastError());
        throw std::runtime_error("::GetClassName Failed");
    }

    HWND_HOST = ::CreateWindowEx(WS_EX_LAYERED,
                                 window_class_name.c_str(),
                                 L"ColorPickerHost",
                                 WS_OVERLAPPEDWINDOW,
                                 0,
                                 0,
                                 CAPTURE_WIDTH,
                                 CAPTURE_HIGHT,
                                 nullptr,
                                 nullptr,
                                 hInstance,
                                 nullptr);
    if (!HWND_HOST)
    {
        printf("::CreateWindow HWND_HOST Failed\n");
        throw std::runtime_error("::CreateWindow HWND_HOST Failed");
    }
    // std::cout << HWND_HOST << " << HWND_HOST \n";

    HWND_MAGNIFIER = ::CreateWindow(WC_MAGNIFIER,
                                    TEXT("MagnifierWidget"),
                                    WS_CHILD | WS_VISIBLE,
                                    0,
                                    0,
                                    CAPTURE_WIDTH,
                                    CAPTURE_HIGHT,
                                    HWND_HOST,
                                    nullptr,
                                    hInstance,
                                    nullptr );
    if (!HWND_MAGNIFIER)
    {
        printf("::CreateWindow HWND_MAGNIFIER Failed\n");
        throw std::runtime_error("::CreateWindow HWND_MAGNIFIER Failed");
    }
    // std::cout << HWND_MAGNIFIER << " << HWND_MAGNIFIER \n";

    if( FALSE == ::MagSetImageScalingCallback(HWND_MAGNIFIER, TheMagnifierCallback) )
    {
        printf("::MagSetImageScalingCallback Failed\n");
        throw std::runtime_error("::MagSetImageScalingCallback Failed");
    }

    MAGNIFIER_UPDATE_TIMER = new QTimer();

    QObject::connect(MAGNIFIER_UPDATE_TIMER, &QTimer::timeout, []()->void
    {
        if( FALSE == ::MagSetWindowFilterList(HWND_MAGNIFIER,
                                              MW_FILTERMODE_EXCLUDE,
                                              EXCLUDE_WINDOW_COUNT,
                                              EXCLUDE_WINDOW_LIST )
          )
        {
            printf("::MagSetWindowFilterList Failed\n");
            throw std::runtime_error("::MagSetWindowFilterList Failed");
        }

        POINT mouse_point;
        ::GetCursorPos(&mouse_point);

        RECT source_rect;
        source_rect.left = mouse_point.x - CAPTURE_WIDTH/2 + 1;
        source_rect.top  = mouse_point.y - CAPTURE_HIGHT/2 + 1;
        source_rect.right =  CAPTURE_WIDTH;
        source_rect.bottom = CAPTURE_HIGHT;

        // Set the source rectangle for the magnifier control.
        ::MagSetWindowSource(HWND_MAGNIFIER, source_rect);

        // Force redraw.
        ::InvalidateRect(HWND_MAGNIFIER, NULL, TRUE);
    });

    MAGNIFIER_UPDATE_TIMER->setSingleShot(false);
    MAGNIFIER_UPDATE_TIMER->setInterval(50); // 20 hz
    MAGNIFIER_UPDATE_TIMER->start();
}


/////////////////////////////////////////////////////////////////////////////////

// TODO: performace improve
BOOL WINAPI TheMagnifierCallback(HWND hWnd, \
                                 void* srcdata, MAGIMAGEHEADER srcheader, \
                                 void* destdata, MAGIMAGEHEADER destheader, \
                                 RECT unclipped, RECT clipped, \
                                 HRGN dirty)
{
    // printf("%s\n", __FUNCTION__);
    // TRACK_CURSOR_PROCESS_START_STATE = true;
    // return true;

    BITMAPINFOHEADER bmif;
    // Setup the bitmap info header
    bmif.biSize = sizeof(BITMAPINFOHEADER);
    bmif.biWidth = srcheader.width;
    bmif.biHeight = srcheader.height;
    bmif.biSizeImage = srcheader.cbSize;

    bmif.biPlanes = 1;
    bmif.biBitCount = (WORD)(bmif.biSizeImage / bmif.biHeight / bmif.biWidth * 8);
    bmif.biCompression = BI_RGB;

    auto pData = (BYTE*)std::malloc(bmif.biSizeImage);
    std::memcpy(pData, srcdata, bmif.biSizeImage);

    // The data bit is in top->bottom order, so we convert it to bottom->top order
    LONG lineSize = bmif.biWidth * bmif.biBitCount / 8;
    BYTE* pLineData = new BYTE[lineSize];
    BYTE* pStart;
    BYTE* pEnd;
    LONG lineStart = 0;
    LONG lineEnd = bmif.biHeight - 1;
    while (lineStart < lineEnd)
    {
        // Get the address of the swap line
        pStart = pData + (lineStart * lineSize);
        pEnd = pData + (lineEnd * lineSize);
        // Swap the top with the bottom
        memcpy(pLineData, pStart, lineSize);
        memcpy(pStart, pEnd, lineSize);
        memcpy(pEnd, pLineData, lineSize);

        // Adjust the line index
        lineStart++;
        lineEnd--;
    }
    delete[] pLineData;

    // Setup the bitmap file header
    BITMAPFILEHEADER bmfh;
    LONG offBits = sizeof(BITMAPFILEHEADER) + bmif.biSize;
    bmfh.bfType = 0x4d42; // "BM"
    bmfh.bfOffBits = offBits;
    bmfh.bfSize = offBits + bmif.biSizeImage;
    bmfh.bfReserved1 = 0;
    bmfh.bfReserved2 = 0;

    int bmp_data_buffer_size = sizeof(BITMAPFILEHEADER) + \
                               sizeof(BITMAPINFOHEADER) + \
                               bmif.biSizeImage;

    uchar* bmp_data_buffer = (uchar*)std::malloc(bmp_data_buffer_size);

    std::memcpy(bmp_data_buffer, &bmfh, sizeof(BITMAPFILEHEADER));
    bmp_data_buffer += sizeof(BITMAPFILEHEADER);
    std::memcpy(bmp_data_buffer, &bmif, sizeof(BITMAPINFOHEADER));
    bmp_data_buffer += sizeof(BITMAPINFOHEADER);
    std::memcpy(bmp_data_buffer, pData, bmif.biSizeImage);
    bmp_data_buffer += bmif.biSizeImage;

    bmp_data_buffer -= bmp_data_buffer_size;

    (*CAPTURED_SURROUND_CURSOR_IMAGE_PTR) = \
            QImage::fromData(bmp_data_buffer, bmp_data_buffer_size, "BMP");

    std::free(bmp_data_buffer);

    std::free(pData);

    // printf("%s\n", __FUNCTION__);

    // std::ofstream outfile ("new.bmp",std::ofstream::binary);

    // outfile.write( (char*) &bmfh, sizeof(BITMAPFILEHEADER)); // bitmap file header
    // outfile.write( (char*) &bmif, sizeof(BITMAPINFOHEADER)); // bitmap info header
    // outfile.write( (char*) pData, bmif.biSizeImage); // converted bitmap data

    // outfile.flush();
    // outfile.close();

    // Set the flag to say that the callback function is finished
    TRACK_CURSOR_PROCESS_START_STATE = true;
    return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////

template<>
void Hack::HideCursor<Hack::OS::Windows>()
{
    ::ShowCursor(FALSE);
}

