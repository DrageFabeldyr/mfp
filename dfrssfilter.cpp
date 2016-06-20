#include "dfrssfilter.h"
#include "ui_dfrssfilter.h"

#include <QtCore>
#include <QtWidgets>
#include <QtNetwork>
#include <QString>

#include "filter.h"
#include "settings.h"
#include "feeds_settings.h"


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

            if (settings->min_to_tray->isChecked())
                this->hide();
        }
    }
    if (event->type() == QEvent::Close && obj != NULL)
    {
        if (settings->close_to_tray->isChecked())
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

DFRSSFilter::~DFRSSFilter()
{
    delete settings;
    delete filter;
    delete feeds_settings;

    delete currentReply;

    delete treeWidget;
    delete fetchButton;
    delete hint;
    delete trayIconMenu,
    delete main_menu;
    delete mainmenubar;
    delete trayIcon;
    delete menu_settings,
    delete menu_feeds;
    delete menu_filters;
    delete menu_quit;
    delete tray_quit;
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
    filter = new Filter(this);
    filter->read_filters(); // считываем фильтры
    feeds_settings = new Feeds_Settings(this);
    feeds_settings->read_feeds(); // считываем ленты
    settings = new Settings;
    settings->read_settings();
    win_max = false; // окно не развёрнуто

    fetchButton = new QPushButton(tr("Поиск"), this);
    fetchButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed); // чтобы кнопка не растягивалась при изменении надписи рядом с ней

    treeWidget = new QTreeWidget(this);
    connect(treeWidget, SIGNAL(itemActivated(QTreeWidgetItem*,int)), this, SLOT(itemActivated(QTreeWidgetItem*)));

    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // здесь решается вопрос растяжения - нужно его решить
    treeWidget->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    treeWidget->header()->hide(); // скроем заголовок таблицы - он нафиг не нужен
    treeWidget->setColumnHidden(1, true); // скроем столбец со ссылками - удалять его нельзя - ссылки берутся из него для открытия

    connect(&manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(finished(QNetworkReply*)));
    connect(fetchButton, SIGNAL(clicked()), this, SLOT(fetch())); // запуск по нажатию кнопки

    // создание основного меню программы
    mainmenubar = new QMenuBar(this);

    main_menu = new QMenu(this);
    main_menu->setTitle("Меню");
    // нужно обязательно создавать action, потому что menu при нажатии не отрабатывает, а раскрывается
    menu_settings = new QAction(tr("Настройки"), this);
    main_menu->addAction(menu_settings);
    menu_settings->setIcon(QIcon(":/settings.ico"));
    connect(menu_settings, SIGNAL(triggered()), this, SLOT(edit_settings()));
    menu_feeds = new QAction(tr("RSS-ленты"), this);
    main_menu->addAction(menu_feeds);
    menu_feeds->setIcon(QIcon(":/rss.ico"));
    connect(menu_feeds, SIGNAL(triggered()), this, SLOT(edit_feeds()));
    menu_filters = new QAction(tr("Фильтры"), this);
    main_menu->addAction(menu_filters);
    menu_filters->setIcon(QIcon(":/filter.ico"));
    connect(menu_filters, SIGNAL(triggered()), this, SLOT(edit_filters()));
    menu_quit = new QAction("Выход", this);
    main_menu->addAction(menu_quit);
    menu_quit->setIcon(QIcon(":/exit.ico"));
    connect(menu_quit, SIGNAL(triggered()), this, SLOT(quit()));

    mainmenubar->addMenu(main_menu);
    mainmenubar->show();

    QVBoxLayout *layout = new QVBoxLayout;

    QHBoxLayout *hboxLayout = new QHBoxLayout;
    hint = new QLabel(this);
    hint->setText("Дождитесь обновления результатов или нажмите кнопку \"Поиск\", чтобы обновить результаты прямо сейчас");

    hboxLayout->addWidget(hint);
    hboxLayout->addWidget(fetchButton);

    layout->setMenuBar(mainmenubar);
    layout->addLayout(hboxLayout);
    layout->addWidget(treeWidget);
    setLayout(layout);

    setWindowTitle(prog_name_ver);
    resize(640,480);

    // создаем пункты контекстного меню в трее
    tray_quit = new QAction(tr("&Выход"), this);
    tray_quit->setIcon(QIcon(":/exit.ico"));
    connect(tray_quit, SIGNAL(triggered()), this, SLOT(quit()));

    // создаем само контекстное меню, добавляем в него созданные только что пункты
    trayIconMenu = new QMenu(this);
    trayIconMenu->addAction(tray_quit);

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

    this->setWindowIcon(QIcon(":/trell.ico"));
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
    treeWidget->clear();
    /* пробег по списку лент происходит быстрее, чем чтение лент
     * поэтому после первого совпадения выходим из цикла, а продолжим по событию finished */
    counter = 0;
    have_news = false;
    for (; counter < feeds_settings->feeds.size(); counter++)
    {
        if (feeds_settings->feeds.at(counter).is_on)
        {
            num_of_results = 0; // обнуляем значение количества выводимых новостей
            fetchButton->setEnabled(false); // кнопка станет неактивной только если есть хоть одна активная лента
            need_a_name = true;
            xml.clear();
            QUrl url(feeds_settings->feeds.at(counter).link);
            get(url);
            counter++;
            break;
        }
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
    for (; counter < feeds_settings->feeds.size(); counter++)
        if (feeds_settings->feeds.at(counter).is_on)
        {
            num_of_results = 0; // обнуляем значение количества выводимых новостей
            need_a_name = true;
            xml.clear();
            QUrl url(feeds_settings->feeds.at(counter).link);
            get(url);
            counter++;
            break;
        }
    if (counter >= feeds_settings->feeds.size()) // если пробежали всё
    {
        if ((!this->isVisible() || this->isMinimized()) && have_news)
        {
            // если окно свёрнуто или убрано - выведем уведомление
            trayIcon->showMessage("", "Есть новости", QSystemTrayIcon::Information, show_period);
        }
        hint->setText("Двойной клик по новости откроет её в браузере");
        fetchButton->setEnabled(true);
    }
}

// Парсит данные XML и создаёт соответственно элементы treeWidget.
void DFRSSFilter::parseXml()
{
    QString str;
    int num_of_active_filters = 0;

    QTreeWidgetItem *feed_item = new QTreeWidgetItem();

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
                num_of_active_filters = filter->GetNumActiveFilter();

                if (num_of_active_filters > 0)
                {
                    for (int i = 0; i < filter->filters.size(); i++)
                    {
                        // добавим обработку разных языковых символов
                        QTextDocument doc;
                        doc.setHtml(titleString);
                        str = doc.toPlainText();

                        if ((str.contains(filter->filters.at(i).title.simplified(), Qt::CaseInsensitive)) &&
                                filter->filters.at(i).is_on)
                            // simplified - чтобы убрать символ переноса строки из сравнения
                        {
                            QTreeWidgetItem *item = new QTreeWidgetItem();

                            item->setText(0, doc.toPlainText());

                            doc.setHtml(linkString);
                            item->setText(1, doc.toPlainText());

                            feed_item->addChild(item);
                            feed_item->setHidden(false); // если находится хоть один результат - делаем "ветку" видимой

                            //treeWidget->addTopLevelItem(item);
                            titleString.clear();
                            linkString.clear();

                            num_of_results++; // считаем количество отфильтрованных новостей
                            have_news = true; // если есть хоть один результат - нужно вывести уведомление
                        }
                    }
                }
                else // если список активных фильтров пуст - выводим всё
                {
                    QTreeWidgetItem *item = new QTreeWidgetItem;
                    QTextDocument doc;

                    doc.setHtml(titleString);
                    item->setText(0, doc.toPlainText());

                    doc.setHtml(linkString);
                    item->setText(1, doc.toPlainText());

                    feed_item->addChild(item);
                    feed_item->setHidden(false); // если находится хоть один результат - делаем "ветку" видимой
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
                    QTextDocument doc;
                    doc.setHtml(titleString);
                    feed_item->setText(0, doc.toPlainText());
                    treeWidget->addTopLevelItem(feed_item);
                    feed_item->setHidden(true); // делаем по умолчанию "ветку" невидимой
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

    filter->setWindowFlags(Qt::WindowStaysOnTopHint | /*Qt::CustomizeWindowHint | */Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
    // задаём параметры - оставаться поверх всех, пользовательские настройки, показать заголовок, показать кнопку закрытия
    // говорят без Qt::CustomizeWindowHint другие флаги не работают, но почему-то всё работает
    filter->setAttribute(Qt::WA_ShowModal, true);         // блокирует родительское окно
    filter->show();                                      // и рисуем его
}

// открытие окна работы с настройками
void DFRSSFilter::edit_settings()
{
    //settings *sett = new settings;
    settings->setWindowFlags(Qt::WindowStaysOnTopHint | /*Qt::CustomizeWindowHint | */Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
    // задаём параметры - оставаться поверх всех, пользовательские настройки, показать заголовок, показать кнопку закрытия
    // говорят без Qt::CustomizeWindowHint другие флаги не работают, но почему-то всё работает
    settings->setAttribute(Qt::WA_ShowModal, true);         // блокирует родительское окно
    settings->show();                                       // и рисуем его
}

// открытие окна работы с лентами
void DFRSSFilter::edit_feeds()
{
    feeds_settings->setWindowFlags(Qt::WindowStaysOnTopHint | /*Qt::CustomizeWindowHint | */Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
    // задаём параметры - оставаться поверх всех, пользовательские настройки, показать заголовок, показать кнопку закрытия
    // говорят без Qt::CustomizeWindowHint другие флаги не работают, но почему-то всё работает
    feeds_settings->setAttribute(Qt::WA_ShowModal, true);         // блокирует родительское окно
    feeds_settings->show();                                       // и рисуем его
}

void DFRSSFilter::quit()
{
    exit(0);
}
