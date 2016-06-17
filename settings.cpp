#include "settings.h"
#include "ui_settings.h"

#include <QVBoxLayout>
#include <QDir>
#include <QFile>
#include <QTextStream>


settings::settings(QWidget *parent) : QDialog(parent), ui(new Ui::settings)
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

settings::~settings()
{
    delete ui;
    delete min_to_tray;
    delete close_to_tray;
    delete run_in_tray;
    delete activate_filters;
    delete activate_feeds;
}

// чтение сохранённых настроек
void settings::read_settings()
{
    QString str;

    QString name = qApp->applicationDirPath() + QDir::separator() + "setts.gsd";
    QFile file(name); // создаем объект класса QFile
    if(file.open(QIODevice::ReadOnly |QIODevice::Text)) // если файл открылся и там текст
    {
        while(!file.atEnd()) // пока не упрёмся в конец файла
        {
            str = file.readLine(); // читаем строку

            if (str.contains("min_to_tray=1"))
                min_to_tray->setChecked(true);
            if (str.contains("min_to_tray=0"))
                min_to_tray->setChecked(false);

            if (str.contains("close_to_tray=1"))
                close_to_tray->setChecked(true);
            if (str.contains("close_to_tray=0"))
                close_to_tray->setChecked(false);

            if (str.contains("run_in_tray=1"))
                run_in_tray->setChecked(true);
            if (str.contains("run_in_tray=0"))
                run_in_tray->setChecked(false);

            if (str.contains("activate_filters=1"))
                activate_filters->setChecked(true);
            if (str.contains("activate_filters=0"))
                activate_filters->setChecked(false);

            if (str.contains("activate_feeds=1"))
                activate_feeds->setChecked(true);
            if (str.contains("activate_feeds=0"))
                activate_feeds->setChecked(false);
        }

        file.close(); // прочли и закрыли
    }
}

// сохранение настроек и выход
void settings::write_settings()
{
    QString name = qApp->applicationDirPath() + QDir::separator() + "setts.gsd";
    QFile file(name); // создаем объект класса QFile
    if(file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) // открываем для записи и предварительно полностью обнуляем файл
    {
        QTextStream outStream(&file);

        if (min_to_tray->isChecked())
            outStream << "min_to_tray=1" << endl;
        else
            outStream << "min_to_tray=0" << endl;

        if (close_to_tray->isChecked())
            outStream << "close_to_tray=1" << endl;
        else
            outStream << "close_to_tray=0" << endl;

        if (run_in_tray->isChecked())
            outStream << "run_in_tray=1" << endl;
        else
            outStream << "run_in_tray=0" << endl;

        if (activate_filters->isChecked())
            outStream << "activate_filters=1" << endl;
        else
            outStream << "activate_filters=0" << endl;

        if (activate_feeds->isChecked())
            outStream << "activate_feeds=1" << endl;
        else
            outStream << "activate_feeds=0" << endl;

        file.close(); // записали и закрыли
    }
}

void settings::closeEvent(QCloseEvent *)
{
    write_settings();
}
