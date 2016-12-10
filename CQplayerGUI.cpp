#include "CQplayerGUI.h"
#include "ui_CQplayerGUI.h"
#include "config.h"
#include "unistd.h"
#include<QDebug>
#include <iostream>
#include <QTimer>
#include <QFile>
#include<QtCore>
#include <QtGui/QApplication>
#include<QProcess>
#include"globalinfo.h"
#include"map_panel.h"
#include"workthread.h"
#include"zutility.h"
#include"ztools.h"

using namespace std;
void CQplayerGUI::stop()
{
    media->stop();
    stackPanel->hide();
    playState = STOP;
}

bool CQplayerGUI::play(int _channel)
{
    if(_channel == 0)
    {
        qDebug()<<"Plag GPS.";
        stackPanel->show();
    }
    else
    {
        QString videoUrl = ZUtility::getVideoUrl(_channel);
        if(videoUrl == "")
        {
            qDebug("Get video url is null");
            return false;
        }
        if(!media->play(videoUrl))
            return false;
    }
    playState = PLAYING;
    currentVideoChannel = _channel;
    return true;
}

static bool heartSwitch = true;
void CQplayerGUI::sendHeart()
{
    static ZTPManager* ztpm = new ZTPManager;
    static int carId = ZTools::getCarID();
    if(!heartSwitch)
        return;

    ZTPprotocol ztp;
    ztp.addPara("T","Heart");
    ztp.addPara("CarID",QString::number(carId));
    ztpm->SendOneZtp(ztp,QHostAddress("224.102.228.41"),6600);
    qDebug("Video send >>>");
    ztp.print();
    qDebug("Video send <<<");
}
void CQplayerGUI::slot_turnOffHeart()
{
    ZTPprotocol ztp;
    ZTPManager* ztpm = (ZTPManager*)sender();
    ztpm->getOneZtp(ztp);
    ztp.print();
    if(ztp.getPara("T") == "debug" && ztp.getPara("heartSwitch") == "off")
        heartSwitch = false;
    if(ztp.getPara("T") == "debug" && ztp.getPara("heartSwitch") == "on")
        heartSwitch = true;
}
CQplayerGUI::CQplayerGUI(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CQplayerGUI)
{
    translator = new MyTranslator(":/language_thai.ini");
    switchLanguage = new QTimer;
    switchLanguage->setSingleShot(false);
    connect(switchLanguage,SIGNAL(timeout()),this,SLOT(refresh()));
    switchLanguage->setInterval(10000);
    switchLanguage->start();
    ui->setupUi(this);
    isARC = false;//初始化不是餐车
    if(workThread->isCJRCar())
    {
        ui->wcStateLabel1->setGeometry(ui->wcStateLabel2->x(),630,ui->wcStateLabel1->width(),ui->wcStateLabel1->width());
        ui->wcStateLabel1->setStyleSheet(QString::fromUtf8("border-image: url(:/images/dabian_out.png);"));
        ui->wcStateLabel2->setGeometry(ui->wcStateLabel3->x(),630,ui->wcStateLabel2->width(),ui->wcStateLabel2->width());
        ui->wcStateLabel2->setStyleSheet(QString::fromUtf8("border-image: url(:/images/dabian_out.png);"));
        ui->wcStateLabel3->setGeometry(1400,630,ui->wcStateLabel2->width(),ui->wcStateLabel2->width());

    }
    QString ip = ZTools::getIP();
    if(ip != "")
    {
        QString ipARC = "192.168." + ip.split(QChar('.'))[2] + ".50";
        QProcess *p = new QProcess;
        if(!QFile::exists("/ping_test.sh"))
        {
            QFile::copy(":/ping_test.sh","/ping_test.sh");
            system("chmod +x /ping_test.sh");
        }
        p->execute("/ping_test.sh",QStringList()<<ipARC);
        delete p;
        QFile f("/tmp/isARC");
        f.open(QIODevice::ReadOnly);
        QString str = f.readAll();
        str = str.trimmed();
        f.close();
        QFile::remove("/tmp/isARC");
        if(str == "yes")
        {
            isARC = true;
        }
    }
    if(isARC)
    {
        ui->wcStateLabel1->setStyleSheet(QString::fromUtf8(""));
        ui->wcStateLabel2->setStyleSheet(QString::fromUtf8(""));
    }
    versionCtrl = new VersionSender("VideoSubServer",1,1,12,ZTools::getCarID());
    ztpmForTest = new ZTPManager(8319,QHostAddress("224.102.228.40"));
    connect(ztpmForTest,SIGNAL(readyRead()),this,SLOT(slot_procTestZtp()));



    ZTPManager* ztpm = new ZTPManager(6633,QHostAddress("224.102.228.41"));
    connect(ztpm,SIGNAL(readyRead()),this,SLOT(slot_turnOffHeart()));
    QTimer* timer = new QTimer();
    timer->setSingleShot(false);
    timer->setInterval(3000);
    timer->start();
    connect(timer,SIGNAL(timeout()),this,SLOT(sendHeart()));

    char buf[10];
    sprintf(buf,"%02d",ZTools::getCarID());
    ui->carIDLabel->setText(buf);
    stackPanel = new StackPanel(translator,ui->frameMovie);
    stackPanel->hide();

    setWindowFlags(Qt::FramelessWindowHint|(windowFlags() & (~Qt::WindowCloseButtonHint)));
    cqAudio = new CQAudio(ui->frameMovie,stackPanel,0);
    media = new CQMedia(ui->frameMovie->winId(),0,CQMedia::SUBVIDEO);
    media->setVolume(90);

    workThread = NULL;
    workThread = new WorkThread();
    GlobalInfo::getInstance()->player = this;

    updateTimer = new QTimer;
    updateTimer->setInterval(1000);
    updateTimer->setSingleShot(false);
    connect(updateTimer,SIGNAL(timeout()),this,SLOT(updateTime()));
    updateTimer->start();

    updateDaemonTimer = new QTimer;
//    updateDaemonTimer->setSingleShot(false);
//    connect(updateDaemonTimer,SIGNAL(timeout()),this,SLOT(updateDaemon()));
//    updateDaemonTimer->setInterval(30000);
//    updateDaemonTimer->start();

    playState = STOP;
}



