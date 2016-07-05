#ifndef FILTERS_STRUCT_H
#define FILTERS_STRUCT_H

#include <QString>

class filters_struct
{
public:
    filters_struct();
    filters_struct(int id);
    int id;
    int idFeed;
    QString title;
    //QString feed;
    QString comment;
    bool is_on;
    //QString GetEncodedTitle( void ) const;
    void Clear();
private:
    // QString static encodeEntities( QString& src, const QString& force=QString()) ;
};

#endif // FILTERS_STRUCT_H
