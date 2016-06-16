#ifndef FEEDS_SETTINGS_H
#define FEEDS_SETTINGS_H

#include <QDialog>
#include <QLineEdit>
#include <QLabel>
#include <QListWidget>
#include <QStatusBar>

struct feeds_struct
{
    QString title;
    QString link;
    bool is_on;
};

extern QList<feeds_struct> feeds;
void read_feeds();

namespace Ui {
class feeds_settings;
}

class feeds_settings : public QDialog
{
    Q_OBJECT

public:
    explicit feeds_settings(QWidget *parent = 0);
    ~feeds_settings();

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

private slots:
    void closeEvent(QCloseEvent *);

};

#endif // FEEDS_SETTINGS_H
