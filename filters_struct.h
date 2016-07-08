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
    QString comment;
    bool is_on;
    void Clear();
};

#endif // FILTERS_STRUCT_H
