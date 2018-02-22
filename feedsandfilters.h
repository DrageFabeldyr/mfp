#ifndef FEEDSANDFILTERS_H
#define FEEDSANDFILTERS_H

#include <QObject>
#include <QWidget>
#include <QSqlQueryModel>
#include <QTreeView>
#include <QTableView>
#include <QVBoxLayout>
#include <QPushButton>

//#include "dfrssfilter.h"
#include "feedmodel.h"
#include "filtermodel.h"
#include "settings.h"
#include "editwindow.h"
#include <QMenu>

 // для пальценажатия
#ifdef Q_OS_ANDROID

#include <QtWidgets>

QT_BEGIN_NAMESPACE
class QGestureEvent;
class QTapAndHoldGesture;
QT_END_NAMESPACE

#endif

class FeedsAndFilters : public QWidget
{
    Q_OBJECT

public:
    explicit FeedsAndFilters(QWidget *parent = 0);
    ~FeedsAndFilters();
    Feeds *pFeeds;
    int parentFeedId;
    Settings *settings = nullptr;

    void grabGestures(const QList<Qt::GestureType> &gestures);

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

    void filters_change_status_all(int status);
    void filters_delete_all();
    void filters_check_all();
    void filters_uncheck_all();

private:
    FeedModel  *feedModel;
    FilterModel *filterModel;
    QSqlQueryModel  *model2;
    QTreeView  *feedList;
    QTreeView  *filterList;
    QVBoxLayout *v_layout;
    /*
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
    */
    EditWindow *editW;
    QLabel *hint1;
    QLabel *hint2;


    QMenu *feeds_menu = nullptr;    // обнулим указатель, чтобы программа не валилась при проверке на существование
    QMenu *filters_menu = nullptr;    // обнулим указатель, чтобы программа не валилась при проверке на существование
    QAction *feeds_menu_add;
    QAction *feeds_menu_edit;
    QAction *feeds_menu_del;
    QAction *feeds_menu_add_filters;
    QAction *feeds_menu_del_all_filters;
    QAction *feeds_menu_on_all_filters;
    QAction *feeds_menu_off_all_filters;
    QAction *filters_menu_add;
    QAction *filters_menu_edit;
    QAction *filters_menu_del;
    QAction *filters_menu_del_all;
    QAction *filters_menu_on_all;
    QAction *filters_menu_off_all;

#ifdef Q_OS_ANDROID
    QPushButton *close_fnf_button;
    bool gestureEvent(QGestureEvent *event);
    void tap_and_holdTriggered(QTapAndHoldGesture*);
#endif

/*
private slots:
    void closeEvent(QCloseEvent * event) Q_DECL_OVERRIDE;
    void showEvent(QShowEvent * event) Q_DECL_OVERRIDE;
*/
protected:
    bool eventFilter(QObject *obj, QEvent *event);
#ifdef Q_OS_ANDROID
    bool event(QEvent *event) override;
#endif

protected slots:
    void slot_feeds_menu();  // обработка контекстного меню
    void slot_filters_menu();  // обработка контекстного меню
};

#endif // FEEDSANDFILTERS_H
