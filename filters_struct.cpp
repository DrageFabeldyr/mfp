#include "filters_struct.h"


QString filters_struct::GetEncodedTitle( void ) const
{
    QString temp = title;
    temp = temp.trimmed();
    return encodeEntities(temp);
}

/* функция преобразования нестандартных символов к виду &#xxx;
 * взята отсюда:
 * http://stackoverflow.com/questions/7696159/how-can-i-convert-entity-characterescape-character-to-html-in-qt
 * автор http://stackoverflow.com/users/1455977/immortalpc
 */
QString filters_struct::encodeEntities( QString& src, const QString& force)
{
    QString tmp(src);
    uint len = tmp.length();
    uint i = 0;
    while(i < len)
    {
        if(tmp[i].unicode() > 128 || force.contains(tmp[i]))
        {
            QString rp = "&#"+QString::number(tmp[i].unicode())+";";
            tmp.replace(i,1,rp);
            len += rp.length()-1;
            i += rp.length();
        }
        else
        {
            ++i;
        }
    }
    return tmp;
}