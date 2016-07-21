#include <QtNetwork>
#include <QDesktopServices>
#include <QTextDocument>

#include "dfrssfilter.h"
#include "ui_dfrssfilter.h"
#include "settings.h"
#include "feedsandfilters.h"

QMutex mutex;

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
    delete currentReply;
    delete layout;
    delete hboxLayout;
    delete treeWidget;
    delete fetchButton;
    delete hint;
    delete trayIconMenu,
    delete main_menu;
    delete mainmenubar;
    delete trayIcon;
    delete menu_settings,
    delete menu_feeds_and_filters;
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
    settings = new Settings(this);
    pFeeds = new Feeds();
    settings->read_settings();

    win_max = false; // окно не развёрнуто
    num_of_results = 0; // обнуляем значение количества выводимых новостей

    fetchButton = new QPushButton(tr("Поиск"), this);
    fetchButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed); // чтобы кнопка не растягивалась при изменении надписи рядом с ней

    clearButton = new QPushButton(tr("Очистить"), this);
    clearButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed); // чтобы кнопка не растягивалась при изменении надписи рядом с ней

    treeWidget = new QTreeWidget(this);
    connect(treeWidget, SIGNAL(itemActivated(QTreeWidgetItem*,int)), this, SLOT(itemActivated(QTreeWidgetItem*)));

    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // здесь решается вопрос растяжения - нужно его решить
    treeWidget->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    treeWidget->header()->hide(); // скроем заголовок таблицы - он нафиг не нужен
    treeWidget->setColumnHidden(1, true); // скроем столбец со ссылками - удалять его нельзя - ссылки берутся из него для открытия

    connect(&manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(finished(QNetworkReply*)));
    connect(fetchButton, SIGNAL(clicked()), this, SLOT(fetch())); // запуск по нажатию кнопки
    connect(clearButton, SIGNAL(clicked()), this, SLOT(clear_results())); // запуск по нажатию кнопки

    // создание основного меню программы
    mainmenubar = new QMenuBar(this);

    main_menu = new QMenu(this);
    main_menu->setTitle("Меню");
    // нужно обязательно создавать action, потому что menu при нажатии не отрабатывает, а раскрывается
    menu_settings = new QAction(tr("Настройки"), this);
    main_menu->addAction(menu_settings);
    menu_settings->setIcon(QIcon(":/settings.ico"));
    connect(menu_settings, SIGNAL(triggered()), this, SLOT(edit_settings()));
    menu_feeds_and_filters = new QAction(tr("RSS-ленты"), this);
    main_menu->addAction(menu_feeds_and_filters);
    menu_feeds_and_filters->setIcon(QIcon(":/rss.ico"));
    connect(menu_feeds_and_filters, SIGNAL(triggered()), this, SLOT(edit_feeds_and_filters()));

    menu_quit = new QAction("Выход", this);
    main_menu->addAction(menu_quit);
    menu_quit->setIcon(QIcon(":/exit.ico"));
    connect(menu_quit, SIGNAL(triggered()), this, SLOT(quit()));

    mainmenubar->addMenu(main_menu);
    mainmenubar->show();

    layout = new QVBoxLayout;

    hboxLayout = new QHBoxLayout;
    hint = new QLabel(this);
    hint->setText("Дождитесь автоматического обновления результатов или нажмите кнопку \"Поиск\"");

    hboxLayout->addWidget(hint);
    hboxLayout->addWidget(clearButton);
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
    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(show_hide(QSystemTrayIcon::ActivationReason)));

    // устанавливаем наш обработчик событий
    installEventFilter(this);

    connect(settings->timer, SIGNAL(timeout()), this, SLOT(fetch()));
    settings->timer->start(settings->request_period); // обновление по таймеру

    connect(treeWidget, SIGNAL(itemClicked(QTreeWidgetItem*,int)), settings->timer, SLOT(start())); // в случае клика по новости таймер сбрасывается

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
    //connect(currentReply, SIGNAL(finished()), this, SLOT(finished()));
}

/*  Начинает скачивать данные из источника новостей указанного в виджете строки ввода.

    Строка ввода делается только для чтения чтобы не дать пользователю изменять её
    содержимое во время скачивания; это только для косметических целей.
    Кнопка получения данных отключена, представление списка очищено и мы
    определяем последний элемент равным 0, означая, что здесь нет существующих элементов в списке.

    URL создаётся с сырым содержимым строки редактирования и получает инициализацию. */
