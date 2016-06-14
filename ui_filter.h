/********************************************************************************
** Form generated from reading UI file 'filter.ui'
**
** Created by: Qt User Interface Compiler version 5.4.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_FILTER_H
#define UI_FILTER_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_filter
{
public:

    void setupUi(QWidget *filter)
    {
        if (filter->objectName().isEmpty())
            filter->setObjectName(QStringLiteral("filter"));
        filter->resize(400, 300);

        retranslateUi(filter);

        QMetaObject::connectSlotsByName(filter);
    } // setupUi

    void retranslateUi(QWidget *filter)
    {
        filter->setWindowTitle(QApplication::translate("filter", "Form", 0));
    } // retranslateUi

};

namespace Ui {
    class filter: public Ui_filter {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_FILTER_H
