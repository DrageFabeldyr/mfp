#ifndef FEEDS_H
#define FEEDS_H

//#include <QtSql>
#include <QApplication>
#include <QDir>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QSqlError>
#include <QSqlQuery>
#include <QMessageBox>


#include <feed.h>


class Feeds
{
public:
    Feeds();
    //~Feeds();
    QSqlDatabase db;

    void GetFeeds(QList<Feed> &feed);
    void GetFilters(QList<filters_struct> &filters, const Feed *feed);
    void GetFeedById(Feed * feed);
    void GetFilterById(filters_struct * filter);
    void SaveFeed(Feed * feed);
    void SaveFilter(filters_struct * filter);
    void DeleteFeedById(int id);
    void DeleteFilterById(int id);

    void GetActiveFeedsList(QList<Feed> &feeds);
    void GetActiveFiltersList(QList<filters_struct> &filters, const int id);

    void ChangeStatusFilter(const int id, const int status);
    void ChangeStatusFeed(const int id, const int status);


private:
    void OpenDB();
    void AddFeed(Feed * feed);
    void UpdateFeed(Feed * feed);
    void AddFilter(filters_struct * filter);
    void UpdateFilter(filters_struct * filter);



};

#endif // FEEDS_H
