// qmfavoritedatabasedialog.cpp is part of QualificationMatrix
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

#include "qmfavoritedatabasedialog.h"
#include "ui_qmfavoritedatabasedialog.h"

#include "data/qmdatamanager.h"
#include "settings/qmapplicationsettings.h"

#include <QListWidgetItem>
#include <QMessageBox>
#include <QFileDialog>

QMFavoriteDatabaseDialog::QMFavoriteDatabaseDialog(QWidget *parent)
    : QDialog(parent)
    , m_selectedFavorite(nullptr)
{
    ui = new Ui::QMFavoriteDatabaseDialog;
    ui->setupUi(this);

    connect(ui->lwFavorites, &QListWidget::itemSelectionChanged, this,
            &QMFavoriteDatabaseDialog::favoriteSelectionChanged);
    connect(ui->leName, &QLineEdit::textChanged, this,
            &QMFavoriteDatabaseDialog::favoriteNameChanged);
    connect(ui->leDatabaseFile, &QLineEdit::textChanged, this,
            &QMFavoriteDatabaseDialog::favoriteDatabaseFileChanged);

    readFavoritesFromSettings();
}

QMFavoriteDatabaseDialog::~QMFavoriteDatabaseDialog()
{
    delete ui;

    while (!m_favorites.isEmpty())
    {
        delete m_favorites.takeFirst();
    }
}

void QMFavoriteDatabaseDialog::favoriteDatabaseFileChanged()
{
    updateUi();
}

void QMFavoriteDatabaseDialog::favoriteNameChanged()
{
    updateUi();
}

void QMFavoriteDatabaseDialog::favoriteSelectionChanged()
{
    // test whether you should save the the configs or not
    if (ui->pbSave->isEnabled())
    {
        auto result = QMessageBox::question(this, tr("Favoriten speichern"),
                tr("Möchten Sie die Änderungen des letzten Favoriten speichern?"),
                QMessageBox::Yes | QMessageBox::No);
        if (result == QMessageBox::Yes)
        {
            saveFavoriteEntry();
        }
    }

    auto selectedItems = ui->lwFavorites->selectedItems();
    if (selectedItems.size() != 1)
    {
        qWarning() << "to many selection should not be possible";
        return;
    }

    auto selectedItem = selectedItems.first();

    bool found = false;
    auto selectedFavoriteName = selectedItem->data(Qt::DisplayRole).toString();
    auto selectedFavorite = getFavoriteFromName(selectedFavoriteName, found);

    if (!found)
    {
        qWarning() << "no favorite struct object with the given name found";
        return;
    }

    ui->leName->setText(selectedFavorite->name);
    ui->leDatabaseFile->setText(selectedFavorite->dbFilePath);

    m_selectedFavorite = selectedFavorite;
    favoriteNameChanged();
}

QMFavoriteDatabaseEntry* QMFavoriteDatabaseDialog::getFavoriteFromName(
        const QString &name, bool &found)
{
    for (const auto &favorite : m_favorites)
    {
        if (favorite->name.compare(name) == 0)
        {
            found = true;
            return favorite;
        }
    }

    found = false;
    return {};
}

void QMFavoriteDatabaseDialog::updateUi()
{
    if (m_selectedFavorite == nullptr)
    {
        return;
    }

    ui->pbSave->setEnabled(
            (ui->leName->text().compare(m_selectedFavorite->name) != 0) ||
            (ui->leDatabaseFile->text().compare(m_selectedFavorite->dbFilePath) != 0));
    ui->pbReset->setEnabled(
            (ui->leName->text().compare(m_selectedFavorite->name) != 0) ||
            (ui->leDatabaseFile->text().compare(m_selectedFavorite->dbFilePath) != 0));
}

void QMFavoriteDatabaseDialog::reject()
{
    QDialog::reject();
}

void QMFavoriteDatabaseDialog::accept()
{
    if (m_selectedFavorite == nullptr)
    {
        return;
    }

    QDialog::accept();

    emit openDatabase(m_selectedFavorite->dbFilePath);
}

void QMFavoriteDatabaseDialog::readFavoritesFromSettings()
{
    // clear all favorites, cause they will be recreated
    m_selectedFavorite = nullptr;
    while (!m_favorites.isEmpty())
    {
        delete m_favorites.takeFirst();
    }

    auto &settings = QMApplicationSettings::getInstance();

    // the data storage is mainly a list of lists. The outer list is separated with ';' and the
    // inner list with ','

    auto data = settings.read("Favorites/Data", QString()).toString();
    auto favListRaw = data.split(";");

    for (const auto &favRaw : favListRaw)
    {
        auto tmpList = favRaw.split(",");
        if (tmpList.size() != 2)
        {
            qWarning() << "parse failure while reading favorites from settings";
            continue;
        }

        const auto &favName = tmpList.at(0);
        const auto &favDbFilePath = tmpList.at(1);

        // create new favorite and add it
        auto favorite = new QMFavoriteDatabaseEntry();
        favorite->name = favName;
        favorite->dbFilePath = favDbFilePath;
        m_favorites.append(favorite);
    }

    // update the ui after reading everything in
    updateFavoritesListWidget();
}

void QMFavoriteDatabaseDialog::updateFavoritesListWidget()
{
    ui->lwFavorites->clear();

    for (const auto &favorite : m_favorites)
    {
        addFavoriteToWidget(favorite);
    }
}

