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

#include "framework/qmdialog.h"
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

/// Enumeration definitions
enum class Mode
{
    CHOOSE,
    MANAGE
};

/// Show certificate files and manage them. The dialog can handle different modes. In "CHOOSE"
/// mode, the dialog save a selection, which can be claaed later. Also the name on the buttons
/// will be different, so that the user can determine which mode is running and what he has to do.
/// Neverless, in both modes, one can manage (search, filter, add, remove, preview, ...)
/// different certificates.
/// \author Christian Kr, Copyright 2020
class QMCertificateDialog: public QMDialog
{
Q_OBJECT

public:
    /// Constructor
    /// \param parent The parent object of the widgt. This is important for a modal dialog.
    /// \param mode The ode of the dialog to choose.
    explicit QMCertificateDialog(Mode mode = Mode::MANAGE, QWidget *parent = nullptr);

    /// Destructor
    ~QMCertificateDialog() override;

    /// Override from QMDialog.
    void saveSettings() override;

    /// Return the selected entry id.
    int getSelectedId() { return selectedId; }

public slots:
    /// Add a new certificate.
    void addCertificate();

    /// Remove an existing and selected certificate.
    void removeCertificate();

    /// Update the models, cause they might have changed.
    void updateData();

    /// Update table with filter from ui components.
    void updateFilter();

    /// Reset the content of the filter.
    void resetFilter();

    /// Show the current selected certificate.
    void showCertificate();

    /// Override from QDialog.
    void accept() override;

private:
    /// Save the given file external: Copy to file system structure.
    /// \param file File to save external.
    /// \return New file name if success, else empty string.
    static QString saveFileExternal(QFile &file);

    Ui::QMCertificateDialog *ui;

    Mode runMode;

    int selectedId;

    QSortFilterProxyModel *typeFilterModel;
    QSortFilterProxyModel *nameFilterModel;

    std::shared_ptr<QSqlTableModel> certificateModel;
    std::shared_ptr<QSqlTableModel> trainDataCertificateModel;
};

#endif // QMCERTIFICATEDIALOG_H
