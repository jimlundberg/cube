// GlobjWin.h : The GLObjectWindow contains a GLBox and three sliders connected to the GLBox's rotation slots.
//
// John Davin, Terrence Wong, Jim Lundberg
// Copyright (c) 2002-2004 JRL Innovations  All Rights Reserved
// 3D5 - The FIVE level 3D tic-tac-toe game
//

#ifndef GLOBJWIN_H
#define GLOBJWIN_H

#include <qwidget.h>
#include <qlabel.h>
#include <qpushbutton.h>

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class GLObjectWindow : public QWidget
{
// ??? Q_OBJECT

public:
    GLObjectWindow(QWidget* parent = 0, const char* name = 0);

    int HelpPage;
    QLabel *welcome;      // made welcome global so help system can access it
    QPushButton *help;    // same here

    public slots:
    void displayhelp();
};

#endif // GLOBJWIN_H
