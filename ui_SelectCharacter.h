/********************************************************************************
** Form generated from reading UI file 'SelectCharacter.ui'
**
** Created by: Qt User Interface Compiler version 5.12.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SELECTCHARACTER_H
#define UI_SELECTCHARACTER_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_SelectCharacter
{
public:
    QVBoxLayout *character_box;

    void setupUi(QDialog *SelectCharacter)
    {
        if (SelectCharacter->objectName().isEmpty())
            SelectCharacter->setObjectName(QString::fromUtf8("SelectCharacter"));
        SelectCharacter->resize(355, 96);
        character_box = new QVBoxLayout(SelectCharacter);
        character_box->setObjectName(QString::fromUtf8("character_box"));

        retranslateUi(SelectCharacter);

        QMetaObject::connectSlotsByName(SelectCharacter);
    } // setupUi

    void retranslateUi(QDialog *SelectCharacter)
    {
        SelectCharacter->setWindowTitle(QApplication::translate("SelectCharacter", "Insert Special Character", nullptr));
    } // retranslateUi

};

namespace Ui {
    class SelectCharacter: public Ui_SelectCharacter {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SELECTCHARACTER_H
