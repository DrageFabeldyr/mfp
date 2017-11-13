#include "feedsandfilters.h"
#include "editwindow.h"

#include "artist_scanner.h"

#include <QFileDialog>


EditWindow::EditWindow(QWidget *parent) : QWidget(parent)
{
    pFeeds = static_cast<FeedsAndFilters*>(parent)->pFeeds;
}

EditWindow::EditWindow(const EditWindow::Type type, const int id, QWidget *parent, bool select) :
    QWidget(parent), type(type), id(id), select(select)
{
    pFeeds = static_cast<FeedsAndFilters*>(parent)->pFeeds; // приводим тип, т.к. parent у нас QWidget
    idFeed = static_cast<FeedsAndFilters*>(parent)->parentFeedId; // приводим тип, т.к. parent у нас QWidget

    nameLabel = new QLabel;
    valueLabel = new QLabel;
    nameEdit = new QLineEdit;
    valueEdit = new QLineEdit;
    enableCheck = new QCheckBox;
    okButton = new QPushButton;
    cancelButton = new QPushButton;
    artistsButton = new QPushButton;

    okButton->setText(tr("Сохранить"));
    cancelButton->setText(tr("Отмена"));
    artistsButton->setText("Поиск исполнителей");
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

    resize(300, 150);

    enableCheck->setText(tr("Активность"));
    if (type == EditWindow::enFilter)
    {
        artistsButton->setVisible(true);
        this->setWindowIcon(QIcon(":/filter.ico")); // если фильтр - ставим значок фильтра
        filters_struct filter(id);
        if (id != -1)
        {
            setWindowTitle("Редактирование фильтра");
            pFeeds->GetFilterById(&filter);
            enableCheck->setChecked(filter.is_on); // если редактирование - берём из БД
        }
        else
        {
            setWindowTitle("Добавление фильтра");
            enableCheck->setChecked(select); // если добавление - из настроек
        }
        nameLabel->setText(tr("Фильтр: (регистр не имеет значения)"));
        valueLabel->setText(tr("Комментарий:"));
        nameEdit->setText(filter.title);
        valueEdit->setText(filter.comment);
    }
    else
    {
        this->setWindowIcon(QIcon(":/rss.ico")); // если лента - ставим значок ленты
        Feed feed(id);
        if (id != -1)
        {
            setWindowTitle("Редактирование ленты");
            pFeeds->GetFeedById(&feed);
            enableCheck->setChecked(feed.is_on); // если редактирование - берём из БД
        }
        else
        {
            setWindowTitle("Добавление ленты");
            enableCheck->setChecked(select); // если добавление - из настроек
            nameEdit->setPlaceholderText("Если не вводить, будет взято из ленты");
        }
        nameLabel->setText(tr("Имя ленты:"));
        valueLabel->setText(tr("Ссылка:"));
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
            hint->setText("Ошибка в адресе ленты");
            return; // сразу выходит из функции
        }
        if (valueEdit->text().isEmpty()) // если поле адреса ленты пустое
        {
            hint->setText("Нельзя добавить пустое значение");
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
    }
    else
    {
        if (nameEdit->text().isEmpty()) // если поле имени фильтра пустое
        {
            hint->setText("Нельзя добавить пустое значение");
            return; // сразу выходит из функции
        }
        filters_struct filter(id);
        filter.comment = valueEdit->text().trimmed();
        filter.title = nameEdit->text().trimmed();
        filter.is_on = enableCheck->checkState();
        filter.idFeed = idFeed;
        pFeeds->SaveFilter(&filter);
    }
    close();
}

void EditWindow::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event);
    this->deleteLater();
}

void EditWindow::search_artists()
{
        QString path = QFileDialog::getExistingDirectory(this,
                                   QString::fromUtf8("Выбрать папку"),
                                   QDir::currentPath(),
                                   QFileDialog::ShowDirsOnly
                                   | QFileDialog::DontResolveSymlinks);
        enableCheck->setEnabled(false);
        nameEdit->setText(path);
        nameLabel->setText(tr("Выбрана папка:"));
        valueEdit->setText("Идёт поиск");
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
    hint->setText(QString("Найдено исполнителей: %1, Файлов проверено: %2").arg(i).arg(j));
    if (flag)
    {
        valueEdit->setText("Поиск окончен");
        cancelButton->setText("ОК");
        cancelButton->setEnabled(true);
    }
}
