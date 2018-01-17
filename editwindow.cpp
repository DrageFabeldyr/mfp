#include "feedsandfilters.h"
#include "editwindow.h"

#include "artist_scanner.h"

#include <QFileDialog>
//#include "dfrssfilter.h"


EditWindow::EditWindow(QWidget *parent) : QWidget(parent)
{
    pFeeds = static_cast<FeedsAndFilters*>(parent)->pFeeds;
    //settings = static_cast<FeedsAndFilters*>(parent)->settings; // приводим тип, т.к. parent у нас QWidget
}

EditWindow::EditWindow(const EditWindow::Type type, const int id, QWidget *parent, bool select) :
    QWidget(parent), type(type), id(id), select(select)
{
    pFeeds = static_cast<FeedsAndFilters*>(parent)->pFeeds; // приводим тип, т.к. parent у нас QWidget
    idFeed = static_cast<FeedsAndFilters*>(parent)->parentFeedId; // приводим тип, т.к. parent у нас QWidget
    settings = static_cast<FeedsAndFilters*>(parent)->settings; // приводим тип, т.к. parent у нас QWidget

    nameLabel = new QLabel;
    valueLabel = new QLabel;
    nameEdit = new QLineEdit;
    valueEdit = new QLineEdit;
    enableCheck = new QCheckBox;
    okButton = new QPushButton;
    cancelButton = new QPushButton;
    artistsButton = new QPushButton;

    artistsButton->setVisible(false);

    mainlayout = new QVBoxLayout;

    buttons_layout = new QHBoxLayout;
    buttons_layout->addWidget(okButton);
    buttons_layout->addWidget(cancelButton);

    filters_layout = new QHBoxLayout;
    filters_layout->addWidget(enableCheck);
    filters_layout->addWidget(artistsButton);

    hint = new QLabel(this);
    hint->setText("");

    connect(okButton, SIGNAL(clicked(bool)), this, SLOT(SaveData()));
    connect(cancelButton, SIGNAL(clicked(bool)), this, SLOT(close()));
    connect(artistsButton, SIGNAL(clicked(bool)), this, SLOT(search_artists()));
    connect(nameEdit, SIGNAL(returnPressed()), this, SLOT(SaveData()));
    connect(valueEdit, SIGNAL(returnPressed()), this, SLOT(SaveData()));

    v_layout = new QVBoxLayout;
    v_layout->addLayout(filters_layout);
    v_layout->addWidget(nameLabel);
    v_layout->addWidget(nameEdit);
    v_layout->addWidget(valueLabel);
    v_layout->addWidget(valueEdit);

    mainlayout->addLayout(v_layout);
    mainlayout->addLayout(buttons_layout);
    mainlayout->addWidget(hint);

    setLayout(mainlayout);



    setWindowFlags(Qt::Window
                             | Qt::WindowCloseButtonHint
                             | Qt::WindowSystemMenuHint
                             | Qt::CustomizeWindowHint
                             | Qt::WindowStaysOnTopHint);

#ifdef Q_OS_WIN32
    resize(300, 150);
#endif
    if (id == -1)
        adding = true;
    else
        adding = false;
    set_text();

    okButton->setText(text_button_ok);
    cancelButton->setText(text_button_cancel);
    artistsButton->setText(text_button_artists);
    enableCheck->setText(text_button_check);

    if (type == EditWindow::enFilter)
    {
        artistsButton->setVisible(true);
        this->setWindowIcon(QIcon(":/img/filter.ico")); // если фильтр - ставим значок фильтра
        filters_struct filter(id);
        if (id != -1)
        {
            setWindowTitle(text_title_edit_filter);
            pFeeds->GetFilterById(&filter);
            enableCheck->setChecked(filter.is_on); // если редактирование - берём из БД
        }
        else
        {
            setWindowTitle(text_title_add_filter);
            enableCheck->setChecked(select); // если добавление - из настроек
        }
        nameLabel->setText(text_namelabel_filter);
        valueLabel->setText(text_valuelabel_filter);
        nameEdit->setText(filter.title);
        valueEdit->setText(filter.comment);
    }
    else
    {
        this->setWindowIcon(QIcon(":/img/rss.ico")); // если лента - ставим значок ленты
        Feed feed(id);
        if (id != -1)
        {
            setWindowTitle(text_title_edit_feed);
            pFeeds->GetFeedById(&feed);
            enableCheck->setChecked(feed.is_on); // если редактирование - берём из БД
        }
        else
        {
            setWindowTitle(text_title_add_feed);
            enableCheck->setChecked(select); // если добавление - из настроек
            nameEdit->setPlaceholderText(text_placeholder);
        }
        nameLabel->setText(text_namelabel_feed);
        valueLabel->setText(text_valuelabel_feed);
        nameEdit->setText(feed.title);
        valueEdit->setText(feed.link);
    }
}

