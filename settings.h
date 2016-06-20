#ifndef SETTINGS_H
#define SETTINGS_H

#include <QDialog>
#include <QCheckBox>


namespace Ui {
class settings;
}

class Settings : public QDialog
{
    Q_OBJECT

public:
    explicit Settings(QWidget *parent = 0);
    ~Settings();
    QCheckBox *min_to_tray, *close_to_tray, *run_in_tray;
    QCheckBox *activate_filters, *activate_feeds;

public slots:
    void read_settings();     // чтение фильтров из файла
    void write_settings();    // запись фильтров в файл

private:
    Ui::settings *ui;

private slots:
    void closeEvent(QCloseEvent *);
};

#endif // SETTINGS_H
