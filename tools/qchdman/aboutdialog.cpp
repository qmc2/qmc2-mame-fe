#include <QtGui>
#include <QFileDialog>
#include <QMessageBox>

#include "aboutdialog.h"
#include "ui_aboutdialog.h"
#include "mainwindow.h"
#include "macros.h"
#include "settings.h"

extern Settings *globalConfig;
extern MainWindow *mainWindow;

AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDialog)
{
    ui->setupUi(this);

    QString title = "<p><font size=\"+4\">" + QCHDMAN_APP_TITLE + "</font></p><p>" + tr("Version") + " " + QCHDMAN_APP_VERSION + "</p>";
    QString credits = "<p>" + tr("Qt based graphical user interface to CHDMAN, the MAME/MESS CHD management tool") + "</p>" +
                      "<p>" + tr("Copyright (C) 2012, R. Reucher. All Rights Reserved.") + "</p>" +
                      "<p>" + tr("This program is free software; you can redistribute it and/or modify it under the terms of "
                                 "the GNU General Public License as published by the Free Software Foundation; either version "
                                 "2 of the license, or (at your option) any later version.") + "<p>" +
                      "<p>" + tr("This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; "
                                 "without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. ") + "</p>" +
                      "<p>" + tr("See the") + " <a href=\"http://www.gnu.org/licenses/old-licenses/gpl-2.0.html\">" + tr("GNU General Public License") + "</a> " + tr("for more details.") +
            "<p>" + tr("Qt CHDMAN GUI is part of the") + " <a href=\"http://qmc2.arcadehits.net/wordpress\">" + tr("QMC2 project") + "</a>.</p>";

    ui->labelTitle->setText(title);
    ui->labelCredits->setText(credits);

    adjustSize();
    setMinimumSize(size());
}

AboutDialog::~AboutDialog()
{
    delete ui;
}
