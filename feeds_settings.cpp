#include "feeds_settings.h"
#include "ui_feeds_settings.h"

#include <QPushButton>
#include <QHBoxLayout>
#include <QListView>
#include <QList>


//QStringList feeds;
QList<feeds_struct> feeds;

// функция вывода фильтров в таблицу
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

void write_feeds()
{

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

// функция получения имени ленты
void get_feed_name()
{

}

feeds_settings::feeds_settings(QWidget *parent) : QDialog(parent), ui(new Ui::feeds_settings)
{
    //setWindowTitle("RSS-ленты");

    lineEdit = new QLineEdit(this);
    lineEdit->setPlaceholderText("Введите ленту...");
    QPushButton *add_button = new QPushButton;       // добавить фильтр
    add_button->setText("Добавить");
    QPushButton *del_button = new QPushButton;       // удалить фильтр
    del_button->setText("Удалить");
    hint = new QLabel(this);

    feeds_list = new QListWidget(this);
    feeds_list->setSelectionMode(QAbstractItemView::ExtendedSelection); // чтобы можно было выделять несколько элементов с помощью Ctrl и Shift
    show_feeds(feeds_list, feeds);

    statusbar = new QStatusBar(this);
    statusbar->showMessage("Используйте Ctrl и Shift для множественного выделения");

    QHBoxLayout *h_layout = new QHBoxLayout;         //
    h_layout->addWidget(lineEdit);
    h_layout->addWidget(add_button);
    h_layout->addWidget(del_button);

    QVBoxLayout *v_layout = new QVBoxLayout;         //
    v_layout->addLayout(h_layout);
    v_layout->addWidget(hint);
    v_layout->addWidget(feeds_list);
    v_layout->addWidget(statusbar);
    setLayout(v_layout); // установка главного лэйаута

    resize(640,480);
    set_feeds_header_label();

    // соединим кнопки с функциями
    QObject::connect(add_button, SIGNAL(clicked()), this, SLOT(add_feed()));  //
    QObject::connect(del_button, SIGNAL(clicked()), this, SLOT(del_feed()));  //
}

feeds_settings::~feeds_settings()
{
    delete ui;
}

void feeds_settings::add_feed()
{
    QString help_str;
    bool add_this = true;

    if (!lineEdit->text().isEmpty() && lineEdit->text().contains("http", Qt::CaseInsensitive) && lineEdit->text().contains("://"))
    {
        help_str.clear();
        help_str = lineEdit->text();

        for (int i = 0; i < feeds.size(); i++)
        {
            if (QString::compare(help_str, feeds.at(i).link.simplified(), Qt::CaseInsensitive) == 0)
            {
                hint->setText("Эта лента уже есть в списке");
                add_this = false;
            }
        }
        if (add_this)
        {
//            values << QString("%1\n").arg(lineEdit->text());
            //feeds.links << QString("%1").arg(lineEdit->text());
            feeds_struct temp;  // создаём элемент типа
            temp.link = QString("%1").arg(lineEdit->text());    // заносим в него данные
            feeds.push_back(temp); // запихиваем элемент в контейнер

            hint->setText("Лента добавлена");
            /**/
            write_feeds();
            show_feeds(feeds_list, feeds);
            set_feeds_header_label();
            /**/
        }
    }
    else
        hint->setText("Bведён неверный адрес");
}

void feeds_settings::del_feed()
{
    save_checked(feeds_list);
    for (int i = feeds.size() - 1; i >= 0; i--) // идём от конца к началу, т.к. при удалении элемента размер списка уменьшается
        if (feeds_list->item(i)->isSelected())
            feeds.removeAt(i);
    /**/
    write_feeds();
    show_feeds(feeds_list, feeds);
    set_feeds_header_label();
    /**/
}

void feeds_settings::set_feeds_header_label()
{
    this->setWindowTitle(QString("RSS-ленты: (Всего: %1)").arg(feeds.size()));
}

// сохраняем галочки
void feeds_settings::closeEvent(QCloseEvent *)
{
    save_checked(feeds_list);
}
