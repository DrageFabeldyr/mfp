#include "dfrssfilter.h"
#include "ui_dfrssfilter.h"

#include <QtCore>
#include <QtWidgets>
#include <QtNetwork>
#include <QString>

#include "filter.h"
#include "settings.h"
#include "feeds_settings.h"

bool need_a_name; // для проверки необходимости обновления имени окна
bool win_max; // переменная для хранения размеров окна
QString feed_name;
int request_period = 1*60*1000; // запрос новостей раз в 1 минут
int show_period = 30*1000; // уведомление в трее будет висеть 30 секунд
int counter = 0;
settings *sett;
bool have_news; // переменная для вывода уведомления о наличии новостей

// обработчик событий
bool DFRSSFilter::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::WindowStateChange && obj != NULL)
    {
        if (this->windowState() & Qt::WindowMinimized) // "==" почему-то не работает, а "&" работает
        {
            if (static_cast<QWindowStateChangeEvent*>(event)->oldState() == Qt::WindowMaximized)
                win_max = true;
            else
                win_max = false;

            if (sett->min_to_tray->isChecked())
                this->hide();
        }
    }
    if (event->type() == QEvent::Close && obj != NULL)
    {
        if (sett->close_to_tray->isChecked())
        {
            if (this->isMaximized())
                win_max = true;
            else
                win_max = false;
            this->hide();
            event->ignore();
            return true; // чтобы событие сбросилось из очереди, иначе его обработает стандартный eventClose
        }
        else
            quit();
    }
    return false;
}


// обработчик нажатия на иконку в трее
void DFRSSFilter::show_hide(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::Trigger) //если нажато левой кнопкой продолжаем
    {
        if (!this->isVisible() || this->isMinimized())// || !this->isActiveWindow())
        {
            if (win_max)
                this->showMaximized();
            else
                this->showNormal();

            this->activateWindow(); // делает окно активным
        }
        else
        {
            if (this->isMaximized())
                win_max = true;
            else
                win_max = false;
            this->hide(); //иначе скрываем
        }
    }
}


/*  Конструирует виджет RSSListing с простым интерфейсом пользователя и устанавливает
 *  ридер XML для использования пользовательского класса обработки.
 *  Интерфейс пользователя состоит из строки ввода, кнопки и
 *  виджета просмотра списка. Строка ввода используется для ввода URL источника новостей;
 *  источников; кнопка начинает процесс чтения новостей. */
DFRSSFilter::DFRSSFilter(QWidget *parent) : QWidget(parent), currentReply(0)
{
    read_filters(); // считываем фильтры
    read_feeds(); // считываем ленты
    sett = new settings;
    sett->read_settings();
    win_max = false; // окно не развёрнуто

    fetchButton = new QPushButton(tr("Поиск"), this);

    treeWidget = new QTreeWidget(this);
    connect(treeWidget, SIGNAL(itemActivated(QTreeWidgetItem*,int)), this, SLOT(itemActivated(QTreeWidgetItem*)));
    QStringList headerLabels;
    headerLabels << tr("Заголовок") << tr("Ссылка");
    treeWidget->setHeaderLabels(headerLabels);
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // здесь решается вопрос растяжения - нужно его решить
    treeWidget->header()->setSectionResizeMode(QHeaderView::ResizeToContents);

    connect(&manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(finished(QNetworkReply*)));
    connect(fetchButton, SIGNAL(clicked()), this, SLOT(fetch())); // запуск по нажатию кнопки

    // создание основного меню программы
    mainmenubar = new QMenuBar(this);

    settings_menu = new QMenu(this);
    settings_menu->setTitle("Меню");
    open_settings = new QAction(tr("Настройки"), this);
    // нужно обязательно создавать action, потому что menu при нажатии не отрабатывает, а раскрывается
    settings_menu->addAction(open_settings);
    connect(open_settings, SIGNAL(triggered()), this, SLOT(edit_settings()));
    open_feeds = new QAction(tr("RSS-ленты"), this);
    settings_menu->addAction(open_feeds);
    connect(open_feeds, SIGNAL(triggered()), this, SLOT(edit_feeds()));
    open_filters = new QAction(tr("Фильтры"), this);
    settings_menu->addAction(open_filters);
    connect(open_filters, SIGNAL(triggered()), this, SLOT(edit_filters()));

    mainmenubar->addMenu(settings_menu);
    mainmenubar->show();

    QVBoxLayout *layout = new QVBoxLayout;

    QHBoxLayout *hboxLayout = new QHBoxLayout;
    statusbar = new QStatusBar(this);
    statusbar->showMessage("Добро пожаловать =))");

    hboxLayout->addWidget(fetchButton);

    layout->addWidget(mainmenubar);
    layout->addLayout(hboxLayout);
    layout->addWidget(treeWidget);
    layout->addWidget(statusbar);
    layout->setMargin(0); // убирает промежуток между layout и границами окна
    setLayout(layout);

    setWindowTitle(prog_name_ver);
    resize(640,480);

    // создаем пункты контекстного меню в трее
    quitAction = new QAction(tr("&Выход"), this);
    connect(quitAction, SIGNAL(triggered()), this, SLOT(quit()));

    // создаем само контекстное меню, добавляем в него созданные только что пункты
    trayIconMenu = new QMenu(this);
    trayIconMenu->addAction(quitAction);

    trayIcon = new QSystemTrayIcon();
    trayIcon->setIcon(QIcon(":/trell.ico"));
    trayIcon->setContextMenu(trayIconMenu); // добавляем контекстное меню
    trayIcon->show();
    // соединяем сигнал активации значка в системном трее со слотом обработки этого события
    connect(trayIcon,SIGNAL(activated(QSystemTrayIcon::ActivationReason)),this,SLOT(show_hide(QSystemTrayIcon::ActivationReason)));

    // устанавливаем наш обработчик событий
    installEventFilter(this);


    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(fetch()));
    timer->start(request_period); // обновление по таймеру
}


