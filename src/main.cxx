#include <QtCore/QtCore>

#include "ColorPicker/GlobalEventHook.hxx"
#include "ColorPicker/ColorPickerCanvas.hxx"


#ifdef Q_OS_MAC
    Q_IMPORT_PLUGIN(QCocoaIntegrationPlugin);
#endif // Q_OS_MAC

#ifdef Q_OS_WIN
    Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin);
#endif // Q_OS_WIN


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    Hack::HideCursor<Hack::OS::Current>();

    BootGlobalEventHook();

    auto screens = qGuiApp->screens();
    for(auto&& screen : screens)
    {
        ColorPickerHost::InitColorPickerForScreen(screen);
    }

    ColorPickerHost::SetColorPickerVisible();

    return app.exec();
}
