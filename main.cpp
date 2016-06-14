#include "dfrssfilter.h"
#include <QApplication>

QString prog_name_ver;

/*
 * Создаёт приложение и главный виджет. Открывает главный виджет для
 * ввода пользователя и выходит с подходящим возвращаемым значением когда он
 * закрывается
*/

int main(int argc, char **argv)
{
    prog_name_ver = "Version: 0.5";
    QApplication app(argc, argv);
    DFRSSFilter *dfrssfilter = new DFRSSFilter;
    dfrssfilter->show();
    return app.exec();
}
