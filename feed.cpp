#include "feed.h"

Feed::Feed()
{
    title = "";
    link = "";
    is_on = false;
    id = -1;
    filters.clear();
}

Feed::Feed(int id)
{
    title = "";
    link = "";
    is_on = false;
    this->id = id;
    filters.clear();
}

void Feed::Clear()
{
    title = "";
    link = "";
    is_on = false;
    id = -1;
    filters.clear();
}
