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
    ZTPManager* recv_car_id_map_ztpm;
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
    void slot_recv_car_id_map();
};

#endif // WORKTHREAD_H
