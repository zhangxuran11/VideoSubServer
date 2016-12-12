#include "workthread.h"
#include "config.h"
#include"CQmedia.h"
#include "globalinfo.h"
#include"ztpmanager.h"
#include "ztpprotocol.h"
#include"ztools.h"
#include"config.h"
#include "QtCore"
#include"CQplayerGUI.h"
#include "zutility.h"
#include"math.h"
#include"globalinfo.h"
WorkThread::WorkThread(QObject *parent) :
    QThread(parent)
{
    start();
}
static void proBroadSignal(bool broadSignal)
{
    CQplayerGUI* player = GlobalInfo::getInstance()->player;
    if(broadSignal && !player->media->isSilence())
        player->media->setSilence(true);
    else if(!broadSignal && player->media->isSilence())
        player->media->setSilence(false);
}
static bool switch_channel_f = false;
static void opt_switch_channel_f()
{
    switch_channel_f = false;
}
static void procMediaChannel(const ZTPprotocol& ztp,int _channel)
{
    CQplayerGUI* player = GlobalInfo::getInstance()->player;
    QString ch = "Ch" + ztp.getPara("ChId");
    qDebug()<<QString("recv MediaChannel %1.").arg(_channel);
    if(_channel == player->currentVideoChannel && player->media->getPlayingState() != CQMedia::STOPPED && ztp.getPara(ch) != "paused")
    {
        qDebug()<<"This channel is playing.";
        return;
    }

    if(switch_channel_f)
        return;
    GlobalInfo::getInstance()->player->stop();

    if(ztp.getPara("ChId") != "0" && ztp.getPara(ch) != "play"){
        qDebug()<<"Current ChId is not play...";
        return;
    }
    if(ztp.getPara("ChId") == "0" && GlobalInfo::getInstance()->player->stackPanel->s_off)
        return;
    GlobalInfo::getInstance()->player->play(_channel);
    switch_channel_f = true;
    ZTools::singleShot(3000,opt_switch_channel_f);

}
static QTimer* carTempTimer = new QTimer;//车厢温度
static QTimer* outTempTimer = new QTimer;//车外温度
void WorkThread::recvBroadCast()
{
    ZTPprotocol ztp;
//    broadCastManager->getOneZtp(ztp);
    ZTPManager* ztpm = dynamic_cast<ZTPManager*>(sender());
    if(ztpm == NULL)
        return;

    ztpm->getOneZtp(ztp);
    if(ztp.getPara("T")=="T_ChId")
    {
//        ztp.print();
        if(ztp.getPara("ChId").toInt() ==  -1 ) {
            qDebug()<<"current ChId is invalid.";
            return;
        }
        procMediaChannel(ztp,ztp.getPara("ChId").toInt());
    }
    else if(ztp.getPara("T") == "G_INFO")
    {
        CQplayerGUI* gui = GlobalInfo::getInstance()->player;
        gui->ui->trainIDLabel->setText(ztp.getPara("TrainNum"));
        double f = ztp.getPara("Speed").toDouble();
        char buf[10];
        sprintf(buf,"%d",(int)round(f));
        gui->ui->speedLabel->setText(QString(buf));
        gui->ui->arriveTimeLabel->setText(ztp.getPara("ArriveTime"));
        gui->ui->nextStationTimeLabel->setText(ztp.getPara("NextStationTime"));

        gui->stackPanel->setSecsOff(ztp.getPara("SECS_OFF").toInt());

        GlobalInfo::getInstance()->g_info.addPara("StartStation",ztp.getPara("StartStation"));
        GlobalInfo::getInstance()->g_info.addPara("EndStation",ztp.getPara("EndStation"));
        GlobalInfo::getInstance()->g_info.addPara("PreStation",ztp.getPara("PreStation"));
        GlobalInfo::getInstance()->g_info.addPara("NextStation",ztp.getPara("NextStation"));
        GlobalInfo::getInstance()->g_info.addPara("CurrentStationEn",ztp.getPara("CurrentStationEn"));
        GlobalInfo::getInstance()->g_info.addPara("StartStation_th",ztp.getPara("StartStation_th"));
        GlobalInfo::getInstance()->g_info.addPara("EndStation_th",ztp.getPara("EndStation_th"));
        GlobalInfo::getInstance()->g_info.addPara("PreStation_th",ztp.getPara("PreStation_th"));
        GlobalInfo::getInstance()->g_info.addPara("NextStation_th",ztp.getPara("NextStation_th"));
        GlobalInfo::getInstance()->g_info.addPara("CurrentStationTh",ztp.getPara("CurrentStationTh"));
        if(gui->translator->curLan== "En") {
            gui->ui->startStaionLabel->setText(ztp.getPara("StartStation"));
            gui->ui->endStationLabel->setText(ztp.getPara("EndStation"));
            gui->ui->preStationLabel->setText(ztp.getPara("PreStation"));
            gui->ui->nextStationLabel->setText(ztp.getPara("NextStation"));
            gui->ui->curStationLabel->setText(ztp.getPara("CurrentStationEn"));

        }
        else{
            gui->ui->startStaionLabel->setText(ztp.getPara("StartStation_th"));
            gui->ui->endStationLabel->setText(ztp.getPara("EndStation_th"));
            gui->ui->preStationLabel->setText(ztp.getPara("PreStation_th"));
            gui->ui->nextStationLabel->setText(ztp.getPara("NextStation_th"));
            gui->ui->curStationLabel->setText(ztp.getPara("CurrentStationTh"));
        }

        int train_id = ztp.getPara("TrainNum").toInt();
        if(GlobalInfo::getInstance()->train_id != train_id)
        {
            GlobalInfo::getInstance()->train_id = train_id;
            gui->stackPanel->setTrainId(train_id);
            gui->stackPanel->loadTimeTable(train_id);
            gui->stackPanel->OnUpdateMap();
        }
    }
    else if(ztp.getPara("T") == "OutTemperature")
    {
        outTempTimer->start(5000);
        CQplayerGUI* gui = GlobalInfo::getInstance()->player;
        int d = ztp.getPara("VALUE").toFloat();
        char buf[10];
        sprintf(buf,"%02d",d);
        gui->ui->outTempLabel->setText(buf);
    }
    else if(ztp.getPara("T") == "CarTemperature" && ztp.getPara("CAR_ID").toInt() == ZTools::getCarID())
    {

        carTempTimer->start(5000);
//        outTempTimer->start(5000);
        CQplayerGUI* gui = GlobalInfo::getInstance()->player;
        int d = ztp.getPara("VALUE").toFloat();
        char buf[10];
        sprintf(buf,"%02d",d);
        gui->ui->innerTempLabel->setText(buf);
        //外温
//        d = ztp.getPara("OUT VALUE").toFloat();
//        memset(buf,0,10);
//        sprintf(buf,"%02d",d);
//        gui->ui->outTempLabel->setText(buf);
    }
    else if(ztp.getPara("T") == "CS_STATE" && ztp.getPara("CAR_ID").toInt() == ZTools::getCarID())
    {
        CQplayerGUI* gui = GlobalInfo::getInstance()->player;
        static QString pre_xiaobian = "--",pre_dabian = "--",pre_dabian2 = "--";
        QString cur_xiaobian = ztp.getPara("CS1");
        QString cur_dabian = ztp.getPara("CS2");
        QString cur_dabian2 = ztp.getPara("CS3");
        if(isCJRCar())
        {
            //cur_xiaobian->CS1->dabian
            //cur_dabian->CS2->dabian2
            if(pre_xiaobian != cur_xiaobian)
            {
                if(cur_xiaobian == "Y")
                {
                    gui->ui->wcStateLabel1->setStyleSheet(QString::fromUtf8("border-image: url(:/images/dabian_out.png);"));
                }
                else
                {
                    gui->ui->wcStateLabel1->setStyleSheet(QString::fromUtf8("border-image: url(:/images/dabian_in.png);"));
                }
            }
            if(pre_dabian != cur_dabian)
            {
                if(cur_dabian == "Y")
                {
                    gui->ui->wcStateLabel2->setStyleSheet(QString::fromUtf8("border-image: url(:/images/dabian_out.png);"));
                }
                else
                {
                    gui->ui->wcStateLabel2->setStyleSheet(QString::fromUtf8("border-image: url(:/images/dabian_in.png);"));
                }
            }

        }
        else if(gui->isARC)
        {
            gui->ui->wcStateLabel1->setStyleSheet(QString::fromUtf8(""));
            gui->ui->wcStateLabel2->setStyleSheet(QString::fromUtf8(""));
            gui->ui->wcStateLabel3->setStyleSheet(QString::fromUtf8(""));
        }
        else
        {
            if(pre_xiaobian != cur_xiaobian)
            {
                if(cur_xiaobian == "Y")
                {
                    gui->ui->wcStateLabel1->setStyleSheet(QString::fromUtf8("border-image: url(:/images/xiaobian_out.png);"));
                }
                else
                {
                    gui->ui->wcStateLabel1->setStyleSheet(QString::fromUtf8("border-image: url(:/images/xiaobian_in.png);"));
                }
            }
            if(pre_dabian != cur_dabian)
            {
                if(cur_dabian == "Y")
                {
                    gui->ui->wcStateLabel2->setStyleSheet(QString::fromUtf8("border-image: url(:/images/dabian_out.png);"));
                }
                else
                {
                    gui->ui->wcStateLabel2->setStyleSheet(QString::fromUtf8("border-image: url(:/images/dabian_in.png);"));
                }
            }
            if(pre_dabian2 != cur_dabian2)
            {
                if(cur_dabian2 == "Y")
                {
                    gui->ui->wcStateLabel3->setStyleSheet(QString::fromUtf8("border-image: url(:/images/dabian_out.png);"));
                }
                else
                {
                    gui->ui->wcStateLabel3->setStyleSheet(QString::fromUtf8("border-image: url(:/images/dabian_in.png);"));
                }
            }
       }
        pre_dabian2 = cur_dabian2;
        pre_xiaobian = cur_xiaobian;
        pre_dabian = cur_dabian;

    }
    else if(ztp.getPara("T") == "IPaddress1")
    {
        if(GlobalInfo::getInstance()->VServerIP != ztp.getPara("mainvideoserver"))
            GlobalInfo::getInstance()->VServerIP = ztp.getPara("mainvideoserver");
        if(GlobalInfo::getInstance()->sysCtlIp != ztp.getPara("syscontroller"))
            GlobalInfo::getInstance()->sysCtlIp = ztp.getPara("syscontroller");
        if(GlobalInfo::getInstance()->recipeServerIp != ztp.getPara("recipeserver"))
        {
            GlobalInfo::getInstance()->recipeServerIp = ztp.getPara("recipeserver");
            QString ip = ZTools::getIP();
            if(ip != "")
            {
                QString ipARC = "192.168." + ip.split(QChar('.'))[2] + ".50";
                if(ipARC == GlobalInfo::getInstance()->recipeServerIp)
                    GlobalInfo::getInstance()->player->isARC = true;
            }
        }

        if(GlobalInfo::getInstance()->VServerIP != NULL &&
            GlobalInfo::getInstance()->sysCtlIp != NULL &&
            GlobalInfo::getInstance()->recipeServerIp != NULL)
            delete ztpm;
    }
    else if(ztp.getPara("T") == "BroadCast")//PORT : 8310
    {
        bool broadSignal = false;
        if(ztp.getPara("HasBroadcast") == "Y" ||
                (ztp.getPara("HasBroadcast_lc") == "Y" &&
                 ztp.getPara("CarId").toInt() == GlobalInfo::getInstance()->carId ))
        {
           broadSignal = true;
        }
        proBroadSignal(broadSignal);
    }

}