// Начинает сетевой запрос и соединяет необходимые сигналы
void DFRSSFilter::get(const QUrl &url)
{
    QNetworkRequest request(url);
    if (currentReply)
    {
        currentReply->disconnect(this);
        currentReply->deleteLater();
    }
    currentReply = manager.get(request);
    connect(currentReply, SIGNAL(readyRead()), this, SLOT(readyRead()));
    connect(currentReply, SIGNAL(metaDataChanged()), this, SLOT(metaDataChanged()));
    connect(currentReply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(error(QNetworkReply::NetworkError)));
}

/*
    Начинает скачивать данные из источника новостей указанного в виджете
    строки ввода.

    Строка ввода делается только для чтения чтобы не дать пользователю изменять её
    содержимое во время скачивания; это только для косметических целей.
    Кнопка получения данных отключена, представление списка очищено и мы
    определяем последний элемент равным 0, означая, что здесь нет
    существующих элементов в списке.

    URL создаётся с сырым содержимым строки редактирования и
    получает инициализацию.
*/
void DFRSSFilter::fetch()
{
    fetchButton->setEnabled(false);
    treeWidget->clear();
    /* пробег по списку лент происходит быстрее, чем чтение лент
     * поэтому после первого совпадения выходим из цикла, а продолжим по событию finished */
    counter = 0;
    have_news = false;
    for (; counter < feeds.size(); counter++)
        if (feeds.at(counter).is_on)
        {
            need_a_name = true;
            xml.clear();
            QUrl url(feeds.at(counter).link);
            get(url);
            statusbar->showMessage(QString("Количество применённых фильтров: %1").arg(filters.size()));
            counter++;
            break;
        }
}

void DFRSSFilter::metaDataChanged()
{
    QUrl redirectionTarget = currentReply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
    if (redirectionTarget.isValid())
    {
        get(redirectionTarget);
    }
}

/*  Считывает данные, полученные из источника RDF.
 *  Считываем все доступные данные и передаём их потоковому XML ридеру, затем вызываем функцию парсинга XML. */
void DFRSSFilter::readyRead()
{
    int statusCode = currentReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (statusCode >= 200 && statusCode < 300) {
        QByteArray data = currentReply->readAll();
        xml.addData(data);
        parseXml();
    }
}

/*  Заканчивает обработку запроса HTTP.
 *  Поведение по умолчанию сохраняет редактор текста в режиме только для чтения.
 *  Если происходит ошибка, интерфейс пользователя становится доступен для дальнейшего ввода, позволяя новой скачке начаться.
 *  Если запрос получения HTTP завершается, мы делаем интерфейс пользователя доступным для дальнейшего ввода. */
void DFRSSFilter::finished(QNetworkReply *reply)
{
    Q_UNUSED(reply);
    qWarning() << "Feed finished: "  << QString(reply->url().toString());
    currentReply->disconnect(this);
    currentReply->deleteLater();
    currentReply = nullptr;

    // продолжаем проход по выбранным лентам
    for (; counter < feeds.size(); counter++)
        if (feeds.at(counter).is_on)
        {
            need_a_name = true;
            xml.clear();
            QUrl url(feeds.at(counter).link);
            get(url);
            statusbar->showMessage(QString("Количество применённых фильтров: %1").arg(filters.size()));
            counter++;
            break;
        }
    if (counter >= feeds.size()) // если пробежали всё
    {
        if ((!this->isVisible() || this->isMinimized()) && have_news)
        {
            // если окно свёрнуто или убрано - выведем уведомление
            trayIcon->showMessage("", "Есть новости", QSystemTrayIcon::Information, show_period);
        }
        fetchButton->setEnabled(true);
    }
}

