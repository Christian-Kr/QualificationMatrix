//
// qmdatabaseupdater.cpp is part of QualificationMatrix
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

#include "qmdatabaseupdater.h"

#include <QSqlDatabase>

QMDatabaseUpdater::QMDatabaseUpdater(QObject *parent)
    : QObject(parent)
{}

QMDatabaseUpdater::~QMDatabaseUpdater()
{}

bool QMDatabaseUpdater::updateDatabase(QSqlDatabase &db)
{
    if (!db.isOpen()) {
        return false;
    }

    // get database version

    return false;
}

// void QMDatabaseUpdater::initVersionScriptFiles()
//{
//    QFileInfo pathInfo("database");

//    if (!pathInfo.isDir() || !pathInfo.exists()) {
//        QMessageBox::critical(
//            this,
//            tr("Datenbank Updater"),
//            tr("Der notwendige Datenbankordner mit den Updateskripten ist nicht verfügbar.")
//        );
//        return;
//    }

//    // get a list of all scripts

//    QStringList filters;
//    filters << QString("sql_*.sql");

//    QDir databaseDir(pathInfo.absoluteFilePath());
//    databaseDir.setNameFilters(filters);
//    databaseDir.setSorting(QDir::Name);
//    QFileInfoList updateScriptList = databaseDir.entryInfoList();

//    for (int i = 0; i < updateScriptList.size(); i++) {
//        ui->lwVersion->addItem(updateScriptList.at(i).fileName());
//    }
//}
