/********************************************************************************
** Form generated from reading UI file 'dfrssfilter.ui'
**
** Created by: Qt User Interface Compiler version 5.4.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DFRSSFILTER_H
#define UI_DFRSSFILTER_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_DFRSSFilter
{
public:
    QMenuBar *menuBar;
    QToolBar *mainToolBar;
    QWidget *centralWidget;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *DFRSSFilter)
    {
        if (DFRSSFilter->objectName().isEmpty())
            DFRSSFilter->setObjectName(QStringLiteral("DFRSSFilter"));
        DFRSSFilter->resize(400, 300);
        menuBar = new QMenuBar(DFRSSFilter);
        menuBar->setObjectName(QStringLiteral("menuBar"));
        DFRSSFilter->setMenuBar(menuBar);
        mainToolBar = new QToolBar(DFRSSFilter);
        mainToolBar->setObjectName(QStringLiteral("mainToolBar"));
        DFRSSFilter->addToolBar(mainToolBar);
        centralWidget = new QWidget(DFRSSFilter);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        DFRSSFilter->setCentralWidget(centralWidget);
        statusBar = new QStatusBar(DFRSSFilter);
        statusBar->setObjectName(QStringLiteral("statusBar"));
        DFRSSFilter->setStatusBar(statusBar);

        retranslateUi(DFRSSFilter);

        QMetaObject::connectSlotsByName(DFRSSFilter);
    } // setupUi

    void retranslateUi(QMainWindow *DFRSSFilter)
    {
        DFRSSFilter->setWindowTitle(QApplication::translate("DFRSSFilter", "DFRSSFilter", 0));
    } // retranslateUi

};

namespace Ui {
    class DFRSSFilter: public Ui_DFRSSFilter {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DFRSSFILTER_H
