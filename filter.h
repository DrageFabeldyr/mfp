#ifndef FILTER_H
#define FILTER_H

#include <QDialog>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QPushButton>
#include <QFile>        // для работы с файлами
#include <QTextStream>
#include <QStringList>
#include <QLabel>
#include <QTimer>
#include <QTextDocument>
#include <QTreeWidgetItem> // для вывода списка фильтров
#include <QTreeWidgetItemIterator>
#include <QStatusBar>

#include "settings.h"

struct filters_struct
{
    QString title;
    QString feed;
    bool is_on;
};

extern QList<filters_struct> filters;

void read_filters();     // чтение фильтров из файла
void write_filters();    // запись фильтров в файл


namespace Ui {
class filter;
}

class filter : public QDialog
{
    Q_OBJECT

public:
    explicit filter(QWidget *parent = 0);
    ~filter();

public slots:
    void add_filter();
    void del_filter();
    void set_filters_header_label();
    void check_all();
    void uncheck_all();

private:
    Ui::filter *ui;
    QLineEdit *lineEdit;
    QLabel *filter_hint, *choice_hint;
    QTreeWidget *filers_list;
    settings *sett;

private slots:
    void closeEvent(QCloseEvent *);
};

#endif // FILTER_H