void QMFavoriteDatabaseDialog::writeFavoritesToSettings()
{
    auto &settings = QMApplicationSettings::getInstance();

    // create the string to safe
    QString favListRaw = "";
    for (const auto &favorite : m_favorites)
    {
        QString favRaw = favorite->name + "," + favorite->dbFilePath;

        if (!favListRaw.isEmpty())
        {
            favListRaw += ";";
        }

        favListRaw += favRaw;
    }

    settings.write("Favorites/Data", favListRaw);
}

void QMFavoriteDatabaseDialog::removeFavoriteEntry()
{
    auto result = QMessageBox::question(this, tr("Favoriten löschen"),
            tr("Möchten Sie den Favoriten löschen?"),
            QMessageBox::Yes | QMessageBox::No);
    if (result != QMessageBox::Yes)
    {
        return;
    }

    auto selectedItems = ui->lwFavorites->selectedItems();
    if (selectedItems.size() != 1)
    {
        qWarning() << "to many selection should not be possible";
        return;
    }

    auto selectedItem = selectedItems.first();

    bool found = false;
    auto selectedFavoriteName = selectedItem->data(Qt::DisplayRole).toString();
    auto selectedFavorite = getFavoriteFromName(selectedFavoriteName, found);

    if (!found)
    {
        qWarning() << "no favorite struct object with the given name found";
        return;
    }

    // because we delete an item safe and reset button should be enabled
    ui->pbSave->setEnabled(false);
    ui->pbReset->setEnabled(false);

    // delete the elements
    delete selectedItem;
    m_favorites.removeOne(selectedFavorite);
    delete selectedFavorite;

    writeFavoritesToSettings();
}

void QMFavoriteDatabaseDialog::createFavoriteEntry()
{
    // test whether you should save the the configs or not
    if (ui->pbSave->isEnabled())
    {
        auto result = QMessageBox::question(this, tr("Favoriten speichern"),
                tr("Möchten Sie die Änderungen des letzten Favoriten speichern?"),
                QMessageBox::Yes | QMessageBox::No);
        if (result == QMessageBox::Yes)
        {
            saveFavoriteEntry();
            if (ui->pbSave->isEnabled())
            {
                return;
            }
        }
        else
        {
            resetFavoriteEntry();
        }
    }

    auto *entry = new QMFavoriteDatabaseEntry;

    QString name = tr("Favorit");
    unsigned int num = 0;
    while (favoriteNameExist(name))
    {
        num++;
        name = tr("Favorit %1").arg(num);
    }

    entry->name = name;
    m_favorites.append(entry);

    addFavoriteToWidget(entry);

    saveFavoriteEntry(false);
}

void QMFavoriteDatabaseDialog::resetFavoriteEntry()
{
    if (m_selectedFavorite == nullptr)
    {
        qWarning() << "no favorite selected to reset";
        return;
    }

    ui->leName->setText(m_selectedFavorite->name);
    ui->leDatabaseFile->setText(m_selectedFavorite->dbFilePath);

    updateUi();
}

void QMFavoriteDatabaseDialog::openDatabaseFile()
{
    // open a database file
    QFileDialog fileDialog(this);

    fileDialog.setWindowTitle(QObject::tr("Datenbank öffnen"));
    fileDialog.setFileMode(QFileDialog::FileMode::ExistingFile);
    fileDialog.setAcceptMode(QFileDialog::AcceptMode::AcceptOpen);
    fileDialog.setNameFilter(QObject::tr("QualificationMatrix Datenbank (*.qmsql)"));

    // if the user does not accept the action, just cancel everything
    if (fileDialog.exec() != QFileDialog::Accepted)
    {
        return;
    }

    auto fileNames = fileDialog.selectedFiles();

    Q_ASSERT(fileNames.count() == 1);

    ui->leDatabaseFile->setText(fileNames.first());
}

void QMFavoriteDatabaseDialog::saveFavoriteEntry(const bool &validation)
{
    if (m_selectedFavorite == nullptr)
    {
        qWarning() << "should never happend";
        return;
    }

    if (validation)
    {
        if (!ui->leName->text().contains(QRegularExpression("^[a-zA-Z0-9\\s]+$")))
        {
            QMessageBox::information(this, tr("Favoriten speichern"),
                    tr("Der Name darf nur alphanumerische Zeichen und Leerzeichen enthalten."),
                    QMessageBox::Ok);
            return;
        }

        if (favoriteNameExist(ui->leName->text()))
        {
            QMessageBox::information(this, tr("Favoriten speichern"),
                    tr("Der Name existiert bereits, bitte wählen Sie einen anderen."),
                    QMessageBox::Ok);
            return;
        }
    }

    m_selectedFavorite->name = ui->leName->text();
    m_selectedFavorite->dbFilePath = ui->leDatabaseFile->text();

    writeFavoritesToSettings();

    updateUi();
}

bool QMFavoriteDatabaseDialog::favoriteNameExist(const QString &name) const
{
    for (const auto &m_favorite : m_favorites)
    {
        if (m_favorite->name.compare(name) == 0)
        {
            return true;
        }
    }

    return false;
}

void QMFavoriteDatabaseDialog::addFavoriteToWidget(QMFavoriteDatabaseEntry *entry)
{
    auto item = new QListWidgetItem(ui->lwFavorites, QListWidgetItem::Type);
    item->setData(Qt::DisplayRole, entry->name);
    item->setSizeHint(QSize(100, 30));

    ui->lwFavorites->addItem(item);
    ui->lwFavorites->setFocus();
    ui->lwFavorites->selectionModel()->select(
            ui->lwFavorites->indexFromItem(item),
            QItemSelectionModel::SelectionFlag::ClearAndSelect);
}
