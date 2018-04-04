#include <QtNetwork>
#include <QDesktopServices>
#include <QTextDocument>

#include "dfrssfilter.h"
#include "ui_dfrssfilter.h"
//#include "settings.h"

//QMutex mutex;

// обработчик событий сворачивания/разворачивания
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
    delete readButton;
    delete clearButton;
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
    feedsandfilters = new FeedsAndFilters(this);

    settings->read_settings();

    win_max = false; // окно не развёрнуто
    num_of_results = 0; // обнуляем значение количества выводимых новостей

    fetchButton = new QPushButton(tr("Поиск"), this);
    fetchButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed); // чтобы кнопка не растягивалась при изменении надписи рядом с ней

    clearButton = new QPushButton(tr("Очистить"), this);
    clearButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed); // чтобы кнопка не растягивалась при изменении надписи рядом с ней

    readButton = new QPushButton(tr("Прочитано"), this);
    readButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed); // чтобы кнопка не растягивалась при изменении надписи рядом с ней

    treeWidget = new QTreeWidget(this);
    connect(treeWidget, SIGNAL(itemActivated(QTreeWidgetItem*,int)), this, SLOT(itemActivated(QTreeWidgetItem*)));

    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // здесь решается вопрос растяжения - нужно его решить
    //treeWidget->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    treeWidget->header()->hide(); // скроем заголовок таблицы - он нафиг не нужен
    //treeWidget->setColumnCount(3);
    //treeWidget->setColumnHidden(1, true); // скроем столбец со ссылками - удалять его нельзя - ссылки берутся из него для открытия

    connect(&manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(finished(QNetworkReply*)));
    connect(fetchButton, SIGNAL(clicked()), this, SLOT(fetch())); // запуск по нажатию кнопки
    connect(clearButton, SIGNAL(clicked()), this, SLOT(clear_results())); // запуск по нажатию кнопки
    connect(readButton, SIGNAL(clicked()), this, SLOT(unlight())); // запуск по нажатию кнопки

    // создание основного меню программы
    mainmenubar = new QMenuBar(this);

    main_menu = new QMenu(this);
    main_menu->setTitle("Меню");
    // нужно обязательно создавать action, потому что menu при нажатии не отрабатывает, а раскрывается
    // пункт работы с настройками
    menu_settings = new QAction(tr("Настройки"), this);
    menu_settings->setIcon(QIcon(":/img/settings.ico"));
    main_menu->addAction(menu_settings);
    connect(menu_settings, SIGNAL(triggered()), this, SLOT(edit_settings()));
    // пункт работы с лентами и фильтрами
    menu_feeds_and_filters = new QAction(tr("RSS-ленты"), this);
    menu_feeds_and_filters->setIcon(QIcon(":/img/rss.ico"));
    main_menu->addAction(menu_feeds_and_filters);
    connect(menu_feeds_and_filters, SIGNAL(triggered()), this, SLOT(edit_feeds_and_filters()));
    // пункт выхода из программы
    menu_quit = new QAction("Выход", this);
    menu_quit->setIcon(QIcon(":/img/exit.ico"));
    main_menu->addAction(menu_quit);
    connect(menu_quit, SIGNAL(triggered()), this, SLOT(quit()));

    lang_menu = new QMenu(this);
    lang_menu->setTitle("Язык");
    lang_ru = new QAction(tr("Русский"), this);
    lang_ru->setIcon(QIcon(":/img/ru.ico"));
    lang_menu->addAction(lang_ru);
    connect(lang_ru, SIGNAL(triggered()), this, SLOT(set_lang_ru()));
    lang_en = new QAction(tr("English"), this);
    lang_en->setIcon(QIcon(":/img/en.ico"));
    lang_menu->addAction(lang_en);
    connect(lang_en, SIGNAL(triggered()), this, SLOT(set_lang_en()));

    mainmenubar->addMenu(main_menu);
    mainmenubar->addMenu(lang_menu);
    mainmenubar->show();

    layout = new QVBoxLayout;

    hboxLayout = new QHBoxLayout;
    hint = new QLabel(this);
