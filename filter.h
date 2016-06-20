#ifndef FILTER_H
#define FILTER_H

#include <QWidget>
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
#include <QCloseEvent>

#include "settings.h"
#include "filters_struct.h"

namespace Ui {
class filter;
}

class Filter : public QWidget
{
    Q_OBJECT

public:
    explicit Filter(QWidget *parent = 0);
    ~Filter();
    void read_filters();
    void write_filters();
    void show_filters(QTreeWidget *treewidget, QList<filters_struct> values);
    void save_checked(QTreeWidget *treewidget);
    QList<filters_struct> filters;
    int GetNumActiveFilter(void);

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
    Settings *settings;


protected:
    void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;
    void showEvent(QShowEvent * event) Q_DECL_OVERRIDE ;
};

#endif // FILTER_H
