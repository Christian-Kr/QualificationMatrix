// qmsigninglistdialog.h is part of QualificationMatrix
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

#ifndef QMSIGNINGLISTDIALOG_H
#define QMSIGNINGLISTDIALOG_H

#include "framework/dialog/qmdialog.h"

#include <memory>

class QSqlTableModel;
class QMSqlTableModel;
class QSqlRelationalTableModel;
class QPrinter;
class QSqlDatabase;
class QMTrainDataConflictDialog;

QT_BEGIN_NAMESPACE
namespace Ui
{
class QMSigningListDialog;
}
QT_END_NAMESPACE

// Struct for train data information.
struct QMTrainDataInfo
{
    // Primary key in database
    int id;

    // The Primary key id of the employee.
    int employeeId;

    // The primary key id of the training.
    int trainId;

    // The primary key id of the training state.
    int trainstateId;

    // The date of the training.
    QString trainDate;
};

// Struct for signing list employee info, holding some information for later handling.
struct QMSigningListEmployeeInfo
{
    // Primary key of the employee in the database.
    int id;

    // Name of the employee in the database.
    QString name;
};

/// Sett different proprties to start creating a signing list.
/// \author Christian Kr, Copyright 2020
class QMSigningListDialog: public QMDialog
{
    Q_OBJECT

public:
    /// Constructor
    /// \param parent The parent object of the widgt. This is important for a modal dialog.
    explicit QMSigningListDialog(QWidget *parent = nullptr);

    /// Destructor
    ~QMSigningListDialog() override;

    /// Override from QMDialog.
    void saveSettings() override;

    /// Override from QMDialog
    void loadSettings() override {};

public slots:
    /// Update data (models).
    void updateData();

    /// Add a new employee.
    void addEmployee();

    /// Add new employees from group.
    [[maybe_unused]] void addEmployeeFromGroup();

    /// Remove an existing and selected employee.
    [[maybe_unused]] void removeEmployee();

    /// Override from QDialog.
    void accept() override;

    /// Print list to pdf.
    void printToPDF();

    /// Request function for the pdf creation of painter object.
    void paintPdfRequest(QPrinter *printer);

    /// Open the image
    [[maybe_unused]] void openImage();

    /// Clear signing list
    [[maybe_unused]] void clearList();

    /// Training changed.
    [[maybe_unused]] void trainingChanged();

    /// Finished the train data conflict dialog.
    /// \param result The result of the dialog (e.g. accepted or rejected).
    [[maybe_unused]] void trainDataConflictDialogFinished(int result);

    /// Called when the check box state of the train data creation changed.
    /// \param state The state of a qt check box.
    [[maybe_unused]] void createTrainDataEntriesChanged(int state);

private:
    /// Test whether the employee list contains the given employee id.
    /// \param employeeId The id (primary key in table) of the employee to test.
    /// \return True if the employee already exist, else false.
    [[nodiscard]] bool listContainsEmployee(const int &employeeName) const;

    /// Finally create the given set of train data entries.
    void createTrainDataEntries();

    /// This method should be called to perform a check on whether there are entries already existing, which should
    /// not be created.
    void createTrainDataEntriesCheck();

    /// Get a list of selected employee ids.
    /// \return List with all ids.
    [[nodiscard]] QStringList getSelectedEmployeeIds() const;

    /// Checks all input fields, whether they are fully filled.
    /// \return True if success, else false.
    [[nodiscard]] bool checkInput();

    Ui::QMSigningListDialog *ui;

    QList<QMSigningListEmployeeInfo> *m_selectedEmployees;
    QList<QMSigningListEmployeeInfo> *m_duplicatedEmployeeEntries;

    QMTrainDataConflictDialog *m_trainDataConflictDialog;

    std::unique_ptr<QSqlTableModel> m_trainDataStateViewModel;
    std::unique_ptr<QSqlTableModel> m_employeeViewModel;
    std::unique_ptr<QSqlTableModel> m_shiftViewModel;
    std::unique_ptr<QSqlTableModel> m_trainViewModel;
};

#endif // QMSIGNINGLISTDIALOG_H
