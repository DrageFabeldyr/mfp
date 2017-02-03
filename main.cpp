#include "dfrssfilter.h"
#include <QApplication>

#include "settings.h"

QString prog_name_ver;


/* Создаёт приложение и главный виджет. Открывает главный виджет для
 * ввода пользователя и выходит с подходящим возвращаемым значением когда он закрывается */
int main(int argc, char **argv)
{
    prog_name_ver = "Version: 0.9";
    QApplication app(argc, argv);
    DFRSSFilter *dfrssfilter = new DFRSSFilter;

    if (!dfrssfilter->settings->run_in_tray->isChecked()) // если нет галочки "запускать свёрнутым в трей"
        dfrssfilter->show(); // рисуем

    return app.exec();
}
