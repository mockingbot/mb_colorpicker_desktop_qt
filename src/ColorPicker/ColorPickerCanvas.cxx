#include "ColorPickerCanvas.hxx"

ColorPickerCanvas::ColorPickerCanvas()
    :QWidget(nullptr, Qt::Tool)
    //
    ,m_pixmap_circle_mask(QPixmap(":/Res/CircleMask"))
    ,m_pixmap_circle_mask_x2(QPixmap(":/Res/CircleMask@2"))
    //
    // ,m_color_info_label(new QLabel(this))
    //
    ,m_current_capture_image(Hack::GetPictureSurroundedCurrentCursor<Hack::OS::Current>())
    //
{
    setAutoFillBackground(false);

    setAttribute(Qt::WA_DeleteOnClose);

    setAttribute(Qt::WA_TranslucentBackground);

    setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint);

    Hack::MakeWindowOverMenubar<Hack::OS::macOS>(winId());

    Hack::ExcluedWindowFromPictureTraceProcess<Hack::OS::Windows>(winId());

    // TODO: HiDPI
    setFixedSize(PANEL_WIDTH, PANEL_WIDTH);

    auto font = this->font();
    font.setPixelSize(13);
    setFont(font);

    // m_color_info_label->resize(80, 20);
    // m_color_info_label->move((PANEL_WIDTH-80)/2, PANEL_HIGHT/2+40);
    // m_color_info_label->setAlignment(Qt::AlignCenter);

    // m_color_info_label->setStyleSheet("QLabel{"
    //                              // "    background-color : #E0101010;"
    //                              "    background-color : #F55D54;"
    //                              "    border: 1px solid #000000;"
    //                              "    border-radius: 10px;"
    //                              "    color: #E0FFFFFF;"
    //                              "}");

    // m_color_info_label->setVisible(true);

    auto update_timer = new QTimer(this);
    connect(update_timer, &QTimer::timeout, [=](){
        m_current_color = m_current_capture_image.pixelColor(18/2-1, 18/2-1);
        // m_color_info_label->setText(m_current_color.name().toUpper());
        update();
    });
    update_timer->setSingleShot(false);
    update_timer->start(20); // 50 hz

}

ColorPickerCanvas::~ColorPickerCanvas()
{
    // TODO:
    printf("%s\n", __FUNCTION__);
}

void
ColorPickerCanvas::mousePressEvent(QMouseEvent * event)
{
    // qGuiApp->clipboard()->setText( QString("%1,%2,%3")
    //                         .arg(m_current_color.red())
    //                         .arg(m_current_color.green())
    //                         .arg(m_current_color.blue())
    //                    );

    qGuiApp->clipboard()->setText(m_current_color.name().toUpper());

    qDebug() << m_current_color.name().toUpper() ;

    close();
    qGuiApp->exit(0);
}

