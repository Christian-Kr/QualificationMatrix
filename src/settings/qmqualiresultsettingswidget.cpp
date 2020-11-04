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

#include "qmqualiresultsettingswidget.h"
#include "ui_qmqualiresultsettingswidget.h"
#include "qmapplicationsettings.h"

#include <QMessageBox>
#include <QColorDialog>

QMQualiResultSettingsWidget::QMQualiResultSettingsWidget(QWidget *parent)
    : QMSettingsWidget(parent),
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
    QStringList ignoreList;

    settings.write("QualiResult/DoIgnore", ui->cbQualiResIgnore->isChecked());

    for (int i = 0; i < ui->lwIgnoreList->count(); i++) {
        ignoreList.append(ui->lwIgnoreList->item(i)->data(Qt::DisplayRole).toString());
    }

    settings.write("QualiResult/IgnoreList", ignoreList);
    settings.write("QualiResult/OkColor", ui->pbOkColor->text().remove("&"));
    settings.write("QualiResult/BadColor", ui->pbBadColor->text().remove("&"));
    settings.write("QualiResult/EnoughColor", ui->pbEnoughColor->text().remove("&"));
}

void QMQualiResultSettingsWidget::revertChanges()
{}

void QMQualiResultSettingsWidget::loadSettings()
{
    auto &settings = QMApplicationSettings::getInstance();

    ui->cbQualiResIgnore->setChecked(settings.read("QualiResult/DoIgnore", true).toBool());

    ui->lwIgnoreList->clear();
    ui->lwIgnoreList->addItems(settings.read("QualiResult/IgnoreList", "").toStringList());

    auto okColor = settings.read("QualiResult/OkColor", "#ffffff").toString();
    ui->pbOkColor->setText(okColor);
    ui->pbOkColor->setStyleSheet("background-color: " + okColor + ";");

    auto badColor = settings.read("QualiResult/BadColor", "#ffffff").toString();
    ui->pbBadColor->setText(badColor);
    ui->pbBadColor->setStyleSheet("background-color: " + badColor + ";");

    auto enoughColor = settings.read("QualiResult/EnoughColor", "#ffffff").toString();
    ui->pbEnoughColor->setText(enoughColor);
    ui->pbEnoughColor->setStyleSheet("background-color: " + enoughColor + ";");
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

    emitSettingsChanged();
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

    emitSettingsChanged();
}

void QMQualiResultSettingsWidget::changeEnoughColor()
{
    auto color = QColorDialog::getColor(
        ui->pbEnoughColor->text().remove("&"), this, tr("Farbe auswählen"));

    if (!color.isValid())
    {
        return;
    }

    ui->pbEnoughColor->setText(color.name());
    ui->pbEnoughColor->setStyleSheet("background-color: " + color.name() + ";");

    emitSettingsChanged();
}

void QMQualiResultSettingsWidget::addIgnore()
{
    // Add nothing if the text is empty.
    if (ui->leIgnoreText->text().isEmpty())
    {
        QMessageBox::information(
            this, tr("Eintrag hinzufügen"),
            tr("Es ist kein Text eingetragen worde, der hinzugefügt werden könnte."
               "\n\nDie Aktion wird abgebrochen."));
        return;
    }

    ui->lwIgnoreList->addItem(ui->leIgnoreText->text());
}

void QMQualiResultSettingsWidget::removeIgnore()
{
    // Remove will only work, if any item has been selected.
    auto selectedItems = ui->lwIgnoreList->selectedItems();

    if (selectedItems.size() != 1)
    {
        QMessageBox::information(
            this, tr("Eintrag löschen"),
            tr("Es ist kein Eintrag selektiert, der gelöscht werden könnte."
               "\n\nDie Aktion wird abgebrochen."));
        return;
    }

    ui->lwIgnoreList->removeItemWidget(selectedItems.first());
    delete selectedItems.first();
}
