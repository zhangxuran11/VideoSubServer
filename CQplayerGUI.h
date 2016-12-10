#ifndef CQPLAYERGUI_H
#define CQPLAYERGUI_H

#include <QDialog>
#include "CQmedia.h"
#include <iostream>
#include <QString>
#include <ui_CQplayerGUI.h>
#include <QFileDialog>
#include <QMutex>
#include<stackpanel.h>
#include"workthread.h"
#include "versionsender.h"
#include"cqaudio.h"
#include "mytranslator.h"
class GstThread;
class SerialThread;
class CQMedia;
class MAP_Panel;
class CQplayerGUI : public QDialog
{
    Q_OBJECT

    QTimer* switchLanguage;
    enum PlayState{PLAYING,STOP,PAUSED};
public:
    MyTranslator* translator;
    VersionSender* versionCtrl;
    WorkThread* workThread;
    ZTPManager* ztpmForTest;
    QTimer* updateDaemonTimer;
    QTimer* updateTimer;//更新时间标签定时器

    bool isARC;
    int currentVideoChannel;
    bool play(int channel);
    void stop();
    bool playback(int _channel);
    void paused();
private:

    PlayState playState;
    //NetworkThread *networkThread;
    //SerialThread *serial;

public:
    explicit CQplayerGUI(QWidget *parent = 0);
    ~CQplayerGUI();

public:
    Ui::CQplayerGUI* ui;
    CQMedia *media;
    CQAudio* cqAudio;
    StackPanel *stackPanel;

public slots:
    void refresh();
    void updateTime();
    void sendHeart();
    void slot_turnOffHeart();

    void updateDaemon();
    void slot_procTestZtp();
};

#endif // CQPLAYERGUI_H
