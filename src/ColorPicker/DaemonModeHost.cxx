#include "DaemonModeHost.hxx"
#include "ColorPickerHost.hxx"

int32_t WaitForGetFired();

class DaemonThread: public QThread
{
private:
    virtual void run() final override;
};
static DaemonThread* DaemonThread_ = nullptr;

void DaemonThread::run()
{
    connect(DaemonCallback::Instance(), SIGNAL(Fire(int)), \
            ColorPickerHost::Instance(), SLOT(GetFired(int)));

    while(true)
    {
        auto value = WaitForGetFired();
        DaemonCallback::Instance()->Fire(value);
    }
}

bool ShouldStartInDaemonMode()
{
    auto the_check_function = []()->bool{
        // qDebug() << "ARGV: " << qApp->arguments();
        const auto&& arguments = qApp->arguments();

        if( (arguments.size() == 2) && (arguments[1] == QString("--daemon")) ){
            // qDebug() << "Should Start In Daemon Mode";
            return true;
        }
        return false;
    };

    static auto value = the_check_function();
    // qDebug() << "ShouldStartInDaemonMode" << value;
    return value;
}

bool IsInDaemonMode() {
    return ShouldStartInDaemonMode();
}


DaemonServer*
GetDaemonServer()
{
    static DaemonServer daemon_server;
    return &daemon_server;
}


DaemonServer::DaemonServer()
{
    // qDebug() << __CURRENT_FUNCTION_NAME__;
}


DaemonServer::~DaemonServer()
{
    // qDebug() << __CURRENT_FUNCTION_NAME__;
}


void
DaemonServer::listen()
{
    static bool listend = false;
    if(listend == true){
        return;
    }
    listend = true;

    DaemonThread_ = new DaemonThread();
    DaemonThread_->start();
}


void DaemonServer::Start()
{
    auto server = GetDaemonServer();
    server->listen();
}


DaemonCallback::DaemonCallback()
{
}


DaemonCallback::~DaemonCallback()
{
}

DaemonCallback*
DaemonCallback::Instance()
{
    static DaemonCallback daemon_callback;
    return &daemon_callback;
}

