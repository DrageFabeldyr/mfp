#include "filters_struct.h"

filters_struct::filters_struct(int id)
{
    this->id = id;
    idFeed = -1;
    title = "";
    comment = "";
    is_on = false;
}

filters_struct::filters_struct()
{
    id = -1;
    idFeed = -1;
    title = "";
    comment = "";
    is_on = false;
}

void filters_struct::Clear()
{
    id = -1;
    idFeed = -1;
    title = "";
    comment = "";
    is_on = false;
}