void WorkThread::tempTimeOutProc()
{
    QTimer* timer = dynamic_cast<QTimer*>(sender());
    if(timer == carTempTimer)
    {
        CQplayerGUI* gui = GlobalInfo::getInstance()->player;
        gui->ui->innerTempLabel->setText("00");
    }
    else if(timer == outTempTimer)
    {
        CQplayerGUI* gui = GlobalInfo::getInstance()->player;
        gui->ui->outTempLabel->setText("00");
    }
}

void WorkThread::run()
{
    //ZTools::singleShot(1000,updateTime);
    recvCarrierHeartTimer = new QTimer;
    recvCarrierHeartTimer->setInterval(10000);
    recvCarrierHeartTimer->setSingleShot(false);
    connect(recvCarrierHeartTimer,SIGNAL(timeout()),this,SLOT(recvCarrierHeartTimeout()));
    recvCarrierHeartTimer->start();

    ztpmCarrierHeart = new ZTPManager(8317,QHostAddress("224.102.228.40"));
    connect(ztpmCarrierHeart,SIGNAL(readyRead()),this,SLOT(OnRecvCarrierHeart()));

    carTempTimer->start(5000);
    outTempTimer->start(5000);
    connect(carTempTimer,SIGNAL(timeout()),this,SLOT(tempTimeOutProc()));
    connect(outTempTimer,SIGNAL(timeout()),this,SLOT(tempTimeOutProc()));

    broadCastManager = new ZTPManager(BROADCAST_PORT,QHostAddress(BROADCAST_IP));
    connect(broadCastManager,SIGNAL(readyRead()),this,SLOT(recvBroadCast()));
    broadCastManager = new ZTPManager;//专用于发送
    ZTPManager* ztpm = new ZTPManager(3321,QHostAddress(BROADCAST_IP));//厕所状态
    connect(ztpm,SIGNAL(readyRead()),this,SLOT(recvBroadCast()));
    ztpm = new ZTPManager(8312,QHostAddress("202,102,228,40"));//车厢温度
    connect(ztpm,SIGNAL(readyRead()),this,SLOT(recvBroadCast()));
    ztpm = new ZTPManager(8313,QHostAddress("202,102,228,40"));//车外温度
    connect(ztpm,SIGNAL(readyRead()),this,SLOT(recvBroadCast()));
     ztpm = new ZTPManager(8315,QHostAddress(BROADCAST_IP));//多媒体频道广播
    connect(ztpm,SIGNAL(readyRead()),this,SLOT(recvBroadCast()));//主线曾调槽
    ZTPManager* g_infoZtpm = new ZTPManager(8311,QHostAddress(BROADCAST_IP));
    connect(g_infoZtpm,SIGNAL(readyRead()),this,SLOT(recvBroadCast()));//由于槽中操作了ui，故必须放入主线程
    exec();
}

void WorkThread::OnRecvCarrierHeart()
{
    ZTPprotocol ztp;
    ztpmCarrierHeart->getOneZtp(ztp);
    if(ztp.getPara("T") != "CarrierHeart")
        return;
    GlobalInfo::getInstance()->player->versionCtrl->setOnline(true);
    recvCarrierHeartTimer->start();
    if(!QFile::exists("/tmp/DevExist"))
        system("touch /tmp/DevExist");
}
void WorkThread::recvCarrierHeartTimeout()
{
    GlobalInfo::getInstance()->player->versionCtrl->setOnline(false);
}

bool WorkThread::isCJRCar()
{
    return (ZTools::getCarGlobalID() >= 89 && ZTools::getCarGlobalID()<=97 ? true:false);
}
