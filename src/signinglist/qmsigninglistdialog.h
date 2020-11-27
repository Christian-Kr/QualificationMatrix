//
// qmsigninglistdialog.h is part of QualificationMatrix
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

#ifndef QMSIGNINGLISTDIALOG_H
#define QMSIGNINGLISTDIALOG_H

#include <QDialog>
#include <memory>

// Forward declaration.
class QSqlTableModel;
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
class QMSigningListDialog: public QDialog
{
Q_OBJECT

public:
    /// Constructor
    /// \param parent The parent object of the widgt. This is important for a modal dialog.
    explicit QMSigningListDialog(QWidget *parent = nullptr);

    /// Destructor
    ~QMSigningListDialog() override;

    /// Save settings from the dialog.
    void saveSettings();

protected:
    /// Override from QDialog.
    void closeEvent(QCloseEvent *event) override;

    /// Override from QDialog.
    void keyPressEvent(QKeyEvent *event) override;

public slots:
    /// Update data (models).
    void updateData();

    /// Add a new employee.
    void addEmployee();

    /// Remove an existing and selected employee.
    void removeEmployee();

    /// Override from QDialog.
    void accept() override;

    /// Print list to pdf.
    void printToPDF();

    /// Request function for the pdf creation of painter object.
    void paintPdfRequest(QPrinter *printer);

private:
    Ui::QMSigningListDialog *ui;

    std::shared_ptr<QSqlRelationalTableModel> employeeModel;
    std::shared_ptr<QSqlTableModel> shiftModel;
    std::shared_ptr<QSqlRelationalTableModel> trainModel;
    std::shared_ptr<QSqlRelationalTableModel> trainDataModel;
};

#endif // QMSIGNINGLISTDIALOG_H
