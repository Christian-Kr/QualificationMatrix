//
// databasedialog.h is part of QualificationMatrix
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

#ifndef DATABASEDIALOG_H
#define DATABASEDIALOG_H

#include <QDialog>
#include <memory>

QT_BEGIN_NAMESPACE
namespace Ui
{
class QMDatabaseDialog;
}
QT_END_NAMESPACE

/// Show information about the current database and load it afterwarts.
/// \author Christian Kr, Copyright 2020
class QMDatabaseDialog: public QDialog
{
Q_OBJECT

public:
    /// Constructor
    /// \param parent The parent object of the widgt. This is important for a modal dialog.
    /// \param dbName The name of the database in the QSqlDatabase to work with.
    explicit QMDatabaseDialog(const QString &dbName = "default", QWidget *parent = nullptr);

    /// Destructor
    ~QMDatabaseDialog() override;

public slots:
    /// Updates the type widget, which shows information about the database driver. This widget
    /// is a stacked widget and needs to be updated, related to the type (LOCAL or REMOTE).
    void updateTypeWidget();

    /// Choose a file for local database types.
    void chooseDatabaseFile();

    /// Changes the connection (connected -> disconnected; disconnected -> connected).
    void switchConnected();

protected:
    /// Override of QDialog.
    virtual void reject() override;

private:
    /// Updates the ui, which means basically  the text parts in the ui elements.
    void updateUi();

    /// Initialize the list of database driver with the available ones in QSqlDatabase.
    void initDatabaseDriverList();

    /// Load information about database from QSqlDatabase into ui elements.
    void loadValuesFromDatabase();

    Ui::QMDatabaseDialog *ui;

    QString dbName;
};

#endif // DATABASEDIALOG_H
