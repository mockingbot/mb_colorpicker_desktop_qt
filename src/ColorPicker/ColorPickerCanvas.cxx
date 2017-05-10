#include "ColorPickerCanvas.hxx"

ColorPickerCanvas::ColorPickerCanvas()
    :QWidget(nullptr, Qt::Tool)
    //
    ,m_pixmap_circle_mask_x1(QPixmap(":/Res/CircleMask"))
    ,m_pixmap_circle_mask_x2(QPixmap(":/Res/CircleMask@2"))
    //
    ,m_pixmap_circle_mask(m_pixmap_circle_mask_x1)
    //
    // ,m_color_info_label(new QLabel(this))
    //
    ,m_current_capture_image(CAPTURE_WIDTH, CAPTURE_HIGHT, QImage::Format_ARGB32)
    //
{
    setAutoFillBackground(false);

    setAttribute(Qt::WA_DeleteOnClose);

    setAttribute(Qt::WA_TranslucentBackground);

    setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint);

    Hack::MakeWindowOverMenubar<Hack::OS::macOS>(winId());

    Hack::BootMagnificationHost<Hack::OS::Windows>(winId());

    Hack::ExcluedWindowFromPictureTraceProcess<Hack::OS::Current>(winId());

    setFixedSize(PANEL_WIDTH, PANEL_WIDTH);

    auto font = this->font();
    font.setPixelSize(13);
    setFont(font);

    /*
    m_color_info_label->resize(80, 20);
    m_color_info_label->move((PANEL_WIDTH-80)/2, PANEL_HIGHT/2+40);
    m_color_info_label->setAlignment(Qt::AlignCenter);

    m_color_info_label->setStyleSheet("QLabel{"
                                 // "    background-color : #E0101010;"
                                 "    background-color : #F55D54;"
                                 "    border: 1px solid #000000;"
                                 "    border-radius: 10px;"
                                 "    color: #E0FFFFFF;"
                                 "}");

    m_color_info_label->setVisible(true);
    */

    auto update_timer = new QTimer(this);
    connect(update_timer, &QTimer::timeout, [=](){
        Hack::GetPictureSurroundedCurrentCursor<Hack::OS::Current>(&m_current_capture_image);
        m_current_color = m_current_capture_image.pixelColor(18/2-1, 18/2-1);
        // m_color_info_label->setText(m_current_color.name().toUpper());
        update();
    });
    update_timer->setSingleShot(false);
    update_timer->start(50); // 20 hz
}

ColorPickerCanvas::~ColorPickerCanvas()
{
    // TODO:
    // printf("%s\n", __FUNCTION__);
}

void
ColorPickerCanvas::mousePressEvent(QMouseEvent* event)
{
    // qGuiApp->clipboard()->setText( QString("%1,%2,%3")
    //                         .arg(m_current_color.red())
    //                         .arg(m_current_color.green())
    //                         .arg(m_current_color.blue())
    //                    );

    qGuiApp->clipboard()->setText(m_current_color.name().toUpper());

    // qDebug() << m_current_color.name().toUpper().toStdString() ;
    printf("%s\n", m_current_color.name().toUpper().toStdString().c_str());

    close();
    qGuiApp->exit(0);
}

void
ColorPickerCanvas::keyPressEvent(QKeyEvent* event)
{
    if( event->key() == Qt::Key_Escape ){
        close();
        qGuiApp->exit(0);
    }
}

