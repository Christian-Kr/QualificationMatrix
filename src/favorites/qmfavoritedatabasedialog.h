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

QT_BEGIN_NAMESPACE
namespace Ui
{
class QMFavoriteDatabaseDialog;
}
QT_END_NAMESPACE

/// Data structure for a favorite.
struct QMFavoriteDatabaseEntry
{
    QString name;
    QString dbFilePath;
    QString fileAccess;
};

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

    /// Save the favorite entry that has been changed.
    /// \param validation true if validation should be done, else false
    [[maybe_unused]] void saveFavoriteEntry(const bool &validation = true);

    /// Reset the text fields to the values of the favorite objects.
    [[maybe_unused]] void resetFavoriteEntry();

    /// The selection fo the favorite list widget changed.
    [[maybe_unused]] void favoriteSelectionChanged();

    /// The name of the a favorite in the line edit field has changed.
    [[maybe_unused]] void favoriteNameChanged();

    /// The database file of the a favorite in the line edit field has changed.
    [[maybe_unused]] void favoriteDatabaseFileChanged();

signals:
    /// Emits, when a group favorite has been selected to open.
    void openDatabase(QString dbFilePath);

protected:
    /// Override from QDialog.
    void reject() override;

    /// Override from QDialog.
    void accept() override;

private:
    /// Updates the ui, which means basically  the text parts in the ui elements.
    void updateUi();

    /// Update the favorites list widget with all favorites objects from m_favorites.
    void updateFavoritesListWidget();

    /// Find existing name in favorites.
    /// \param name the name
    /// \return true if the name already exist, else false
    [[nodiscard]] bool favoriteNameExist(const QString &name) const;

    /// Read the favorites data from the internal settings file.
    void readFavoritesFromSettings();

    /// Write settings data to the internal settings file.
    void writeFavoritesToSettings();

    /// Add the given favorite object to the list widget.
    /// \param entry the favorite entry to add
    void addFavoriteToWidget(QMFavoriteDatabaseEntry *entry);

    /// Find the right favorite database entry struct for the given name.
    /// \param found true if favorite has been found, else false
    /// \param name the name of the favorite to search for
    QMFavoriteDatabaseEntry* getFavoriteFromName(const QString &name, bool &found);

    Ui::QMFavoriteDatabaseDialog *ui;

    QList<QMFavoriteDatabaseEntry*> m_favorites;
    QMFavoriteDatabaseEntry *m_selectedFavorite;
};

#endif // QMFAVORITEDATABASEDIALOG_H
