#ifndef EDITWINDOW_H
#define EDITWINDOW_H

#include <QWidget>
#include <QLineEdit>
#include <QVBoxLayout>

#include "feeds.h"

#include <pthread.h> // для использования потоков для "оживления" поиска
#include <semaphore.h>


class EditWindow : public QWidget
{
    Q_OBJECT
public:
    enum Type {enFeed, enFilter};
    explicit EditWindow(QWidget *parent = 0);
    EditWindow(const EditWindow::Type type, const int id, QWidget *parent=0, bool select=false);
    ~EditWindow();

signals:

public slots:
    private slots:
    void SaveData();
    void search_artists();
    void searching(QString path);
    void closeEvent(QCloseEvent * event) Q_DECL_OVERRIDE;

private:
    Feeds *pFeeds;
    Type type;
    int id;
    int idFeed;
    bool select;
    bool search_init = true;

    QLabel *nameLabel;
    QLabel *valueLabel;
    QLineEdit *nameEdit;
    QLineEdit *valueEdit;
    QCheckBox *enableCheck;
    QPushButton *okButton;
    QPushButton *cancelButton;

    QVBoxLayout *mainlayout;
    QVBoxLayout *v_layout;
    QHBoxLayout *filters_layout;
    QHBoxLayout *buttons_layout;
    QLabel *hint;
    QPushButton *artistsButton;
    QStringList artists;

    pthread_t artists_trd;  // поток артистов
    sem_t artists_sem;    // семафор артистов
};

#endif // EDITWINDOW_H
