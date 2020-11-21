//
// datamanager.cpp is part of QualificationMatrix
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

#include "qmdatamanager.h"
#include "qualimatrix/qmqualimatrixmodel.h"
#include "qualiresult/qmqualiresultmodel.h"
#include "qmfunctionmodel.h"
#include "qmtrainingmodel.h"
#include "qmtrainingdatamodel.h"
#include "qmfunctiongroupmodel.h"
#include "qmtraininggroupmodel.h"
#include "qmtrainingdatastatemodel.h"
#include "qmemployeemodel.h"
#include "qmemployeefunctionmodel.h"
#include "qmqualificationmatrixmodel.h"
#include "qmtrainingexceptionmodel.h"
#include "qminfomodel.h"

#include <QSqlRelationalTableModel>
#include <QSqlQuery>
#include <QDebug>

// maximum version that exist with the release of this software version - the software cannot work
// with versions higher than this
#define DB_CUR_MAJOR 1
#define DB_CUR_MINOR 0

QMDataManager *QMDataManager::instance = nullptr;

QMDataManager::QMDataManager()
    : QObject()
{
    qualiResultModel = std::make_shared<QMQualiResultModel>();
    qualiMatrixModel = std::make_shared<QualiMatrixModel>();
}

bool QMDataManager::testVersion(QSqlDatabase &db)
{
    QSqlQuery query(db);
    QString queryText =
        "SELECT name, value FROM Info "
        "WHERE name == \"MINOR\" OR name == \"MAJOR\"";

    if (!query.exec(queryText))
    {
        return false;
    }

    int major = -1;
    int minor = -1;

    while (query.next())
    {
        if (query.value("name").toString() == "MAJOR")
        {
            major = query.value("value").toInt();
            continue;
        }

        if (query.value("name").toString() == "MINOR")
        {
            minor = query.value("value").toInt();
            continue;
        }

        if (minor != -1 && major != -1)
        {
            break;
        }
    }

    if (major == QMDataManager::getMajor() && minor == QMDataManager::getMinor())
    {
        return true;
    }

    return false;
}

bool QMDataManager::testTableStructure(QSqlDatabase &db)
{
    return true;
}

// bool QMDataManager::filesLocation(QSqlDatabase &db)
//{
//    // Files can be inside a central database or as part of sqlite in a local file structure
//    place.
//    // You should not use local file structure for a central database with access of many people,
//    // cause then files might not be found.

//    // Switching the system should be done carefully too. The local folder needs to be an equal
//    // path for every user. In best case, this folder is next to the sqlite database file.

//        QSqlQuery query(db);
//    QString queryText = "SELECT name, value FROM Info"
//            " WHERE name == \"version_minor\" OR name == \"version_major\"";

//    if (!query.exec(queryText)) {
//        return false;
//    }

//    while (query.next()) {
//        if (query.value("name").toString() == "version_major") {
//            major = query.value("value").toInt();
//            continue;
//        }

//        if (query.value("name").toString() == "version_minor") {
//            minor = query.value("value").toInt();
//            continue;
//        }

//        if (minor != -1 && major != -1) {
//            break;
//        }
//    }
//}

void QMDataManager::initializeModels(QSqlDatabase &db)
{
    emit beforeInitializeModels(13);

    // Initialize all models with the given database object.

    funcModel = std::make_shared<QMFunctionModel>(this, db);
    funcModel->select();

    emit updateInitializeModels(1);

    trainModel = std::make_shared<QMTrainingModel>(nullptr, db);
    trainModel->select();

    emit updateInitializeModels(2);

    trainDataModel = std::make_shared<QMTrainingDataModel>(nullptr, db);
    trainDataModel->select();

    emit updateInitializeModels(3);

    funcGroupModel = std::make_shared<QMFunctionGroupModel>(nullptr, db);
    funcGroupModel->select();

    emit updateInitializeModels(4);

    trainGroupModel = std::make_shared<QMTrainingGroupModel>(nullptr, db);
    trainGroupModel->select();

    emit updateInitializeModels(5);

    trainDataStateModel = std::make_shared<QMTrainingDataStateModel>(nullptr, db);
    trainDataStateModel->select();

    emit updateInitializeModels(6);

    employeeModel = std::make_shared<QMEmployeeModel>(nullptr, db);
    employeeModel->select();

    emit updateInitializeModels(7);

    employeeFuncModel = std::make_shared<QMEmployeeFunctionModel>(nullptr, db);
    employeeFuncModel->select();

    emit updateInitializeModels(8);

    qualiModel = std::make_shared<QMQualificationMatrixModel>(nullptr, db);
    qualiModel->select();

    emit updateInitializeModels(9);

    trainExceptionModel = std::make_shared<QMTrainingExceptionModel>(nullptr, db);
    trainExceptionModel->select();

    emit updateInitializeModels(10);

    shiftModel = std::make_shared<QSqlTableModel>(nullptr, db);
    shiftModel->setTable("Shift");
    shiftModel->setEditStrategy(QSqlTableModel::OnManualSubmit);
    shiftModel->setHeaderData(1, Qt::Horizontal, tr("Name"));
    shiftModel->select();

    emit updateInitializeModels(11);

    infoModel = std::make_shared<QMInfoModel>(nullptr, db);

    // Inform all customer about the update.
    emit updateInitializeModels(13);

    // Inform all customer about the update.
    emit modelsInitialized();

    // Inform that everything is finish and every was informed.
    emit afterInitializeModels();
}

bool QMDataManager::isAnyDirty() const
{
    if (funcModel != nullptr)
    {
        if (funcModel->isDirty())
        {
            return true;
        }
    }

    if (trainModel != nullptr)
    {
        if (trainModel->isDirty())
        {
            return true;
        }
    }

    if (trainDataModel != nullptr)
    {
        if (trainDataModel->isDirty())
        {
            return true;
        }
    }

    if (trainDataStateModel != nullptr)
    {
        if (trainDataStateModel->isDirty())
        {
            return true;
        }
    }

    if (funcGroupModel != nullptr)
    {
        if (funcGroupModel->isDirty())
        {
            return true;
        }
    }

    if (trainGroupModel != nullptr)
    {
        if (trainGroupModel->isDirty())
        {
            return true;
        }
    }

    if (employeeModel != nullptr)
    {
        if (employeeModel->isDirty())
        {
            return true;
        }
    }

    if (employeeFuncModel != nullptr)
    {
        if (employeeFuncModel->isDirty())
        {
            return true;
        }
    }

    return false;
}
