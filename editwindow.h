#ifndef EDITWINDOW_H
#define EDITWINDOW_H

#include <QWidget>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QLabel>

#include "feeds.h"
#include "settings.h"


class EditWindow : public QWidget
{
    Q_OBJECT

public:
    enum Type {enFeed, enFilter};
    explicit EditWindow(QWidget *parent = 0);
    EditWindow(const EditWindow::Type type, const int id, QWidget *parent=0, bool select = false);
    ~EditWindow();
    Settings *settings = nullptr;

signals:
    void send_params(QString, int, int, bool);

public slots:

private slots:
    void SaveData();
    void search_artists();
    void showEvent(QShowEvent * event) Q_DECL_OVERRIDE;
    void closeEvent(QCloseEvent * event) Q_DECL_OVERRIDE;
    void update (int i, int j, bool flag);
    void set_text();
    void set_focus_line();

private:
    Feeds *pFeeds;
    Type type;
    int id;
    int idFeed;
    bool select;
    bool adding;    // флаг для разграничения - ввод нового или редактирование
    int added_values;

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

    QString text_title_add_feed;
    QString text_title_add_filter;
    QString text_title_edit_feed;
    QString text_title_edit_filter;
    QString text_namelabel_feed;
    QString text_valuelabel_feed;
    QString text_namelabel_filter;
    QString text_valuelabel_filter;
    QString text_placeholder;
    QString text_error_link;
    QString text_error_empty;
    QString text_search_dialog;
    QString text_search_folder;
    QString text_search_process;
    QString text_search_result;
    QString text_search_finished;
    QString text_button_ok;
    QString text_button_cancel;
    QString text_button_artists;
    QString text_button_check;
    QString text_added_feeds;
    QString text_added_filters;
};

#endif // EDITWINDOW_H
