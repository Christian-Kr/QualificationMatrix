//
// qmtraindatadetailsdialog.h is part of QualificationMatrix
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

#ifndef QMTRAINDATADETAILSDIALOG_H
#define QMTRAINDATADETAILSDIALOG_H

#include "framework/qmdialog.h"
#include <memory>

// Forward declaration.
class QSqlRelationalTableModel;
class QSortFilterProxyModel;
class QSqlTableModel;

// Typedefs
typedef std::shared_ptr<QSqlRelationalTableModel> sp_relTableModel;
typedef std::shared_ptr<QSqlTableModel> sp_tableModel;

namespace Ui
{
class QMTrainDataDetailsDialog;
}

class QMTrainDataDetailsDialog: public QMDialog
{
Q_OBJECT

public:
    /// Constructor
    /// \param parent
    /// \param selRow Selected row in trainDataModel to edit.
    /// \param trainDataModel Model to edit.
    explicit QMTrainDataDetailsDialog(
        QSortFilterProxyModel *trainDataModel, int selRow, QWidget *parent = nullptr);

    /// Destructor
    ~QMTrainDataDetailsDialog() override;

    /// Override from QMDialog.
    void saveSettings() override;

    /// Override from QDialog.
    void accept() override;

    /// Override from QDialog.
    void reject() override;

public slots:
    /// The apply button has been pressed.
    void apply();

    /// Add an existing certificate.
    void addCertificate();

    /// Remove an existing certificate.
    void removeCertificate();

private:
    /// Update ui data from models.
    void updateUi();

    /// Check data for unvalid entries.
    /// \return True is check is ok, else false.
    bool checkData() const;

    Ui::QMTrainDataDetailsDialog *ui;

    int selRowEdit;

    QSortFilterProxyModel *trainDataModelEdit;
    QSortFilterProxyModel *trainDataCertViewFilterModel;

    sp_tableModel certificateModel;
    sp_tableModel trainDataCertificateModel;
    sp_tableModel trainDataCertificationViewModel;
    sp_relTableModel employeeModel;
    sp_relTableModel trainModel;
    sp_tableModel trainDataStateModel;
};

#endif // QMTRAINDATADETAILSDIALOG_H
