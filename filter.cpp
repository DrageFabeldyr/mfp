#include "filter.h"
#include "ui_filter.h"

#include <QDir>

#include "dfrssfilter.h"

QList<filters_struct> filters;


/* функция преобразования нестандартных символов к виду &#xxx;
 * взята отсюда:
 * http://stackoverflow.com/questions/7696159/how-can-i-convert-entity-characterescape-character-to-html-in-qt
 * автор http://stackoverflow.com/users/1455977/immortalpc
 */
QString encodeEntities(const QString& src, const QString& force=QString())
{
    QString tmp(src);
    uint len = tmp.length();
    uint i = 0;
    while(i < len)
    {
        if(tmp[i].unicode() > 128 || force.contains(tmp[i]))
        {
            QString rp = "&#"+QString::number(tmp[i].unicode())+";";
            tmp.replace(i,1,rp);
            len += rp.length()-1;
            i += rp.length();
        }
        else
        {
            ++i;
        }
    }
    return tmp;
}

void read_filters()
{
    QString name = qApp->applicationDirPath() + QDir::separator() + "filters.gsd";
    QFile file(name); // создаем объект класса QFile
    if(file.open(QIODevice::ReadOnly |QIODevice::Text)) // если файл открылся и там текст
    {
        while(!file.atEnd()) // пока не упрёмся в конец файла
        {
            QString str = file.readLine(); // читаем строку
            if (str.trimmed() != "") // проверим, что строка не пуста
            {
                QTextDocument doc;
                doc.setHtml(str.trimmed());
                filters_struct temp;  // создаём элемент типа
                temp.title = doc.toPlainText();    // заносим в него данные
                filters.push_back(temp); // запихиваем элемент в контейнер
            }
        }
        file.close(); // прочли и закрыли
    }
}

void write_filters()
{
    QString name = qApp->applicationDirPath() + QDir::separator() + "filters.gsd";
    QFile file(name); // создаем объект класса QFile
    if(file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) // открываем для записи и предварительно полностью обнуляем файл
    {
        QTextStream outStream(&file);
        QTextDocument doc;

        for (int i = 0; i < filters.size(); i++)
            outStream << QString("%1").arg(encodeEntities(filters.at(i).title.trimmed())) << endl;
        file.close(); // записали и закрыли
    }
}

// функция вывода фильтров в таблицу
void show_filters(QTreeWidget *treewidget, QList<filters_struct> values)
{
    treewidget->clear();
    for (int i = 0; i < values.size(); i++)
    {
        QTreeWidgetItem *item = new QTreeWidgetItem;
        item->setText(0, values.at(i).title.simplified());
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable); // добавляем возможность выбора
        // и задаём начальное значение
        if (values.at(i).is_on)
            item->setCheckState(0, Qt::Checked);
        else
            item->setCheckState(0, Qt::Unchecked);
        treewidget->addTopLevelItem(item);
    }
}

// функция запоминания расставленных галочек
void save_checked(QTreeWidget *treewidget)
{
    for (int i = 0; i < filters.size(); i++)
    {
        if (treewidget->topLevelItem(i)->checkState(0))
            filters[i].is_on = true;
        else
            filters[i].is_on = false;
    }
    write_filters();
}

