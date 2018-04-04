#include "feeds.h"
#include <QStandardPaths>   // для размещения базы в андроиде

Feeds::Feeds()
{
    OpenDB();
}

void Feeds::GetFeeds(QList<Feed> &feeds)
{
    QSqlQuery query;
    query.exec("SELECT * FROM FEEDS ");
    while (query.next())
    {
        Feed feed;
        feed.id = query.value(0).toInt();
        feed.link = query.value(1).toString();
        feed.title = query.value(2).toString();
        feed.is_on = query.value(3).toBool();
        feeds.push_back(feed);
    }
}

void Feeds::GetFilters(QList<filters_struct> &filters, const Feed *feed)
{
    QSqlQuery query;
    QString prepQuery = "SELECT * FROM FILTER WHERE idFeed = :id ";
    query.prepare(prepQuery);
    query.bindValue(":id", feed->id);
    while (query.next())
    {
        filters_struct filter;
        filter.id = query.value(0).toInt();
        filter.title = query.value(2).toString();
        filter.comment = query.value(3).toString();
        filter.is_on = query.value(4).toBool();
        filters.push_back(filter);
    }
}

void Feeds::OpenDB()
{
#ifdef Q_OS_WIN32
    QString name = qApp->applicationDirPath() + QDir::separator() + "DATA.DB";
#endif
#ifdef Q_OS_ANDROID

    QString folder = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    //QFileInfo databaseFileInfo(QString("%1/%2").arg(folder).arg("DATA.DB"));
    //QString  name = databaseFileInfo.absoluteFilePath();
    QString  name = folder + "/DATA.DB";
    //*/
    QString new_name = QStandardPaths::locate(QStandardPaths::DownloadLocation, "DATA.DB", QStandardPaths::LocateFile);

    //QString name = "assets:/DATA.DB";
    //QFile(name).setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ReadUser | QFileDevice::WriteUser | QFileDevice::ReadOther | QFileDevice::WriteOther);
    //QFile(new_name).setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ReadUser | QFileDevice::WriteUser | QFileDevice::ReadOther | QFileDevice::WriteOther);

    QFileInfo databaseFileInfo(name);
    if (databaseFileInfo.exists())
    {
        QMessageBox::critical(0, QString("Yeah:"), QString("Database exists at %1").arg(name), QMessageBox::Ok);
    }
    else
    {
        QMessageBox::critical(0, QString("Fuck:"), QString("No database").arg(name), QMessageBox::Ok);
    }
    QFileInfo newdatabaseFileInfo(new_name);
    if (newdatabaseFileInfo.exists())
    {
        QMessageBox::critical(0, QString("Attention!"), QString("New database at %1").arg(new_name), QMessageBox::Ok);
        QFile::remove(name); // удалим старую базу
        bool copySuccess = QFile::copy(new_name, name);
        if (copySuccess)
        {
            QMessageBox::critical(0, QString("Yeah!"), "Database update success", QMessageBox::Ok);
            //newdatabaseFileInfo.setCaching(false);
            bool removetest = QFile::remove(new_name); // не отображается в проводнике, но работает
            if (removetest)
                QMessageBox::critical(0, QString("Yeah!"), "Success", QMessageBox::Ok);
            else
                QMessageBox::critical(0, QString("Error!"), "Suck ass", QMessageBox::Ok);

        }
        else
        {
            QMessageBox::critical(0, QString("Error!"), "Database update sucks ass", QMessageBox::Ok);
        }
    }
    else
    {
        QMessageBox::critical(0, QString("Fuck:"), QString("No new database").arg(name), QMessageBox::Ok);
    }
#endif

    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(name);
    if (!db.open())
    {
        QMessageBox::critical(0, qApp->tr("Невозможно открыть базу"), qApp->tr("Невозможно создать подключение"), QMessageBox::Cancel);
        return ;
    }

    QSqlQuery query;
    query.exec("CREATE TABLE IF NOT EXISTS FEEDS (id INTEGER PRIMARY KEY AUTOINCREMENT UNIQUE, "
               "link varchar(100), "
               "title varchar(100), "
               "status INTEGER)");
    query.exec("CREATE TABLE IF NOT EXISTS FILTER (id INTEGER PRIMARY KEY AUTOINCREMENT UNIQUE, "
               "idFeed INTEGER,  "
               "title varchar(100), "
               "value varchar(100), "
               "status INTEGER)");
}

