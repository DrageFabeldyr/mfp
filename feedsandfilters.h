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
#include "feedmodel.h"
#include "filtermodel.h"
#include "settings.h"
#include "editwindow.h"


class FeedsAndFilters : public QWidget
{
    Q_OBJECT
public:
    FeedsAndFilters(QWidget *parent);
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
    void filters_check_all();
    void filters_uncheck_all();
    void filters_delete_all();

private:
    FeedModel  *feedModel;
    FilterModel *filterModel;
    QSqlQueryModel  *model2;
    QTreeView  *feedList;
    QTreeView  *filterList;
    QVBoxLayout *v_layout;
    QVBoxLayout *vb_layout;     // объединение рядов кнопок фильтров
    QHBoxLayout *h1_layout;
    QPushButton * feedAdd;
    QPushButton * feedDel;
    QPushButton * feedEdit;
    QHBoxLayout *h2_layout;     // верхний ряд кнопок фильтров
    QHBoxLayout *h3_layout;     // нижний ряд кнопок фильтров
    QPushButton * filterAdd;
    QPushButton * filterDel;
    QPushButton * filterEdit;
    QPushButton * filterCheckAll;
    QPushButton * filterUncheckAll;
    QPushButton * filterDeleteAll;
    EditWindow *editW;
    QLabel *hint1;
    QLabel *hint2;

private slots:
    void closeEvent(QCloseEvent * event) Q_DECL_OVERRIDE;
    void showEvent(QShowEvent * event) Q_DECL_OVERRIDE;
};

#endif // FEEDSANDFILTERS_H