void DFRSSFilter::fetch()
{
    /* пробег по списку лент происходит быстрее, чем чтение лент
     * поэтому после первого совпадения выходим из цикла, а продолжим по событию finished */
    counter = 0;
    QList <Feed> feeds;
    pFeeds->GetActiveFeedsList(feeds);

    if (feeds.size() == 0)
        return;
    have_news = false; // сбрасываем флаг информирования
    fetchButton->setEnabled(false); // кнопка станет неактивной только если есть хоть одна активная лента
    need_a_name = true;
    xml.clear();
    QUrl url(feeds.at(0).link);
    activeFeed = feeds.at(0);
    get(url);

    counter = 1;
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
    //Q_UNUSED(reply);
    qWarning() << "Feed finished: " << QString(reply->url().toString());
    reply->disconnect(this);
    reply->deleteLater();
    reply = nullptr;

    if (currentReply)
    {
        currentReply->disconnect(this);
        currentReply->deleteLater();
        currentReply = nullptr;
    }

    QList <Feed> feeds;
    pFeeds->GetActiveFeedsList(feeds);

    if (counter < feeds.size())
    {
        fetchButton->setEnabled(false); // кнопка станет неактивной только если есть хоть одна активная лента
        need_a_name = true;
        xml.clear();
        QUrl url(feeds.at(counter).link);
        activeFeed = feeds.at(counter);
        get(url);
        ++counter;
    }
    else
    {
        int new_num_of_results = 0;
        for (int i = 0; i < treeWidget->topLevelItemCount(); i++)
        {
            QTreeWidgetItem *test = treeWidget->topLevelItem(i);
            new_num_of_results += test->childCount();
        }
        if (new_num_of_results > num_of_results)
        {
            have_news = true;
            num_of_results = new_num_of_results;
        }

        if ((!this->isVisible() || this->isMinimized()) && have_news)
        {
            // если окно свёрнуто или убрано - выведем уведомление
            trayIcon->showMessage("", "Есть новости", QSystemTrayIcon::Information, settings->show_period);
        }
        hint->setText("Двойной клик по новости откроет её в браузере");
        fetchButton->setEnabled(true);
    }
}

/*
void DFRSSFilter::finished()
{
    //Q_UNUSED(reply);
    qWarning() << "Feed finished: " << QString(currentReply->url().toString());
    currentReply->disconnect(this);
    currentReply->deleteLater();
    currentReply = nullptr;

    QList <Feed> feeds;
    pFeeds->GetActiveFeedsList(feeds);

    if (counter < feeds.size())
    {
        fetchButton->setEnabled(false); // кнопка станет неактивной только если есть хоть одна активная лента
        need_a_name = true;
        xml.clear();
        QUrl url(feeds.at(counter).link);
        activeFeed = feeds.at(counter);
        get(url);
        ++counter;
    }
    else
    {
        int new_num_of_results = 0;
        for (int i = 0; i < treeWidget->topLevelItemCount(); i++)
        {
            QTreeWidgetItem *test = treeWidget->topLevelItem(i);
            new_num_of_results += test->childCount();
        }
        if (new_num_of_results > num_of_results)
        {
            have_news = true;
            num_of_results = new_num_of_results;
        }

        if ((!this->isVisible() || this->isMinimized()) && have_news)
        {
            // если окно свёрнуто или убрано - выведем уведомление
            trayIcon->showMessage("", "Есть новости", QSystemTrayIcon::Information, settings->show_period);
        }
        hint->setText("Двойной клик по новости откроет её в браузере");
        fetchButton->setEnabled(true);
    }
}
*/

