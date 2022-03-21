// qmimportcsvdialog.h is part of QualificationMatrix
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

#ifndef QMIMPORTCSVDIALOG_H
#define QMIMPORTCSVDIALOG_H

#include "framework/dialog/qmdialog.h"

#include <memory>

QT_BEGIN_NAMESPACE
namespace Ui
{
class QMImportCsvDialog;
}
QT_END_NAMESPACE

class QFile;
class QSqlTableModel;
class QMSqlTableModel;
class QSqlRelationalTableModel;

/// Dialog and business logic for import data from csv certificate. In future the business logic
/// should be implemented inside the QMDataManager.
/// \author Christian Kr, Copyright 2020
class QMImportCsvDialog: public QMDialog
{
Q_OBJECT

public:
    /// Constructor
    /// \param parent The main parent object.
    explicit QMImportCsvDialog(QWidget *parent = nullptr);

    /// Destructor
    ~QMImportCsvDialog() override;

public slots:
    /// Open the certificate path from a open dialog window.
    void openImportFile();

    /// Open the certificate path from a open dialog window.
    void openBackupFile();

    /// Start the process of importing.
    void accept() override;

    /// Update model data.
    void updateData();

private:
    /// Parse the csv certificate and import data.
    /// \param importFile The certificate object which should already be opened.
    void parseCsv(QFile &importFile);

    /// Handles a string line from certificate, when an error occured.
    /// \param line
    void handleErrorLine(QString &line);

    /// Check the backup certificate for existence and writablility.
    /// \return True if everything ok, else false.
    bool checkBackupFile();

    Ui::QMImportCsvDialog *ui;

    std::unique_ptr<QSqlTableModel> trainViewModel;
    std::unique_ptr<QSqlTableModel> employeeViewModel;
    std::unique_ptr<QSqlTableModel> shiftViewModel;
    std::unique_ptr<QSqlTableModel> trainGroupViewModel;
    std::unique_ptr<QSqlTableModel> trainDataStateViewModel;
    std::unique_ptr<QMSqlTableModel> trainDataModel;
};

#endif // QMIMPORTCSVDIALOG_H