void Feeds::GetFeedById(Feed * feed)
{
    QSqlQuery query;
    QString prepQuery = "SELECT * FROM FEEDS WHERE id = :id ";
    query.prepare(prepQuery);
    query.bindValue(":id", feed->id);
    if(!query.exec())
    {
        return;
    }
    query.first();

    feed->title = query.value(2).toString();
    feed->link = query.value(1).toString();
    feed->is_on = query.value(3).toBool();
}

void Feeds::GetFilterById(filters_struct * filter)
{
    QSqlQuery query;
    QString prepQuery = "SELECT * FROM FILTER WHERE id = :id ";
    query.prepare(prepQuery);
    query.bindValue(":id", filter->id);
    if(!query.exec())
    {
        return;
    }
    query.first();

    filter->title = query.value(2).toString();
    filter->comment = query.value(3).toString();
    filter->is_on = query.value(4).toBool();
}

void Feeds::SaveFeed(Feed * feed)
{
    if (feed->id == -1)
    {
        AddFeed(feed);
    }
    else
    {
        UpdateFeed(feed);
    }
}
void Feeds::SaveFilter(filters_struct * filter)
{
    if (filter->id == -1)
    {
        AddFilter(filter);
    }
    else
    {
        UpdateFilter(filter);
    }
}
void Feeds::AddFeed(Feed *feed)
{
    QSqlQuery query;
    // сперва ищем совпадения
    QString prepQuery = "SELECT * FROM FEEDS WHERE link = :link";
    query.prepare(prepQuery);
    query.bindValue(":link", feed->link);
    query.exec();
    if (query.next()) // если нашли
        return; // выходим
    query.clear();

    // если не нашли - добавляем
    prepQuery = "INSERT INTO FEEDS (link, title, status) VALUES (:link, :title, :status) ";
    query.prepare(prepQuery);
    query.bindValue(":link", feed->link);
    query.bindValue(":title", feed->title);
    query.bindValue(":status", feed->is_on);
    query.exec();
}

void Feeds::UpdateFeed(Feed *feed)
{
    QSqlQuery query;
    QString prepQuery = "UPDATE FEEDS SET link = :link, title = :title, status = :status WHERE id = :id ";
    query.prepare(prepQuery);
    query.bindValue(":link", feed->link);
    query.bindValue(":title", feed->title);
    query.bindValue(":status", feed->is_on);
    query.bindValue(":id", feed->id);
    query.exec();
}

void Feeds::AddFilter(filters_struct *filter)
{
    QSqlQuery query;
    // сперва ищем совпадения
    QString prepQuery = "SELECT * FROM FILTER WHERE LOWER(title) = LOWER(:title) AND idFeed = :idFeed";
    // lower нужен для того, чтобы привести значение в таблице и наше к нижнему регистру
    // для регистронезависимого поиска
    query.prepare(prepQuery);
    query.bindValue(":title", filter->title);
    query.bindValue(":idFeed", filter->idFeed); // в конкретной ленте
    query.exec();
    if (query.next()) // если нашли
        return; // выходим
    query.clear();

    // если не нашли - добавляем
    prepQuery = "INSERT INTO FILTER (idFeed, title, value, status) VALUES (:idFeed, :title, :value, :status) ";
    query.prepare(prepQuery);
    query.bindValue(":idFeed", filter->idFeed);
    query.bindValue(":title", filter->title);
    query.bindValue(":value", filter->comment);
    query.bindValue(":status", filter->is_on);
    query.exec();
}