filter::filter(QWidget *parent) : QDialog(), ui(new Ui::filter)
{
    sett = static_cast<DFRSSFilter*>(parent)->sett;
    lineEdit = new QLineEdit(this);
    lineEdit->setPlaceholderText("Введите фильтр...");

    QPushButton *add_button = new QPushButton;       // добавить фильтр
    add_button->setText("Добавить");
    QPushButton *del_button = new QPushButton;       // удалить фильтр
    del_button->setText("Удалить");
    QPushButton *check_all_button = new QPushButton;       // выбрать все
    check_all_button->setText("Выбрать все");
    QPushButton *uncheck_all_button = new QPushButton;       // снять выбор
    uncheck_all_button->setText("Отменить все");

    filter_hint = new QLabel(this);

    filers_list = new QTreeWidget(this);
    filers_list->setSelectionMode(QAbstractItemView::ExtendedSelection); // чтобы можно было выделять несколько элементов с помощью Ctrl и Shift
    set_filters_header_label();
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // здесь решается вопрос растяжения - нужно его решить
    filers_list->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    filers_list->header()->hide(); // скроем заголовок таблицы - он нафиг не нужен
    show_filters(filers_list, filters);

    choice_hint = new QLabel(this);
    choice_hint->setText("Используйте Ctrl и Shift для множественного выделения удаляемых фильтров");

    QHBoxLayout *h_layout = new QHBoxLayout;         //
    h_layout->addWidget(lineEdit);
    h_layout->addWidget(add_button);
    h_layout->addWidget(del_button);

    QHBoxLayout *bottom_h_layout = new QHBoxLayout;  // нижний
    bottom_h_layout->addWidget(choice_hint);
    bottom_h_layout->addWidget(check_all_button);
    bottom_h_layout->addWidget(uncheck_all_button);

    QVBoxLayout *v_layout = new QVBoxLayout;         //
    v_layout->addLayout(h_layout);
    v_layout->addWidget(filter_hint);
    v_layout->addWidget(filers_list);
    v_layout->addLayout(bottom_h_layout);
    setLayout(v_layout); // установка главного лэйаута
    resize(640,480);

    // соединим кнопки с функциями
    connect(add_button, SIGNAL(clicked()), this, SLOT(add_filter()));  //
    connect(del_button, SIGNAL(clicked()), this, SLOT(del_filter()));  //
    connect(check_all_button, SIGNAL(clicked()), this, SLOT(check_all()));  //
    connect(uncheck_all_button, SIGNAL(clicked()), this, SLOT(uncheck_all()));  //
    connect(lineEdit, SIGNAL(returnPressed()), this, SLOT(add_filter())); // добавление по нажатию клавиши ENTER

    this->setWindowIcon(QIcon(":/filter.ico"));
}

filter::~filter()
{
    delete ui;
    delete lineEdit;
    delete filter_hint;
    delete choice_hint;
    delete filers_list;
}

void filter::add_filter()
{
    QString help_str;
    bool add_this = true;

    if (!lineEdit->text().isEmpty())
    {
        save_checked(filers_list); // сохраним галочки на случай если они будут изменены одновременно с изменением количества элементов
        help_str.clear();
        help_str = lineEdit->text();

        for (int i = 0; i < filters.size(); i++)
        {
            if (QString::compare(help_str, filters.at(i).title.simplified(), Qt::CaseInsensitive) == 0)
            {
                filter_hint->setText("Такой фильтр уже есть");
                add_this = false;
            }
        }
        if (add_this)
        {
            filters_struct temp;  // создаём элемент типа
            temp.title = QString("%1").arg(lineEdit->text());    // заносим в него данные
            if (sett->activate_filters->isChecked())
                temp.is_on = true;
            else
                temp.is_on = false;
            filters.push_back(temp); // запихиваем элемент в контейнер

            filter_hint->setText("Фильтр добавлен");
            /**/
            write_filters();
            show_filters(filers_list, filters);
            set_filters_header_label();
            /**/
        }
    }
    else
        filter_hint->setText("Нельзя добавить пустой фильтр");
}

// удаление фильтра
void filter::del_filter()
{
    save_checked(filers_list); // сохраним галочки
    for (int i = filters.size() - 1; i >= 0; i--) // идём от конца к началу, т.к. при удалении элемента размер списка уменьшается
        if (filers_list->topLevelItem(i)->isSelected())
            filters.removeAt(i);
    /**/
    write_filters();
    show_filters(filers_list, filters);
    set_filters_header_label();
    /**/
}

// устанавливаем заголовок окна
void filter::set_filters_header_label()
{
    this->setWindowTitle(QString("Фильтры: (Всего: %1)").arg(filters.size()));
}

void filter::closeEvent(QCloseEvent *)
{
    save_checked(filers_list);
}

void filter::check_all()
{
    for (int i = 0; i < filters.size(); i++)
        filters[i].is_on = true;
    show_filters(filers_list, filters);
}

void filter::uncheck_all()
{
    for (int i = 0; i < filters.size(); i++)
        filters[i].is_on = false;
    show_filters(filers_list, filters);
}
