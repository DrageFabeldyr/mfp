#ifndef FEED_H
#define FEED_H

#include <QList>

#include <filters_struct.h>


class Feed
{
public:
    Feed();
    Feed(int id);
    QString title;
    QString link;
    bool is_on;
    int id;
    QList <filters_struct*> filters;
    void Clear();
};

#endif // FEED_H