#ifdef Q_OS_WIN32
    hint->setText("Дождитесь автоматического обновления результатов или нажмите кнопку \"Поиск\"");
#endif
#ifdef Q_OS_ANDROID
    hint->setText("Нажмите \"Поиск\"");
#endif

    hboxLayout->addWidget(hint);
    hboxLayout->addWidget(clearButton);
    hboxLayout->addWidget(fetchButton);
    hboxLayout->addWidget(readButton);

    layout->setMenuBar(mainmenubar);
    layout->addLayout(hboxLayout);
    layout->addWidget(treeWidget);
    setLayout(layout);

    setWindowTitle("DFRSS-Filter " + prog_name_ver);

#ifdef Q_OS_WIN32
    screen_height = 480;
    screen_width = 640;
#endif
    //*
#ifdef Q_OS_ANDROID
    //QScreen *screen = QApplication::screens().at(0);
    screen = QApplication::screens().at(0);
    screen_height = screen->availableSize().height();
    screen_width = screen->availableSize().width();
//    screen_height = screen->size().height();
//    screen_width = screen->size().width();
    // size - размер экрана, availableSize - рабочая область
#endif
//*/
    //if ((screen_height > 100) && (screen_width > 100))  // в андроиде переменные останутся нулями
        resize(screen_width, screen_height);
    // создаем пункты контекстного меню в трее
    tray_quit = new QAction(tr("&Выход"), this);
    tray_quit->setIcon(QIcon(":/img/exit.ico"));
    connect(tray_quit, SIGNAL(triggered()), this, SLOT(quit()));

    // создаем само контекстное меню, добавляем в него созданные только что пункты
    trayIconMenu = new QMenu(this);
    trayIconMenu->addAction(tray_quit);

    trayIcon = new QSystemTrayIcon();
    trayIcon->setIcon(QIcon(":/img/trell.ico"));
    trayIcon->setContextMenu(trayIconMenu); // добавляем контекстное меню
    trayIcon->show();
    // соединяем сигнал активации значка в системном трее со слотом обработки этого события
    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(show_hide(QSystemTrayIcon::ActivationReason)));

    // устанавливаем наш обработчик событий
    installEventFilter(this);

    // первый запуск процедуры поиска
    connect(settings->timer, SIGNAL(timeout()), this, SLOT(fetch()));

    connect(treeWidget, SIGNAL(itemClicked(QTreeWidgetItem*,int)), settings->timer, SLOT(start())); // в случае клика по новости таймер сбрасывается

    this->setWindowIcon(QIcon(":/img/trell.ico"));

    fetch();
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
    settings->timer->stop(); // остановим таймер на время чтения лент
    /* пробег по списку лент происходит быстрее, чем чтение лент
     * поэтому после первого совпадения выходим из цикла, а продолжим по событию finished */
    counter = 0;
    QList <Feed> feeds;
    pFeeds->GetActiveFeedsList(feeds);

    if (feeds.size() == 0)
    {
        settings->timer->start(settings->request_period); // после обновления списка результатов запускаем таймер на нормальный интервал
        return;
    }
    have_news = false; // сбрасываем флаг информирования
    fetchButton->setEnabled(false); // кнопка станет неактивной только если есть хоть одна активная лента
    clearButton->setEnabled(false); // чтобы не сбить работу программы
    readButton->setEnabled(false);  // чтобы не сбить работу программы
    main_menu->setEnabled(false); // чтобы не сбить работу программы
    lang_menu->setEnabled(false);  // чтобы не сбить работу программы
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
    if (statusCode >= 200 && statusCode < 300)
    {
        QByteArray data = currentReply->readAll();
        /* Парсинг бэндкампа обсерается, когда натыкается на джаваскриптовую запись типа
         * for ( var i=0; i < libs.length; i++ )
         * потому как воспринимает знак сравнения как открывающую скобку тега, а пробел после неё как пустой тег и ошибку
         * нужно убрать все подобные пробелы
        */
        if (activeFeed.link.contains("bandcamp", Qt::CaseInsensitive) && !activeFeed.link.contains("feed", Qt::CaseInsensitive))
        {
            parsebandcamp(data);
        }
        else
        {
            xml.addData(data);
            parseXml();
        }
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
        clearButton->setEnabled(false); // чтобы не сбить работу программы
        readButton->setEnabled(false);  // чтобы не сбить работу программы
        main_menu->setEnabled(false); // чтобы не сбить работу программы
        lang_menu->setEnabled(false);  // чтобы не сбить работу программы
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
        for (int i = 0; i < treeWidget->topLevelItemCount(); i++) // пробегаем по всем раскрывающимся записям
        {
            QTreeWidgetItem *test = treeWidget->topLevelItem(i);
            new_num_of_results += test->childCount(); // и считаем общее число дочерних записей
        }
        if (new_num_of_results > num_of_results) // если их стало больше, чем было
        {
            have_news = true; // значит есть новые
            num_of_new_news += new_num_of_results - num_of_results; // считаем кол-во новых новостей
            num_of_results = new_num_of_results; // запомним новое значение для следующей проверки
            //readButton->setStyleSheet("color: red;");
        }

        // если окно свёрнуто или убрано и есть новости - выведем уведомление
        if ((!this->isVisible() || this->isMinimized()) && have_news)
        {
            switch (settings->current_language)
            {
            case 1:
                trayIcon->showMessage("", QString("Есть новости (%1)").arg(num_of_new_news), QSystemTrayIcon::Information, settings->show_period);
                break;
            case 2:
                trayIcon->showMessage("", QString("Have news (%1)").arg(num_of_new_news), QSystemTrayIcon::Information, settings->show_period);
                break;
            default:
                break;
            }
        }
        switch (settings->current_language)
        {
#ifdef Q_OS_WIN32
        case 1:
            hint->setText(QString("Двойной клик по новости откроет её в браузере (Всего новостей: %1, Новых: %2)").arg(num_of_results).arg(num_of_new_news));
            break;
        case 2:
            hint->setText(QString("Double click will open new in browser (Total news: %1, New: %2)").arg(num_of_results).arg(num_of_new_news));
            break;
        default:
            break;
#endif
#ifdef Q_OS_ANDROID
        case 1:
            hint->setText(QString("Новостей: %1, Новых: %2").arg(num_of_results).arg(num_of_new_news));
            break;
        case 2:
            hint->setText(QString("Total: %1, New: %2)").arg(num_of_results).arg(num_of_new_news));
            break;
        default:
            break;
#endif
        }
        fetchButton->setEnabled(true);
        clearButton->setEnabled(true);
        readButton->setEnabled(true);
        main_menu->setEnabled(true);
        lang_menu->setEnabled(true);
        settings->timer->start(settings->request_period); // после обновления списка результатов запускаем таймер на нормальный интервал
    }
}

