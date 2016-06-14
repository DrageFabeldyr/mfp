#ifndef SETTINGS_H
#define SETTINGS_H

#include <QDialog>
#include <QCheckBox>

namespace Ui {
class settings;
}

class settings : public QDialog
{
    Q_OBJECT

public:
    explicit settings(QWidget *parent = 0);
    ~settings();

private:
    Ui::settings *ui;
    QCheckBox *min_to_tray, *close_to_tray, *run_in_tray;
    QCheckBox *activate_filter, *activate_feed;

};

#endif // SETTINGS_H
