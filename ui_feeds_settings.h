/********************************************************************************
** Form generated from reading UI file 'feeds_settings.ui'
**
** Created by: Qt User Interface Compiler version 5.4.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_FEEDS_SETTINGS_H
#define UI_FEEDS_SETTINGS_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDialog>
#include <QtWidgets/QHeaderView>

QT_BEGIN_NAMESPACE

class Ui_feeds_settings
{
public:

    void setupUi(QDialog *feeds_settings)
    {
        if (feeds_settings->objectName().isEmpty())
            feeds_settings->setObjectName(QStringLiteral("feeds_settings"));
        feeds_settings->resize(400, 300);

        retranslateUi(feeds_settings);

        QMetaObject::connectSlotsByName(feeds_settings);
    } // setupUi

    void retranslateUi(QDialog *feeds_settings)
    {
        feeds_settings->setWindowTitle(QApplication::translate("feeds_settings", "Dialog", 0));
    } // retranslateUi

};

namespace Ui {
    class feeds_settings: public Ui_feeds_settings {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_FEEDS_SETTINGS_H
