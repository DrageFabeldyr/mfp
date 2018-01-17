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
#include <QMouseEvent>

#ifdef Q_OS_ANDROID
    #include <QScreen>
#endif


#include "settings.h"
#include "filter.h"
#include "feeds.h"
#include "feedsandfilters.h"

extern QString prog_name_ver;

class DFRSSFilter : public QWidget
{
    Q_OBJECT

public:
    DFRSSFilter(QWidget *widget = 0);
    ~DFRSSFilter();

    Settings *settings = nullptr;
    Feeds *pFeeds;
    FeedsAndFilters *feedsandfilters = nullptr; // обнулим указатель, чтобы программа не валилась при проверке на существование

public slots:
    void fetch();
    void finished(QNetworkReply *reply);
    void readyRead();
    void metaDataChanged();
    void itemActivated(QTreeWidgetItem * item);
    void error(QNetworkReply::NetworkError);
    // dm -->
    void edit_settings();
    void edit_feeds_and_filters();
    void quit();
    void unlight();
    void set_language(int language);

protected:
    bool eventFilter(QObject *obj, QEvent *event);

private:
    void parsebandcamp(QByteArray bandcampdata);
    // <-- dm
    void parseXml();
    void get(const QUrl &url);

    Feed activeFeed;

    QXmlStreamReader xml;
    QString currentTag;
    QString linkString;
    QString titleString;

    QNetworkAccessManager manager;
    QNetworkReply *currentReply;
    QVBoxLayout *layout;
    QHBoxLayout *hboxLayout;

    QTreeWidget *treeWidget;
    QPushButton *fetchButton;
    // dm -->
    QPushButton *clearButton;
    QPushButton *readButton;
    QLabel *hint;
    QMenu *trayIconMenu, *main_menu;
    QMenuBar *mainmenubar;
    QSystemTrayIcon *trayIcon;
    QAction *menu_settings;
    QAction *menu_feeds_and_filters;
    QAction *menu_quit;
    QAction *tray_quit;
    QMenu *lang_menu;
    QAction *lang_ru;
    QAction *lang_en;


    bool need_a_name; // для проверки необходимости обновления имени окна
    bool win_max; // переменная для хранения размеров окна
    int counter = 0; // счётчик количества выводимых лент

    bool have_news; // переменная для вывода уведомления о наличии новостей
    int num_of_results; // переменная для подсчёта интересующих новостей в ленте
    int num_of_new_news = 0; // количество новых новостей для вывода в уведомлении

    QString bandcamp_sucks;

#ifdef Q_OS_ANDROID
    QScreen *screen;
#endif
    int screen_height = 0;
    int screen_width = 0;

private slots:
    void show_hide(QSystemTrayIcon::ActivationReason);
    void clear_results();
    void set_lang_ru();
    void set_lang_en();
    // <-- dm
};

#endif
