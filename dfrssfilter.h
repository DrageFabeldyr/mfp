#ifndef DFRSSFILTER_H
#define DFRSSFILTER_H

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QWidget>
#include <QBuffer>
#include <QXmlStreamReader>
#include <QUrl>
#include <QStatusBar>
#include <QMenu>
#include <QMenuBar>
#include <QSignalMapper>
#include <QSystemTrayIcon>


extern QString prog_name_ver;

//class QLineEdit;
/*
class QTreeWidget;
class QTreeWidgetItem;
class QPushButton;
*/
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QPushButton>

class DFRSSFilter : public QWidget
{
    Q_OBJECT
public:
    DFRSSFilter(QWidget *widget = 0);

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
    QStatusBar *statusbar;
    QMenu *trayIconMenu, *settings_menu;
    QMenuBar *mainmenubar;
    QSystemTrayIcon *trayIcon;
    QAction *quitAction, *nocover, *noclose, *open_settings, *open_feeds, *open_filters;

private slots:
    void show_hide(QSystemTrayIcon::ActivationReason);
    // <-- dm
};

#endif
