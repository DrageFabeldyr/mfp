#ifndef DFRSSFILTER_H
#define DFRSSFILTER_H

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QWidget>
#include <QBuffer>
#include <QXmlStreamReader>
#include <QUrl>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QSignalMapper>
#include <QSystemTrayIcon>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QPushButton>

#include "settings.h"
#include "filter.h"
#include "feeds_settings.h"

extern QString prog_name_ver;



class DFRSSFilter : public QWidget
{
    Q_OBJECT
public:
    DFRSSFilter(QWidget *widget = 0);
    ~DFRSSFilter();

    Settings *settings = nullptr;
    Filter *filter = nullptr;
    Feeds_Settings *feeds_settings = nullptr;

    void GetNumActiveFilters(int num_of_active_filters);

public slots:
    void fetch();
    void finished(QNetworkReply *reply);
    void readyRead();
    void metaDataChanged();
    void itemActivated(QTreeWidgetItem * item);
    void error(QNetworkReply::NetworkError);
    // dm -->
    void edit_filters();
    void edit_settings();
    void edit_feeds();
    //void read_feeds();
    //int read_settings();
    //void write_settings_and_quit();
    void quit();

protected:
    bool eventFilter(QObject *obj, QEvent *event);
    // <-- dm

private:
    void parseXml();
    void get(const QUrl &url);

    QXmlStreamReader xml;
    QString currentTag;
    QString linkString;
    QString titleString;

    QNetworkAccessManager manager;
    QNetworkReply *currentReply;

    QTreeWidget *treeWidget;
    QPushButton *fetchButton;
    // dm -->
    QLabel *hint;
    QMenu *trayIconMenu, *main_menu;
    QMenuBar *mainmenubar;
    QSystemTrayIcon *trayIcon;
    QAction *menu_settings, *menu_feeds, *menu_filters, *menu_quit;
    QAction *tray_quit;


    bool need_a_name; // для проверки необходимости обновления имени окна
    bool win_max; // переменная для хранения размеров окна
    int request_period = 5*60*1000; // запрос новостей раз в 5 минут
    int show_period = 30*1000; // уведомление в трее будет висеть 30 секунд
    int counter = 0;
    bool have_news; // переменная для вывода уведомления о наличии новостей
    int num_of_results; // переменная для подсчёта интересующих новостей в ленте

private slots:
    void show_hide(QSystemTrayIcon::ActivationReason);
    // <-- dm
};

#endif
