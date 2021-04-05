// qmdatamanager.h is part of QualificationMatrix
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

#ifndef QMDATAMANAGER_H
#define QMDATAMANAGER_H

#include <QObject>
#include <memory>

// minimum version db must have, otherwise the software won't work with it - 0, 0 means no version
// table or entry is available, cause the first version doesn't include this information
#define DB_MIN_MAJOR 1
#define DB_MIN_MINOR 2

// Forward declaration
class QMSqlRelationalTableModel;
class QSqlRelationalTableModel;
class QSqlDatabase;
class QSqlTableModel;
class QMQualiResultModel;
class QMQualiMatrixModel;

// Typedefs
typedef std::shared_ptr<QMSqlRelationalTableModel> sp_qmRelTableModel;
typedef std::shared_ptr<QSqlRelationalTableModel> sp_relTableModel;
typedef std::shared_ptr<QSqlTableModel> sp_tableModel;

// Enumerations
enum class CertLoc
{
    INTERNAL,
    EXTERNAL
};

/// Manager for data including models.
/// \author Christian Kr, Copyright 2020
class QMDataManager: public QObject
{
Q_OBJECT

public:
    /// Get a singleton instance of the data manager.
    /// \return Singleton instance of the data manager.
    static QMDataManager * getInstance()
    {
        if (instance == nullptr)
        {
            instance = new QMDataManager();
        }

        return instance;
    }

    /// Delete the static variable.
    static void resetInstance()
    {
        if (instance != nullptr)
        {
            delete instance;
            instance = nullptr;
        }
    }

    static int getMinor() { return DB_MIN_MINOR; }
    static int getMajor() { return DB_MIN_MAJOR; }

    /// Get the location of the certificates. The setting is found in "Info" table.
    /// \return Certification location from enumeration type CertLoc.
    CertLoc getCertificateLocation() { return certLoc; }

    /// Get the location of the certificates. The setting is found in "Info" table.
    /// \return Certification location from enumeration type CertLoc.
    QString getCertificateLocationPath() { return certLocPath; }

    /// Read the certification location setting from table.
    /// \param db Database to work on.
    /// \return True if setting could be found and read, else false.
    bool readCertificateLocation(const QSqlDatabase &db);

    /// Read the certification location path setting from table.
    /// \param db Database to work on.
    /// \return True if setting could be found and read, else false.
    bool readCertificateLocationPath(const QSqlDatabase &db);

    // Delete functions that should not be used, cause of singleton pattern. They should be deleted
    // public for better error message if anything goes wrong.
    QMDataManager(QMDataManager const &) = delete;
    void operator=(QMDataManager const &) = delete;

    /// Initialize all models with the given database object. The initializing process might
    /// include the loss of all model pointer references. Cause of using smart pointers, this
    /// won't be a problem in the sense of a memory leak. But objects that use the model pointers
    /// have to get the new pointers. The custom quali* models are NOT PART OF OBJECT CREATION
    /// UPDATE.
    void initializeModels(QSqlDatabase &db);

    /// Test for the database version. This is only a test of an entry with the right version
    /// number. It does not include any test of available tables and/or columns.
    /// \param db The database to test.
    /// \return True if right version, else false.
    static bool testVersion(QSqlDatabase &db);

    /// Test for right table structure. This test only has a look for the number of tables and the
    /// table names. It does not has a look into the structure of a single table itself.
    /// \param db The database to test.
    /// \return True if structure is ok, else false.
    static bool testTableStructure(QSqlDatabase &db);

    sp_qmRelTableModel getFuncModel() { return funcModel; }
    sp_qmRelTableModel getTrainModel() { return trainModel; }
    sp_qmRelTableModel getTrainDataModel() { return trainDataModel; }
    sp_tableModel getTrainGroupModel() { return trainGroupModel; }
    sp_tableModel getTrainDataStateModel() { return trainDataStateModel; }
    sp_tableModel getFuncGroupModel() { return funcGroupModel; }
    sp_qmRelTableModel getEmployeeModel() { return employeeModel; }
    sp_qmRelTableModel getEmployeeFuncModel() { return employeeFuncModel; }
    std::shared_ptr<QMQualiResultModel> getQualiResultModel() { return qualiResultModel; }
    std::shared_ptr<QMQualiMatrixModel> getQualiMatrixModel() { return qualiMatrixModel; }
    sp_qmRelTableModel getQualiModel() { return qualiModel; }
    sp_relTableModel getTrainExceptionModel() { return trainExceptionModel; }
    sp_tableModel getShiftModel() { return shiftModel; }
    sp_tableModel getInfoModel() { return infoModel; }
    sp_tableModel getCertificateModel() { return certificateModel; }
    sp_tableModel getTrainDataCertificateModel() { return trainDataCertificateModel; }
    sp_tableModel getTrainDataCertificateViewModel() { return trainDataCertificateViewModel; }
    sp_relTableModel getEmployeeViewModel() { return employeeViewModel; }

    /// QSqlTableModel dirty test
    bool isAnyDirty() const;

    /// Send a signal to all connected models.
    /// \param sender The sender of the information (mostly object calling this function).
    void sendModelChangedInformation(QObject *sender);

private:
    /// Constructor
    QMDataManager();

    /// Destructor
    ~QMDataManager() override = default;

    // Models as smart pointer (shared). They are needed all over the application. And they all use
    // this single model. The objects might change on database change. The customer has to take
    // about that. (Get up to date objects.)
    sp_qmRelTableModel funcModel;
    sp_qmRelTableModel trainModel;
    sp_qmRelTableModel trainDataModel;
    sp_tableModel trainGroupModel;
    sp_tableModel funcGroupModel;
    sp_qmRelTableModel employeeModel;
    sp_qmRelTableModel employeeFuncModel;
    sp_qmRelTableModel qualiModel;
    sp_relTableModel trainExceptionModel;
    sp_tableModel shiftModel;
    sp_tableModel trainDataStateModel;
    sp_tableModel infoModel;
    sp_tableModel trainDataCertificateModel;
    sp_tableModel trainDataCertificateViewModel;
    sp_tableModel certificateModel;
    sp_relTableModel employeeViewModel;

    // These models are custom ones. They won't get updated as an object itself, but they are based
    // on the basic models above. So the basic models have to be updated inside the quali models.
    std::shared_ptr<QMQualiResultModel> qualiResultModel;
    std::shared_ptr<QMQualiMatrixModel> qualiMatrixModel;

signals:
    /// Whent he models will be initialized, this signal gets emited. This is useful to inform
    /// every customer of the shared pointer models, that they might have been changed. So the
    /// customer has to update the reference pointer.
    void modelsInitialized();

    /// Emited before models will be initialized.
    /// \param maxSteps Maximum number of initializing models.
    void beforeInitModels(int maxSteps);

    /// Emited after models have been initialized and everyone has been iformed about it.
    void afterInitModels();

    /// Emited while models will be initialized.
    /// \param currentStep Current number of initializing model.
    void updateInitModels(int currentSteps);

    /// Informate all registered models, that an table has been changed.
    /// \param sender The object sending the signal.
    void modelChanged(QObject *sender);

private:
    static QMDataManager *instance;

    CertLoc certLoc;
    QString certLocPath;
};

#endif // QMDATAMANAGER_H
