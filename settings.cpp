#include "settings.h"
#include "ui_settings.h"

#include <QVBoxLayout>
#include <QDir>
#include <QFile>
#include <QSettings>
#include <QTextStream>


Settings::Settings(QWidget *parent) : QDialog(parent), ui(new Ui::settings)
{
    setWindowTitle("Настройки");

    min_to_tray = new QCheckBox(this);
    min_to_tray->setText("Сворачивать в трей");
    close_to_tray = new QCheckBox(this);
    close_to_tray->setText("Сворачивать в трей при закрытии");
    run_in_tray = new QCheckBox(this);
    run_in_tray->setText("Сворачивать в трей при запуске");
    activate_filters = new QCheckBox(this);
    activate_filters->setText("Активировать фильтры при добавлении");
    activate_feeds = new QCheckBox(this);
    activate_feeds->setText("Активировать ленты при добавлении");


    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(min_to_tray);
    layout->addWidget(close_to_tray);
    layout->addWidget(run_in_tray);
    layout->addWidget(activate_filters);
    layout->addWidget(activate_feeds);
    layout->setMargin(0); // убирает промежуток между layout и границами окна
    setLayout(layout);

    this->setWindowIcon(QIcon(":/settings.ico"));
}

Settings::~Settings()
{
    delete ui;
    delete min_to_tray;
    delete close_to_tray;
    delete run_in_tray;
    delete activate_filters;
    delete activate_feeds;
}

// чтение сохранённых настроек
void Settings::read_settings()
{
    QString str;


    QString name = qApp->applicationDirPath() + QDir::separator() + settingFile;
    QSettings setting_file(name, QSettings::IniFormat);
    min_to_tray->setChecked(setting_file.value("min_to_tray", false).toBool());
    close_to_tray->setChecked(setting_file.value("close_to_tray", false).toBool());
    run_in_tray->setChecked(setting_file.value("run_in_tray", false).toBool());
    activate_filters->setChecked(setting_file.value("activate_filters", false).toBool());
    activate_feeds->setChecked(setting_file.value("activate_feeds", false).toBool());
}

// сохранение настроек и выход
void Settings::write_settings()
{
    QString name = qApp->applicationDirPath() + QDir::separator() + settingFile;
    QSettings setting_file(name, QSettings::IniFormat);
    if (setting_file.isWritable())
    {
        setting_file.setValue("min_to_tray", min_to_tray->isChecked());
        setting_file.setValue("close_to_tray", close_to_tray->isChecked());
        setting_file.setValue("run_in_tray", run_in_tray->isChecked());
        setting_file.setValue("activate_filters", activate_filters->isChecked());
        setting_file.setValue("activate_feeds", activate_feeds->isChecked());
    }
}

void Settings::closeEvent(QCloseEvent *)
{
    write_settings();
}
