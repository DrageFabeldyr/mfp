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
int num_of_feeds;
QStringList old_feeds;
bool win_norm, win_max;
QString feed_name;
int request_period = 1*60*1000, show_period = 30*1000;


// обработчик событий
bool DFRSSFilter::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::WindowStateChange && obj != NULL)
    {
        if (this->windowState() & Qt::WindowMinimized) // "==" почему-то не работает, а "&" работает
        {
            if (static_cast<QWindowStateChangeEvent*>(event)->oldState() == Qt::WindowNoState)
            {
                win_norm = true;
                win_max = false;
            }
            if (static_cast<QWindowStateChangeEvent*>(event)->oldState() == Qt::WindowMaximized)
            {
                win_norm = false;
                win_max = true;
            }
            if (nocover->isChecked())
                this->hide();
        }
    }
    if (event->type() == QEvent::Close && obj != NULL)
    {
        if (noclose->isChecked())
        {
            if (this->isMaximized())
            {
                win_norm = false;
                win_max = true;
            }
            else
            {
                win_norm = true;
                win_max = false;
            }
            this->hide();
            event->ignore();
            return true; // чтобы событие сбросилось из очереди, иначе его обработает стандартный eventClose
        }
        else
            write_settings_and_quit();
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
            if (win_norm)
                this->showNormal();
            if (win_max)
                this->showMaximized();

            this->activateWindow(); // делает окно активным
        }
        else
        {
            if (this->isMaximized())
            {
                win_norm = false;
                win_max = true;
            }
            else
            {
                win_norm = true;
                win_max = false;
            }
            this->hide(); //иначе скрываем
        }
    }
}

void DFRSSFilter::set_feed(QAction *action_name)
{
    lineEdit->setText(action_name->text().trimmed());
    // trimmed обрезает \n в конце строки, иначе он превращается в пробел в строке поиска и в лишнюю строку в файле
}

void DFRSSFilter::refreshlastfeeds(QMenu *menu)
{
    QAction *feedsaction;
    menu->clear();
    QString actionname;

    for (int  i = 0; i < num_of_feeds; i++)
    {
        feedsaction = new QAction(old_feeds.at(i), this);
        menu->addAction(feedsaction);
        connect(feedsaction, &QAction::triggered, [this, feedsaction]() { set_feed(feedsaction); });
        // тут используется "лямбда" - что это такое я не очень понимаю
    }
}

