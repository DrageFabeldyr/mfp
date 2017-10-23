#ifndef ARTIST_SCANNER_H
#define ARTIST_SCANNER_H

#include <QThread>

#include "feeds.h"

class artist_scanner : public QThread
{
    Q_OBJECT

public:
    artist_scanner();

public slots:
    void receive_params(QString path, int id, int idFeed, bool is_checked); // принимает параметры из основного потока
    void searching(void);
    void search(QString path);

signals:
    void send(int, int, bool); // будет отправлять в основной поток количество и сигнал об окончании

private:
    Feeds *pFeeds;
    //Type type;
    int id;
    int idFeed;
    bool select;

    QString path;
    bool is_checked;
    int artist_num = 0;
    int file_num = 0;
    bool is_done = false;
    QStringList artists;
};

#endif // ARTIST_SCANNER_H
