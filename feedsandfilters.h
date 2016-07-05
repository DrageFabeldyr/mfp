#ifndef FEEDSANDFILTERS_H
#define FEEDSANDFILTERS_H

#include <QObject>
#include <QWidget>
#include <QSqlQueryModel>
#include <QTreeView>
#include <QTableView>
#include <QVBoxLayout>
#include <QPushButton>

#include "dfrssfilter.h"
/*
#include "feeds.h"
*/
#include "feedmodel.h"
#include "filtermodel.h"
#include "settings.h"
#include "editwindow.h"


class FeedsAndFilters : public QWidget
{
    Q_OBJECT
public:
    FeedsAndFilters(QWidget * parent);
    Feeds *pFeeds;
    int parentFeedId;
    Settings *settings = nullptr;


public slots:
    void UpdateFilter(QModelIndex index);
    void ShowEditFeed(QModelIndex index);
    void ShowEditFilter(QModelIndex index);
    void ShowAddFeed();
    void ShowAddFilter();
    void updateFilters();
    void updateFeeds();
    void ShowEditFilter();
    void ShowEditFeed();
    void FilterDel();
    void FeedDel();

private:
    FeedModel  *feedModel;
    FilterModel *filterModel;
    QSqlQueryModel  *model2;
    QTreeView  *feedList;
    QTreeView  *filterList;
    QVBoxLayout *v_layout;
    QHBoxLayout *h1_layout;
    QPushButton * feedAdd;
    QPushButton * feedDel;
    QPushButton * feedEdit;
    QHBoxLayout *h2_layout;
    QPushButton * filterAdd;
    QPushButton * filterDel;
    QPushButton * filterEdit;
    EditWindow *editW;
    QLabel *hint1;
    QLabel *hint2;
};

#endif // FEEDSANDFILTERS_H