EditWindow::~EditWindow()
{
    delete nameLabel;
    delete valueLabel;
    delete nameEdit;
    delete valueEdit;
    delete enableCheck;
    delete okButton;
    delete cancelButton;
    delete artistsButton;
    delete hint;

    delete filters_layout;
    delete v_layout;
    delete buttons_layout;
    delete mainlayout;
}

void EditWindow::SaveData()
{
    if (type == enFeed)
    {
        QString feed_link = valueEdit->text().trimmed();
        if (!(feed_link.contains("http", Qt::CaseInsensitive) && feed_link.contains("://")) || (feed_link.count(QRegExp("//")) != 1))
        {
            hint->setText(text_error_link);
            return; // сразу выходит из функции
        }
        if (valueEdit->text().isEmpty()) // если поле адреса ленты пустое
        {
            hint->setText(text_error_empty);
            return; // сразу выходит из функции
        }
        Feed feed(id);
        feed.link = valueEdit->text().trimmed();
        QString feed_name = nameEdit->text().trimmed();
        if (feed_name.length() == 0)
        {
            int start = feed_link.indexOf("//") + 2;
            int end = feed_link.indexOf(".");
            feed_name = feed_link.mid(start, end - start);
        }
        feed.title = feed_name;
        feed.is_on = enableCheck->checkState();
        pFeeds->SaveFeed(&feed);

        if (adding)
        {
            valueEdit->clear();
            nameEdit->clear();
            added_values++;
            hint->setText(text_added_feeds + QString("%1").arg(added_values));
        }
    }
    else
    {
        if (nameEdit->text().isEmpty()) // если поле имени фильтра пустое
        {
            hint->setText(text_error_empty);
            return; // сразу выходит из функции
        }
        filters_struct filter(id);
        filter.comment = valueEdit->text().trimmed();
        filter.title = nameEdit->text().trimmed();
        filter.is_on = enableCheck->checkState();
        filter.idFeed = idFeed;
        pFeeds->SaveFilter(&filter);

        if (adding)
        {
            valueEdit->clear();
            nameEdit->clear();
            added_values++;
            hint->setText(text_added_filters + QString("%1").arg(added_values));
        }
    }
    set_focus_line(); // вернём фокус в поле ввода
    // если редактировали запись - закрываем окно
    if (!adding)
        close();
}

void EditWindow::showEvent(QShowEvent * event)
{
    Q_UNUSED(event);
    set_focus_line();
    added_values = 0;
}

void EditWindow::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event);
    this->deleteLater();
}

