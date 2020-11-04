//
// importcsvdialog.h is part of QualificationMatrix
//
// QualificationMatrix is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// QualificationMatrix is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with QualificationMatrix.  If not, see <http://www.gnu.org/licenses/>.
//

#ifndef IMPORTCSVDIALOG_H
#define IMPORTCSVDIALOG_H

#include <QDialog>

#include <memory>

QT_BEGIN_NAMESPACE
namespace Ui
{
class ImportCsvDialog;
}
QT_END_NAMESPACE

// Forward delcaration for faster compiling.
class QFile;

class QSqlTableModel;

class QSqlRelationalTableModel;

/**
 * @brief Dialog and business logic for import data from csv file.
 *
 * In future the business logic should be implemented inside the QMDataManager.
 *
 * @author Christian Kr, Copyright (c) 2020
 */
class ImportCsvDialog: public QDialog
{
Q_OBJECT

public:
    /**
     * @brief Constructor
     * @param parent the main parent object
     */
    explicit ImportCsvDialog(QWidget *parent = nullptr);

    ~ImportCsvDialog() override;

public slots:

    /**
     * @brief Open the file path from a open dialog window.
     */
    void openImportFile();

    /**
     * @brief Open the file path from a open dialog window.
     */
    void openBackupFile();

    /**
     * @brief Start the process of importing.
     */
    virtual void accept() override;

    /**
     * @brief Update model data.
     */
    void updateData();

private:
    /**
     * @brief Parse the csv file and import data.
     * @param importFile The file object which should already be opened.
     */
    void parseCsv(QFile &importFile);

    /**
     * @brief Handles a string line from file, when an error occured.
     */
    void handleErrorLine(QString &line);

    /**
     * @brief Check the backup file for existence and writablility.
     * @return True if everything ok, else false.
     */
    bool checkBackupFile();

    Ui::ImportCsvDialog *ui;

    std::shared_ptr<QSqlRelationalTableModel> trainModel;
    std::shared_ptr<QSqlRelationalTableModel> employeeModel;
    std::shared_ptr<QSqlTableModel> shiftModel;
    std::shared_ptr<QSqlTableModel> trainGroupModel;
    std::shared_ptr<QSqlTableModel> trainDataStateModel;
    std::shared_ptr<QSqlRelationalTableModel> trainDataModel;
};

#endif // IMPORTCSVDIALOG_H