// Парсит данные XML и создаёт соответственно элементы treeWidget.
void DFRSSFilter::parseXml()
{
    QString str;

    QTreeWidgetItem *feed_item = new QTreeWidgetItem;
//    treeWidget->addTopLevelItem(feed_item);

    while (!xml.atEnd())
    {
        xml.readNext();
        if (xml.isStartElement())
        {
            if (xml.name() == "item")
                linkString = xml.attributes().value("rss:about").toString();
            currentTag = xml.name().toString();
        }
        else if (xml.isEndElement())
        {
            if (xml.name() == "item")
            {
                if (filters.size() > 0)
                {
                    for (int i = 0; i < filters.size(); i++)
                    {
                        // добавим обработку разных языковых символов
                        QTextDocument doc;
                        doc.setHtml(titleString);
                        str = doc.toPlainText();

                        if (str.contains(filters.at(i).simplified(), Qt::CaseInsensitive))
                            // simplified - чтобы убрать символ переноса строки из сравнения
                        {
                            QTreeWidgetItem *item = new QTreeWidgetItem;

                            item->setText(0, doc.toPlainText());

                            doc.setHtml(linkString);
                            item->setText(1, doc.toPlainText());

                            feed_item->addChild(item);

                            //treeWidget->addTopLevelItem(item);
                            titleString.clear();
                            linkString.clear();

                            have_news = true; // если есть хоть один результат - нужно вывести уведомление
                        }
                    }
                }
                else // если список фильтров пуст - выводим всё
                {
                    QTreeWidgetItem *item = new QTreeWidgetItem;
                    QTextDocument doc;

                    doc.setHtml(titleString);
                    item->setText(0, doc.toPlainText());

                    doc.setHtml(linkString);
                    item->setText(1, doc.toPlainText());

                    feed_item->addChild(item);
                    //treeWidget->addTopLevelItem(item);

                    titleString.clear();
                    linkString.clear();
                }
            }
        }
        else if (xml.isCharacters() && !xml.isWhitespace())
        {
            if (currentTag == "title")
            {
                // dm-->
                titleString.clear();
                /* это нужно для того, чтобы убрать лишний текст из первой новости,
                 * т.к. все новости чередуют название - ссылка,
                 * а два названия подряд быть не может
                 */
                // <--dm
                titleString += xml.text().toString();
                if (need_a_name)
                {
                    /*
                    feed_name.clear();
                    feed_name = titleString;
*/
                    QTextDocument doc;
                    doc.setHtml(titleString);
                    feed_item->setText(0, doc.toPlainText());
                    treeWidget->addTopLevelItem(feed_item);
                    //setWindowTitle(titleString);
                    need_a_name = false;
                }
            }
            else if (currentTag == "link")
                linkString += xml.text().toString();
        }
    }
    if (xml.error() && xml.error() != QXmlStreamReader::PrematureEndOfDocumentError)
    {
        qWarning() << "XML ERROR:" << xml.lineNumber() << ": " << xml.errorString();
    }
}

// Открывает ссылку в браузере
void DFRSSFilter::itemActivated(QTreeWidgetItem * item)
{
    QDesktopServices::openUrl(QUrl(item->text(1)));
}

void DFRSSFilter::error(QNetworkReply::NetworkError)
{
    qWarning("error retrieving RSS feed");
    currentReply->disconnect(this);
    currentReply->deleteLater();
    currentReply = 0;
}


// dm -->
// открытие окна работы с фильтрами
void DFRSSFilter::edit_filters()
{
    filter *new_filter = new filter();
    new_filter->setWindowFlags(Qt::WindowStaysOnTopHint | /*Qt::CustomizeWindowHint | */Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
    // задаём параметры - оставаться поверх всех, пользовательские настройки, показать заголовок, показать кнопку закрытия
    // говорят без Qt::CustomizeWindowHint другие флаги не работают, но почему-то всё работает
    new_filter->setAttribute(Qt::WA_ShowModal, true);         // блокирует родительское окно
    new_filter->show();                                      // и рисуем его
}

// открытие окна работы с настройками
void DFRSSFilter::edit_settings()
{
    //settings *sett = new settings;
    sett->setWindowFlags(Qt::WindowStaysOnTopHint | /*Qt::CustomizeWindowHint | */Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
    // задаём параметры - оставаться поверх всех, пользовательские настройки, показать заголовок, показать кнопку закрытия
    // говорят без Qt::CustomizeWindowHint другие флаги не работают, но почему-то всё работает
    sett->setAttribute(Qt::WA_ShowModal, true);         // блокирует родительское окно
    sett->show();                                       // и рисуем его
}

// открытие окна работы с лентами
void DFRSSFilter::edit_feeds()
{
    feeds_settings *f_sett = new feeds_settings;
    f_sett->setWindowFlags(Qt::WindowStaysOnTopHint | /*Qt::CustomizeWindowHint | */Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
    // задаём параметры - оставаться поверх всех, пользовательские настройки, показать заголовок, показать кнопку закрытия
    // говорят без Qt::CustomizeWindowHint другие флаги не работают, но почему-то всё работает
    f_sett->setAttribute(Qt::WA_ShowModal, true);         // блокирует родительское окно
    f_sett->show();                                       // и рисуем его
}

void DFRSSFilter::quit()
{
    exit(0);
}
