#ifndef GLOBALINFO_H
#define GLOBALINFO_H

#include <QObject>
#include"ztpmanager.h"
class CQMedia;
class CQplayerGUI;
class GlobalInfo : public QObject
{
    Q_OBJECT
    static GlobalInfo* _instance;
    explicit GlobalInfo(QObject *parent = 0);
public:
    static GlobalInfo* getInstance();
    int carId;
    int train_id;
    ZTPprotocol g_info;
    QString VServerIP;
    QString sysCtlIp;
    QString recipeServerIp;
//    QString sysControllerIP;
//    int sysControllerPort;
//    int carNu;//车厢号  -1表示车厢好获取异常
    CQplayerGUI* player;
    int currentChannel;//--多媒体视频（最后8频道对应多媒体1-8频道）
    
signals:
    
public slots:
    
};

#endif // GLOBALINFO_H