void Feeds::UpdateFilter(filters_struct *filter)
{
    QSqlQuery query;
    QString prepQuery = "UPDATE FILTER SET title = :title, value = :value, status = :status WHERE id = :id ";
    query.prepare(prepQuery);
    query.bindValue(":title", filter->title);
    query.bindValue(":value", filter->comment);
    query.bindValue(":status", filter->is_on);
    query.bindValue(":id", filter->id);
    query.exec();
}

void Feeds::DeleteFeedById(int id)
{
    QSqlQuery query;
    QString prepQuery = "DELETE FROM FEEDS  WHERE id = :id ";
    query.prepare(prepQuery);
    query.bindValue(":id", id);
    query.exec();
    query.clear();
    prepQuery = "DELETE FROM FILTER  WHERE idFeed = :id ";
    query.prepare(prepQuery);
    query.bindValue(":id", id);
    query.exec();
}

void Feeds::DeleteFilterById(int id)
{
    QSqlQuery query;
    QString prepQuery = "DELETE FROM FILTER  WHERE id = :id ";
    query.prepare(prepQuery);
    query.bindValue(":id", id);
    query.exec();
}

void Feeds::GetActiveFeedsList(QList<Feed> &feeds)
{
    QSqlQuery query;
    QString prepQuery = "SELECT * FROM FEEDS WHERE status = :status ";
    query.prepare(prepQuery);
    query.bindValue(":status", 1);
    if(!query.exec())
    {
        return;
    }
    Feed feed;
    while (query.next())
    {
        feed.id = query.value(0).toInt();
        feed.link = query.value(1).toString();
        feed.title = query.value(2).toString();
        feed.is_on = query.value(3).toBool();

        feeds.push_back(feed);
        feed.Clear();
    }
}

void Feeds::GetActiveFiltersList(QList<filters_struct> &filters, const int id)
{
    QSqlQuery query;
    QString prepQuery = "SELECT * FROM FILTER WHERE idFeed = :id and status = '1' ";
    query.prepare(prepQuery);
    query.bindValue(":id", id);
    if(!query.exec())
    {
        return;
    }
    filters_struct filter;
    while (query.next())
    {
        filter.id = query.value(0).toInt();
        filter.idFeed = query.value(1).toInt();
        filter.title = query.value(2).toString();
        filter.comment = query.value(3).toString();
        filter.is_on = query.value(4).toBool();
        filters.push_back(filter);
        filter.Clear();
    }
}

// чтобы по клику менялись галочки фильтров
void Feeds::ChangeStatusFilter(const int id, const int status)
{
    QSqlQuery query;
    QString prepQuery = "UPDATE FILTER SET status=:status WHERE id = :id ";
    query.prepare(prepQuery);
    query.bindValue(":id", id);
    query.bindValue(":status", status);
    if(!query.exec())
    {
        return;
    }
}

// чтобы по клику менялись галочки лент
void Feeds::ChangeStatusFeed(const int id, const int status)
{
    QSqlQuery query;
    QString prepQuery = "UPDATE FEEDS SET status = :status WHERE id = :id ";
    query.prepare(prepQuery);
    query.bindValue(":id", id);
    query.bindValue(":status", status);
    if(!query.exec())
    {
        return;
    }
}

// удаление всех фильтров
void Feeds::DeleteAllFilters(int id_feed)
{
    QSqlQuery query;
    QString prepQuery = "DELETE FROM FILTER  WHERE idFeed = :id ";
    query.prepare(prepQuery);
    query.bindValue(":id", id_feed);
    query.exec();
}

void Feeds::ChangeStatusAllFilters(int id_feed, int status)
{
    QSqlQuery query;
    QString prepQuery = "UPDATE FILTER SET status = :status WHERE idFeed = :id ";
    query.prepare(prepQuery);
    query.bindValue(":id", id_feed);
    query.bindValue(":status", status);
    if(!query.exec())
    {
        return;
    }
}
