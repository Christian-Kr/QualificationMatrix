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
#include "settings/qmapplicationsettings.h"
#include "favorites/qmfavoritemodel.h"

#include <QFileDialog>
#include <QMessageBox>

QMFavoriteDatabaseDialog::QMFavoriteDatabaseDialog(QWidget *parent)
    : QDialog(parent)
    , m_favoriteModel(new QMFavoriteModel(this))
{
    m_ui = new Ui::QMFavoriteDatabaseDialog;
    m_ui->setupUi(this);

    // favorite table view
    m_favoriteModel->readFavoritesFromSettings();

    m_ui->tvFavorites->setModel(m_favoriteModel);
    m_ui->tvFavorites->resizeColumnsToContents();
    m_ui->tvFavorites->setItemDelegateForColumn(1, nullptr);

    connect(m_ui->tvFavorites, &QTableView::doubleClicked, this,
            &QMFavoriteDatabaseDialog::favoriteDoubleClicked);
}

QMFavoriteDatabaseDialog::~QMFavoriteDatabaseDialog()
{
    delete m_ui;
}

void QMFavoriteDatabaseDialog::favoriteDoubleClicked(const QModelIndex &index)
{
    // only handle column for database file path
    if (index.column() != 1)
    {
        return;
    }

    // open a database file
    QFileDialog fileDialog(this);

    fileDialog.setWindowTitle(QObject::tr("Datenbank öffnen"));
    fileDialog.setFileMode(QFileDialog::FileMode::ExistingFile);
    fileDialog.setAcceptMode(QFileDialog::AcceptMode::AcceptOpen);
    fileDialog.setNameFilter(QObject::tr("QualificationMatrix Datenbank (*.qmsql)"));

    // get current path name if available
    auto variFileName = index.data(Qt::DisplayRole);
    if (variFileName.canConvert<QString>())
    {
        auto currFileName = index.data(Qt::DisplayRole).toString();

        if (QFile::exists(currFileName) && currFileName.split(".").last().compare("qmsql") == 0)
        {
            fileDialog.selectFile(currFileName);
        }
    }

    // if the user does not accept the action, just cancel everything
    if (fileDialog.exec() != QFileDialog::Accepted)
    {
        return;
    }

    auto fileNames = fileDialog.selectedFiles();

    Q_ASSERT(fileNames.count() == 1);

    m_favoriteModel->setData(index, fileNames.first(), Qt::EditRole);
}

void QMFavoriteDatabaseDialog::reject()
{
    QDialog::reject();
}

void QMFavoriteDatabaseDialog::accept()
{
    auto rowsIndex = m_ui->tvFavorites->selectionModel()->selectedRows();
    if (rowsIndex.size() != 1)
    {
        return;
    }

    auto row = rowsIndex.first().row();

    auto index = m_favoriteModel->index(row, 1);
    if (!index.isValid())
    {
        qWarning() << "no valid index selected";
        return;
    }

    auto fileName = index.data(Qt::DisplayRole).toString();

    if (!QFile::exists(fileName) || fileName.split(".").last().compare("qmsql") != 0)
    {
        QMessageBox::warning(
                this, tr("Favoriten öffnen"),
                tr("Der Favorit kann nicht geöffnet werden oder die Dateiendung ist falsch"),
                QMessageBox::StandardButton::Ok);

        qWarning() << "cannot open because file does not exist or has wrong extension";
        return;
    }

    QDialog::accept();

    emit openDatabase(fileName);
}

void QMFavoriteDatabaseDialog::removeFavoriteEntry()
{
    auto rowsIndex = m_ui->tvFavorites->selectionModel()->selectedRows();
    if (rowsIndex.size() != 1)
    {
        return;
    }

    auto row = rowsIndex.first().row();

    if (!m_favoriteModel->removeRecord(row))
    {
        qWarning() << "remove row should never fail";
        return;
    }
}

void QMFavoriteDatabaseDialog::createFavoriteEntry()
{
    m_favoriteModel->addDefaultRecord();
}
