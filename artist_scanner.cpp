#include "artist_scanner.h"
#include "feedsandfilters.h"

#include "taglib/fileref.h"
#include "taglib/taglib.h"
#include "taglib/tag.h"

#include <QDir>
#include <QDebug>

artist_scanner::artist_scanner()
{

}

void artist_scanner::receive_params(QString path, int id, int idFeed, bool is_checked)
{
    qDebug() << "path = " << path;
    this->path = path;
    this->id = id;
    this->idFeed = idFeed;
    this->is_checked = is_checked;
}


void artist_scanner::searching(void)
{
#ifdef Q_OS_WIN32
    artists.clear();

    search(path);

    is_done = true; // папка проверена
    emit send(artist_num, file_num, is_done);

    // запись данных в БД (надо как-то потом переработать дублирование функции SaveData)
    for (int i = 0; i < artists.size(); i++)
    {
        filters_struct filter(id);
        filter.comment = "Автопоиск";
        filter.title = artists.at(i);// + valueEdit->text();
        filter.is_on = is_checked;
        filter.idFeed = idFeed;
        pFeeds->SaveFilter(&filter);
    }
#endif
}

void artist_scanner::search(QString path)
{
#ifdef Q_OS_WIN32
    //qDebug() <<  "*********************************************************************************************************************";
    //qDebug() <<  path;
    bool new_artist;

    // http://www.cyberforum.ru/qt/thread644532.html   -   там есть пример и без рекурсии, но так как-то понятнее
    QDir currentFolder(path);

    //currentFolder.setFilter(QDir::Dirs | QDir::Files);
    currentFolder.setFilter(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot | QDir::QDir::NoSymLinks);
    currentFolder.setSorting(QDir::Name);

    QFileInfoList folderitems(currentFolder.entryInfoList());

    foreach (QFileInfo i_file, folderitems)
    {
        QString i_filename(i_file.fileName());
        if (/*i_filename == "." || i_filename == ".." || */i_filename.isEmpty())
            continue;

        if (i_file.isDir())
            search(path + "/" + i_filename);
        else
        {
            QString full_filename = path + "/" + i_filename;
            //step++;
            //qDebug() <<  step << " - " << full_filename;


            if (full_filename.contains(".mp3", Qt::CaseInsensitive) || full_filename.contains(".flac", Qt::CaseInsensitive) || full_filename.contains(".wma", Qt::CaseInsensitive) || full_filename.contains(".wav", Qt::CaseInsensitive))
            {
                TagLib::FileRef ref(full_filename.toStdWString().c_str()); // только с таким преобразованием не игнорируются папки с нелатинскими буквами
                if (!ref.isNull() && ref.tag() != NULL)
                {
                    //QString data = QString("%1").arg(ref.tag()->artist().toCString(true));
                    //QString data = QString::fromStdString(ref.tag()->artist().toCString(true));
                    //QString data = QString::fromStdWString(ref.tag()->artist().toWString());
                    QString data = TStringToQString(ref.tag()->artist());
                    new_artist = true;
                    for (int i = 0; i < artists.size(); i++)
                        if (data.trimmed() == artists.at(i).trimmed())
                            new_artist = false;
                    if (new_artist && data != "") // на случай, если в поле "исполнитель" ничего нет
                    {
                        artists.push_back(data);
                        artist_num++;
                    }
                }
           }
           file_num++;
        }
        emit send(artist_num, file_num, is_done);
    }
#endif
}
