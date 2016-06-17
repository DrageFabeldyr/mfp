#include "feeds_settings.h"
#include "ui_feeds_settings.h"

#include <QPushButton>
#include <QHBoxLayout>
#include <QListView>
#include <QList>
#include <QDir>
#include <QFile>
#include <QTextStream>

#include "dfrssfilter.h"


QList<feeds_struct> feeds;

// функция вывода лент в таблицу
void show_feeds(QListWidget *listwidget, QList<feeds_struct> values)
{
    listwidget->clear();
    for (int i = 0; i < values.size(); i++)
    {
        QListWidgetItem *item = new QListWidgetItem(values.at(i).link.simplified(), listwidget);;
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable); // добавляем возможность выбора
        // и задаём начальное значение
        if (feeds.at(i).is_on)
            item->setCheckState(Qt::Checked);
        else
            item->setCheckState(Qt::Unchecked);

        listwidget->addItem(item);
    }
}

void read_feeds()
{
    QString name = qApp->applicationDirPath() + QDir::separator() + "feeds.gsd";
    QFile file(name); // создаем объект класса QFile
    if(file.open(QIODevice::ReadOnly |QIODevice::Text)) // если файл открылся и там текст
    {
        while(!file.atEnd()) // пока не упрёмся в конец файла
        {
            QString str = file.readLine(); // читаем строку
            if (str.trimmed() != "") // проверим, что строка не пуста
            {
                feeds_struct temp;  // создаём элемент типа
                temp.link = str.trimmed();    // заносим в него данные
                feeds.push_back(temp); // запихиваем элемент в контейнер
            }
        }
        file.close(); // прочли и закрыли
    }
}

void write_feeds()
{
    QString name = qApp->applicationDirPath() + QDir::separator() + "feeds.gsd";
    QFile file(name); // создаем объект класса QFile
    if(file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) // открываем для записи и предварительно полностью обнуляем файл
    {
        QTextStream outStream(&file);

        for (int i = 0; i < feeds.size(); i++)
            outStream << feeds.at(i).link.trimmed() << endl;
        file.close(); // записали и закрыли
    }
}

// функция запоминания расставленных галочек
void save_checked(QListWidget *listwidget)
{
    for (int i = 0; i < feeds.size(); i++)
    {
        if (listwidget->item(i)->checkState())
            feeds[i].is_on = true;
        else
            feeds[i].is_on = false;
    }
    write_feeds();
}

feeds_settings::feeds_settings(QWidget *parent) : QDialog(), ui(new Ui::feeds_settings)
{
    sett = static_cast<DFRSSFilter*>(parent)->sett;
    lineEdit = new QLineEdit(this);
    lineEdit->setPlaceholderText("Введите адрес ленты...");

    QPushButton *add_button = new QPushButton;       // добавить фильтр
    add_button->setText("Добавить");
    QPushButton *del_button = new QPushButton;       // удалить фильтр
    del_button->setText("Удалить");
    QPushButton *check_all_button = new QPushButton;       // выбрать все
    check_all_button->setText("Выбрать все");
    QPushButton *uncheck_all_button = new QPushButton;       // снять выбор
    uncheck_all_button->setText("Отменить все");

    feed_hint = new QLabel(this);

    feeds_list = new QListWidget(this);
    feeds_list->setSelectionMode(QAbstractItemView::ExtendedSelection); // чтобы можно было выделять несколько элементов с помощью Ctrl и Shift
    show_feeds(feeds_list, feeds);

    choice_hint = new QLabel(this);
    choice_hint->setText("Используйте Ctrl и Shift для множественного выделения удаляемых лент");

    QHBoxLayout *h_layout = new QHBoxLayout;         // верхний
    h_layout->addWidget(lineEdit);
    h_layout->addWidget(add_button);
    h_layout->addWidget(del_button);

    QHBoxLayout *bottom_h_layout = new QHBoxLayout;  // нижний
    bottom_h_layout->addWidget(choice_hint);
    bottom_h_layout->addWidget(check_all_button);
    bottom_h_layout->addWidget(uncheck_all_button);

    QVBoxLayout *v_layout = new QVBoxLayout;         //
    v_layout->addLayout(h_layout);
    v_layout->addWidget(feed_hint);
    v_layout->addWidget(feeds_list);
    v_layout->addLayout(bottom_h_layout);
    setLayout(v_layout); // установка главного лэйаута

    resize(640,480);
    set_feeds_header_label();

    // соединим кнопки с функциями
    connect(add_button, SIGNAL(clicked()), this, SLOT(add_feed()));  //
    connect(del_button, SIGNAL(clicked()), this, SLOT(del_feed()));  //
    connect(check_all_button, SIGNAL(clicked()), this, SLOT(check_all()));  //
    connect(uncheck_all_button, SIGNAL(clicked()), this, SLOT(uncheck_all()));  //
    //connect(lineEdit, SIGNAL(returnPressed()), this, SLOT(add_feed())); // добавление по нажатию клавиши ENTER

    this->setWindowIcon(QIcon(":/rss.ico"));
}

feeds_settings::~feeds_settings()
{
    delete ui;
    delete lineEdit;
    delete feed_hint;
    delete choice_hint;
    delete feeds_list;
}

void feeds_settings::add_feed()
{
    QString help_str;
    bool add_this = true;

    if (!lineEdit->text().isEmpty() && lineEdit->text().contains("http", Qt::CaseInsensitive) && lineEdit->text().contains("://"))
    {
        save_checked(feeds_list); // сохраним галочки на случай если они будут изменены одновременно с изменением количества элементов
        help_str.clear();
        help_str = lineEdit->text();

        for (int i = 0; i < feeds.size(); i++)
        {
            if (QString::compare(help_str, feeds.at(i).link.simplified(), Qt::CaseInsensitive) == 0)
            {
                feed_hint->setText("Эта лента уже есть в списке");
                add_this = false;
            }
        }
        if (add_this)
        {
            feeds_struct temp;  // создаём элемент типа
            temp.link = QString("%1").arg(lineEdit->text());    // заносим в него данные
            if (sett->activate_feeds->isChecked())
                temp.is_on = true;
            else
                temp.is_on = false;
            feeds.push_back(temp); // запихиваем элемент в контейнер

            feed_hint->setText("Лента добавлена");
            /**/
            write_feeds();
            show_feeds(feeds_list, feeds);
            set_feeds_header_label();
            /**/
        }
        lineEdit->setText("");
    }
    else
        feed_hint->setText("Bведён неверный адрес");
}

void feeds_settings::del_feed()
{
    save_checked(feeds_list); // сохраним галочки
    for (int i = feeds.size() - 1; i >= 0; i--) // идём от конца к началу, т.к. при удалении элемента размер списка уменьшается
        if (feeds_list->item(i)->isSelected())
            feeds.removeAt(i);
    /**/
    write_feeds();
    show_feeds(feeds_list, feeds);
    set_feeds_header_label();
    /**/
}

// устанавливаем заголовок окна
void feeds_settings::set_feeds_header_label()
{
    this->setWindowTitle(QString("RSS-ленты: (Всего: %1)").arg(feeds.size()));
}

// сохраняем галочки
void feeds_settings::closeEvent(QCloseEvent *)
{
    save_checked(feeds_list);
}

void feeds_settings::check_all()
{
    for (int i = 0; i < feeds.size(); i++)
        feeds[i].is_on = true;
    show_feeds(feeds_list, feeds);
}

void feeds_settings::uncheck_all()
{
    for (int i = 0; i < feeds.size(); i++)
        feeds[i].is_on = false;
    show_feeds(feeds_list, feeds);
}
