#include "feed.h"

Feed::Feed()
{
    title = "";
    link = "";
    is_on = false;
    id = -1;
    filters.clear();
    filters_num = 0;
}

Feed::Feed(int id)
{
    title = "";
    link = "";
    is_on = false;
    this->id = id;
    filters.clear();
    filters_num = 0;
}

void Feed::Clear()
{
    title = "";
    link = "";
    is_on = false;
    id = -1;
    filters.clear();
    filters_num = 0;
}