void
ColorPickerCanvas::drawGrid(QPainter& painter)
{
    painter.setRenderHint(QPainter::Antialiasing, false);

    // painter.setPen(Qt::blue);
    // painter.setPen(Qt::white);
    // painter.setPen(QColor(0x84, 0x84, 0x84, 0x84));
    // painter.setPen(QColor(0xFF, 0xFF, 0xFF, 0xFF*0.95));
    painter.setPen(QColor(0x00, 0x00, 0x00, 0xFF*0.05));

    float center = PANEL_WIDTH/2.0;
    float shift = ZOOM_PIXEL_GRID_SIZE;

    // Draw the grid from center or you will FUCK the grid

    float y_t = center - ZOOM_PIXEL_GRID_SIZE/2.0;
    painter.drawLine(QPointF(0, y_t), QPointF(PANEL_WIDTH, y_t)); // horizental top

    float x_l = center - ZOOM_PIXEL_GRID_SIZE/2.0;
    painter.drawLine(QPointF(x_l, 0), QPointF(x_l, PANEL_WIDTH)); // vertical left

    float y_b = center + ZOOM_PIXEL_GRID_SIZE/2.0;
    painter.drawLine(QPointF(0, y_b), QPointF(PANEL_WIDTH, y_b)); // horizental bottom

    float x_r = center + ZOOM_PIXEL_GRID_SIZE/2.0;
    painter.drawLine(QPointF(x_r, 0), QPointF(x_r, PANEL_WIDTH)); // vertical right

    for(int idx=0; idx < ZOOM_PIXEL_COUNT/2; ++idx)
    {
        y_t -= shift;
        painter.drawLine(QPointF(0, y_t), QPointF(PANEL_WIDTH, y_t)); // horizental

        x_l -= shift;
        painter.drawLine(QPointF(x_l, 0), QPointF(x_l, PANEL_WIDTH)); // horizental

        y_b += shift;
        painter.drawLine(QPointF(0, y_b), QPointF(PANEL_WIDTH, y_b)); // horizental

        x_r += shift;
        painter.drawLine(QPointF(x_r, 0), QPointF(x_r, PANEL_WIDTH)); // horizental
    }

    // draw center black box
    float lt = center - ZOOM_PIXEL_GRID_SIZE/2.0; // left top

    // painter.setPen(Qt::black);
    painter.setPen(QColor(0x00, 0x00, 0x00, 0xFF*0.95));
    // painter.setPen(QColor(0x84, 0x84, 0x84, 0x84));
    // painter.drawRect(QRectF(lt-1, lt-1, \
    //                          ZOOM_PIXEL_GRID_SIZE+2, ZOOM_PIXEL_GRID_SIZE+2));
    painter.drawRect(QRectF(lt, lt, \
                             ZOOM_PIXEL_GRID_SIZE, ZOOM_PIXEL_GRID_SIZE));

    // painter.setPen(Qt::white);
    painter.setPen(QColor(0xFF, 0xFF, 0xFF, 0xFF*0.95));
    // painter.setPen(QColor(0xFF, 0xFF, 0xFF, 255*0.94));
    painter.drawRect(QRectF(lt+1, lt+1, \
                            ZOOM_PIXEL_GRID_SIZE-2, ZOOM_PIXEL_GRID_SIZE-2));
    // painter.drawRect(QRectF(lt, lt, \
    //                         ZOOM_PIXEL_GRID_SIZE, ZOOM_PIXEL_GRID_SIZE));
}

void
ColorPickerCanvas::drawCross(QPainter& painter)
{
    painter.setRenderHint(QPainter::Antialiasing, false);

    // painter.setPen(Qt::black);
    painter.setPen(Qt::white);

    float center = PANEL_WIDTH/2.0;

    painter.drawLine(QPointF(center - ZOOM_PIXEL_GRID_SIZE/2.0, center),
                     QPointF(center - PANEL_WIDTH, center));
    painter.drawLine(QPointF(center + ZOOM_PIXEL_GRID_SIZE/2.0, center),
                     QPointF(center + PANEL_WIDTH, center));

    painter.drawLine(QPointF(center, center - PANEL_WIDTH),
                     QPointF(center, center - ZOOM_PIXEL_GRID_SIZE/2.0));
    painter.drawLine(QPointF(center, center + PANEL_WIDTH),
                     QPointF(center, center + ZOOM_PIXEL_GRID_SIZE/2.0));
}

void
ColorPickerCanvas::drawCaptureImage(QPainter& painter)
{
    painter.setRenderHint(QPainter::Antialiasing, false);

    float center = PANEL_WIDTH/2.0;
    float radius = ZOOM_PIXEL_GRID_SIZE*ZOOM_PIXEL_COUNT/2.0;

    float x = center - radius - ZOOM_PIXEL_GRID_SIZE/2;
    float y = center - radius - ZOOM_PIXEL_GRID_SIZE/2;
    float w = radius*2 + ZOOM_PIXEL_GRID_SIZE*2;
    float h = w;

    painter.drawImage(QRectF(x, y, w, h), m_current_capture_image);
}

