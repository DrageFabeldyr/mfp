#include "filter.h"
#include "ui_filter.h"

int num_of_filters;
QStringList filters, temp_values;
QStringList headerLabels;


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
    QFile file("filters.gsd"); // создаем объект класса QFile
    int  i = 0;
    if(file.open(QIODevice::ReadOnly |QIODevice::Text)) // если файл открылся и там текст
    {
        while(!file.atEnd()) // пока не упрёмся в конец файла
        {
            QString str = file.readLine(); // читаем строку
            QTextDocument doc;
            doc.setHtml(str);
            filters << doc.toPlainText();
            i++;
        }
        file.close(); // прочли и закрыли
    }
    num_of_filters = i;
}

void write_filters()
{
    QFile file("filters.gsd"); // создаем объект класса QFile
    if(file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) // открываем для записи и предварительно полностью обнуляем файл
    {
        QTextStream outStream(&file);
        QTextDocument doc;

        for (int i = 0; i < num_of_filters; i++)
//            outStream << encodeEntities(values.at(i));
            outStream << QString("%1\n").arg(encodeEntities(filters.at(i)));
        file.close(); // записали и закрыли
    }
}

// функция вывода фильтров в таблицу
void show_filters(QTreeWidget *treewidget, QStringList filters, int num)
{
    treewidget->clear();
    for (int i = 0; i < num; i++)
    {
        QTreeWidgetItem *item = new QTreeWidgetItem;
        item->setText(0, filters.at(i));
        treewidget->addTopLevelItem(item);
    }
}

void set_filters_header_label(QTreeWidget *treewidget)
{
    headerLabels.clear();
    headerLabels << QString("Имеющиеся фильтры: (Всего: %1)").arg(num_of_filters);
    treewidget->setHeaderLabels(headerLabels);
}


filter::filter(QDialog *parent) : QDialog(parent), ui(new Ui::filter)
{
    setWindowTitle(tr("Фильтры"));
    lineEdit = new QLineEdit(this);
    lineEdit->setPlaceholderText("Введите фильтр...");
    QPushButton *add_button = new QPushButton;       // добавить фильтр
    add_button->setText("Добавить");
    QPushButton *del_button = new QPushButton;       // удалить фильтр
    del_button->setText("Удалить");
    hint = new QLabel(this);

    filers_list = new QTreeWidget(this);
    filers_list->setSelectionMode(QAbstractItemView::ExtendedSelection); // чтобы можно было выделять несколько элементов с помощью Ctrl и Shift
    set_filters_header_label(filers_list);
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // здесь решается вопрос растяжения - нужно его решить
    filers_list->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    show_filters(filers_list, filters, num_of_filters);

    statusbar = new QStatusBar(this);
    statusbar->showMessage("Используйте Ctrl и Shift для множественного выделения");

    QHBoxLayout *h_layout = new QHBoxLayout;         //
    h_layout->addWidget(lineEdit);
    h_layout->addWidget(add_button);
    h_layout->addWidget(del_button);
    QVBoxLayout *v_layout = new QVBoxLayout;         //
    v_layout->addLayout(h_layout);
    v_layout->addWidget(hint);
    v_layout->addWidget(filers_list);
    v_layout->addWidget(statusbar);
    setLayout(v_layout); // установка главного лэйаута
    resize(640,480);

    // соединим кнопки с функциями
    QObject::connect(add_button, SIGNAL(clicked()), this, SLOT(add_filter()));  //
    QObject::connect(del_button, SIGNAL(clicked()), this, SLOT(del_filter()));  //

}

filter::~filter()
{
    delete ui;
}

void filter::closeEvent(QCloseEvent *)
{
    write_filters();
}

void filter::add_filter()
{
    QString help_str;
    bool add_this = true;

    if (!lineEdit->text().isEmpty())
    {
        help_str.clear();
        help_str = lineEdit->text();

        for (int i = 0; i < num_of_filters; i++)
        {
            if (QString::compare(help_str, filters.at(i).simplified(), Qt::CaseInsensitive) == 0)
            {
                hint->setText("Такой фильтр уже есть");
                add_this = false;
            }
        }
        if (add_this)
        {
//            values << QString("%1\n").arg(lineEdit->text());
            filters << QString("%1").arg(lineEdit->text());
            num_of_filters++;
            hint->setText("Фильтр добавлен");
            /**/
            write_filters();
            show_filters(filers_list, filters, num_of_filters);
            set_filters_header_label(filers_list);
            /**/
        }
    }
    else
        hint->setText("Нельзя добавить пустой фильтр");
}

void filter::del_filter()
{
    int filters_left = 0;
    filters.clear();

    for (int i = 0; i < num_of_filters; i++)
        if (!filers_list->topLevelItem(i)->isSelected())
        {
            filters << QString("%1").arg(filers_list->topLevelItem(i)->text(0));
            filters_left++;
        }
    num_of_filters = filters_left;
    /**/
    write_filters();
    show_filters(filers_list, filters, num_of_filters);
    set_filters_header_label(filers_list);
    /**/
}
