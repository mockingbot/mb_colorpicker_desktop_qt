#include <memory>

#include <QtCore/QtCore>

#include "ColorPicker/DaemonModeHost.hxx"
#include "ColorPicker/GlobalEventHook.hxx"
#include "ColorPicker/ColorPickerHost.hxx"


#ifdef Q_OS_MAC
    Q_IMPORT_PLUGIN(QCocoaIntegrationPlugin);
#endif // Q_OS_MAC

#ifdef Q_OS_WIN
    Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin);
#endif // Q_OS_WIN


int main(int argc, char *argv[])
{
    if( Hack::WhetherOneInstanceStarted<Hack::OS::Current>() == true ){
        return 0;
    }

    QApplication app(argc, argv);

    BootGlobalEventHook();

    auto screens = qGuiApp->screens();
    for(auto&& screen : screens)
    {
        ColorPickerHost::InitColorPickerForScreen(screen);
    }

    if( ShouldStartInDaemonMode() == true ) {
        DaemonServer::Start();
    } else {
        ColorPickerHost::SetColorPickerVisible();
    }

    return app.exec();
}
