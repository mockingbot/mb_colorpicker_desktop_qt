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

class ScreenCaptureHost: public QWidget
{
public:
    ScreenCaptureHost();
    ~ScreenCaptureHost();
private:
    QTimer* m_update_timer;
private:
    HWND m_hwnd_host;
    HWND m_hwnd_magnifier;
};

ScreenCaptureHost* capture_host = nullptr;


template<>
void Hack::BootProcessForTrackPictureSurroundCursor<Hack::OS::Windows>()
{
    // printf("%s\n", __FUNCTION__);
    if( capture_host == nullptr ){

        if (FALSE == ::MagInitialize())
        {
            printf("::MagInitialize Failed\n");
            throw std::runtime_error("::MagInitialize Failed");
        }

        capture_host = new ScreenCaptureHost;
    }
}

template<>
void Hack::ShutdonwProcessForTrackPictureSurroundCursor<Hack::OS::Windows>()
{
    // printf("%s\n", __FUNCTION__);
    if( capture_host != nullptr ){

        capture_host->close();
        delete capture_host;

        if (FALSE == ::MagUninitialize())
        {
            printf("::MagUninitialize Failed\n");
            throw std::runtime_error("::MagUninitialize Failed");
        }

    }
}

/////////////////////////////////////////////////////////////////////////////////

// TODO: performace improve
BOOL WINAPI CaptureCallback( HWND hWnd,
                             void* srcdata, MAGIMAGEHEADER srcheader,
                             void* destdata, MAGIMAGEHEADER destheader,
                             RECT unclipped, RECT clipped,
                             HRGN dirty
                            )
{
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

ScreenCaptureHost::ScreenCaptureHost()
    :QWidget(nullptr, Qt::Tool)
    //
    ,m_update_timer(new QTimer(this))
{
    setAutoFillBackground(false);

    // setAttribute(Qt::WA_DeleteOnClose);

    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_TransparentForMouseEvents);

    setWindowFlags(windowFlags() | Qt::FramelessWindowHint);

    setFixedSize(CAPTURE_WIDTH, CAPTURE_HIGHT);

    m_hwnd_host = HWND(winId());

    // must set this
    ::SetLayeredWindowAttributes(m_hwnd_host, 0, 255, LWA_ALPHA);

    // Create a magnifier control that fills the client area.
    RECT mag_window_rect;
    ::GetClientRect(m_hwnd_host, &mag_window_rect);

    m_hwnd_magnifier = ::CreateWindow(WC_MAGNIFIER,
                            TEXT("MagnifierWidget"),
                            WS_CHILD | WS_VISIBLE,
                            mag_window_rect.left,      // x
                            mag_window_rect.top,       // y
                            mag_window_rect.right,     // width
                            mag_window_rect.bottom,    // hight
                            m_hwnd_host,               // parent window
                            nullptr,                   // menu
                            GetModuleHandle(nullptr),  // hInstance
                            nullptr                    // lpParam
                        );

    if (!m_hwnd_magnifier)
    {
        printf("::CreateWindow Failed\n");
        throw std::runtime_error("::CreateWindow Failed");
    }

    // Set the magnification factor.
    MAGTRANSFORM matrix;
    memset(&matrix, 0, sizeof(matrix));
    matrix.v[0][0] = 1;
    matrix.v[1][1] = 1;
    matrix.v[2][2] = 1.0f;

    if( FALSE == ::MagSetWindowTransform(m_hwnd_magnifier, &matrix) )
    {
        printf("::MagSetWindowTransform Failed\n");
        throw std::runtime_error("::MagSetWindowTransform Failed");
    }

    if( FALSE == ::MagSetImageScalingCallback(m_hwnd_magnifier, CaptureCallback) )
    {
        printf("::MagSetImageScalingCallback Failed\n");
        throw std::runtime_error("::MagSetImageScalingCallback Failed");
    }

    connect(m_update_timer, &QTimer::timeout, [=]()->void
    {
        if( FALSE == ::MagSetWindowFilterList( m_hwnd_magnifier,
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
        source_rect.left = mouse_point.x - CAPTURE_WIDTH/2;
        source_rect.top  = mouse_point.y - CAPTURE_HIGHT/2;
        source_rect.right = source_rect.left + CAPTURE_WIDTH;
        source_rect.bottom = source_rect.top + CAPTURE_HIGHT;

        // Set the source rectangle for the magnifier control.
        ::MagSetWindowSource(m_hwnd_magnifier, source_rect);

        // Force redraw.
        ::InvalidateRect(m_hwnd_magnifier, NULL, TRUE);
    });

    m_update_timer->setSingleShot(false);
    m_update_timer->setInterval(50); // 20 hz
    m_update_timer->start();
}

ScreenCaptureHost::~ScreenCaptureHost()
{
    // printf("%s\n", __FUNCTION__);
}

/////////////////////////////////////////////////////////////////////////////////

template<>
void Hack::HideCursor<Hack::OS::Windows>()
{
    ::ShowCursor(FALSE);
}

