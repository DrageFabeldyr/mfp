#include "feedmodel.h"


FeedModel::FeedModel(QObject *parent) : QSqlQueryModel(parent)
{
    pFeeds = static_cast<DFRSSFilter*>(parent)->pFeeds;
    prepQuery = "select status, title, link, id from FEEDS";
    setQuery(prepQuery, pFeeds->db);
    checkedColumn = 0; // колонка в которой CheckBox
}

void FeedModel::Update()
{
    setQuery(prepQuery, pFeeds->db);
}

QVariant FeedModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
        switch (section) {
        case 0:
            return "";
            break;
        case 1:
            return "Заголовок";
            break;
        case 2:
            return "Ссылка";
            break;
        case 3:
            return "id";
            break;
        default:
            return "";
            break;
        }
    }
    return QVariant();

}


QVariant FeedModel::data(const QModelIndex &index, int role) const
{
    QVariant value = QSqlQueryModel::data(index, role);

    if (role == Qt::CheckStateRole && index.column() == checkedColumn)
    {
        Qt::CheckState eChecked = static_cast< Qt::CheckState >( index.data().toInt() );
        int temp = static_cast< int >( eChecked ? Qt::Checked : Qt::Unchecked );
        return temp;//static_cast< int >( eChecked ? Qt::Checked : Qt::Unchecked );
    }

    if (role == Qt::TextColorRole && index.column() == checkedColumn)
    {
       return QColor(Qt::white);
    }
    return value;

}

bool FeedModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    Q_UNUSED(value);

    if (role != Qt::EditRole && role != Qt::CheckStateRole )
        return false;

    bool result;

    if (role == Qt::CheckStateRole && index.column() == checkedColumn)
    {
        Qt::CheckState eChecked = static_cast< Qt::CheckState >( index.data().toInt() );
        int temp = static_cast< int >( eChecked ? Qt::Checked : Qt::Unchecked );
        QModelIndex id = this->index(index.row(), 3);
        if (temp)
            pFeeds->ChangeStatusFeed(id.data().toInt(), 0);
        else
            pFeeds->ChangeStatusFeed(id.data().toInt(), 1);
        Update();
        result = true; // запись в базу
    }
    else
    {
        result = true;
    }
    if (result)
        emit dataChanged(index, index);
    return result;
}

Qt::ItemFlags FeedModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;

    Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

    if (index.column() == checkedColumn)
        flags |= Qt::ItemIsUserCheckable;

    return flags;
}
