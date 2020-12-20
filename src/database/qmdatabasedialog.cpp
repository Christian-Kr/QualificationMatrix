//
// qmdatabasedialog.cpp is part of QualificationMatrix
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

#include "qmdatabasedialog.h"
#include "ui_qmdatabasedialog.h"
#include "settings/qmapplicationsettings.h"

#include <QSqlDatabase>
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>

#define LOCAL  0
#define REMOTE 1

QMDatabaseDialog::QMDatabaseDialog(const QString &dbName, QWidget *parent)
    : QDialog(parent)
{
    ui = new Ui::QMDatabaseDialog;
    ui->setupUi(this);

    this->dbName = dbName;

    updateTypeWidget();
    initDatabaseDriverList();
    loadValuesFromDatabase();
    updateUi();
}

QMDatabaseDialog::~QMDatabaseDialog()
{
    delete ui;
}

void QMDatabaseDialog::initDatabaseDriverList()
{
    auto driverNames = QSqlDatabase::drivers();

    // Both combo lists (local, remote) need to be updated. For now only "QSQLITE" is a known type
    // for a local database driver.
    if (driverNames.removeOne("QSQLITE"))
    {
        ui->cbDriverLocal->addItem("QSQLITE");
    }

    ui->cbDriverRemote->addItems(driverNames);
}

void QMDatabaseDialog::loadValuesFromDatabase()
{
    if (QSqlDatabase::contains(dbName))
    {
        auto db = QSqlDatabase::database(dbName, false);
        auto driver = db.driverName();

        // For now there is only "QSQLITE" which could be a local driver.
        if (driver == "QSQLITE")
        {
            ui->cbType->setCurrentIndex(LOCAL);
            ui->leLocalFile->setText(db.databaseName());
        }
        else
        {
            ui->cbType->setCurrentIndex(REMOTE);
            ui->leServer->setText(db.hostName());
            ui->spPort->setValue(db.port());
            ui->leDatabaseName->setText(db.databaseName());
            ui->leUsername->setText(db.userName());
            ui->lePassword->setText(db.password());
        }
    }
}

void QMDatabaseDialog::switchConnected()
{
    if (QSqlDatabase::contains(dbName) && QSqlDatabase::database(dbName, false).isOpen())
    {
        // The database exist and is open. Just close it.
        QSqlDatabase::database(dbName, false).close();
    }
    else
    {
        // If the database connection exist, it should be removed first. This needs to be done,
        // cause the driver might change which leads to a new database creation.
        if (QSqlDatabase::contains(dbName))
        {
            QSqlDatabase::removeDatabase(dbName);
        }

        // Create a new database, with the new driver.
        switch (ui->cbType->currentIndex())
        {
            case LOCAL:
                QSqlDatabase::addDatabase(ui->cbDriverLocal->currentText(), dbName);
                break;
            case REMOTE:
                QSqlDatabase::addDatabase(ui->cbDriverRemote->currentText(), dbName);
                break;
        }

        auto db = QSqlDatabase::database(dbName, false);

        // Settings of the database object.
        switch (ui->cbType->currentIndex())
        {
            case LOCAL:
                db.setDatabaseName(ui->leLocalFile->text());
                break;
            case REMOTE:
                db.setDatabaseName(ui->leDatabaseName->text());
                db.setHostName(ui->leServer->text());
                db.setPort(ui->spPort->value());
                db.setUserName(ui->leUsername->text());
                db.setPassword(ui->lePassword->text());
                break;
        }

        if (!db.open())
        {
            QMessageBox::critical(
                this, tr("Datenbankverbindung"), tr(
                    "Die Datenbank konnte nicht verbunden werden. Bitte überprüfen Sie ihre"
                    " Ihre Einstellungen."));
            return;
        }
    }

    updateUi();
}

void QMDatabaseDialog::updateUi()
{
    if (QSqlDatabase::contains(dbName) && QSqlDatabase::database(dbName, false).isOpen())
    {
        setWindowTitle(tr("Datenbank verbunden"));
        ui->pbSwitchConnected->setText(tr("Trennen"));
    }
    else
    {
        setWindowTitle(tr("Datenbank nicht verbunden"));
        ui->pbSwitchConnected->setText(tr("Verbinden"));
    }
}

void QMDatabaseDialog::updateTypeWidget()
{
    ui->swDatabaseSetting->setCurrentIndex(ui->cbType->currentIndex());
}

void QMDatabaseDialog::chooseDatabaseFile()
{
    auto &settings = QMApplicationSettings::getInstance();
    auto defaultDir = settings.read("localdb/defaultopendir", QDir::homePath()).toString();

    // Create a custom QFileDialog, cause the FileMode can not be set in static versions.
    QFileDialog fileDialog(this);
    fileDialog.setFileMode(QFileDialog::AnyFile);
    fileDialog.setWindowTitle(tr("Öffne lokale Datenbank"));
    fileDialog.setAcceptMode(QFileDialog::AcceptOpen);
    fileDialog.setNameFilter(tr("Datenbank (*.qmsql)"));

    fileDialog.exec();

    auto fileName = fileDialog.selectedFiles();
    if (fileName.size() > 1)
    {
        QMessageBox::information(
            this, tr("Lokale Datenbank öffnen"),
            tr("Es darf nicht mehr als eine Datenbank ausgewählt werden."));

        return;
    }

    if (fileName.size() == 1)
    {
        ui->leLocalFile->setText(fileName.first());
    }
}

void QMDatabaseDialog::reject()
{
    QDialog::reject();
}
