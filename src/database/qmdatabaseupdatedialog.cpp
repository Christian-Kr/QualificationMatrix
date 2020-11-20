//
// qmdatabaseupdatedialog.cpp is part of QualificationMatrix
//
// QualificationMatrix is free software: you can redistribute it and/or modify it under the terms of
// the GNU General Public License as published by the Free Software Foundation, either version 3 of
// the License, or (at your option) any later version.
//
// QualificationMatrix is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
// without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
// the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with QualificationMatrix.
// If not, see <http://www.gnu.org/licenses/>.
//

#include "qmdatabaseupdatedialog.h"
#include "ui_qmdatabaseupdatedialog.h"

#include <QSqlDatabase>
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>

QMDatabaseUpdateDialog::QMDatabaseUpdateDialog(const QString &dbName, QWidget *parent)
    : QDialog(parent)
{
    ui = new Ui::QMDatabaseUpdateDialog;
    ui->setupUi(this);

    this->dbName = dbName;

    updateUi();
}

QMDatabaseUpdateDialog::~QMDatabaseUpdateDialog()
{
    delete ui;
}

void QMDatabaseUpdateDialog::updateUi()
{
}

void QMDatabaseUpdateDialog::reject()
{
    QDialog::reject();
}
