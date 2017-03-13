#pragma once

#include <type_traits>

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
    void paintEvent(QPaintEvent* event) override;
private:
    const QPixmap m_pixmap_circle_mask;
    const QPixmap m_pixmap_circle_mask_x2;
private:
    const QImage& m_current_capture_image;
private:
    QColor m_current_color;
    QLabel* m_color_info_label;
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
};


class ColorPickerHost
{
    friend class GlobalEventHook;
private:
    ColorPickerHost();
    ~ColorPickerHost();
private:
    ColorPickerCanvas* m_color_picker_canvas = nullptr;
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
            using rt = typename std::decay<RETURN_TYPE>::type; \
            return rt();\
        } \


    DECLARE_FUNCTION_FOR_OS(void, MakeWindowOverMenubar, WId);
    DECLARE_FUNCTION_FOR_OS(void, ExcluedWindowFromPictureTraceProcess, WId);

    DECLARE_FUNCTION_FOR_OS(void, BootProcessForTrackPictureSurroundCursor);
    DECLARE_FUNCTION_FOR_OS(void, ShutdonwProcessForTrackPictureSurroundCursor);

    DECLARE_FUNCTION_FOR_OS(bool, IsTrackCursorProcessStarted);
    DECLARE_FUNCTION_FOR_OS(const QImage&, GetPictureSurroundedCurrentCursor);

    DECLARE_FUNCTION_FOR_OS(void, GetCurrentCursorPosition, int*, int*);

    DECLARE_FUNCTION_FOR_OS(void, HideCursor);

}// namespace Hack
