/********************************************************************************
** Form generated from reading UI file 'main.ui'
**
** Created by: Qt User Interface Compiler version 5.12.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAIN_H
#define UI_MAIN_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QAction *actionSave;
    QAction *actionCut;
    QAction *actionPaste;
    QAction *actionUndo;
    QAction *actionRedo;
    QAction *actionCopy;
    QAction *actionAlignLeft;
    QAction *actionAlignRight;
    QAction *actionAlignCenter;
    QAction *actionAlignJustify;
    QAction *actionBold;
    QAction *actionItalic;
    QAction *actionOpen;
    QAction *actionUnderline;
    QAction *actionExit;
    QAction *actionInsertSpecialCharacter;
    QAction *actionInsertNumberedList;
    QAction *actionInsertBulletedList;
    QAction *actionStrikethrough;
    QAction *actionSubscript;
    QAction *actionSuperscript;
    QAction *actionZoomIn;
    QAction *actionZoomOut;
    QAction *actionIncreaseIndent;
    QAction *actionDecreaseIndent;
    QAction *actionInsertSGFSectionMarker;
    QAction *actionPreferences;
    QAction *actionZoomReset;
    QAction *actionHeading1;
    QAction *actionHeading2;
    QAction *actionHeading3;
    QAction *actionHeading4;
    QAction *actionHeading5;
    QAction *actionHeading6;
    QAction *actionHeadingNormal;
    QAction *actionHeadingPreserveAttributes;
    QAction *actionSelectAll;
    QWidget *centralwidget;
    QVBoxLayout *verticalLayout;
    QMenuBar *menubar;
    QMenu *menuFile;
    QMenu *menuEdit;
    QMenu *menuInsert;
    QMenu *menuFormat;
    QMenu *menuHeadings;
    QMenu *menuView;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(1000, 700);
        MainWindow->setIconSize(QSize(48, 48));
        MainWindow->setToolButtonStyle(Qt::ToolButtonIconOnly);
        actionSave = new QAction(MainWindow);
        actionSave->setObjectName(QString::fromUtf8("actionSave"));
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/icons/document-save_48px.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionSave->setIcon(icon);
        actionCut = new QAction(MainWindow);
        actionCut->setObjectName(QString::fromUtf8("actionCut"));
        QIcon icon1;
        icon1.addFile(QString::fromUtf8(":/icons/edit-cut_48px.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionCut->setIcon(icon1);
        actionPaste = new QAction(MainWindow);
        actionPaste->setObjectName(QString::fromUtf8("actionPaste"));
        QIcon icon2;
        icon2.addFile(QString::fromUtf8(":/icons/edit-paste_48px.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionPaste->setIcon(icon2);
        actionUndo = new QAction(MainWindow);
        actionUndo->setObjectName(QString::fromUtf8("actionUndo"));
        QIcon icon3;
        icon3.addFile(QString::fromUtf8(":/icons/edit-undo_48px.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionUndo->setIcon(icon3);
        actionRedo = new QAction(MainWindow);
        actionRedo->setObjectName(QString::fromUtf8("actionRedo"));
        QIcon icon4;
        icon4.addFile(QString::fromUtf8(":/icons/edit-redo_48px.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionRedo->setIcon(icon4);
        actionCopy = new QAction(MainWindow);
        actionCopy->setObjectName(QString::fromUtf8("actionCopy"));
        QIcon icon5;
        icon5.addFile(QString::fromUtf8(":/icons/edit-copy_48px.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionCopy->setIcon(icon5);
        actionAlignLeft = new QAction(MainWindow);
        actionAlignLeft->setObjectName(QString::fromUtf8("actionAlignLeft"));
        actionAlignLeft->setCheckable(true);
        QIcon icon6;
        icon6.addFile(QString::fromUtf8(":/icons/format-justify-left_48px.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionAlignLeft->setIcon(icon6);
        actionAlignRight = new QAction(MainWindow);
        actionAlignRight->setObjectName(QString::fromUtf8("actionAlignRight"));
        actionAlignRight->setCheckable(true);
        QIcon icon7;
        icon7.addFile(QString::fromUtf8(":/icons/format-justify-right_48px.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionAlignRight->setIcon(icon7);
        actionAlignCenter = new QAction(MainWindow);
        actionAlignCenter->setObjectName(QString::fromUtf8("actionAlignCenter"));
        actionAlignCenter->setCheckable(true);
        QIcon icon8;
        icon8.addFile(QString::fromUtf8(":/icons/format-justify-center_48px.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionAlignCenter->setIcon(icon8);
        actionAlignJustify = new QAction(MainWindow);
        actionAlignJustify->setObjectName(QString::fromUtf8("actionAlignJustify"));
        actionAlignJustify->setCheckable(true);
        QIcon icon9;
        icon9.addFile(QString::fromUtf8(":/icons/format-justify-fill_48px.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionAlignJustify->setIcon(icon9);
        actionBold = new QAction(MainWindow);
        actionBold->setObjectName(QString::fromUtf8("actionBold"));
        actionBold->setCheckable(true);
        QIcon icon10;
        icon10.addFile(QString::fromUtf8(":/icons/format-text-bold_48px.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionBold->setIcon(icon10);
        actionItalic = new QAction(MainWindow);
        actionItalic->setObjectName(QString::fromUtf8("actionItalic"));
        actionItalic->setCheckable(true);
        QIcon icon11;
        icon11.addFile(QString::fromUtf8(":/icons/format-text-italic_48px.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionItalic->setIcon(icon11);
        actionOpen = new QAction(MainWindow);
        actionOpen->setObjectName(QString::fromUtf8("actionOpen"));
        QIcon icon12;
        icon12.addFile(QString::fromUtf8(":/icons/document-open_48px.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionOpen->setIcon(icon12);
        actionUnderline = new QAction(MainWindow);
        actionUnderline->setObjectName(QString::fromUtf8("actionUnderline"));
        actionUnderline->setCheckable(true);
        QIcon icon13;
        icon13.addFile(QString::fromUtf8(":/icons/format-text-underline_48px.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionUnderline->setIcon(icon13);
        actionExit = new QAction(MainWindow);
        actionExit->setObjectName(QString::fromUtf8("actionExit"));
        QIcon icon14;
        icon14.addFile(QString::fromUtf8(":/icons/process-stop_48px.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionExit->setIcon(icon14);
        actionInsertSpecialCharacter = new QAction(MainWindow);
        actionInsertSpecialCharacter->setObjectName(QString::fromUtf8("actionInsertSpecialCharacter"));
        QIcon icon15;
        icon15.addFile(QString::fromUtf8(":/icons/insert-special-character_48px.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionInsertSpecialCharacter->setIcon(icon15);
        actionInsertNumberedList = new QAction(MainWindow);
        actionInsertNumberedList->setObjectName(QString::fromUtf8("actionInsertNumberedList"));
        actionInsertNumberedList->setCheckable(true);
        QIcon icon16;
        icon16.addFile(QString::fromUtf8(":/icons/insert-numbered-list_48px.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionInsertNumberedList->setIcon(icon16);
        actionInsertBulletedList = new QAction(MainWindow);
        actionInsertBulletedList->setObjectName(QString::fromUtf8("actionInsertBulletedList"));
        actionInsertBulletedList->setCheckable(true);
        QIcon icon17;
        icon17.addFile(QString::fromUtf8(":/icons/insert-bullet-list_48px.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionInsertBulletedList->setIcon(icon17);
        actionStrikethrough = new QAction(MainWindow);
        actionStrikethrough->setObjectName(QString::fromUtf8("actionStrikethrough"));
        actionStrikethrough->setCheckable(true);
        QIcon icon18;
        icon18.addFile(QString::fromUtf8(":/icons/format-text-strikethrough_48px.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionStrikethrough->setIcon(icon18);
        actionSubscript = new QAction(MainWindow);
        actionSubscript->setObjectName(QString::fromUtf8("actionSubscript"));
        actionSubscript->setCheckable(true);
        QIcon icon19;
        icon19.addFile(QString::fromUtf8(":/icons/format-text-subscript_48px.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionSubscript->setIcon(icon19);
        actionSuperscript = new QAction(MainWindow);
        actionSuperscript->setObjectName(QString::fromUtf8("actionSuperscript"));
        actionSuperscript->setCheckable(true);
        QIcon icon20;
        icon20.addFile(QString::fromUtf8(":/icons/format-text-superscript_48px.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionSuperscript->setIcon(icon20);
        actionZoomIn = new QAction(MainWindow);
        actionZoomIn->setObjectName(QString::fromUtf8("actionZoomIn"));
        QIcon icon21;
        icon21.addFile(QString::fromUtf8(":/icons/list-add_48px.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionZoomIn->setIcon(icon21);
        actionZoomOut = new QAction(MainWindow);
        actionZoomOut->setObjectName(QString::fromUtf8("actionZoomOut"));
        QIcon icon22;
        icon22.addFile(QString::fromUtf8(":/icons/list-remove_48px.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionZoomOut->setIcon(icon22);
        actionIncreaseIndent = new QAction(MainWindow);
        actionIncreaseIndent->setObjectName(QString::fromUtf8("actionIncreaseIndent"));
        QIcon icon23;
        icon23.addFile(QString::fromUtf8(":/icons/format-indent-more_48px.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionIncreaseIndent->setIcon(icon23);
        actionDecreaseIndent = new QAction(MainWindow);
        actionDecreaseIndent->setObjectName(QString::fromUtf8("actionDecreaseIndent"));
        QIcon icon24;
        icon24.addFile(QString::fromUtf8(":/icons/format-indent-less_48px.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionDecreaseIndent->setIcon(icon24);
        actionInsertSGFSectionMarker = new QAction(MainWindow);
        actionInsertSGFSectionMarker->setObjectName(QString::fromUtf8("actionInsertSGFSectionMarker"));
        actionPreferences = new QAction(MainWindow);
        actionPreferences->setObjectName(QString::fromUtf8("actionPreferences"));
        actionZoomReset = new QAction(MainWindow);
        actionZoomReset->setObjectName(QString::fromUtf8("actionZoomReset"));
        actionHeading1 = new QAction(MainWindow);
        actionHeading1->setObjectName(QString::fromUtf8("actionHeading1"));
        actionHeading1->setCheckable(true);
        QIcon icon25;
        icon25.addFile(QString::fromUtf8(":/icons/heading-1_48px.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionHeading1->setIcon(icon25);
        actionHeading2 = new QAction(MainWindow);
        actionHeading2->setObjectName(QString::fromUtf8("actionHeading2"));
        actionHeading2->setCheckable(true);
        QIcon icon26;
        icon26.addFile(QString::fromUtf8(":/icons/heading-2_48px.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionHeading2->setIcon(icon26);
        actionHeading3 = new QAction(MainWindow);
        actionHeading3->setObjectName(QString::fromUtf8("actionHeading3"));
        actionHeading3->setCheckable(true);
        QIcon icon27;
        icon27.addFile(QString::fromUtf8(":/icons/heading-3_48px.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionHeading3->setIcon(icon27);
        actionHeading4 = new QAction(MainWindow);
        actionHeading4->setObjectName(QString::fromUtf8("actionHeading4"));
        actionHeading4->setCheckable(true);
        QIcon icon28;
        icon28.addFile(QString::fromUtf8(":/icons/heading-4_48px.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionHeading4->setIcon(icon28);
        actionHeading5 = new QAction(MainWindow);
        actionHeading5->setObjectName(QString::fromUtf8("actionHeading5"));
        actionHeading5->setCheckable(true);
        QIcon icon29;
        icon29.addFile(QString::fromUtf8(":/icons/heading-5_48px.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionHeading5->setIcon(icon29);
        actionHeading6 = new QAction(MainWindow);
        actionHeading6->setObjectName(QString::fromUtf8("actionHeading6"));
        actionHeading6->setCheckable(true);
        QIcon icon30;
        icon30.addFile(QString::fromUtf8(":/icons/heading-6_48px.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionHeading6->setIcon(icon30);
        actionHeadingNormal = new QAction(MainWindow);
        actionHeadingNormal->setObjectName(QString::fromUtf8("actionHeadingNormal"));
        actionHeadingNormal->setCheckable(true);
        QIcon icon31;
        icon31.addFile(QString::fromUtf8(":/icons/heading-normal_48px.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionHeadingNormal->setIcon(icon31);
        actionHeadingPreserveAttributes = new QAction(MainWindow);
        actionHeadingPreserveAttributes->setObjectName(QString::fromUtf8("actionHeadingPreserveAttributes"));
        actionHeadingPreserveAttributes->setCheckable(true);
        actionSelectAll = new QAction(MainWindow);
        actionSelectAll->setObjectName(QString::fromUtf8("actionSelectAll"));
        QIcon icon32;
        icon32.addFile(QString::fromUtf8(":/icons/edit-select-all_48px.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionSelectAll->setIcon(icon32);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        centralwidget->setLayoutDirection(Qt::LeftToRight);
        verticalLayout = new QVBoxLayout(centralwidget);
        verticalLayout->setSpacing(0);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName(QString::fromUtf8("menubar"));
        menubar->setGeometry(QRect(0, 0, 1000, 22));
        menuFile = new QMenu(menubar);
        menuFile->setObjectName(QString::fromUtf8("menuFile"));
        menuEdit = new QMenu(menubar);
        menuEdit->setObjectName(QString::fromUtf8("menuEdit"));
        menuInsert = new QMenu(menubar);
        menuInsert->setObjectName(QString::fromUtf8("menuInsert"));
        menuFormat = new QMenu(menubar);
        menuFormat->setObjectName(QString::fromUtf8("menuFormat"));
        menuHeadings = new QMenu(menuFormat);
        menuHeadings->setObjectName(QString::fromUtf8("menuHeadings"));
        menuView = new QMenu(menubar);
        menuView->setObjectName(QString::fromUtf8("menuView"));
        MainWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName(QString::fromUtf8("statusbar"));
        MainWindow->setStatusBar(statusbar);

        menubar->addAction(menuFile->menuAction());
        menubar->addAction(menuEdit->menuAction());
        menubar->addAction(menuInsert->menuAction());
        menubar->addAction(menuFormat->menuAction());
        menubar->addAction(menuView->menuAction());
        menuFile->addAction(actionOpen);
        menuFile->addAction(actionSave);
        menuFile->addAction(actionExit);
        menuFile->addAction(actionPreferences);
        menuEdit->addAction(actionUndo);
        menuEdit->addAction(actionRedo);
        menuEdit->addSeparator();
        menuEdit->addAction(actionCut);
        menuEdit->addAction(actionCopy);
        menuEdit->addAction(actionPaste);
        menuEdit->addAction(actionSelectAll);
        menuInsert->addAction(actionInsertSGFSectionMarker);
        menuInsert->addAction(actionInsertSpecialCharacter);
        menuInsert->addSeparator();
        menuInsert->addAction(actionInsertBulletedList);
        menuInsert->addAction(actionInsertNumberedList);
        menuFormat->addAction(menuHeadings->menuAction());
        menuFormat->addSeparator();
        menuFormat->addAction(actionBold);
        menuFormat->addAction(actionItalic);
        menuFormat->addAction(actionUnderline);
        menuFormat->addAction(actionStrikethrough);
        menuFormat->addAction(actionSubscript);
        menuFormat->addAction(actionSuperscript);
        menuFormat->addSeparator();
        menuFormat->addAction(actionAlignLeft);
        menuFormat->addAction(actionAlignCenter);
        menuFormat->addAction(actionAlignRight);
        menuFormat->addAction(actionAlignJustify);
        menuFormat->addSeparator();
        menuFormat->addAction(actionDecreaseIndent);
        menuFormat->addAction(actionIncreaseIndent);
        menuHeadings->addAction(actionHeading1);
        menuHeadings->addAction(actionHeading2);
        menuHeadings->addAction(actionHeading3);
        menuHeadings->addAction(actionHeading4);
        menuHeadings->addAction(actionHeading5);
        menuHeadings->addAction(actionHeading6);
        menuHeadings->addAction(actionHeadingNormal);
        menuHeadings->addSeparator();
        menuHeadings->addAction(actionHeadingPreserveAttributes);
        menuView->addAction(actionZoomIn);
        menuView->addAction(actionZoomOut);
        menuView->addAction(actionZoomReset);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "PageEditor", nullptr));
        actionSave->setText(QApplication::translate("MainWindow", "&Save", nullptr));
#ifndef QT_NO_TOOLTIP
        actionSave->setToolTip(QApplication::translate("MainWindow", "Save the current book.", nullptr));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_SHORTCUT
        actionSave->setShortcut(QApplication::translate("MainWindow", "Ctrl+S", nullptr));
#endif // QT_NO_SHORTCUT
        actionCut->setText(QApplication::translate("MainWindow", "Cu&t", nullptr));
#ifndef QT_NO_TOOLTIP
        actionCut->setToolTip(QApplication::translate("MainWindow", "Cuts the selected text from the document and puts it on the clipboard.", nullptr));
#endif // QT_NO_TOOLTIP
        actionPaste->setText(QApplication::translate("MainWindow", "&Paste", nullptr));
#ifndef QT_NO_TOOLTIP
        actionPaste->setToolTip(QApplication::translate("MainWindow", "Pastes the content from the clipboard into the book.", nullptr));
#endif // QT_NO_TOOLTIP
        actionUndo->setText(QApplication::translate("MainWindow", "&Undo", nullptr));
#ifndef QT_NO_TOOLTIP
        actionUndo->setToolTip(QApplication::translate("MainWindow", "Reverts the changes of the previous operation.", nullptr));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_SHORTCUT
        actionUndo->setShortcut(QApplication::translate("MainWindow", "Ctrl+Z", nullptr));
#endif // QT_NO_SHORTCUT
        actionRedo->setText(QApplication::translate("MainWindow", "&Redo", nullptr));
#ifndef QT_NO_TOOLTIP
        actionRedo->setToolTip(QApplication::translate("MainWindow", "Restores the changes reverted by the previous Undo action.", nullptr));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_SHORTCUT
        actionRedo->setShortcut(QApplication::translate("MainWindow", "Ctrl+Y", nullptr));
#endif // QT_NO_SHORTCUT
        actionCopy->setText(QApplication::translate("MainWindow", "&Copy", nullptr));
#ifndef QT_NO_TOOLTIP
        actionCopy->setToolTip(QApplication::translate("MainWindow", "Copies the selected text and puts it on the clipboard.", nullptr));
#endif // QT_NO_TOOLTIP
        actionAlignLeft->setText(QApplication::translate("MainWindow", "Align &Left", nullptr));
#ifndef QT_NO_TOOLTIP
        actionAlignLeft->setToolTip(QApplication::translate("MainWindow", "Align the paragraph to the left.", nullptr));
#endif // QT_NO_TOOLTIP
        actionAlignRight->setText(QApplication::translate("MainWindow", "Align &Right", nullptr));
#ifndef QT_NO_TOOLTIP
        actionAlignRight->setToolTip(QApplication::translate("MainWindow", "Align the paragraph to the right.", nullptr));
#endif // QT_NO_TOOLTIP
        actionAlignCenter->setText(QApplication::translate("MainWindow", "&Center", nullptr));
#ifndef QT_NO_TOOLTIP
        actionAlignCenter->setToolTip(QApplication::translate("MainWindow", "Center the paragraph.", nullptr));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_SHORTCUT
        actionAlignCenter->setShortcut(QApplication::translate("MainWindow", "Ctrl+E", nullptr));
#endif // QT_NO_SHORTCUT
        actionAlignJustify->setText(QApplication::translate("MainWindow", "&Justify", nullptr));
#ifndef QT_NO_TOOLTIP
        actionAlignJustify->setToolTip(QApplication::translate("MainWindow", "Align the paragraph to both the left and right margins.", nullptr));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_SHORTCUT
        actionAlignJustify->setShortcut(QApplication::translate("MainWindow", "Ctrl+J", nullptr));
#endif // QT_NO_SHORTCUT
        actionBold->setText(QApplication::translate("MainWindow", "&Bold", nullptr));
#ifndef QT_NO_TOOLTIP
        actionBold->setToolTip(QApplication::translate("MainWindow", "Make the selected text bold.", nullptr));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_SHORTCUT
        actionBold->setShortcut(QApplication::translate("MainWindow", "Ctrl+B", nullptr));
#endif // QT_NO_SHORTCUT
        actionItalic->setText(QApplication::translate("MainWindow", "&Italic", nullptr));
#ifndef QT_NO_TOOLTIP
        actionItalic->setToolTip(QApplication::translate("MainWindow", "Make the selected text italic.", nullptr));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_SHORTCUT
        actionItalic->setShortcut(QApplication::translate("MainWindow", "Ctrl+I", nullptr));
#endif // QT_NO_SHORTCUT
        actionOpen->setText(QApplication::translate("MainWindow", "&Open...", nullptr));
#ifndef QT_NO_TOOLTIP
        actionOpen->setToolTip(QApplication::translate("MainWindow", "Open a book from disk.", nullptr));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_SHORTCUT
        actionOpen->setShortcut(QApplication::translate("MainWindow", "Ctrl+O", nullptr));
#endif // QT_NO_SHORTCUT
        actionUnderline->setText(QApplication::translate("MainWindow", "&Underline", nullptr));
#ifndef QT_NO_TOOLTIP
        actionUnderline->setToolTip(QApplication::translate("MainWindow", "Underline the selected text.", nullptr));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_SHORTCUT
        actionUnderline->setShortcut(QApplication::translate("MainWindow", "Ctrl+U", nullptr));
#endif // QT_NO_SHORTCUT
        actionExit->setText(QApplication::translate("MainWindow", "&Quit", nullptr));
#ifndef QT_NO_TOOLTIP
        actionExit->setToolTip(QApplication::translate("MainWindow", "Quit", nullptr));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_SHORTCUT
        actionExit->setShortcut(QApplication::translate("MainWindow", "Ctrl+Q", nullptr));
#endif // QT_NO_SHORTCUT
        actionInsertSpecialCharacter->setText(QApplication::translate("MainWindow", "&Special Character...", nullptr));
#ifndef QT_NO_TOOLTIP
        actionInsertSpecialCharacter->setToolTip(QApplication::translate("MainWindow", "Select a character to insert into your text.", nullptr));
#endif // QT_NO_TOOLTIP
        actionInsertNumberedList->setText(QApplication::translate("MainWindow", "&Numbered List", nullptr));
#ifndef QT_NO_TOOLTIP
        actionInsertNumberedList->setToolTip(QApplication::translate("MainWindow", "Create a numbered list from selection.", nullptr));
#endif // QT_NO_TOOLTIP
        actionInsertBulletedList->setText(QApplication::translate("MainWindow", "Bulle&ted List", nullptr));
#ifndef QT_NO_TOOLTIP
        actionInsertBulletedList->setToolTip(QApplication::translate("MainWindow", "Create a bulleted list from selection.", nullptr));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_SHORTCUT
        actionInsertBulletedList->setShortcut(QApplication::translate("MainWindow", "Ctrl+Shift+L", nullptr));
#endif // QT_NO_SHORTCUT
        actionStrikethrough->setText(QApplication::translate("MainWindow", "Stri&kethrough", nullptr));
#ifndef QT_NO_TOOLTIP
        actionStrikethrough->setToolTip(QApplication::translate("MainWindow", "Draw a line through the selected text.", nullptr));
#endif // QT_NO_TOOLTIP
        actionSubscript->setText(QApplication::translate("MainWindow", "&Subscript", nullptr));
#ifndef QT_NO_TOOLTIP
        actionSubscript->setToolTip(QApplication::translate("MainWindow", "Set the selected text slightly smaller and below the normal line.", nullptr));
#endif // QT_NO_TOOLTIP
        actionSuperscript->setText(QApplication::translate("MainWindow", "Su&perscript", nullptr));
#ifndef QT_NO_TOOLTIP
        actionSuperscript->setToolTip(QApplication::translate("MainWindow", "Set the selected text slightly smaller and above the normal line.", nullptr));
#endif // QT_NO_TOOLTIP
        actionZoomIn->setText(QApplication::translate("MainWindow", "Zoom &In", nullptr));
#ifndef QT_NO_TOOLTIP
        actionZoomIn->setToolTip(QApplication::translate("MainWindow", "Zoom In", nullptr));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_SHORTCUT
        actionZoomIn->setShortcut(QApplication::translate("MainWindow", "Ctrl+=", nullptr));
#endif // QT_NO_SHORTCUT
        actionZoomOut->setText(QApplication::translate("MainWindow", "Zoom &Out", nullptr));
#ifndef QT_NO_TOOLTIP
        actionZoomOut->setToolTip(QApplication::translate("MainWindow", "Zoom Out", nullptr));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_SHORTCUT
        actionZoomOut->setShortcut(QApplication::translate("MainWindow", "Ctrl+-", nullptr));
#endif // QT_NO_SHORTCUT
        actionIncreaseIndent->setText(QApplication::translate("MainWindow", "Incre&ase Indent", nullptr));
#ifndef QT_NO_TOOLTIP
        actionIncreaseIndent->setToolTip(QApplication::translate("MainWindow", "Increase the indent level of the paragraph.", nullptr));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_SHORTCUT
        actionIncreaseIndent->setShortcut(QApplication::translate("MainWindow", "Ctrl+Alt+M", nullptr));
#endif // QT_NO_SHORTCUT
        actionDecreaseIndent->setText(QApplication::translate("MainWindow", "&Decrease Indent", nullptr));
#ifndef QT_NO_TOOLTIP
        actionDecreaseIndent->setToolTip(QApplication::translate("MainWindow", "Decrease the indent level of the paragraph.", nullptr));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_SHORTCUT
        actionDecreaseIndent->setShortcut(QApplication::translate("MainWindow", "Ctrl+Shift+M", nullptr));
#endif // QT_NO_SHORTCUT
        actionInsertSGFSectionMarker->setText(QApplication::translate("MainWindow", "Split &Marker", nullptr));
#ifndef QT_NO_TOOLTIP
        actionInsertSGFSectionMarker->setToolTip(QApplication::translate("MainWindow", "Insert Sigil split file marker", nullptr));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_SHORTCUT
        actionInsertSGFSectionMarker->setShortcut(QApplication::translate("MainWindow", "Ctrl+Shift+Return", nullptr));
#endif // QT_NO_SHORTCUT
        actionPreferences->setText(QApplication::translate("MainWindow", "&Preferences...", nullptr));
#ifndef QT_NO_SHORTCUT
        actionPreferences->setShortcut(QApplication::translate("MainWindow", "F5", nullptr));
#endif // QT_NO_SHORTCUT
        actionZoomReset->setText(QApplication::translate("MainWindow", "&Zoom Reset", nullptr));
#ifndef QT_NO_TOOLTIP
        actionZoomReset->setToolTip(QApplication::translate("MainWindow", "Zoom Reset", nullptr));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_SHORTCUT
        actionZoomReset->setShortcut(QApplication::translate("MainWindow", "Ctrl+0", nullptr));
#endif // QT_NO_SHORTCUT
        actionHeading1->setText(QApplication::translate("MainWindow", "Heading &1", nullptr));
#ifndef QT_NO_TOOLTIP
        actionHeading1->setToolTip(QApplication::translate("MainWindow", "Format paragraph as a level 1 heading.", nullptr));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_SHORTCUT
        actionHeading1->setShortcut(QApplication::translate("MainWindow", "Ctrl+1", nullptr));
#endif // QT_NO_SHORTCUT
        actionHeading2->setText(QApplication::translate("MainWindow", "Heading &2", nullptr));
#ifndef QT_NO_TOOLTIP
        actionHeading2->setToolTip(QApplication::translate("MainWindow", "Format paragraph as a level 2 heading.", nullptr));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_SHORTCUT
        actionHeading2->setShortcut(QApplication::translate("MainWindow", "Ctrl+2", nullptr));
#endif // QT_NO_SHORTCUT
        actionHeading3->setText(QApplication::translate("MainWindow", "Heading &3", nullptr));
#ifndef QT_NO_TOOLTIP
        actionHeading3->setToolTip(QApplication::translate("MainWindow", "Format paragraph as a level 3 heading.", nullptr));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_SHORTCUT
        actionHeading3->setShortcut(QApplication::translate("MainWindow", "Ctrl+3", nullptr));
#endif // QT_NO_SHORTCUT
        actionHeading4->setText(QApplication::translate("MainWindow", "Heading &4", nullptr));
#ifndef QT_NO_TOOLTIP
        actionHeading4->setToolTip(QApplication::translate("MainWindow", "Format paragraph as a level 4 heading.", nullptr));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_SHORTCUT
        actionHeading4->setShortcut(QApplication::translate("MainWindow", "Ctrl+4", nullptr));
#endif // QT_NO_SHORTCUT
        actionHeading5->setText(QApplication::translate("MainWindow", "Heading &5", nullptr));
#ifndef QT_NO_TOOLTIP
        actionHeading5->setToolTip(QApplication::translate("MainWindow", "Format paragraph as a level 5 heading.", nullptr));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_SHORTCUT
        actionHeading5->setShortcut(QApplication::translate("MainWindow", "Ctrl+5", nullptr));
#endif // QT_NO_SHORTCUT
        actionHeading6->setText(QApplication::translate("MainWindow", "Heading &6", nullptr));
#ifndef QT_NO_TOOLTIP
        actionHeading6->setToolTip(QApplication::translate("MainWindow", "Format paragraph as a level 6 heading.", nullptr));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_SHORTCUT
        actionHeading6->setShortcut(QApplication::translate("MainWindow", "Ctrl+6", nullptr));
#endif // QT_NO_SHORTCUT
        actionHeadingNormal->setText(QApplication::translate("MainWindow", "&Normal", nullptr));
#ifndef QT_NO_TOOLTIP
        actionHeadingNormal->setToolTip(QApplication::translate("MainWindow", "ormat paragraph as a normal paragraph.", nullptr));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_SHORTCUT
        actionHeadingNormal->setShortcut(QApplication::translate("MainWindow", "Ctrl+7", nullptr));
#endif // QT_NO_SHORTCUT
        actionHeadingPreserveAttributes->setText(QApplication::translate("MainWindow", "&Preserve Existing Attributes", nullptr));
#ifndef QT_NO_TOOLTIP
        actionHeadingPreserveAttributes->setToolTip(QApplication::translate("MainWindow", "When applying this style, preserve any existing attributes on the tag", nullptr));
#endif // QT_NO_TOOLTIP
        actionSelectAll->setText(QApplication::translate("MainWindow", "&SelectAll", nullptr));
#ifndef QT_NO_TOOLTIP
        actionSelectAll->setToolTip(QApplication::translate("MainWindow", "Select all text in the document.", nullptr));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_SHORTCUT
        actionSelectAll->setShortcut(QApplication::translate("MainWindow", "Ctrl+A", nullptr));
#endif // QT_NO_SHORTCUT
        menuFile->setTitle(QApplication::translate("MainWindow", "&File", nullptr));
        menuEdit->setTitle(QApplication::translate("MainWindow", "&Edit", nullptr));
        menuInsert->setTitle(QApplication::translate("MainWindow", "&Insert", nullptr));
        menuFormat->setTitle(QApplication::translate("MainWindow", "For&mat", nullptr));
        menuHeadings->setTitle(QApplication::translate("MainWindow", "&Heading", nullptr));
        menuView->setTitle(QApplication::translate("MainWindow", "&View", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAIN_H
