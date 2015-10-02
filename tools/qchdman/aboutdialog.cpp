#include <QtGui>

#include "aboutdialog.h"
#include "ui_aboutdialog.h"
#include "mainwindow.h"
#include "macros.h"
#include "qchdmansettings.h"

extern QtChdmanGuiSettings *globalConfig;
extern MainWindow *mainWindow;

AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDialog)
{
    ui->setupUi(this);
    setModal(true);
#if QCHDMAN_SVN_REV == 0
    QString credits = "<p><font size=\"+2\"><b>" + QCHDMAN_APP_TITLE + " " + QCHDMAN_APP_VERSION + "</b></font></p>" +
#else
    QString credits = "<p><font size=\"+2\"><b>" + QCHDMAN_APP_TITLE + " " + QCHDMAN_APP_VERSION + QString(" (SVN r%1)").arg(QCHDMAN_SVN_REV) + "</b></font></p>" +
#endif
                      "<p>" + tr("Qt based graphical user interface to CHDMAN, the MAME CHD management tool") + "</p>" +
                      "<p>" + tr("Copyright") + " &copy; 2012 - 2015, R. Reucher. " + tr("All Rights Reserved.") + "</p>" +
                      "<p>" + tr("This program is free software; you can redistribute it and/or modify it under the terms of "
                                 "the GNU General Public License as published by the Free Software Foundation; either version "
                                 "2 of the license, or (at your option) any later version.") + "<p>" +
                      "<p>" + tr("This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; "
                                 "without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. ") + "</p>" +
                      "<p>" + tr("See the") + " <a href=\"http://www.gnu.org/licenses/old-licenses/gpl-2.0.html\">" + tr("GNU General Public License") + "</a> " + tr("for more details.") +
                      "<p>" + tr("Qt CHDMAN GUI is part of the") + " <a href=\"http://qmc2.batcom-it.net/\">" + tr("QMC2 project") + "</a>.</p>";
    ui->labelCredits->setText(credits);
}

AboutDialog::~AboutDialog()
{
    delete ui;
}

void AboutDialog::showEvent(QShowEvent *e)
{
    setMinimumSize(QSize(0, 0));
    adjustSize();
    setMinimumSize(size());
    QDialog::showEvent(e);
}
