#include "dfrssfilter.h"
#include <QApplication>

#include "settings.h"

QString prog_name_ver;
//extern settings *sett; // чтобы взять тот, что уже определён в dfrssfilter.cpp


/* Создаёт приложение и главный виджет. Открывает главный виджет для
 * ввода пользователя и выходит с подходящим возвращаемым значением когда он закрывается */
int main(int argc, char **argv)
{
    prog_name_ver = "Version: 0.5";
    QApplication app(argc, argv);
    DFRSSFilter *dfrssfilter = new DFRSSFilter;

    if (!dfrssfilter->sett->run_in_tray->isChecked()) // если нет галочки "запускать свёрнутым в трей"
        dfrssfilter->show(); // рисуем

    return app.exec();
}