void CQplayerGUI::updateTime()
{
    ui->dateTimeLabel->setText(QDateTime::currentDateTime().toString("20yy-MM-dd          hh:mm:ss"));
    //ZTools::singleShot(1000,updateTime);
}
CQplayerGUI::~CQplayerGUI()
{
    delete ui;
    gst_deinit();
    delete media;
}


void CQplayerGUI::updateDaemon()
{
    system("ps | grep [s]ub-daemon | awk '{print $1}'|  xargs kill -9");
//    QFile::copy(":/sub-daemon","/tmp/sub-daemon");
//    system("chmod +x /tmp/sub-daemon");
    system("/tmp/sub-daemon   &");
//    system("rm /tmp/sub-daemon");
}

void CQplayerGUI::slot_procTestZtp()
{
    ZTPprotocol ztp;
    ztpmForTest->getOneZtp(ztp);
    if(ztp.getPara("T") == "TEST" && ztp.getPara("CARID").toInt() == GlobalInfo::getInstance()->carId &&  ztp.getPara("DEV") == "SUB"
            &&  ztp.getPara("CMD") == "EXIT_WORKTHREAD")
    {
        versionCtrl->setTestOffline();
    }
}

void CQplayerGUI::refresh()
{

    if(translator->curLan == "En")
    {
        translator->curLan = "Th";
    }
    else
    {
        translator->curLan = "En";
    }

    ui->_speedLabel->setText(translator->tr("Speed"));
    ui->_speedUnitLabel->setText(translator->tr("km/h"));
    ui->_startStaionLabel->setText(translator->tr("Origin station"));
    ui->_endStationLabel->setText(translator->tr("Destination station"));
    ui->_preStationLabel->setText(translator->tr("Previous station"));
    ui->_nextStationLabel->setText(translator->tr("Next station"));
    ui->_carIDLabel->setText(translator->tr("CAR NO"));
    ui->_trainIDLabel->setText(translator->tr("Train No"));
    ui->_curStationLabel->setText(translator->tr("This Station"));
    ui->_arriveTimeLabel->setText(translator->tr("Destination station time"));
    ui->_nextStationTimeLabel->setText(translator->tr("Next station time"));
    ui->_TempLabel->setText(translator->tr("TEMPERATURE"));
    ui->_outTempLabel->setText(translator->tr("Outside T"));
    ui->_innerTempLabel->setText(translator->tr("Room T"));
    if(translator->curLan == "En") {
        ui->startStaionLabel->setText(GlobalInfo::getInstance()->g_info.getPara("StartStation"));
        ui->endStationLabel->setText(GlobalInfo::getInstance()->g_info.getPara("EndStation"));
        ui->preStationLabel->setText(GlobalInfo::getInstance()->g_info.getPara("PreStation"));
        ui->nextStationLabel->setText(GlobalInfo::getInstance()->g_info.getPara("NextStation"));
        ui->curStationLabel->setText(GlobalInfo::getInstance()->g_info.getPara("CurrentStationEn"));
    }
    else{
        ui->startStaionLabel->setText(GlobalInfo::getInstance()->g_info.getPara("StartStation_th"));
        ui->endStationLabel->setText(GlobalInfo::getInstance()->g_info.getPara("EndStation_th"));
        ui->preStationLabel->setText(GlobalInfo::getInstance()->g_info.getPara("PreStation_th"));
        ui->nextStationLabel->setText(GlobalInfo::getInstance()->g_info.getPara("NextStation_th"));
        ui->curStationLabel->setText(GlobalInfo::getInstance()->g_info.getPara("CurrentStationTh"));
    }
    stackPanel->updateLan();

}
