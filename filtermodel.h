#ifndef FILTERMODEL_H
#define FILTERMODEL_H

#include <QSqlQueryModel>
#include <QObject>
#include <QString>

#include "dfrssfilter.h"
#include "feeds.h"

class FilterModel : public QSqlQueryModel
{
    Q_OBJECT
public:
    explicit FilterModel(QObject * parent = 0);
    QVariant data(const QModelIndex &index, int role) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole) Q_DECL_OVERRIDE;

    QString idFeed;
    void Update();

private:
    Feeds *pFeeds;
    QString prepQuery;
    int checkedCollum;
};



#endif // FILTERMODEL_H
