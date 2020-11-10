/********************************************************************************
** Form generated from reading UI file 'showimagewidget.ui'
**
** Created by: Qt User Interface Compiler version 5.7.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SHOWIMAGEWIDGET_H
#define UI_SHOWIMAGEWIDGET_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_showImageWidget
{
public:
    QVBoxLayout *verticalLayout;
    QLabel *label;

    void setupUi(QWidget *showImageWidget)
    {
        if (showImageWidget->objectName().isEmpty())
            showImageWidget->setObjectName(QStringLiteral("showImageWidget"));
        showImageWidget->resize(400, 300);
        verticalLayout = new QVBoxLayout(showImageWidget);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        label = new QLabel(showImageWidget);
        label->setObjectName(QStringLiteral("label"));

        verticalLayout->addWidget(label);


        retranslateUi(showImageWidget);

        QMetaObject::connectSlotsByName(showImageWidget);
    } // setupUi

    void retranslateUi(QWidget *showImageWidget)
    {
        showImageWidget->setWindowTitle(QApplication::translate("showImageWidget", "Form", Q_NULLPTR));
        label->setText(QApplication::translate("showImageWidget", "TextLabel", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class showImageWidget: public Ui_showImageWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SHOWIMAGEWIDGET_H
