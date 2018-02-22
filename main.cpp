#include "dfrssfilter.h"
#include <QApplication>
#include <QtWidgets>
//#include "settings.h"

QString prog_name_ver;


/* Создаёт приложение и главный виджет. Открывает главный виджет для
 * ввода пользователя и выходит с подходящим возвращаемым значением когда он закрывается */
int main(int argc, char **argv)
{
    prog_name_ver = "1.0.1";
    QApplication app(argc, argv);
/*
    QCommandLineParser commandLineParser;
    const QCommandLineOption disableTapAndHoldOption("no-TapAndHold", "Disable TapAndHold gesture");
    commandLineParser.addOption(disableTapAndHoldOption);
*/
    QList<Qt::GestureType> gestures;
  //  if (!commandLineParser.isSet(disableTapAndHoldOption))
        gestures << Qt::TapAndHoldGesture;

    DFRSSFilter *dfrssfilter = new DFRSSFilter;
    dfrssfilter->grabGestures(gestures);

    if (!dfrssfilter->settings->run_in_tray->isChecked()) // если нет галочки "запускать свёрнутым в трей"
        dfrssfilter->show(); // рисуем

    return app.exec();
}
