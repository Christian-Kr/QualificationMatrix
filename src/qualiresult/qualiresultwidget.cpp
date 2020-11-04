//
// qualiresultwidget.cpp is part of QualificationMatrix
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

#include "qualiresultwidget.h"
#include "ui_qualiresultwidget.h"
#include "model/qmdatamanager.h"
#include "qualiresultmodel.h"
#include "settings/qmapplicationsettings.h"

#include <QSortFilterProxyModel>
#include <QSqlRelationalTableModel>
#include <QPrinter>
#include <QPrintPreviewDialog>
#include <QTextTable>
#include <QTextDocument>
#include <QTextCursor>
#include <QPainter>
#include <QFileDialog>
#include <QDir>
#include <QDebug>
#include <QMessageBox>
#include <QTextStream>
#include <QProgressDialog>

QualiResultWidget::QualiResultWidget(QWidget *parent)
    : QWidget(parent), ui(new Ui::QualiResultWidget), qualiResultModel(nullptr), funcModel(nullptr),
      trainModel(nullptr), employeeModel(nullptr), employeeGroupModel(nullptr),
      trainDataStateModel(nullptr), qualiResultFilterTRState(new QSortFilterProxyModel(this)),
      qualiResultFilterTState(new QSortFilterProxyModel(this))
{
    ui->setupUi(this);

    // Initializing settings for table view.
    ui->tvQualiResult->horizontalHeader()->setStretchLastSection(false);
    ui->tvQualiResult->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->tvQualiResult->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
    ui->tvQualiResult->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tvQualiResult->setEnabled(true);
    ui->tvQualiResult->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->cbTrainResultState->setCurrentText("");

    loadSettings();
}

QualiResultWidget::~QualiResultWidget()
{
    delete ui;
}

void QualiResultWidget::loadSettings()
{
    QMApplicationSettings &settings = QMApplicationSettings::getInstance();
    ui->dwFilter->setVisible(settings.read("QualiResult/ShowFilter", true).toBool());
}

void QualiResultWidget::updateData()
{
    // Get the model data.
    auto dm = QMDataManager::getInstance();
    qualiResultModel = dm->getQualiResultModel();
    funcModel = dm->getFuncModel();
    trainModel = dm->getTrainModel();
    trainDataStateModel = dm->getTrainDataStateModel();
    employeeModel = dm->getEmployeeModel();
    employeeGroupModel = dm->getShiftModel();

    qualiResultFilterTRState->setSourceModel(qualiResultModel.get());
    qualiResultFilterTRState->setFilterKeyColumn(6);

    qualiResultFilterTState->setSourceModel(qualiResultFilterTRState);
    qualiResultFilterTState->setFilterKeyColumn(7);

    // Update the views.
    ui->tvQualiResult->setModel(qualiResultFilterTState);

    // Update all table views with the new models.
    ui->cbFilterFunc->setModel(funcModel.get());
    ui->cbFilterFunc->setModelColumn(1);

    ui->cbFilterTrain->setModel(trainModel.get());
    ui->cbFilterTrain->setModelColumn(1);

    ui->cbFilterEmployeeGroup->setModel(employeeGroupModel.get());
    ui->cbFilterEmployeeGroup->setModelColumn(1);

    ui->cbFilterEmployee->setModel(employeeModel.get());
    ui->cbFilterEmployee->setModelColumn(1);

    ui->cbTrainStateFilter->setModel(trainDataStateModel.get());
    ui->cbTrainStateFilter->setModelColumn(1);

    resetFilter();

    // Update quali result model data.
    qualiResultModel->updateModels();
}

void QualiResultWidget::resetFilter()
{
    ui->cbFilterFunc->setCurrentText("");
    ui->cbFilterTrain->setCurrentText("");
    ui->cbFilterEmployee->setCurrentText("");
    ui->cbFilterEmployeeGroup->setCurrentText("");
    ui->cbTrainStateFilter->setCurrentText("");
}

void QualiResultWidget::resetModel()
{
    qualiResultModel->resetModel();
}

void QualiResultWidget::updateFilterAndCalculate()
{
    if (qualiResultModel == nullptr) {
        return;
    }

    QString tmpFunc = ui->cbFilterFunc->currentText();
    QString tmpTrain = ui->cbFilterTrain->currentText();
    QString tmpEmployee = ui->cbFilterEmployee->currentText();
    QString tmpEmployeeGroup = ui->cbFilterEmployeeGroup->currentText();

    qualiResultModel->updateQualiInfo(tmpEmployee, tmpFunc, tmpTrain, tmpEmployeeGroup);
    qualiResultFilterTRState->setFilterRegExp(ui->cbTrainResultState->currentText());
    qualiResultFilterTState->setFilterRegExp(ui->cbTrainStateFilter->currentText());
    ui->tvQualiResult->resizeColumnsToContents();
}

