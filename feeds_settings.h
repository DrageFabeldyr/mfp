#ifndef FEEDS_SETTINGS_H
#define FEEDS_SETTINGS_H

#include <QDialog>
#include <QLineEdit>
#include <QLabel>
#include <QListWidget>
#include <QStatusBar>
#include <QPushButton>
#include <QHBoxLayout>
#include <QListView>
#include <QList>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QEvent>

#include "settings.h"
#include "feeds_struct.h"

namespace Ui {
class feeds_settings;
}

class Feeds_Settings : public QWidget
{
    Q_OBJECT

public:
    explicit Feeds_Settings(QWidget *parent = 0);
    ~Feeds_Settings();
    QList<feeds_struct> feeds;

    void read_feeds();

public slots:
    void add_feed();
    void del_feed();
    void set_feeds_header_label();
    void check_all();
    void uncheck_all();

private:
    Ui::feeds_settings *ui;
    QLineEdit *lineEdit;
    QLabel *feed_hint, *choice_hint;
    QListWidget *feeds_list;
    Settings *settings;

    void show_feeds(QListWidget *listwidget, QList<feeds_struct> values);
    void write_feeds();
    void save_checked(QListWidget *listwidget);

private slots:
    void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;
    void showEvent(QShowEvent * event) Q_DECL_OVERRIDE ;

};

#endif // FEEDS_SETTINGS_H