// Парсит данные XML и создаёт соответственно элементы treeWidget.
void DFRSSFilter::parseXml()
{
    QString str;
    QString has_the_world1 = "тысячелистник";
    QString has_the_world2 = "trelleborg";

    QTreeWidgetItem *feed_item = new QTreeWidgetItem();

    while (!xml.atEnd()) // пока XML не кончился
    {
        xml.readNext();
        if (xml.isStartElement()) // если открывающий элемент - считываем его имя
        {
            currentTag = xml.name().toString();
        }
        else if (xml.isEndElement())
        {
            if (xml.name() == "item") // если закрывающий элемент и это item - записываем
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

                        /*
                        if ((filter.comment.simplified() == "Автопоиск" && str.contains(filter.title.simplified(), Qt::CaseSensitive)) || // найденных исполнителей проверяем по высоте букв
                            (filter.comment.simplified() != "Автопоиск" && str.contains(filter.title.simplified(), Qt::CaseInsensitive)) || // всё остальное - без разницы
                            (str.contains(has_the_world1.simplified(), Qt::CaseInsensitive)) ||
                            (str.contains(has_the_world2.simplified(), Qt::CaseInsensitive)))
                        */
                        QString temp_str1 = filter.title.simplified() + " -"; // единственный исполнитель
                        QString temp_str2 = filter.title.simplified() + " &"; // первый в сплите
                        QString temp_str3 = "& " + filter.title.simplified(); // не первый в сплите

                        if ((activeFeed.link.indexOf("metalarea", 0, Qt::CaseInsensitive) >= 0 && ((str.indexOf(temp_str1, 0, Qt::CaseInsensitive) == 0) ||
                                                                                                                                  (str.indexOf(temp_str2, 0, Qt::CaseInsensitive) == 0) ||
                                                                                                                                  (str.indexOf(temp_str3, 0, Qt::CaseInsensitive) > 0))) ||
                            (activeFeed.link.indexOf("metalarea", 0, Qt::CaseInsensitive) == -1 && str.contains(filter.title.simplified(), Qt::CaseInsensitive)) || // всё остальное - без разницы
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
                                // проверим на совпадение
                                // был случай, когда в новости "Vs" заменили на "vs" и она вывелась второй раз
                                // если только изменился регистр - игнорируем
                                int a = QString::compare(feed_item->child(i)->text(0).trimmed(), item->text(0).trimmed(), Qt::CaseInsensitive); // 0 - если совпали
                                // был случай, когда в ссылкe добавили "https:" и она вывелась второй раз
                                // если изменился заголовок, но адрес тот же - игнорируем
                                int b = QString::compare(feed_item->child(i)->text(1).trimmed(), item->text(1).trimmed(), Qt::CaseInsensitive); // 0 - если совпали
                                int c = 1;
                                // если добавились символы впереди - игнорируем
                                if (feed_item->child(i)->text(1).trimmed().indexOf(item->text(1).trimmed(), Qt::CaseInsensitive) > 0 ||
                                    item->text(1).trimmed().indexOf(feed_item->child(i)->text(1).trimmed(), Qt::CaseInsensitive) > 0)
                                    c = 0;
                                if (a == 0 || b == 0 || c == 0)
                                {
                                    new_new = false;
                                    break;
                                }
                            }

                            if (new_new)
                            {
                                feed_item->addChild(item);
                                feed_item->setHidden(false); // если нашёлся хоть один результат - сделаем "ветку" видимой

                                // выделение новой строки
                                feed_item->setForeground(0,*(new QBrush(Qt::red,Qt::Dense6Pattern)));
                                item->setForeground(0,*(new QBrush(Qt::red,Qt::Dense6Pattern)));
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
                        // проверим на совпадение
                        // был случай, когда в новости "Vs" заменили на "vs" и она вывелась второй раз
                        // если только изменился регистр - игнорируем
                        int a = QString::compare(feed_item->child(i)->text(0).trimmed(), item->text(0).trimmed(), Qt::CaseInsensitive); // 0 - если совпали
                        // был случай, когда в ссылкe добавили "https:" и она вывелась второй раз
                        // если изменился заголовок, но адрес тот же - игнорируем
                        int b = QString::compare(feed_item->child(i)->text(1).trimmed(), item->text(1).trimmed(), Qt::CaseInsensitive); // 0 - если совпали
                        int c = 1;
                        // если добавились символы впереди - игнорируем
                        if (feed_item->child(i)->text(1).trimmed().indexOf(item->text(1).trimmed(), Qt::CaseInsensitive) > 0 ||
                            item->text(1).trimmed().indexOf(feed_item->child(i)->text(1).trimmed(), Qt::CaseInsensitive) > 0)
                            c = 0;
                        if (a == 0 || b == 0 || c == 0)
                        {
                            new_new = false;
                            break;
                        }
                    }

                    if (new_new)
                    {
                        feed_item->addChild(item);
                        feed_item->setHidden(false); // если нашёлся хоть один результат - сделаем "ветку" видимой

                        // выделение новой строки
                        feed_item->setForeground(0,*(new QBrush(Qt::red,Qt::Dense6Pattern)));
                        item->setForeground(0,*(new QBrush(Qt::red,Qt::Dense6Pattern)));
                    }
                }
            }
        }
        else if (xml.isCharacters() && !xml.isWhitespace()) // если внутри тега есть какой-то текст
        {
            if (currentTag == "title")
            {
                titleString.clear(); // очистим перед записью нового значения
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
            {
                linkString.clear(); // очистим перед записью нового значения
                linkString += xml.text().toString();
            }
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
    int red_lines; // кол-во выделенных новостей в ленте
    int total_red_lines = 0; // общее кол-во выделенных новостей во всех лентах

    QDesktopServices::openUrl(QUrl(item->text(1)));
    // если наша запись единственная новая в ленте, снимем выделение и с ленты тоже
    for (int i = 0; i < treeWidget->topLevelItemCount(); i++)
    {
        red_lines = 0;
        for (int j = 0; j < treeWidget->topLevelItem(i)->childCount(); j++)
        {
            // посчитаем количество выделенных строк
            if (treeWidget->topLevelItem(i)->child(j)->foreground(0) == QBrush(Qt::red, Qt::Dense6Pattern))
            {
                red_lines++;
                total_red_lines++;
            }
            // снимем выделение с новости, если она новая (это должно быть здесь, чтобы двойным кликом не снималось выделение с имени ленты)
            if (treeWidget->topLevelItem(i)->child(j)->text(1) == item->text(1) && item->foreground(0) == QBrush(Qt::red,Qt::Dense6Pattern))
            {
                item->setForeground(0,*(new QBrush(Qt::black,Qt::Dense6Pattern)));
                num_of_new_news -= 1;
                red_lines--;
                total_red_lines--;
                switch (settings->current_language)
                {
#ifdef Q_OS_WIN32
                    case 1:
                        hint->setText(QString("Двойной клик по новости откроет её в браузере (Всего новостей: %1, Новых: %2)").arg(num_of_results).arg(num_of_new_news));
                        break;
                    case 2:
                        hint->setText(QString("Double click will open new in browser (Total news: %1, New: %2)").arg(num_of_results).arg(num_of_new_news));
                        break;
                    default:
                        break;
#endif
#ifdef Q_OS_ANDROID
                    case 1:
                        hint->setText(QString("Новостей: %1, Новых: %2").arg(num_of_results).arg(num_of_new_news));
                        break;
                    case 2:
                        hint->setText(QString("Total: %1, New: %2)").arg(num_of_results).arg(num_of_new_news));
                        break;
                    default:
                        break;
#endif
                }
            }
        }
        // если новость была единственной новой в ленте - снимаем выделение с ленты
        if (red_lines <= 0) // на всякий случай - мало ли уйдёт в минус
            treeWidget->topLevelItem(i)->setForeground(0,*(new QBrush(Qt::black,Qt::Dense6Pattern)));
    }
    // если новость была единственной новой - снимем выделение с кнопки
    //if (total_red_lines <= 0) // на всякий случай - мало ли уйдёт в минус
        //readButton->setStyleSheet("color: black;");
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
    //settings->setWindowFlags(Qt::WindowStaysOnTopHint | /*Qt::CustomizeWindowHint | */Qt::WindowTitleHint | Qt::WindowCloseButtonHint/* | Qt::Tool*/);
    settings->setWindowFlags(Qt::Window | Qt::WindowCloseButtonHint | Qt::WindowSystemMenuHint | Qt::CustomizeWindowHint | Qt::WindowStaysOnTopHint);
    // задаём параметры - оставаться поверх всех, пользовательские настройки, показать заголовок, показать кнопку закрытия, убрать иконку из панели задач
    // говорят без Qt::CustomizeWindowHint другие флаги не работают, но почему-то всё работает
    settings->setAttribute(Qt::WA_ShowModal, true);         // блокирует родительское окно
    settings->show();                                       // и рисуем его
}

// открытие окна работы с лентами и фильтрами
void DFRSSFilter::edit_feeds_and_filters()
{
    //FeedsAndFilters *win = new FeedsAndFilters(this);
    if (!feedsandfilters)
        feedsandfilters = new FeedsAndFilters(this);
    feedsandfilters->setWindowFlags(Qt::Window | Qt::WindowCloseButtonHint | Qt::WindowSystemMenuHint | Qt::CustomizeWindowHint | Qt::WindowStaysOnTopHint);
    // задаём параметры - оставаться поверх всех, пользовательские настройки, показать заголовок, показать кнопку закрытия, убрать иконку из панели задач
    // говорят без Qt::CustomizeWindowHint другие флаги не работают, но почему-то всё работает
    feedsandfilters->setAttribute(Qt::WA_ShowModal, true);         // блокирует родительское окно
    feedsandfilters->show();                                       // и рисуем его
}

// выход из программы
void DFRSSFilter::quit()
{
    exit(0);
}

// очистка окна программы
void DFRSSFilter::clear_results()
{
    treeWidget->clear();
    num_of_new_news = 0; // сбросим кол-во новых новостей
    num_of_results = 0; // сбросим количество выведенных новостей
    //readButton->setStyleSheet("color: black;"); // снимем выделение с кнопки (если есть)
    switch (settings->current_language)
    {
#ifdef Q_OS_WIN32
    case 1:
        hint->setText("Дождитесь автоматического обновления результатов или нажмите кнопку \"Поиск\"");
        break;
    case 2:
        hint->setText("Wait until results appear automatically or press \"Search\" button");
        break;
    default:
        break;
#endif
#ifdef Q_OS_ANDROID
    case 1:
        hint->setText("");
        break;
    case 2:
        hint->setText("Wait or press \"Search\"");
        break;
    default:
        break;
#endif
    }
}

// снятие выделения с новых новостей
void DFRSSFilter::unlight()
{
    for (int i = 0; i < treeWidget->topLevelItemCount(); i++) // пробегаем по всем раскрывающимся записям
    {
        treeWidget->topLevelItem(i)->setForeground(0,*(new QBrush(Qt::black,Qt::Dense6Pattern)));
        for (int j = 0; j < treeWidget->topLevelItem(i)->childCount(); j++)
        {
            treeWidget->topLevelItem(i)->child(j)->setForeground(0,*(new QBrush(Qt::black,Qt::Dense6Pattern)));
        }
    }
    num_of_new_news = 0;
    //readButton->setStyleSheet("color: black;");
    switch (settings->current_language)
    {
#ifdef Q_OS_WIN32
        case 1:
            hint->setText(QString("Двойной клик по новости откроет её в браузере (Всего новостей: %1, Новых: %2)").arg(num_of_results).arg(num_of_new_news));
            break;
        case 2:
            hint->setText(QString("Double click will open new in browser (Total news: %1, New: %2)").arg(num_of_results).arg(num_of_new_news));
            break;
        default:
            break;
#endif
#ifdef Q_OS_ANDROID
        case 1:
            hint->setText(QString("Новостей: %1, Новых: %2").arg(num_of_results).arg(num_of_new_news));
            break;
        case 2:
            hint->setText(QString("Total: %1, New: %2)").arg(num_of_results).arg(num_of_new_news));
            break;
        default:
            break;
#endif
    }
}

// функция парсинга теговых страниц бэндкампа
void DFRSSFilter::parsebandcamp(QByteArray bandcampdata)
{
    QString title, artist, album;
    int i, start_position = 0, current_position = 0;
    QString str, tempstr;
    QString has_the_world1 = "тысячелистник";
    QString has_the_world2 = "trelleborg";

    QTreeWidgetItem *feed_item = new QTreeWidgetItem();

    title.clear();
    for (i = bandcampdata.indexOf("<title>", 0) + QString("<title>").length(); i < bandcampdata.indexOf("</title>", 0); i++)
        title += bandcampdata[i]; // выцепляем имя ленты

    // костыль (забавно, но случай с именованной лентой так тоже обработается)
    if (title != "")
    {
        bandcamp_sucks.clear();
        bandcamp_sucks += title;
    }
    else
    {
        title += bandcamp_sucks;
    }

    if (need_a_name || (title == bandcamp_sucks)) // если это первый проход и мы должны задать имя ленте + костыль
    {
        bool new_feed = true;
        QTextDocument doc;
        doc.setHtml(QString(title));
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
    // загружаем фильтры
    QList<filters_struct> filters;
    pFeeds->GetActiveFiltersList(filters, activeFeed.id);
    // отсекаем всё, что до релизов - всякие java-скрипты и прочее дерьмо
    start_position = bandcampdata.indexOf("item_list", 0) + QString("item_list").length();
    tempstr.clear();
    for (i = start_position; i < bandcampdata.size(); i++)
        tempstr += bandcampdata[i];
    while (tempstr.indexOf("<li class=\"item ", 0) != -1) // пока есть элементы
    {
        titleString.clear();
        linkString.clear();
        current_position = tempstr.indexOf("href=\"", 0) + QString("href=\"").length();
        for (i = current_position; i < tempstr.indexOf("\"", current_position); i++)
            linkString += tempstr[i];
        artist.clear();
        current_position = tempstr.indexOf("itemsubtext\">", 0) + QString("itemsubtext\">").length();
        for (i = current_position; i < tempstr.indexOf("<", current_position); i++)
            artist += tempstr[i];
        album.clear();
        current_position = tempstr.indexOf("itemtext\">", 0) + QString("itemtext\">").length();
        for (i = current_position; i < tempstr.indexOf("<", current_position); i++)
            album += tempstr[i];
        titleString = artist + " - " + album;

        // вывод
        if (filters.size() != 0) // если есть фильтры - ищем
        {
            foreach (filters_struct filter, filters)
            {
                // добавим обработку разных языковых символов
                QTextDocument doc;
                doc.setHtml(titleString);
                str = doc.toPlainText();

                if ((filter.comment.simplified() == "Автопоиск" && str.contains(filter.title.simplified(), Qt::CaseSensitive)) || // найденных исполнителей проверяем по высоте букв
                    (filter.comment.simplified() != "Автопоиск" && str.contains(filter.title.simplified(), Qt::CaseInsensitive)) || // всё остальное - без разницы
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
                        // проверим на совпадение
                        // был случай, когда в новости "Vs" заменили на "vs" и она вывелась второй раз
                        // если только изменился регистр - игнорируем
                        int a = QString::compare(feed_item->child(i)->text(0).trimmed(), item->text(0).trimmed(), Qt::CaseInsensitive); // 0 - если совпали
                        // был случай, когда в ссылкe добавили "https:" и она вывелась второй раз
                        // если изменился заголовок, но адрес тот же - игнорируем
                        int b = QString::compare(feed_item->child(i)->text(1).trimmed(), item->text(1).trimmed(), Qt::CaseInsensitive); // 0 - если совпали
                        int c = 1;
                        // если добавились символы впереди - игнорируем
                        if (feed_item->child(i)->text(1).trimmed().indexOf(item->text(1).trimmed(), Qt::CaseInsensitive) > 0 ||
                            item->text(1).trimmed().indexOf(feed_item->child(i)->text(1).trimmed(), Qt::CaseInsensitive) > 0)
                            c = 0;
                        if (a == 0 || b == 0 || c == 0)
                        {
                            new_new = false;
                            break;
                        }
                    }

                    if (new_new)
                    {
                        feed_item->addChild(item);
                        feed_item->setHidden(false); // если нашёлся хоть один результат - сделаем "ветку" видимой

                        // выделение новой строки
                        feed_item->setForeground(0,*(new QBrush(Qt::red,Qt::Dense6Pattern)));
                        item->setForeground(0,*(new QBrush(Qt::red,Qt::Dense6Pattern)));
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
                // проверим на совпадение
                // был случай, когда в новости "Vs" заменили на "vs" и она вывелась второй раз
                // если только изменился регистр - игнорируем
                int a = QString::compare(feed_item->child(i)->text(0).trimmed(), item->text(0).trimmed(), Qt::CaseInsensitive); // 0 - если совпали
                // был случай, когда в ссылкe добавили "https:" и она вывелась второй раз
                // если изменился заголовок, но адрес тот же - игнорируем
                int b = QString::compare(feed_item->child(i)->text(1).trimmed(), item->text(1).trimmed(), Qt::CaseInsensitive); // 0 - если совпали
                int c = 1;
                // если добавились символы впереди - игнорируем
                if (feed_item->child(i)->text(1).trimmed().indexOf(item->text(1).trimmed(), Qt::CaseInsensitive) > 0 ||
                    item->text(1).trimmed().indexOf(feed_item->child(i)->text(1).trimmed(), Qt::CaseInsensitive) > 0)
                    c = 0;
                if (a == 0 || b == 0 || c == 0)
                {
                    new_new = false;
                    break;
                }
            }

            if (new_new)
            {
                feed_item->addChild(item);
                feed_item->setHidden(false); // если нашёлся хоть один результат - сделаем "ветку" видимой

                // выделение новой строки
                feed_item->setForeground(0,*(new QBrush(Qt::red,Qt::Dense6Pattern)));
                item->setForeground(0,*(new QBrush(Qt::red,Qt::Dense6Pattern)));

            }
        }

        // отбрасываем обработанный элемент
        current_position = tempstr.indexOf("</li>", 0) + QString("</li>").length(); // находим закрывающий элемент тег
        start_position += current_position; // смещаем точку старта
        tempstr.clear(); // очищаем рабочую строку
        for (i = start_position; i < bandcampdata.size(); i++)
            tempstr += bandcampdata[i]; // и переписываем её

    }
}

void DFRSSFilter::set_language(int language)
{
    switch (language)
    {
    case 1:
        main_menu->setTitle("Меню");
        menu_settings->setText("Настройки");
        menu_feeds_and_filters->setText("RSS-ленты");
        menu_quit->setText("Выход");
        lang_menu->setTitle("Язык");
        lang_ru->setText("Русский");
        lang_en->setText("English");
        fetchButton->setText("Поиск");
        clearButton->setText("Очистить");
        readButton->setText("Прочитано");
#ifdef Q_OS_WIN32
        if (num_of_new_news > 0)
            hint->setText(QString("Двойной клик по новости откроет её в браузере (Всего новостей: %1, Новых: %2)").arg(num_of_results).arg(num_of_new_news));
        else if (num_of_results > 0)
            hint->setText(QString("Двойной клик по новости откроет её в браузере (Всего новостей: %1, Новых: 0)").arg(num_of_results));
        else
            hint->setText("Дождитесь автоматического обновления результатов или нажмите кнопку \"Поиск\"");
#endif
#ifdef Q_OS_ANDROID
        if (num_of_new_news > 0)
            hint->setText(QString("Новостей: %1, Новых: %2)").arg(num_of_results).arg(num_of_new_news));
        else if (num_of_results > 0)
            hint->setText(QString("Новостей: %1, Новых: 0)").arg(num_of_results));
        else
            hint->setText("");
#endif
       tray_quit->setText("Выход");
        update();
        break;
    case 2:
        main_menu->setTitle("Menu");
        menu_settings->setText("Settings");
        menu_feeds_and_filters->setText("RSS-feeds");
        menu_quit->setText("Quit");
        lang_menu->setTitle("Language");
        lang_ru->setText("Русский");
        lang_en->setText("English");
        fetchButton->setText("Search");
        clearButton->setText("Clear");
        readButton->setText("Checked");
#ifdef Q_OS_WIN32
        if (num_of_new_news > 0)
            hint->setText(QString("Double click will open new in browser (Total news: %1, New: %2)").arg(num_of_results).arg(num_of_new_news));
        else if (num_of_results > 0)
            hint->setText(QString("Double click will open new in browser (Total news: %1, New: 0)").arg(num_of_results));
        else
            hint->setText("Wait until results appear automatically or press \"Search\" button");
#endif
#ifdef Q_OS_ANDROID
        if (num_of_new_news > 0)
            hint->setText(QString("Total: %1, New: %2)").arg(num_of_results).arg(num_of_new_news));
        else if (num_of_results > 0)
            hint->setText(QString("Total: %1, New: 0)").arg(num_of_results));
        else
            hint->setText("Wait or press \"Search\"");
#endif
        tray_quit->setText("Quit");
        break;
    default:
        break;
    }
}

void DFRSSFilter::set_lang_ru()
{
    settings->current_language = 1;
    set_language(settings->current_language);
}

void DFRSSFilter::set_lang_en()
{
    settings->current_language = 2;
    set_language(settings->current_language);
}

void DFRSSFilter::grabGestures(const QList<Qt::GestureType> &gestures)
{
    feedsandfilters->grabGestures(gestures);
}