/*
    Конструирует виджет RSSListing с простым интерфейсом пользователя и устанавливает
    ридер XML для использования пользовательского класса обработки.

    Интерфейс пользователя состоит из строки ввода, кнопки и
    виджета просмотра списка. Строка ввода используется для ввода URL источника новостей;
    источников; кнопка начинает процесс чтения новостей.
*/
DFRSSFilter::DFRSSFilter(QWidget *parent) : QWidget(parent), currentReply(0)
{
    read_filters(); // считываем фильтры
    read_feeds(); // считываем ленты
    win_norm = true; // окно обычного размера
    win_max = false; // окно не развёрнуто

    lineEdit = new QLineEdit(this);
    lineEdit->setPlaceholderText("Введите адрес ленты или используйте недавние");

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
    connect(lineEdit, SIGNAL(returnPressed()), this, SLOT(fetch()));
    connect(fetchButton, SIGNAL(clicked()), this, SLOT(fetch()));

    // создание основного меню программы
    mainmenubar = new QMenuBar(this);
    mainmenu = new QMenu(this);
    mainmenu->setTitle("Недавние ленты");
    refreshlastfeeds(mainmenu);
    old_settings_menu = new QMenu(this);
    old_settings_menu->setTitle("Старые настройки");
    nocover = new QAction(tr("&Сворачивать в трей"), this);
    nocover->setCheckable(true);
    old_settings_menu->addAction(nocover);
    noclose = new QAction(tr("&Закрывать в трей"), this);
    noclose->setCheckable(true);
    old_settings_menu->addAction(noclose);

    int set = read_settings();
    switch (set)
    {
    case 0:
        nocover->setChecked(true);
        noclose->setChecked(true);
        break;
    case 1:
        nocover->setChecked(false);
        noclose->setChecked(true);
        break;
    case 2:
        nocover->setChecked(true);
        noclose->setChecked(false);
        break;
    case 3:
        nocover->setChecked(false);
        noclose->setChecked(false);
        break;
    default:
        nocover->setChecked(true);
        noclose->setChecked(true);
        break;
    }

    settings_menu = new QMenu(this);
    settings_menu->setTitle("Меню");
    open_settings = new QAction(tr("Настройки"), this);
    // нужно обязательно создавать action, потому что menu при нажатии не отрабатывает, а раскрывается
    settings_menu->addAction(open_settings);
    connect(open_settings, SIGNAL(triggered()), this, SLOT(set_settings()));
    open_feeds = new QAction(tr("RSS-ленты"), this);
    settings_menu->addAction(open_feeds);
    connect(open_feeds, SIGNAL(triggered()), this, SLOT(edit_feeds()));
    open_filters = new QAction(tr("Фильтры"), this);
    settings_menu->addAction(open_filters);
    connect(open_filters, SIGNAL(triggered()), this, SLOT(addfilter()));

    mainmenubar->addMenu(mainmenu);
    mainmenubar->addMenu(old_settings_menu);
    mainmenubar->addMenu(settings_menu);
    mainmenubar->show();

    QVBoxLayout *layout = new QVBoxLayout;

    QHBoxLayout *hboxLayout = new QHBoxLayout;
    statusbar = new QStatusBar(this);
    statusbar->showMessage("Добро пожаловать =))");

    hboxLayout->addWidget(lineEdit);
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
    connect(quitAction, SIGNAL(triggered()), this, SLOT(write_settings_and_quit()));

    // создаем само контекстное меню, добавляем в него созданные только что пункты
    trayIconMenu = new QMenu(this);
    trayIconMenu->addAction(quitAction);

    trayIcon = new QSystemTrayIcon();
    trayIcon->setIcon(QIcon("trell.gsd"));
    trayIcon->setContextMenu(trayIconMenu); // добавляем контекстное меню
    trayIcon->show();
    // соединяем сигнал активации значка в систсемном трее со слотом обработки этого события
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
    treeWidget->clear();
    for (int i = 0; i < feeds.size(); i++)
        if (feeds.at(i).is_on)
        {
            xml.clear();
            QUrl url(feeds.at(i).link);
            get(url);
            statusbar->showMessage(QString("Количество применённых фильтров: %1").arg(num_of_filters));
        }
    /*
    if (!lineEdit->text().isEmpty())
    {
        need_a_name = true; // установка флага на переименование окна

        lineEdit->setReadOnly(true);
        fetchButton->setEnabled(false);
        treeWidget->clear();

        xml.clear();

        QUrl url(lineEdit->text());

        get(url);

        add_feed(lineEdit->text()); // запись ленты (при необходимости)
        refreshlastfeeds(mainmenu); // перестроение списка последних лент

        statusbar->showMessage(QString("Количество применённых фильтров: %1").arg(num_of_filters));
    }
    */
}

void DFRSSFilter::metaDataChanged()
{
    QUrl redirectionTarget = currentReply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
    if (redirectionTarget.isValid())
    {
        get(redirectionTarget);
    }
}

/*
    Считывает данные, полученные из источника RDF.

    Мы считываем все доступные данные и передаём их потоковому XML
    ридеру. Затем мы вызываем функцию парсинга XML.
*/
void DFRSSFilter::readyRead()
{
    int statusCode = currentReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (statusCode >= 200 && statusCode < 300) {
        QByteArray data = currentReply->readAll();
        xml.addData(data);
        parseXml();
    }
}

/*
    Заканчивает обработку запроса HTTP.

    Поведение по умолчанию сохраняет редактор текста в режиме только для чтения.

    Если происходит ошибка, интерфейс пользователя становится доступен
    пользователю для дальнейшего ввода, позволяя новой скачке начаться.

    Если запрос получения HTTP завершается, мы делаем
    интерфейс пользователя доступным пользователю для дальнейшего ввода.
*/
void DFRSSFilter::finished(QNetworkReply *reply)
{
    Q_UNUSED(reply);
    lineEdit->setReadOnly(false);
    fetchButton->setEnabled(true);
}

