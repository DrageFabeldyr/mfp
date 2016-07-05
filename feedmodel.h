#ifndef FEEDMODEL_H
#define FEEDMODEL_H

#include <QSqlQueryModel>
#include <QObject>
#include <QString>

#include "dfrssfilter.h"
#include "feeds.h"

class FeedModel : public QSqlQueryModel
{
    Q_OBJECT
public:
    explicit FeedModel(QObject * parent = 0);
    QVariant data(const QModelIndex &index, int role) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    void Update();

private:
    Feeds *pFeeds;
    QString prepQuery;
    int checkedColumn;
};



#endif // FEEDMODEL_H