void EditWindow::search_artists()
{
        QString path = QFileDialog::getExistingDirectory(this,
                                   //QString::fromUtf8("Выбрать папку"),
                                   text_search_dialog,
                                   QDir::currentPath(),
                                   QFileDialog::ShowDirsOnly
                                   | QFileDialog::DontResolveSymlinks);
        enableCheck->setEnabled(false);
        nameEdit->setText(path);
        nameLabel->setText(text_search_folder);
        valueEdit->setText(text_search_process);
        nameEdit->setEnabled(false);
        valueEdit->setEnabled(false);
        okButton->setEnabled(false);
        artistsButton->setEnabled(false);

        // запускаем поток поиска
        QThread *thread= new QThread;
        artist_scanner *my = new artist_scanner;

        my->moveToThread(thread);

        connect(my, SIGNAL(send(int, int, bool)), this, SLOT(update(int, int, bool)));
        connect(thread, SIGNAL(started()), my, SLOT(searching()));
        connect(this, SIGNAL(send_params(QString, int, int, bool)), my, SLOT(receive_params(QString, int, int, bool)), Qt::DirectConnection);

        emit send_params(path, id, idFeed, enableCheck->checkState()); // путь к папке, id ленты, галочка активности

        thread->start();
}

void EditWindow::update(int i, int j, bool flag)
{
    switch (settings->current_language)
    {
    case 1:
        hint->setText(QString("Найдено исполнителей: %1, Файлов проверено: %2").arg(i).arg(j));
        break;
    case 2:
        hint->setText(QString("Artists found: %1, Files checked: %2").arg(i).arg(j));
        break;
    default:
        break;
    }

    if (flag)
    {
        valueEdit->setText(text_search_finished);
        cancelButton->setText("ОК");
        cancelButton->setEnabled(true);
    }
}

void EditWindow::set_text()
{
    switch (settings->current_language)
    {
    case 1:
        if (adding)
            text_button_ok = "Добавить";
        else
            text_button_ok = "Сохранить";
        text_button_cancel = "Отмена";
        text_button_artists = "Поиск исполнителей";
        text_button_check = "Активность";
        text_title_add_feed = "Добавление ленты";
        text_title_add_filter = "Добавление фильтра";
        text_title_edit_feed = "Редактирование ленты";
        text_title_edit_filter = "Редактирование фильтра";
        text_namelabel_feed = "Имя ленты:";
        text_valuelabel_feed = "Ссылка:";
        text_namelabel_filter = "Фильтр: (регистр не имеет значения)";
        text_valuelabel_filter = "Комментарий:";
        text_placeholder = "Если не вводить, будет взято из ленты";
        text_error_link = "Ошибка в адресе ленты";
        text_error_empty = "Нельзя добавить пустое значение";
        text_search_dialog = "Выбрать папку";
        text_search_folder = "Выбрана папка:";
        text_search_process = "Идёт поиск";
        text_search_finished = "Поиск окончен";
        text_added_feeds = "Лент добавлено: ";
        text_added_filters = "Фильтров добавлено: ";
        break;
    case 2:
        if (adding)
            text_button_ok = "Add";
        else
            text_button_ok = "Save";
        text_button_cancel = "Cancel";
        text_button_artists = "Search artists";
        text_button_check = "Active";
        text_title_add_feed = "Feed adding";
        text_title_add_filter = "Filter adding";
        text_title_edit_feed = "Feed editing";
        text_title_edit_filter = "Filter editing";
        text_namelabel_feed = "Feed name:";
        text_valuelabel_feed = "URL:";
        text_namelabel_filter = "Filter: (case unsensitive)";
        text_valuelabel_filter = "Comment:";
        text_placeholder = "If empty will be taken from URL";
        text_error_link = "URL error";
        text_error_empty = "Can't add empty value";
        text_search_dialog = "Choose folder";
        text_search_folder = "Music folder:";
        text_search_process = "Searching...";
        text_search_finished = "Searching finished";
        text_added_feeds = "New feeds: ";
        text_added_filters = "New filters: ";
        break;
    default:
        break;
    }
}

void EditWindow::set_focus_line()
{
    if (type == enFeed)
    {
        valueEdit->setFocus();
    }
    else
    {
        nameEdit->setFocus();
    }
}
