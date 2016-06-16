#include "filter.h"
#include "ui_filter.h"

#include <QDir>

#include "settings.h"
extern settings *sett; // чтобы взять тот, что уже определён в dfrssfilter.cpp

QStringList filters;
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
    QString name = qApp->applicationDirPath() + QDir::separator() + "filters.gsd";
    QFile file(name); // создаем объект класса QFile
    int  i = 0;
    if(file.open(QIODevice::ReadOnly |QIODevice::Text)) // если файл открылся и там текст
    {
        while(!file.atEnd()) // пока не упрёмся в конец файла
        {
            QString str = file.readLine(); // читаем строку
            if (str.trimmed() != "") // проверим, что строка не пуста
            {
                QTextDocument doc;
                doc.setHtml(str.trimmed());
                filters << doc.toPlainText();
                i++;
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
            outStream << QString("%1").arg(encodeEntities(filters.at(i))) << endl;
        file.close(); // записали и закрыли
    }
}

// функция вывода фильтров в таблицу
void show_filters(QTreeWidget *treewidget, QStringList filters)
{
    treewidget->clear();
    for (int i = 0; i < filters.size(); i++)
    {
        QTreeWidgetItem *item = new QTreeWidgetItem;
        item->setText(0, filters.at(i));
        treewidget->addTopLevelItem(item);
    }
}

void set_filters_header_label(QTreeWidget *treewidget)
{
    headerLabels.clear();
    headerLabels << QString("Имеющиеся фильтры: (Всего: %1)").arg(filters.size());
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
    filter_hint = new QLabel(this);

    filers_list = new QTreeWidget(this);
    filers_list->setSelectionMode(QAbstractItemView::ExtendedSelection); // чтобы можно было выделять несколько элементов с помощью Ctrl и Shift
    set_filters_header_label(filers_list);
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // здесь решается вопрос растяжения - нужно его решить
    filers_list->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    show_filters(filers_list, filters);

    choice_hint = new QLabel(this);
    choice_hint->setText("Используйте Ctrl и Shift для множественного выделения удаляемых фильтров");

    QHBoxLayout *h_layout = new QHBoxLayout;         //
    h_layout->addWidget(lineEdit);
    h_layout->addWidget(add_button);
    h_layout->addWidget(del_button);
    QVBoxLayout *v_layout = new QVBoxLayout;         //
    v_layout->addLayout(h_layout);
    v_layout->addWidget(filter_hint);
    v_layout->addWidget(filers_list);
    v_layout->addWidget(choice_hint);
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

        for (int i = 0; i < filters.size(); i++)
        {
            if (QString::compare(help_str, filters.at(i).simplified(), Qt::CaseInsensitive) == 0)
            {
                filter_hint->setText("Такой фильтр уже есть");
                add_this = false;
            }
        }
        if (add_this)
        {
            filters << QString("%1").arg(lineEdit->text());
            filter_hint->setText("Фильтр добавлен");
            /**/
            write_filters();
            show_filters(filers_list, filters);
            set_filters_header_label(filers_list);
            /**/
        }
    }
    else
        filter_hint->setText("Нельзя добавить пустой фильтр");
}

void filter::del_filter()
{
    for (int i = filters.size() - 1; i >= 0; i--) // идём от конца к началу, т.к. при удалении элемента размер списка уменьшается
        if (filers_list->topLevelItem(i)->isSelected())
            filters.removeAt(i);
    /**/
    write_filters();
    show_filters(filers_list, filters);
    set_filters_header_label(filers_list);
    /**/
}
