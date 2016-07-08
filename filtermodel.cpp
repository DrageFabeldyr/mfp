#include "filtermodel.h"


FilterModel::FilterModel(QObject *parent) : QSqlQueryModel(parent)
{
    pFeeds = static_cast<DFRSSFilter*>(parent)->pFeeds;
    prepQuery = "select status, title, value, id from FILTER WHERE idFeed = " + idFeed;
    setQuery(prepQuery, pFeeds->db);
    checkedCollum = 0; // колонка в которой CheckBox
}

void FilterModel::Update()
{
    prepQuery = "select status, title, value, id from FILTER WHERE idFeed = " + idFeed;
    setQuery(prepQuery, pFeeds->db);
}

bool FilterModel::setData(const QModelIndex & index, const QVariant & value, int role)
{
    Q_UNUSED(value)
    if ( role == Qt::CheckStateRole && index.column() == checkedCollum )
    {
        Qt::CheckState eChecked = static_cast< Qt::CheckState >( index.data().toInt() );
        int temp = static_cast< int >( eChecked ? Qt::Checked : Qt::Unchecked );
        QModelIndex id = this->index(index.row(), 3);
        if (temp)
            pFeeds->ChangeStatusFilter(id.data().toInt(), 0);
        else
            pFeeds->ChangeStatusFilter(id.data().toInt(), 1);
        Update();
        return true;
    }
    return false;
}

QVariant FilterModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
        switch (section) {
        case 0:
            return "";
            break;
        case 1:
            return "Значение";
            break;
        case 2:
            return "Комментарий";
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


QVariant FilterModel::data(const QModelIndex &index, int role) const
{
    QVariant value = QSqlQueryModel::data(index, role);

    if ( role == Qt::CheckStateRole && index.column() == checkedCollum )
    {
        Qt::CheckState eChecked = static_cast< Qt::CheckState >( index.data().toInt() );
        int temp = static_cast< int >( eChecked ? Qt::Checked : Qt::Unchecked );
        return temp;
    }

    if ( role == Qt::TextColorRole && index.column() == checkedCollum)
    {
       return QColor(Qt::white);
    }
    return value;

}

Qt::ItemFlags FilterModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;

    Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

    if (index.column() == checkedCollum)
        flags |= Qt::ItemIsUserCheckable;

    return flags;
}
