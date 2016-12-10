#define __MAIN__
#include <QtGui/QApplication>
#include "CQplayerGUI.h"
#include "config.h"
#include<QDebug>
#include<QFile>
#include"crashtool.h"
static void start_daemon();
int main(int argc, char *argv[])
{
//    EnableCrashHandler();
    QApplication a(argc, argv);
    QApplication::setOverrideCursor(Qt::BlankCursor);

    QFile file(":qss/text_style.qss");
    file.open(QFile::ReadOnly);
    a.setStyleSheet(file.readAll());
    start_daemon();
    file.close();
    CQplayerGUI w;
    w.show();

    return a.exec();
}
static void start_daemon()
{
    system("ps | grep [s]ub-daemon | awk '{print $1}'|  xargs kill -9");
    QFile::copy(":/sub-daemon","/tmp/sub-daemon");
    system("chmod +x /tmp/sub-daemon");
    system("/tmp/sub-daemon   &");
    system("rm /tmp/sub-daemon");
}
