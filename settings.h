#ifndef SETTINGS_H
#define SETTINGS_H

#include <QWidget>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QCloseEvent>
#include <QTimer>


namespace Ui {
class settings;
}

class Settings : public QWidget
{
    Q_OBJECT

public:
    explicit Settings(QWidget *parent = 0);
    ~Settings();
    QCheckBox *min_to_tray, *close_to_tray, *run_in_tray;
    QCheckBox *activate_filters, *activate_feeds;
    bool activateFilters ;
    bool activateFeeds;
    int request_period; // период, через который запрашиваются новости в миллисекундах
    int show_period; // время, которое висит уведомление в трее
    QTimer *timer;

public slots:
    void read_settings();     // чтение настроек из файла
    void write_settings();    // запись настроек в файл

private:
    Ui::settings *ui;
    QVBoxLayout *layout;
    QString settingFile = "Setting.ini";
    QWidget *parentW;

private slots:
    void closeEvent(QCloseEvent * event) Q_DECL_OVERRIDE;
    void showEvent(QShowEvent * event) Q_DECL_OVERRIDE;
};

#endif // SETTINGS_H
