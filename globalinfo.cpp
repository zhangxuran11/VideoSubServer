#include "globalinfo.h"
#include"CQmedia.h"
#include "ztools.h"
GlobalInfo* GlobalInfo::_instance = NULL;
GlobalInfo::GlobalInfo(QObject *parent) :
    QObject(parent)
{
    train_id = -1;
    VServerIP = "";
    sysCtlIp = "";
    recipeServerIp = "";

    g_info.addPara("StartStation","--");
    g_info.addPara("EndStation","--");
    g_info.addPara("PreStation","--");
    g_info.addPara("NextStation","--");
    g_info.addPara("CurrentStationEn","--");
    g_info.addPara("StartStation_th","--");
    g_info.addPara("EndStation_th","--");
    g_info.addPara("PreStation_th","--");
    g_info.addPara("NextStation_th","--");
    g_info.addPara("CurrentStationTh","--");
}
GlobalInfo* GlobalInfo::getInstance()
{
    if(_instance == NULL)
        _instance = new GlobalInfo();
    return _instance;
}
