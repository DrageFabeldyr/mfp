#ifndef FILTERS_STRUCT_H
#define FILTERS_STRUCT_H

#include <QString>

class filters_struct
{
public:
    QString title;
    QString feed;
    bool is_on;
    QString GetEncodedTitle( void ) const;
private:
     QString static encodeEntities( QString& src, const QString& force=QString()) ;
};

#endif // FILTERS_STRUCT_H
