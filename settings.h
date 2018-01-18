#ifndef SETTINGS_H
#define SETTINGS_H

#include <QWidget>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QCloseEvent>
#include <QTimer>
#include <QPushButton>  // чтобы закрыть окно в андроиде

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
    int current_language = 1;

public slots:
    void read_settings();     // чтение настроек из файла
    void write_settings();    // запись настроек в файл

private:
    QVBoxLayout *layout;
    QString settingFile = "Setting.ini";
    QWidget *parentW;
#ifdef Q_OS_ANDROID
    QPushButton *save_settings_button;
#endif

private slots:
    void closeEvent(QCloseEvent * event) Q_DECL_OVERRIDE;
    void showEvent(QShowEvent * event) Q_DECL_OVERRIDE;
};

#endif // SETTINGS_H
