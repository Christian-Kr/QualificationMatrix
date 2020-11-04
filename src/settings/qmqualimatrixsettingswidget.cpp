//
// qmqualimatrixsettingswidget.cpp is part of QualificationMatrix
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

#include "qmqualimatrixsettingswidget.h"
#include "ui_qmqualimatrixsettingswidget.h"
#include "qmapplicationsettings.h"

#include <QColorDialog>

QMQualiMatrixSettingsWidget::QMQualiMatrixSettingsWidget(QWidget *parent)
    : QMSettingsWidget(parent),
    ui(new Ui::QMQualiMatrixSettingsWidget)
{
    ui->setupUi(this);
}

QMQualiMatrixSettingsWidget::~QMQualiMatrixSettingsWidget()
{
    delete ui;
}

void QMQualiMatrixSettingsWidget::saveSettings()
{
    auto &settings = QMApplicationSettings::getInstance();

    settings.write("QualiMatrix/SelectionColor", ui->pbSelectionColor->text().remove("&"));
    settings.write("QualiMatrix/GridColor", ui->pbGridColor->text().remove("&"));
    settings.write("QualiMatrix/TrainBottomTop", ui->cbWriteTrainBottomToTop->isChecked());
    settings.write("QualiMatrix/VertHeaderWidth", ui->sbQMVertWidth->value());
    settings.write("QualiMatrix/HorHeaderHeight", ui->sbQMHorHeight->value());
}

void QMQualiMatrixSettingsWidget::revertChanges()
{
    loadSettings();
}

void QMQualiMatrixSettingsWidget::loadSettings()
{
    auto &settings = QMApplicationSettings::getInstance();

    auto selectionColor = settings.read("QualiMatrix/SelectionColor", "#ffffff").toString();
    ui->pbSelectionColor->setText(selectionColor);
    ui->pbSelectionColor->setStyleSheet("background-color: " + selectionColor + ";");

    auto gridColor = settings.read("QualiMatrix/GridColor", "#ffffff").toString();
    ui->pbGridColor->setText(gridColor);
    ui->pbGridColor->setStyleSheet("background-color: " + gridColor + ";");

    auto trainBottomTop = settings.read("QualiMatrix/TrainBottomTop", false).toBool();
    ui->cbWriteTrainBottomToTop->setChecked(trainBottomTop);

    auto qmVertWidth = settings.read("QualiMatrix/VertHeaderWidth", 200).toInt();
    ui->sbQMVertWidth->setValue(qmVertWidth);

    auto qmHorHeight = settings.read("QualiMatrix/HorHeaderHeight", 200).toInt();
    ui->sbQMHorHeight->setValue(qmHorHeight);
}

void QMQualiMatrixSettingsWidget::changeGridColor()
{
    auto color = QColorDialog::getColor(
        ui->pbGridColor->text().remove("&"), this, tr("Farbe auswählen"));

    if (!color.isValid())
    {
        return;
    }

    ui->pbGridColor->setText(color.name());
    ui->pbGridColor->setStyleSheet("background-color: " + color.name() + ";");

    emitSettingsChanged();
}

void QMQualiMatrixSettingsWidget::changeSelectionColor()
{
    auto color = QColorDialog::getColor(
        ui->pbSelectionColor->text().remove("&"), this, tr("Farbe auswählen"));

    if (!color.isValid())
    {
        return;
    }

    ui->pbSelectionColor->setText(color.name());
    ui->pbSelectionColor->setStyleSheet("background-color: " + color.name() + ";");

    emitSettingsChanged();
}
