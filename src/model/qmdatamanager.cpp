// qmdatamanager.cpp is part of QualificationMatrix
//
// QualificationMatrix is free software: you can redistribute it and/or modify it under the terms of the GNU General
// Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option)
// any later version.
//
// QualificationMatrix is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the
// implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
// details.
//
// You should have received a copy of the GNU General Public License along with QualificationMatrix.
// If not, see <http://www.gnu.org/licenses/>.

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
#include "qmemployeeviewmodel.h"
#include "qmemployeefunctionmodel.h"
#include "qmqualificationmatrixmodel.h"
#include "qmtrainingexceptionmodel.h"
#include "qminfomodel.h"
#include "qmcertificatemodel.h"
#include "qmtraindatacertificatemodel.h"
#include "qmtraindatacertificateviewmodel.h"

#include <QSqlRelationalTableModel>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

QMDataManager *QMDataManager::instance = nullptr;

QMDataManager::QMDataManager()
    : QObject()
{
    certLoc = CertLoc::EXTERNAL;

    qualiResultModel = std::make_shared<QMQualiResultModel>();
    qualiMatrixModel = std::make_shared<QMQualiMatrixModel>();
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

void QMDataManager::initializeModels(QSqlDatabase &db)
{
    emit beforeInitModels(14);

    // Initialize all models with the given database object.

    funcModel = std::make_shared<QMFunctionModel>(this, db);
    funcModel->initModel();
    funcModel->select();

    emit updateInitModels(1);

    trainModel = std::make_shared<QMTrainingModel>(nullptr, db);
    trainModel->initModel();
    trainModel->select();

    emit updateInitModels(2);

    trainDataModel = std::make_shared<QMTrainingDataModel>(nullptr, db);
    trainDataModel->initModel();
    trainDataModel->select();

    emit updateInitModels(3);

    funcGroupModel = std::make_shared<QMFunctionGroupModel>(nullptr, db);
    funcGroupModel->select();

    emit updateInitModels(4);

    trainGroupModel = std::make_shared<QMTrainingGroupModel>(nullptr, db);
    trainGroupModel->select();

    emit updateInitModels(5);

    trainDataStateModel = std::make_shared<QMTrainingDataStateModel>(nullptr, db);
    trainDataStateModel->select();

    emit updateInitModels(6);

    employeeModel = std::make_shared<QMEmployeeModel>(nullptr, db);
    employeeModel->initModel();
    employeeModel->select();

    emit updateInitModels(7);

    employeeFuncModel = std::make_shared<QMEmployeeFunctionModel>(nullptr, db);
    employeeFuncModel->select();

    emit updateInitModels(8);

    qualiModel = std::make_shared<QMQualificationMatrixModel>(nullptr, db);
    qualiModel->initModel();
    qualiModel->select();

    emit updateInitModels(9);

    trainExceptionModel = std::make_shared<QMTrainingExceptionModel>(nullptr, db);
    trainExceptionModel->select();

    emit updateInitModels(10);

    shiftModel = std::make_shared<QSqlTableModel>(nullptr, db);
    shiftModel->setTable("Shift");
    shiftModel->setEditStrategy(QSqlTableModel::OnManualSubmit);
    shiftModel->setHeaderData(1, Qt::Horizontal, tr("Name"));
    shiftModel->sort(1, Qt::AscendingOrder);
    shiftModel->select();

    emit updateInitModels(11);

    infoModel = std::make_shared<QMInfoModel>(nullptr, db);

    emit updateInitModels(12);

    certificateModel = std::make_shared<QMCertificateModel>(nullptr, db);
    certificateModel->select();
    trainDataCertificateModel = std::make_shared<QMTrainDataCertificateModel>(nullptr, db);
    trainDataCertificateModel->select();
    trainDataCertificateViewModel = std::make_shared<QMTrainDataCertificateViewModel>(nullptr, db);
    trainDataCertificateViewModel->select();

    emit updateInitModels(13);

    employeeViewModel = std::make_shared<QMEmployeeViewModel>(nullptr, db);
    employeeViewModel->select();

    // Inform all customer about the update.
    emit updateInitModels(14);

    // Inform all customer about the update.
    emit modelsInitialized();

    // Inform that everything is finish and every was informed.
    emit afterInitModels();

    // Read some settings from the new database info table.
    readCertificateLocation(db);
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

bool QMDataManager::readCertificateLocation(const QSqlDatabase &db)
{
    QSqlQuery query(db);
    QString queryText =
        "SELECT name, value FROM Info "
        "WHERE name == \"certificate_location\"";

    if (!query.exec(queryText))
    {
        qWarning() << "cannot execute query";
        qWarning() << query.lastError().text();
        return false;
    }

    auto found = false;
    while (query.next())
    {
        if (query.value("name").toString() == "certificate_location")
        {
            found = true;
            auto value = query.value("value").toString();

            if (value == "internal")
            {
                certLoc = CertLoc::INTERNAL;
            }
            else if (value == "external")
            {
                certLoc = CertLoc::EXTERNAL;
            }
            else
            {
                qWarning() << "wrong value for certificate location setting in table";
                return false;
            }

            break;
        }
    }

    if (!found)
    {
        qWarning() << "cannot find certificate location setting";
        return false;
    }

    return false;
}

bool QMDataManager::readCertificateLocationPath(const QSqlDatabase &db)
{
    QSqlQuery query(db);
    QString queryText =
        "SELECT name, value FROM Info "
        "WHERE name == \"certificate_location\"";

    if (!query.exec(queryText))
    {
        qWarning() << "cannot execute query";
        qWarning() << query.lastError().text();
        return false;
    }

    auto found = false;
    while (query.next())
    {
        if (query.value("name").toString() == "certificate_location")
        {
            found = true;
            auto value = query.value("value").toString();
            certLocPath = value;

            break;
        }
    }

    if (!found)
    {
        qWarning() << "cannot find certificate location path setting";
        return false;
    }

    return false;
}

void QMDataManager::sendModelChangedInformation(QObject *sender)
{
    emit modelChanged(sender);
}
