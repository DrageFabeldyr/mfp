#include "feedsandfilters.h"
#include "editwindow.h"
/*
#include "taglib/fileref.h"
#include "taglib/taglib.h"
#include "taglib/tag.h"
*/
#include "artist_scanner.h"

//int step = 0;


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
        if (!(valueEdit->text().contains("http", Qt::CaseInsensitive) && valueEdit->text().contains("://")) )
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
        feed.title = nameEdit->text().trimmed();
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
    if (search_init)
    {
        search_init = !search_init;
        okButton->setEnabled(false);
        artistsButton->setText("Начать поиск");
        nameLabel->setText("Папка с музыкой:");
        nameEdit->setText("");
        nameEdit->setPlaceholderText("Вставьте адрес папки с музыкой...");
        /*
        valueLabel->setText("Добавочные символы");
        valueEdit->setText("");
        valueEdit->setPlaceholderText("Введите добавочные символы фильтров...");
        */
        valueEdit->setText("Автопоиск");
        valueEdit->setEnabled(false);
        hint->setText("Если файлов много, поиск может занять пару минут");
    }
    else
    {
        if (nameEdit->text().isEmpty()) // если не задан адрес
            return; // выходим
        search_init = !search_init;
        QString path = nameEdit->text().trimmed();
        /* всё равно не работает, нужна многопоточность*/
        artistsButton->setEnabled(false);
        //cancelButton->setEnabled(false);
        hint->setText("Идёт поиск");
        /**/

        // запускаем поток поиска
        QThread *thread= new QThread;
        artist_scanner *my = new artist_scanner;

        my->moveToThread(thread);

        connect(my, SIGNAL(send(int, int, bool)), this, SLOT(update(int, int, bool)));
        connect(thread, SIGNAL(started()), my, SLOT(searching()));
        connect(this, SIGNAL(send_params(QString, int, int, bool)), my, SLOT(receive_params(QString, int, int, bool)), Qt::DirectConnection);

        emit send_params(path, id, idFeed, enableCheck->checkState()); // путь к папке, id ленты, галочка активности

        thread->start();
/*
        artists.clear();
        searching(path);

        // запись данных в БД (надо как-то потом переработать дублирование функции SaveData)
        for (int i = 0; i < artists.size(); i++)
        {
            filters_struct filter(id);
            filter.comment = "Автопоиск";
            filter.title = artists.at(i);// + valueEdit->text();
            filter.is_on = enableCheck->checkState();
            filter.idFeed = idFeed;
            pFeeds->SaveFilter(&filter);
        }

        hint->setText(QString("Найдено исполнителей: %1").arg(artists.size()));
        artistsButton->setEnabled(false);
        cancelButton->setText("ОК");
        */
    }
}

void EditWindow::update(int i, int j, bool flag)
{
    hint->setText(QString("Найдено исполнителей: %1, Файлов проверено: %2").arg(i).arg(j));
    if (flag)
    {
        cancelButton->setText("ОК");
        cancelButton->setEnabled(true);
    }
}

/*
void EditWindow::searching(QString path)
{
    //qDebug() <<  "*********************************************************************************************************************";
    //qDebug() <<  path;
    bool new_artist;

    // http://www.cyberforum.ru/qt/thread644532.html   -   там есть пример и без рекурсии, но так как-то понятнее
    QDir currentFolder(path);

    //currentFolder.setFilter(QDir::Dirs | QDir::Files);
    currentFolder.setFilter(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot | QDir::QDir::NoSymLinks);
    currentFolder.setSorting(QDir::Name);

    QFileInfoList folderitems(currentFolder.entryInfoList());

    foreach (QFileInfo i_file, folderitems)
    {
        QString i_filename(i_file.fileName());
        //if (i_filename == "." || i_filename == ".." || i_filename.isEmpty())
        if (i_filename.isEmpty())
            continue;

        if (i_file.isDir())
            searching(path + "/" + i_filename);
        else
        {
            QString full_filename = path + "/" + i_filename;
            //step++;
            //qDebug() <<  step << " - " << full_filename;


            if (full_filename.contains(".mp3", Qt::CaseInsensitive) || full_filename.contains(".flac", Qt::CaseInsensitive) || full_filename.contains(".wma", Qt::CaseInsensitive) || full_filename.contains(".wav", Qt::CaseInsensitive))
            {
                TagLib::FileRef ref(full_filename.toStdWString().c_str()); // только с таким преобразованием не игнорируются папки с нелатинскими буквами
                if (!ref.isNull() && ref.tag() != NULL)
                {
                    //QString data = QString("%1").arg(ref.tag()->artist().toCString(true));
                    //QString data = QString::fromStdString(ref.tag()->artist().toCString(true));
                    //QString data = QString::fromStdWString(ref.tag()->artist().toWString());
                    QString data = TStringToQString(ref.tag()->artist());
                    new_artist = true;
                    for (int i = 0; i < artists.size(); i++)
                        if (data.trimmed() == artists.at(i).trimmed())
                            new_artist = false;
                    if (new_artist && data != "") // на случай, если в поле "исполнитель" ничего нет
                        artists.push_back(data);
                }
           }
        }
    }
}
*/