void
ColorPickerCanvas::setCircleClipRegion(QPainter& painter)
{
    QRegion circle_region;

    float center = PANEL_WIDTH/2.0;
    float radius = ZOOM_PIXEL_GRID_SIZE*(ZOOM_PIXEL_COUNT+1)/2.0 + 3;

    circle_region = QRegion(QRect(center-radius, center-radius, radius*2, radius*2), QRegion::Ellipse);

    painter.setClipRegion(circle_region);
}

void
ColorPickerCanvas::paintEvent(QPaintEvent* event)
{
    // printf("%s\n", __FUNCTION__);

    QPainter painter(this);
    // painter.fillRect(rect(), Qt::red);
    // painter.drawPixmap(0, 0, PANEL_WIDTH, PANEL_HIGHT, m_pixmap_circle_mask);
    // return;

    if( true == Hack::IsTrackCursorProcessStarted<Hack::OS::Current>() )
    {
        /* zoomed snapshort */
        painter.save();
        setCircleClipRegion(painter);
        drawCaptureImage(painter);
        painter.restore();
    }

    painter.save();
    setCircleClipRegion(painter);
    drawGrid(painter);
    painter.restore();

    // painter.save();
    // drawCross(painter);
    // painter.restore();

    painter.drawPixmap(0, 0, PANEL_WIDTH, PANEL_HIGHT, m_pixmap_circle_mask);
}

ColorPickerHost*
ColorPickerHost::Instance()
{
    static ColorPickerHost host;
    return &host;
}

ColorPickerHost::ColorPickerHost()
{
    printf("%s\n", __FUNCTION__);
    Hack::BootProcessForTrackPictureSurroundCursor<Hack::OS::Current>();
}

ColorPickerHost::~ColorPickerHost()
{
    printf("%s\n", __FUNCTION__);
    Hack::ShutdonwProcessForTrackPictureSurroundCursor<Hack::OS::Current>();
}

void
ColorPickerHost::initColorPickerForScreen(QScreen* screen)
{
    if( m_color_picker_canvas == nullptr ){
        m_color_picker_canvas = new ColorPickerCanvas();
    }
    m_screen_geometry_list.push_back(screen->geometry());
    m_screen_device_pixel_ratio_list.push_back(screen->devicePixelRatio());

    // qDebug() << screen << screen->geometry() << screen->devicePixelRatio();
}

void
ColorPickerHost::setColorPickerVisible()
{
    int x, y;
    Hack::GetCurrentCursorPosition<Hack::OS::Current>(&x, &y);

    for (int idx = 0; idx < m_screen_geometry_list.size(); ++idx)
    {
        if( m_screen_geometry_list[idx].contains(x, y) )
        {
            m_color_picker_canvas->updateDevicePixelRatio( \
                            m_screen_device_pixel_ratio_list[idx]);
            break;
        }
    }

    m_color_picker_canvas->moveCenterToPosition(x, y);
    m_color_picker_canvas->setVisible(true);

    Hack::SetWindowFocus<Hack::OS::Windows>(m_color_picker_canvas->winId());
}

void
ColorPickerHost::traceMouseMove(const int x, const int y)
{
    for (int idx = 0; idx < m_screen_geometry_list.size(); ++idx)
    {
        if( m_screen_geometry_list[idx].contains(x, y) )
        {
            m_color_picker_canvas->updateDevicePixelRatio( \
                            m_screen_device_pixel_ratio_list[idx]);
            break;
        }
    }

    m_color_picker_canvas->moveCenterToPosition(x, y);

    /*
     * PATCH :
     *   sometime, this window don't get focused
     *   so, right now, we call SetWindowFocus while we move the mouse
     */
    Hack::SetWindowFocus<Hack::OS::Windows>(m_color_picker_canvas->winId());

}
