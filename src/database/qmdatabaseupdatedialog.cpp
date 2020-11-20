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
#include <QSqlQuery>
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>

QMDatabaseUpdateDialog::QMDatabaseUpdateDialog(const QString &dbName, QWidget *parent)
    : QDialog(parent)
{
    ui = new Ui::QMDatabaseUpdateDialog;
    ui->setupUi(this);

    major = -1;
    minor = -1;
    this->dbName = dbName;

    updateUi();
}

QMDatabaseUpdateDialog::~QMDatabaseUpdateDialog()
{
    delete ui;
}

void QMDatabaseUpdateDialog::readDatabaseVersion()
{
    auto db = QSqlDatabase::database(dbName);

    if (!db.isOpen())
    {
        return;
    }

    QSqlQuery query(db);
    QString queryText =
        "SELECT name, value FROM Info "
        "WHERE name == \"version_minor\" OR name == \"version_major\"";

    if (!query.exec(queryText))
    {
        return;
    }

    major = -1;
    minor = -1;

    while (query.next())
    {
        if (query.value("name").toString() == "version_major")
        {
            major = query.value("value").toInt();
            continue;
        }

        if (query.value("name").toString() == "version_minor")
        {
            minor = query.value("value").toInt();
            continue;
        }

        if (minor != -1 && major != -1)
        {
            break;
        }
    }
}

void QMDatabaseUpdateDialog::updateUi()
{
    readDatabaseVersion();

    if (minor != -1 && major != -1)
    {
        ui->laCurrentVersion->setText(QString("%1.%2").arg(major).arg(minor));
    }
    else
    {
        ui->laCurrentVersion->setText(tr("Nicht vorhanden!"));
    }
}

void QMDatabaseUpdateDialog::reject()
{
    QDialog::reject();
}
