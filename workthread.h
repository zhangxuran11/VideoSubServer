#ifndef WORKTHREAD_H
#define WORKTHREAD_H
#include <QThread>
#include<QTimer>
class ZTPManager;
class WorkThread : public QThread
{
    Q_OBJECT
    ZTPManager *broadCastManager;
    QTimer* recvCarrierHeartTimer;
public:
    ZTPManager* ztpmCarrierHeart;
    explicit WorkThread(QObject *parent = 0);
    ~WorkThread(){}
    bool isCJRCar();
private slots:

    void run();
    void recvBroadCast();
    void tempTimeOutProc();
    void OnRecvCarrierHeart();
    void recvCarrierHeartTimeout();
    void OnRecvCJRCar();

    void setCJRCar(bool set);
};

#endif // WORKTHREAD_H
