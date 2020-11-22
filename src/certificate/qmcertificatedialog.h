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

private:
    Ui::QMCertificateDialog *ui;
};

#endif // QMCERTIFICATEDIALOG_H
