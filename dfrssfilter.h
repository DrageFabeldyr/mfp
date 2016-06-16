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
    QLabel *hint;
    QMenu *trayIconMenu, *main_menu;
    QMenuBar *mainmenubar;
    QSystemTrayIcon *trayIcon;
    QAction *menu_settings, *menu_feeds, *menu_filters, *menu_quit;
    QAction *tray_quit;

private slots:
    void show_hide(QSystemTrayIcon::ActivationReason);
    // <-- dm
};

#endif
