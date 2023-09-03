// qmqualimatrixsettingswidget.cpp is part of QualificationMatrix
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

#include "qmqualiresultsettingswidget.h"
#include "ui_qmqualiresultsettingswidget.h"
#include "qmapplicationsettings.h"

#include <QColorDialog>

QMQualiResultSettingsWidget::QMQualiResultSettingsWidget(QWidget *parent)
    : QMSettingsWidget(parent, false),
    ui(new Ui::QMQualiResultSettingsWidget)
{
    ui->setupUi(this);
}

QMQualiResultSettingsWidget::~QMQualiResultSettingsWidget()
{
    delete ui;
}

void QMQualiResultSettingsWidget::saveSettings()
{
    auto &settings = QMApplicationSettings::getInstance();

    settings.write("QualiResult/OkColor", ui->pbOkColor->text().remove("&"));
    settings.write("QualiResult/BadColor", ui->pbBadColor->text().remove("&"));
    settings.write("QualiResult/MonthExpire", ui->sbMonthTrainExpire->value());
}

void QMQualiResultSettingsWidget::revertChanges()
{}

void QMQualiResultSettingsWidget::loadSettings()
{
    auto &settings = QMApplicationSettings::getInstance();

    auto okColor = settings.read("QualiResult/OkColor", "#ffffff").toString();
    ui->pbOkColor->setText(okColor);
    ui->pbOkColor->setStyleSheet("background-color: " + okColor + ";");

    auto badColor = settings.read("QualiResult/BadColor", "#ffffff").toString();
    ui->pbBadColor->setText(badColor);
    ui->pbBadColor->setStyleSheet("background-color: " + badColor + ";");

    ui->sbMonthTrainExpire->setValue(settings.read("QualiResult/MonthExpire", 6).toInt());
}

void QMQualiResultSettingsWidget::changeOkColor()
{
    auto color = QColorDialog::getColor(
        ui->pbOkColor->text().remove("&"), this, tr("Farbe auswählen"));

    if (!color.isValid())
    {
        return;
    }

    ui->pbOkColor->setText(color.name());
    ui->pbOkColor->setStyleSheet("background-color: " + color.name() + ";");
}

void QMQualiResultSettingsWidget::changeBadColor()
{
    auto color = QColorDialog::getColor(
        ui->pbBadColor->text().remove("&"), this, tr("Farbe auswählen"));

    if (!color.isValid())
    {
        return;
    }

    ui->pbBadColor->setText(color.name());
    ui->pbBadColor->setStyleSheet("background-color: " + color.name() + ";");
}