void
ColorPickerCanvas::paintEvent(QPaintEvent* event)
{
    printf("%s\n", __FUNCTION__);

    QPainter painter(this);
    // painter.fillRect(rect(), Qt::red);
    // painter.drawPixmap(0, 0, PANEL_WIDTH, PANEL_HIGHT, m_pixmap_circle_mask);
    // return;

    if( true == Hack::IsTrackCursorProcessStarted<Hack::OS::Current>() )
    {
        /* zoomed snapshort */
        painter.save();
        { /// Done !!!!!!!!!!!!
            painter.setRenderHint(QPainter::Antialiasing, true);

            QRegion circle_region;
            {
                auto center = PANEL_WIDTH/2.0;
                auto radius = 86.0;
                circle_region = QRegion(QRect(center-radius, center-radius,
                                        radius*2, radius*2),
                                        QRegion::Ellipse);
            }
            painter.setClipRegion(circle_region);

            // painter.drawImage(0, 0, m_current_capture_image);

            // painter.fillRect(rect(), Qt::white); // background for debug
            // painter.fillRect(rect(), Qt::red); // background for debug

            int center = PANEL_WIDTH/2;
            int radius = ZOOM_PIXEL_GRID_SIZE*ZOOM_PIXEL_COUNT/2.0;

            int x = center - radius - ZOOM_PIXEL_GRID_SIZE/2;
            int y = center - radius - ZOOM_PIXEL_GRID_SIZE/2;
            int w = radius*2 + ZOOM_PIXEL_GRID_SIZE*2;
            int h = w;

            // // int sx = m_posion_x;
            // // int sy = m_posion_y;
            // int sw = ZOOM_PIXEL_COUNT + 2;
            // int sh = ZOOM_PIXEL_COUNT + 2;

            // // painter.drawPixmap( x, y, w, h, m_layer_image, sx, sy, sw, sh );
            // painter.drawPixmap( x, y, w, h, m_pixmap_snapshot, 0, 0, sw, sh);

            painter.drawImage(QRect(x, y, w, h), m_current_capture_image);
        }
        painter.restore();
    }

    /*  grid */
    painter.save();
    {  // Done !!!!!!!!!
        painter.setRenderHint(QPainter::Antialiasing, false);

        QRegion circle_region;
        {
            auto center = PANEL_WIDTH/2.0;
            auto radius = 86.0;
            circle_region = QRegion(QRect(center-radius, center-radius,
                                    radius*2, radius*2),
                                    QRegion::Ellipse);
        }
        painter.setClipRegion(circle_region);

        // painter.setPen(Qt::blue);
        painter.setPen(QColor(85, 106, 113, 104));
        // painter.setPen(QColor(0x84, 0x84, 0x84, 0x84));

        int center = (PANEL_WIDTH-1)/2;
        int shift = ZOOM_PIXEL_GRID_SIZE;

        /** Draw the grid from center or you will FUCK the grid **/

        int y_t =  center - ZOOM_PIXEL_GRID_SIZE/2;
        painter.drawLine(0, y_t, PANEL_WIDTH, y_t); // horizental top

        int x_l =  center - ZOOM_PIXEL_GRID_SIZE/2;
        painter.drawLine(x_l, 0, x_l, PANEL_WIDTH); // vertical left

        int y_b =  center + ZOOM_PIXEL_GRID_SIZE/2;
        painter.drawLine(0, y_b, PANEL_WIDTH, y_b); // horizental bottom

        int x_r =  center + ZOOM_PIXEL_GRID_SIZE/2;
        painter.drawLine(x_r, 0, x_r, PANEL_WIDTH); // vertical right

        for(int idx=0;  idx < ZOOM_PIXEL_COUNT/2; ++idx)
        {
            y_t -= shift;
            painter.drawLine(0, y_t, PANEL_WIDTH, y_t); // horizental

            x_l -= shift;
            painter.drawLine(x_l, 0, x_l, PANEL_WIDTH); // horizental

            y_b += shift;
            painter.drawLine(0, y_b, PANEL_WIDTH, y_b); // horizental

            x_r += shift;
            painter.drawLine(x_r, 0, x_r, PANEL_WIDTH); // horizental
        }

        // draw center black box
        int lt =  center - ZOOM_PIXEL_GRID_SIZE/2; // left top
        // painter.setPen(Qt::black);
        painter.setPen(QColor(0x00, 0x00, 0x00, 255*0.94));
        // painter.setPen(QRgb(0xF55D54));
        // painter.setPen(QColor(0xaa, 0xaa, 0xaa, 0xaa));
        // painter.setPen(QColor(0x84, 0x84, 0x84, 0x84));
        painter.drawRect(lt, lt, ZOOM_PIXEL_GRID_SIZE, ZOOM_PIXEL_GRID_SIZE);

        // painter.setPen(Qt::white);
        painter.setPen(QColor(0xFF, 0xFF, 0xFF, 255*0.94));
        painter.drawRect(lt+1, lt+1, ZOOM_PIXEL_GRID_SIZE-2, ZOOM_PIXEL_GRID_SIZE-2);

    }
    painter.restore();

    // /* center crose for debug */
    // painter.save();
    // { // Done !!!!!!!!
    //     painter.setPen(Qt::black);
    //     // painter.setPen(QRgb(0xF55D54));
    //     auto center = (PANEL_WIDTH-1)/2;

    //     painter.drawLine(center - ZOOM_PIXEL_GRID_SIZE/2, center,
    //                      center - PANEL_WIDTH, center);
    //     painter.drawLine(center + ZOOM_PIXEL_GRID_SIZE/2, center,
    //                      center + PANEL_WIDTH, center);

    //     painter.drawLine(center, center - PANEL_WIDTH,
    //                      center, center - ZOOM_PIXEL_GRID_SIZE/2);
    //     painter.drawLine(center, center + PANEL_WIDTH,
    //                      center, center + ZOOM_PIXEL_GRID_SIZE/2);
    // }
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
    Hack::BootProcessForTrackPictureSurroundCursor<Hack::OS::Windows>();
}

ColorPickerHost::~ColorPickerHost()
{
    printf("%s\n", __FUNCTION__);
    Hack::ShutdonwProcessForTrackPictureSurroundCursor<Hack::OS::Windows>();
}

void
ColorPickerHost::initColorPickerForScreen(QScreen* screen)
{
    if( m_color_picker_canvas == nullptr ){
        m_color_picker_canvas = new ColorPickerCanvas();
    }
}

void
ColorPickerHost::setColorPickerVisible()
{
    int x, y;
    Hack::GetCurrentCursorPosition<Hack::OS::Current>(&x, &y);

    m_color_picker_canvas->moveCenterToPosition(x, y);
    m_color_picker_canvas->setVisible(true);
}

void
ColorPickerHost::traceMouseMove(const int x, const int y)
{
    m_color_picker_canvas->moveCenterToPosition(x, y);
}
