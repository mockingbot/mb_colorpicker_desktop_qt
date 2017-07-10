#pragma once

#include <QtWidgets/QtWidgets>


class ColorPickerCanvas: public QWidget
{
    friend class ColorPickerHost;
    Q_OBJECT
private:
    ColorPickerCanvas();
    ~ColorPickerCanvas();
private:
    void mousePressEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void paintEvent(QPaintEvent* event) override;
private:
    const QPixmap m_pixmap_circle_mask_x1;
    const QPixmap m_pixmap_circle_mask_x2;
private:
    QPixmap m_pixmap_circle_mask;
private:
    void drawGrid(QPainter& painter);
    void drawCross(QPainter& painter);
    void drawCaptureImage(QPainter& painter);
    void setCircleClipRegion(QPainter& painter);
private:
    const QImage& m_current_capture_image;
private:
    QColor m_current_color;
    // QLabel* m_color_info_label;
public:
    QColor currentColor() const
    {
        return m_current_color;
    }
public:
    static const int PANEL_WIDTH = 220;
    static const int PANEL_HIGHT = 220;
private:
    static const int ZOOM_PIXEL_COUNT = 16;
    static const int ZOOM_PIXEL_GRID_SIZE = 10;
public:
    void moveCenterToPosition(const int x, const int y)
    {
        move(x - PANEL_WIDTH/2, y - ColorPickerCanvas::PANEL_HIGHT/2);
    }
private:
    float m_device_pixel_ratio = 1.0;
public:
    void updateDevicePixelRatio(const float devicePixelRatio)
    {
        if( m_device_pixel_ratio == devicePixelRatio ){
            return;
        }
        #ifdef Q_OS_WIN
            return; // do nothing on Windows
        #endif // Q_OS_WIN

        if( devicePixelRatio == 2.0 ){
            m_pixmap_circle_mask = m_pixmap_circle_mask_x2;
        } else {
            m_pixmap_circle_mask = m_pixmap_circle_mask_x1;
        }
        m_device_pixel_ratio = devicePixelRatio;
        update();
    }
};


class ColorPickerHost
{
    friend class GlobalEventHook;
private:
    ColorPickerHost();
    ~ColorPickerHost();
private:
    ColorPickerCanvas* m_color_picker_canvas = nullptr;
private:
    QVector<QRect> m_screen_geometry_list;
    QVector<float> m_screen_device_pixel_ratio_list;
public:
    static ColorPickerHost* Instance();
public:
    static void InitColorPickerForScreen(QScreen* screen){
        return Instance()->initColorPickerForScreen(screen);
    }
    static void SetColorPickerVisible(){
        return Instance()->setColorPickerVisible();
    }
private:
    static void TraceMouseMove(const int x, const int y) {
        Instance()->traceMouseMove(x, y);
    }
private:
    void initColorPickerForScreen(QScreen* screen);
    void setColorPickerVisible();
private:
    void traceMouseMove(const int x, const int y);
};


namespace Hack
{
    struct OS
    {
       class macOS;
       class Windows;

    #ifdef Q_OS_MAC
        typedef OS::macOS Current;
        typedef OS::Windows NotCurrent;
    #endif //Q_OS_MAC

    #ifdef Q_OS_WIN
        typedef OS::Windows Current;
        typedef OS::macOS NotCurrent;
    #endif //Q_OS_WIN

    };

    #define DECLARE_FUNCTION_FOR_OS(RETURN_TYPE, FUN, ...)                \
        template <typename OS_TYPE> RETURN_TYPE FUN(__VA_ARGS__);         \
        template <> inline RETURN_TYPE FUN<OS::NotCurrent>(__VA_ARGS__) { \
            return RETURN_TYPE();\
        } \


    DECLARE_FUNCTION_FOR_OS(void, SetWindowFocus, WId);

    DECLARE_FUNCTION_FOR_OS(void, MakeWindowOverMenubar, WId);
    DECLARE_FUNCTION_FOR_OS(void, ExcluedWindowFromPictureTraceProcess, WId);

    DECLARE_FUNCTION_FOR_OS(void, BootMagnificationHost, WId);
    DECLARE_FUNCTION_FOR_OS(void, BootProcessForTrackPictureSurroundCursor);
    DECLARE_FUNCTION_FOR_OS(void, ShutdonwProcessForTrackPictureSurroundCursor);

    DECLARE_FUNCTION_FOR_OS(bool, IsTrackCursorProcessStarted);
    DECLARE_FUNCTION_FOR_OS(void, GetPictureSurroundedCurrentCursor, QImage**);

    DECLARE_FUNCTION_FOR_OS(void, GetCurrentCursorPosition, int*, int*);

    DECLARE_FUNCTION_FOR_OS(void, HideCursor);

    DECLARE_FUNCTION_FOR_OS(bool, WhetherOneInstanceStarted);

}// namespace Hack