// Парсит данные XML и создаёт соответственно элементы treeWidget.
void DFRSSFilter::parseXml()
{
    QString str;

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
                if (num_of_filters > 0)
                {
                    for (int i = 0; i < num_of_filters; i++)
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

                            treeWidget->addTopLevelItem(item);
                            titleString.clear();
                            linkString.clear();
                            if (!this->isVisible() || this->isMinimized())
                            {
                                // если окно свёрнуто или убрано - выведем уведомление
                                trayIcon->showMessage("", "Есть новости", QSystemTrayIcon::Information, show_period);
                            }
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

                    treeWidget->addTopLevelItem(item);
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
                    feed_name.clear();
                    feed_name = titleString;
                    setWindowTitle(titleString);
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
void DFRSSFilter::addfilter()
{
    filter *new_filter = new filter();
    new_filter->setWindowFlags(Qt::WindowStaysOnTopHint | /*Qt::CustomizeWindowHint | */Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
    // задаём параметры - оставаться поверх всех, пользовательские настройки, показать заголовок, показать кнопку закрытия
    // говорят без Qt::CustomizeWindowHint другие флаги не работают, но почему-то всё работает
    new_filter->setAttribute(Qt::WA_ShowModal, true);         // блокирует родительское окно
    new_filter->show();                                      // и рисуем его
}

// для вывода из памяти последних просмотренных rss-лент, для удобства
void DFRSSFilter::read_feeds()
{
    QFile file("feeds.gsd"); // создаем объект класса QFile
    int  i = 0;
    if(file.open(QIODevice::ReadOnly |QIODevice::Text)) // если файл открылся и там текст
    {
        while(!file.atEnd()) // пока не упрёмся в конец файла
        {
            QString str = file.readLine(); // читаем строку
            old_feeds << str;

            feeds_struct temp;  // создаём элемент типа
            temp.link = str;    // заносим в него данные
            feeds.push_back(temp); // запихиваем элемент в контейнер

            i++;
        }
        file.close(); // прочли и закрыли
    }
    num_of_feeds = i;
}

// для запоминания последних просмотренных rss-лент, для удобства
void DFRSSFilter::write_feeds()
{
    QFile file("feeds.gsd"); // создаем объект класса QFile
    if(file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) // открываем для записи и предварительно полностью обнуляем файл
    {
        QTextStream outStream(&file);

        for (int i = 0; i < num_of_feeds; i++)
            outStream << old_feeds.at(i);
        file.close(); // записали и закрыли
    }
}

// добавление ленты в список, при необходимости
void DFRSSFilter::add_feed(QString str)
{
    bool add_this = true;
    int i = 0, old_feed_pos;
    QStringList temp_list;

    for (i = 0; i < num_of_feeds; i++)
        if (QString::compare(str, old_feeds.at(i).simplified()) == 0)
        {
            add_this = false;
            old_feed_pos = i; // запоминаем позицию ленты, которая "повторилась"
        }

    if (add_this) // если лента новая - надо добавить
    {
        temp_list << QString("%1\n").arg(str); // чтобы добавить перенос строки в конец нашей ссылки
        for (i = 0; i < num_of_feeds; i++)
            temp_list << old_feeds.at(i);
        num_of_feeds++;
        if (num_of_feeds > 10)
            num_of_feeds = 10;
        old_feeds.clear();
        for (i = 0; i < num_of_feeds; i++)
            old_feeds << temp_list.at(i);
    }
    else // если уже есть в списке - перенести вверх
    {
        temp_list << old_feeds.at(old_feed_pos);
        for (i = 0; i < num_of_feeds; i++)
            if (i != old_feed_pos)
                temp_list << old_feeds.at(i);
        old_feeds.clear();
        for (i = 0; i < num_of_feeds; i++)
            old_feeds << temp_list.at(i);
    }
    write_feeds();
}

// чтение сохранённых настроек
int DFRSSFilter::read_settings()
{
    QString str;

    QFile file("setts.gsd"); // создаем объект класса QFile
    if(file.open(QIODevice::ReadOnly |QIODevice::Text)) // если файл открылся и там текст
    {
        while(!file.atEnd()) // пока не упрёмся в конец файла
            str = file.readLine(); // читаем строку
        file.close(); // прочли и закрыли
        int i = str.toInt();
        return i;
    }
    return 0;
}

// сохранение настроек и выход
void DFRSSFilter::write_settings_and_quit()
{
    QFile file("setts.gsd"); // создаем объект класса QFile
    if(file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) // открываем для записи и предварительно полностью обнуляем файл
    {
        QTextStream outStream(&file);

        if (nocover->isChecked() && noclose->isChecked())
            outStream << "0";
        if (!nocover->isChecked() && noclose->isChecked())
            outStream << "1";
        if (nocover->isChecked() && !noclose->isChecked())
            outStream << "2";
        if (!nocover->isChecked() && !noclose->isChecked())
            outStream << "3";

        file.close(); // записали и закрыли
    }
    exit(0);
}

// открытие окна работы с настройками
void DFRSSFilter::set_settings()
{
    settings *sett = new settings;
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