// Парсит данные XML и создаёт соответственно элементы treeWidget.
void DFRSSFilter::parseXml()
{
    QString str;
    QString has_the_world1 = "тысячелистник";
    QString has_the_world2 = "trelleborg";

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
                QList<filters_struct> filters;
                pFeeds->GetActiveFiltersList(filters, activeFeed.id);
                if (filters.size() != 0) // если есть фильтры - ищем
                {

                    foreach (filters_struct filter, filters)
                    {
                        // добавим обработку разных языковых символов
                        QTextDocument doc;
                        doc.setHtml(titleString);
                        str = doc.toPlainText();

                        if ((filter.comment.simplified() == "Автопоиск" && str.contains(filter.title.simplified(), Qt::CaseSensitive)) ||
                            (filter.comment.simplified() != "Автопоиск" && str.contains(filter.title.simplified(), Qt::CaseInsensitive)) ||
                            (str.contains(has_the_world1.simplified(), Qt::CaseInsensitive)) ||
                            (str.contains(has_the_world2.simplified(), Qt::CaseInsensitive)))
                            // simplified - чтобы убрать символ переноса строки из сравнения
                        {
                            QTreeWidgetItem *item = new QTreeWidgetItem();

                            item->setText(0, doc.toPlainText());

                            doc.setHtml(linkString);
                            item->setText(1, doc.toPlainText());

                            bool new_new = true; // новая новость
                            for (int i = 0; i < feed_item->childCount(); i++)
                            {
                                //if (feed_item->child(i)->text(0).trimmed() == item->text(0).trimmed())
                                int x = QString::compare(feed_item->child(i)->text(0).trimmed(), item->text(0).trimmed(), Qt::CaseInsensitive);
                                // был случай, когда в новости "Vs" заменили на "vs" и она вывелась второй раз
                                if (x == 0)
                                {
                                    new_new = false;
                                    break;
                                }
                            }

                            if (new_new)
                            {
                                feed_item->addChild(item);
                                feed_item->setHidden(false); // если потом найдётся хоть один результат - сделаем "ветку" видимой

                                titleString.clear();
                                linkString.clear();
                            }
                        }

                    }
                }
                else // если фильтров нет - выводим всё
                {
                    QTreeWidgetItem *item = new QTreeWidgetItem;
                    QTextDocument doc;

                    doc.setHtml(titleString);
                    item->setText(0, doc.toPlainText());

                    doc.setHtml(linkString);
                    item->setText(1, doc.toPlainText());

                    bool new_new = true; // новая новость
                    for (int i = 0; i < feed_item->childCount(); i++)
                    {
                        //if (feed_item->child(i)->text(0).trimmed() == item->text(0).trimmed())
                        int x = QString::compare(feed_item->child(i)->text(0).trimmed(), item->text(0).trimmed(), Qt::CaseInsensitive);
                        // был случай, когда в новости "Vs" заменили на "vs" и она вывелась второй раз
                        if (x == 0)
                        {
                            new_new = false;
                            break;
                        }
                    }

                    if (new_new)
                    {
                        feed_item->addChild(item);
                        feed_item->setHidden(false); // если потом найдётся хоть один результат - сделаем "ветку" видимой

                        titleString.clear();
                        linkString.clear();
                    }
                }
            }
        }
        else if (xml.isCharacters() && !xml.isWhitespace())
        {
            if (currentTag == "title")
            {
                // dm-->
                titleString.clear();
                /* это нужно для того, чтобы убрать лишний текст из первой новости (т.е. заголовка),
                 * т.к. все новости чередуют название - ссылка, а два названия подряд быть не может
                 */
                // <--dm
                titleString += xml.text().toString();
                if (need_a_name) // если это первый проход и мы должны задать имя ленте
                {
                    bool new_feed = true;
                    QTextDocument doc;
                    doc.setHtml(titleString);
                    if (activeFeed.title == "")
                        feed_item->setText(0, doc.toPlainText()); // если у ленты нет имени - берём из ленты
                    else
                        feed_item->setText(0, activeFeed.title); // если есть - берём то, которое задано

                    // ищем текущую ленту среди уже выведенных
                    for (int i = 0; i < treeWidget->topLevelItemCount(); i++)
                        if (treeWidget->topLevelItem(i)->text(0).trimmed() == feed_item->text(0))
                        {
                            treeWidget->setCurrentItem(treeWidget->topLevelItem(i));
                            feed_item = treeWidget->currentItem(); // чтобы новости добавлялись в ту ленту, что уже есть
                            new_feed = false;
                            break;
                        }

                    // если не нашли - добавляем
                    if (new_feed)
                    {
                        treeWidget->addTopLevelItem(feed_item);
                        feed_item->setHidden(true); // делаем по умолчанию "ветку" невидимой
                    }
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

// открытие окна работы с настройками
void DFRSSFilter::edit_settings()
{
    settings->setWindowFlags(Qt::WindowStaysOnTopHint | /*Qt::CustomizeWindowHint | */Qt::WindowTitleHint | Qt::WindowCloseButtonHint/* | Qt::Tool*/);
    // задаём параметры - оставаться поверх всех, пользовательские настройки, показать заголовок, показать кнопку закрытия, убрать иконку из панели задач
    // говорят без Qt::CustomizeWindowHint другие флаги не работают, но почему-то всё работает
    settings->setAttribute(Qt::WA_ShowModal, true);         // блокирует родительское окно
    settings->show();                                       // и рисуем его
}

// открытие окна работы с лентами и фильтрами
void DFRSSFilter::edit_feeds_and_filters()
{
    FeedsAndFilters *win = new FeedsAndFilters(this);
    win->setWindowFlags(Qt::WindowStaysOnTopHint | /*Qt::CustomizeWindowHint | */Qt::WindowTitleHint | Qt::WindowCloseButtonHint/* | Qt::Tool*/);
    // задаём параметры - оставаться поверх всех, пользовательские настройки, показать заголовок, показать кнопку закрытия, убрать иконку из панели задач
    // говорят без Qt::CustomizeWindowHint другие флаги не работают, но почему-то всё работает

    win->setAttribute(Qt::WA_ShowModal, true);         // блокирует родительское окно
    win->show();                                       // и рисуем его
}

void DFRSSFilter::quit()
{
    exit(0);
}

void DFRSSFilter::clear_results()
{
    treeWidget->clear();
}
