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
class QFile;

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
    /// Add a new certificate.
    void addCertificate();

    /// Update the models, cause they might have changed.
    void updateData();

    /// Update table with filter from ui components.
    void updateFilter();

    /// Reset the content of the filter.
    void resetFilter();

private:
    /// Save the given file internal: Writing to database blob.
    /// \param file File to save internal.
    /// \return True if success, else false.
    bool saveFileInternal(QFile &file);

    /// Save the given file external: Copy to file system structure.
    /// \param file File to save external.
    /// \return New file name if success, else empty string.
    static QString saveFileExternal(QFile &file);

    Ui::QMCertificateDialog *ui;

    QSortFilterProxyModel *typeFilterModel;
    QSortFilterProxyModel *nameFilterModel;

    std::shared_ptr<QSqlTableModel> certificateModel;
};

#endif // QMCERTIFICATEDIALOG_H
