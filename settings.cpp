#include "settings.h"
#include "ui_settings.h"

#include <QVBoxLayout>

settings::settings(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::settings)
{
    setWindowTitle("Настройки");

    min_to_tray = new QCheckBox(this);
    min_to_tray->setText("Сворачивать в трей");
    min_to_tray->setChecked(true);
    close_to_tray = new QCheckBox(this);
    close_to_tray->setText("Сворачивать в трей при закрытии");
    close_to_tray->setChecked(true);
    run_in_tray = new QCheckBox(this);
    run_in_tray->setText("Запускать свёрнутым в трей");
    run_in_tray->setChecked(false);
    activate_filter = new QCheckBox(this);
    activate_filter->setText("Активировать фильтры при добавлении");
    activate_filter->setChecked(true);
    activate_feed = new QCheckBox(this);
    activate_feed->setText("Активировать ленты при добавлении");
    activate_feed->setChecked(true);


    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(min_to_tray);
    layout->addWidget(close_to_tray);
    layout->addWidget(run_in_tray);
    layout->addWidget(activate_filter);
    layout->addWidget(activate_feed);
    layout->setMargin(0); // убирает промежуток между layout и границами окна
    setLayout(layout);
}

settings::~settings()
{
    delete ui;
}