void QualiResultWidget::switchFilterVisibility()
{
    ui->dwFilter->setVisible(!ui->dwFilter->isVisible());
}

void QualiResultWidget::filterVisibilityChanged()
{
    if (ui->dwFilter->isVisible()) {
        ui->tbFilterVisible->setVisible(false);
    }
    else {
        ui->tbFilterVisible->setVisible(true);
    }

    QMApplicationSettings &settings = QMApplicationSettings::getInstance();
    settings.write("QualiResult/ShowFilter", ui->dwFilter->isVisible());
}

void QualiResultWidget::saveToCsv()
{
    // Ask where to save the csv file.
    QString fileName = QFileDialog::getSaveFileName(
        this, tr("Qualifizierungsresultat speichern"), QDir::homePath(),
        tr("Comma-separated values (*.csv)"));
    if (fileName.isEmpty()) {
        return;
    }

    // Open file for writing and write the whole content to the file.
    QFile csvFile(fileName);

    if (!csvFile.open(QFile::ReadWrite)) {
        QMessageBox::critical(
            this, tr("Qualifizierungsresultat speichern"), tr(
                "Datei konnte nicht zum Schreiben geöffnet werden. Bitte prüfen Sie die"
                " Berechtigungen."
                "\n\nAktion wird abgebrochen."
            ));
        return;
    }

    QTextStream csvStream(&csvFile);

    int colCount = qualiResultFilterTState->columnCount();
    int rowCount = qualiResultFilterTState->rowCount();

    // Write header.
    for (int i = 0; i < colCount; i++) {
        csvStream << qualiResultFilterTState->headerData(i, Qt::Horizontal).toString();
        if (i < colCount - 1) {
            csvStream << ";";
        }
        else {
            csvStream << "\n";
        }
    }

    // Write data.
    for (int i = 0; i < rowCount; i++) {
        for (int j = 0; j < colCount; j++) {
            csvStream
                << qualiResultFilterTState->data(qualiResultFilterTState->index(i, j)).toString();
            if (j < colCount - 1) {
                csvStream << ";";
            }
            else {
                csvStream << "\n";
            }
        }
    }

    csvFile.close();

    QMessageBox::information(
        this, tr("Qualifizierungsresultat speichern"),
        tr("Die Daten wurden erfolgreich gespeichert."));
}

void QualiResultWidget::printToPDF()
{
    // Set up default printer.
    QPrinter *printer = new QPrinter();
    printer->setPageOrientation(QPageLayout::Orientation::Landscape);

    // Call printer dialog.
    QPrintPreviewDialog previewDialog(printer, this);
    connect(
        &previewDialog, &QPrintPreviewDialog::paintRequested, this,
        &QualiResultWidget::paintPdfRequest
    );
    previewDialog.exec();
}

void QualiResultWidget::paintPdfRequest(QPrinter *printer)
{
    // TODO: Support multiple pages when table is too long.
    QTextDocument document;
    QTextCursor cursor(&document);
    QSortFilterProxyModel *model = qualiResultFilterTState;

    // Set default font.
    QFont font;
    font.setPointSize(8);
    document.setDefaultFont(font);

    // Create new table object and format it.
    QTextTable *table = cursor.insertTable(model->rowCount() + 1, model->columnCount() + 1);
    QTextTableFormat tableFormat = table->format();
    tableFormat.setBorderCollapse(true); // Is not available in lower qt versions.
    tableFormat.setCellPadding(2);
    tableFormat.setHeaderRowCount(1);
    table->setFormat(tableFormat);

    // Go through all header cells and set them.
    for (int i = 0; i < model->columnCount() + 1; i++) {
        QTextTableCell cell = table->cellAt(0, i);

        // Set background of header.
        QTextCharFormat format = cell.format();
        format.setBackground(QColor("#d9d9d9"));
        cell.setFormat(format);

        if (i > 0) {
            cursor.insertText(model->headerData(i - 1, Qt::Horizontal).toString());
        }

        cursor.movePosition(QTextCursor::NextCell);
    }

    // Put data of the model into the table and style them.
    for (int i = 0; i < model->rowCount(); i++) {
        for (int j = 0; j < model->columnCount() + 1; j++) {
            QTextTableCell cell = table->cellAt(i + 1, j);

            // Set styles. The background of every second row should be light grey.
            QTextCharFormat format = cell.format();

            if ((i + 1) % 2 == 0) {
                format.setBackground(QColor("#f2f2f2"));
            }

            cell.setFormat(format);

            // First number column.
            if (j == 0) {
                cursor.insertText(QString().number(i + 1));
            }
            else {
                cursor.insertText(model->data(model->index(i, j - 1)).toString());
            }
            cursor.movePosition(QTextCursor::NextCell);
        }
    }

    // Default printer settings.
    document.print(printer);
}
