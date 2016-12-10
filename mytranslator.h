#ifndef MYTRANSLATOR_H
#define MYTRANSLATOR_H
#include<QSettings>
#include<QDebug>
class MyTranslator
{
    QSettings *configIniRead;
public:
    QString curLan;
    void setLan(QString lan){curLan = lan;}
    MyTranslator(QString iniFile);
    QString tr(QString str){ return configIniRead->value("/"+str+"/"+curLan).toString();}
};

#endif // MYTRANSLATOR_H
