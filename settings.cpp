#include "settings.h"
#include "ui_settings.h"

#include <QDir>
#include <QSettings>

#include "dfrssfilter.h"


Settings::Settings(QWidget *parent) : QWidget(parent)
{
    parentW = parent;

    layout = new QVBoxLayout;
    min_to_tray = new QCheckBox(this);
    close_to_tray = new QCheckBox(this);
    run_in_tray = new QCheckBox(this);
    activate_filters = new QCheckBox(this);
    activate_feeds = new QCheckBox(this);

    layout->addWidget(min_to_tray);
    layout->addWidget(close_to_tray);
    layout->addWidget(run_in_tray);
    layout->addWidget(activate_filters);
    layout->addWidget(activate_feeds);

#ifdef Q_OS_ANDROID
    min_to_tray->setVisible(false);
    close_to_tray->setVisible(false);
    run_in_tray->setVisible(false);
    save_settings_button = new QPushButton(tr("Сохранить"), this);
    connect(save_settings_button, SIGNAL(clicked()), this, SLOT(close())); // запуск по нажатию кнопки
    layout->addWidget(save_settings_button);
#endif

    layout->setMargin(0); // убирает промежуток между layout и границами окна
    setLayout(layout);

    timer = new QTimer(this);

    this->setWindowIcon(QIcon(":/img/settings.ico"));
}

Settings::~Settings()
{
    delete layout;
    delete min_to_tray;
    delete close_to_tray;
    delete run_in_tray;
    delete activate_filters;
    delete activate_feeds;
    delete timer;
}

// чтение сохранённых настроек
void Settings::read_settings()
{
#ifdef Q_OS_WIN32
    QString name = qApp->applicationDirPath() + QDir::separator() + settingFile;
#endif
#ifdef Q_OS_ANDROID
    QString name = "assets:/" + settingFile;
#endif
    QSettings setting_file(name, QSettings::IniFormat);
    min_to_tray->setChecked(setting_file.value("min_to_tray", false).toBool());
    close_to_tray->setChecked(setting_file.value("close_to_tray", false).toBool());
    run_in_tray->setChecked(setting_file.value("run_in_tray", false).toBool());
    activate_filters->setChecked(setting_file.value("activate_filters", false).toBool());
    activate_feeds->setChecked(setting_file.value("activate_feeds", false).toBool());

    activateFilters = setting_file.value("activate_filters", false).toBool();
    activateFeeds = setting_file.value("activate_feeds", false).toBool();

    request_period = 10*60*1000; // запрос новостей раз в 10 минут (потом будет чтение из файла)
    show_period = 20*1000; // уведомление в трее будет висеть 20 секунд

    //timer->setInterval(request_init_period);
}

// сохранение настроек
void Settings::write_settings()
{
    activateFilters = activate_filters->isChecked();
    activateFeeds = activate_feeds->isChecked();
#ifdef Q_OS_WIN32
    QString name = qApp->applicationDirPath() + QDir::separator() + settingFile;
#endif
#ifdef Q_OS_ANDROID
    QString name = "assets:/" + settingFile;
#endif
    QSettings setting_file(name, QSettings::IniFormat);
    if (setting_file.isWritable())
    {
        setting_file.setValue("min_to_tray", min_to_tray->isChecked());
        setting_file.setValue("close_to_tray", close_to_tray->isChecked());
        setting_file.setValue("run_in_tray", run_in_tray->isChecked());
        setting_file.setValue("activate_filters", activate_filters->isChecked());
        setting_file.setValue("activate_feeds", activate_feeds->isChecked());
    }
#ifdef Q_OS_ANDROID
    QMessageBox::critical(0, QString("Yeah:"), QString("Settings saved at %1").arg(name), QMessageBox::Ok);
#endif
}

void Settings::closeEvent(QCloseEvent *event)
{
    write_settings();
    hide();
    event->ignore();
    timer->start(request_period); // закрыли окно - запустили таймер с новым периодом
}

void Settings::showEvent(QShowEvent * event)
{
    Q_UNUSED(event);
    read_settings();
    //всегда в центре родителя
    move(parentW->window()->frameGeometry().topLeft() +
         parentW->window()->rect().center() - rect().center());
    timer->stop(); // останавливаем таймер на время работы с окном
    switch (current_language)
    {
    case 1:
        setWindowTitle("Настройки");
        min_to_tray->setText("Сворачивать в трей");
        close_to_tray->setText("Сворачивать в трей при закрытии");
        run_in_tray->setText("Сворачивать в трей при запуске");
        activate_filters->setText("Активировать фильтры при добавлении");
        activate_feeds->setText("Активировать ленты при добавлении");
        break;
    case 2:
        setWindowTitle("Settings");
        min_to_tray->setText("Minimize to tray");
        close_to_tray->setText("Close to tray");
        run_in_tray->setText("Start minimized to tray");
        activate_filters->setText("Set new filters active");
        activate_feeds->setText("Set new feeds active");
        break;
    default:
        break;
    }
}
