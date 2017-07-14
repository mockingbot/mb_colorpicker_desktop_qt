#pragma once

#include <QtCore/QtCore>

bool ShouldStartInDaemonMode();
bool IsInDaemonMode();

class DaemonServer
{
    friend DaemonServer* GetDaemonServer();
private:
    DaemonServer();
    ~DaemonServer();
public:
    static void Start();
private:
    void listen();
};

class DaemonCallback: public QObject
{
    Q_OBJECT
public:
    static DaemonCallback* Instance();
private:
    DaemonCallback();
    ~DaemonCallback();
Q_SIGNALS:
    void Fire(int value);
};
