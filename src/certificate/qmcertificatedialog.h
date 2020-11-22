//
// qmcertificatedialog.h is part of QualificationMatrix
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

#ifndef QMCERTIFICATEDIALOG_H
#define QMCERTIFICATEDIALOG_H

#include <QDialog>
#include <memory>

// Forward declaration.
class QSqlTableModel;
class QSortFilterProxyModel;

QT_BEGIN_NAMESPACE
namespace Ui
{
class QMCertificateDialog;
}
QT_END_NAMESPACE

/// Show certificate files and manage them.
/// \author Christian Kr, Copyright 2020
class QMCertificateDialog: public QDialog
{
Q_OBJECT

public:
    /// Constructor
    /// \param parent The parent object of the widgt. This is important for a modal dialog.
    explicit QMCertificateDialog(QWidget *parent = nullptr);

    /// Destructor
    ~QMCertificateDialog() override;

    /// Save settings from the dialog.
    void saveSettings();

protected:
    /// Override from QDialog.
    void closeEvent(QCloseEvent *event) override;

public slots:
    /// Update the models, cause they might have changed.
    void updateData();

    /// Update table with filter from ui components.
    void updateFilter();

    /// Reset the content of the filter.
    void resetFilter();

private:
    Ui::QMCertificateDialog *ui;

    QSortFilterProxyModel *typeFilterModel;
    QSortFilterProxyModel *nameFilterModel;

    std::shared_ptr<QSqlTableModel> certificateModel;
};

#endif // QMCERTIFICATEDIALOG_H
