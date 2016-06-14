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

extern int num_of_filters;
extern QStringList filters;
void read_filters();     // чтение фильтров из файла
void write_filters();    // запись фильтров в файл


namespace Ui {
class filter;
}

class filter : public QDialog
{
    Q_OBJECT

public:
    explicit filter(QDialog *parent = 0);
    ~filter();

public slots:
    void add_filter();
    void del_filter();

private:
    Ui::filter *ui;
    QLineEdit *lineEdit;
    QLabel *hint;
    QTreeWidget *filers_list;
    QStatusBar *statusbar;

private slots:
    void closeEvent(QCloseEvent *);
};

#endif // FILTER_H
