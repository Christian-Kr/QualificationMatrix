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

QT_BEGIN_NAMESPACE
namespace Ui
{
class QMSigningListDialog;
}
QT_END_NAMESPACE

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

public slots:
    /// Update data (models).
    void updateData();

    /// Add a new employee.
    void addEmployee();

    /// Add new employees from group.
    void addEmployeeFromGroup();

    /// Remove an existing and selected employee.
    void removeEmployee();

    /// Override from QDialog.
    void accept() override;

    /// Print list to pdf.
    void printToPDF();

    /// Request function for the pdf creation of painter object.
    void paintPdfRequest(QPrinter *printer);

    /// Open the image
    void openImage();

    /// Clear signing list
    void clearList();

    /// Training changed.
    void trainingChanged();

private:
    /// Test whether the employee list on the dialog contains the given employee name.
    /// \param employeeName Name of the employee to test.
    /// \return True if the employee already exist, else false.
    bool listContainsEmployee(const QString &employeeName) const;

    /// Create entries for every employee with the given training information. All entries will be set to be planned.
    /// If the entry already exist, don't create them again. Informate the user afterwarts about the amount of entries
    /// that have been created and that already exist.
    void createTrainDataEntries();

    Ui::QMSigningListDialog *ui;

    std::unique_ptr<QSqlTableModel> employeeViewModel;
    std::unique_ptr<QSqlTableModel> shiftViewModel;
    std::unique_ptr<QSqlTableModel> trainViewModel;
};

#endif // QMSIGNINGLISTDIALOG_H
