// qmfavoritedatabasedialog.h is part of QualificationMatrix
//
// QualificationMatrix is free software: you can redistribute it and/or modify it under the terms
// of the GNU General Public License as published by the Free Software Foundation, either version
// 3 of the License, or (at your option) any later version.
//
// QualificationMatrix is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
// without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See
// the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with QualificationMatrix.
// If not, see <http://www.gnu.org/licenses/>.

#ifndef QMFAVORITEDATABASEDIALOG_H
#define QMFAVORITEDATABASEDIALOG_H

#include <QDialog>
#include <memory>

// Forward declaration
class QMFavoriteModel;

QT_BEGIN_NAMESPACE
namespace Ui
{
class QMFavoriteDatabaseDialog;
}
QT_END_NAMESPACE

/// Show information about the current database version and update it.
/// \author Christian Kr, Copyright 2020
class QMFavoriteDatabaseDialog: public QDialog
{
    Q_OBJECT

public:
    /// Constructor
    /// \param parent The parent object of the widgt. This is important for a modal dialog.
    /// \param dbName The name of the database in the QSqlDatabase to work with.
    explicit QMFavoriteDatabaseDialog(QWidget *parent = nullptr);

    /// Destructor
    ~QMFavoriteDatabaseDialog() override;

public slots:
    /// Create empty database entry.
    [[maybe_unused]] void createFavoriteEntry();

    /// Remove the selected database entry.
    [[maybe_unused]] void removeFavoriteEntry();

    /// Triggered, when someone double clicks on a table view element.
    /// \param index the model index that has been clicked
    void favoriteDoubleClicked(const QModelIndex &index);

signals:
    /// Emits, when a group favorite has been selected to open.
    void openDatabase(QString dbFilePath);

protected:
    /// Override from QDialog.
    void reject() override;

    /// Override from QDialog.
    void accept() override;

private:
    Ui::QMFavoriteDatabaseDialog *m_ui;

    QMFavoriteModel *m_favoriteModel;
};

#endif // QMFAVORITEDATABASEDIALOG_H
