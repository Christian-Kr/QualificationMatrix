//
// qmdatabaseupdatedialog.h is part of QualificationMatrix
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

#ifndef DATABASEUPDATEDIALOG_H
#define DATABASEUPDATEDIALOG_H

#include <QDialog>
#include <memory>

QT_BEGIN_NAMESPACE
namespace Ui
{
class QMDatabaseUpdateDialog;
}
QT_END_NAMESPACE

/// Show information about the current database version and update it.
/// \author Christian Kr, Copyright 2020
class QMDatabaseUpdateDialog: public QDialog
{
Q_OBJECT

public:
    /// Constructor
    /// \param parent The parent object of the widgt. This is important for a modal dialog.
    /// \param dbName The name of the database in the QSqlDatabase to work with.
    explicit QMDatabaseUpdateDialog(const QString &dbName = "default", QWidget *parent = nullptr);

    /// Destructor
    ~QMDatabaseUpdateDialog() override;

protected:
    /// Override of QDialog.
    virtual void reject() override;

private:
    /// Read version from database.
    void readDatabaseVersion();

    /// Updates the ui, which means basically  the text parts in the ui elements.
    void updateUi();

    Ui::QMDatabaseUpdateDialog *ui;

    QString dbName;

    int major;
    int minor;
};

#endif // DATABASEUPDATEDIALOG_H
