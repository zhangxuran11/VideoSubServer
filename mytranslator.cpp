#include "mytranslator.h"

MyTranslator::MyTranslator(QString iniFile)
{
    curLan = "En";
    configIniRead = new QSettings(iniFile, QSettings::IniFormat);
    configIniRead->setIniCodec("UTF-8");
}
