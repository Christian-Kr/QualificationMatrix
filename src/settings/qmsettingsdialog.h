// qmsettings.h is part of QualificationMatrix
//
// QualificationMatrix is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by the
// Free Software Foundation, either version 3 of the License, or (at your
// option) any later version.
//
// QualificationMatrix is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
// or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
// more details.
//
// You should have received a copy of the GNU General Public License along
// with QualificationMatrix. If not, see <http://www.gnu.org/licenses/>.

#ifndef QMSETTINGSDIALOG_H
#define QMSETTINGSDIALOG_H

#include "framework/qmdialog.h"

QT_BEGIN_NAMESPACE
namespace Ui
{
class QMSettingsDialog;
}
QT_END_NAMESPACE

// Forward declarations
class QTreeWidgetItem;
class QMSettingsWidget;

/// Handling the settings dialog and all of its children.
/// \author Christian Kr, Copyright 2020
class QMSettingsDialog: public QMDialog
{
Q_OBJECT

public:
    /// Constrcutor
    /// \param parent The parent object for the qt system.
    explicit QMSettingsDialog(QWidget *parent = nullptr);

    /// Destructor
    ~QMSettingsDialog() override;

public slots:
    /// Dialog button "OK" has been pressed.
    void accept() override;

    /// Dialog button "Cancel" has been pressed.
    void reject() override;

    /// Dialog button "Apply" has been pressed.
    void apply();

    /// Data of a model has changed.
    /// \param topLeft not interested
    /// \param bottomRight not interested
    /// \param roles not interested
    void modelDataChanged(
        const QModelIndex &topLeft, const QModelIndex &bottomRight,
        const QVector<int> &roles = QVector<int>());

    /// Change the visible widget to the one that represents the settings group.
    /// \param item The item that has been choosed.
    void changeSettingsGroup(QTreeWidgetItem *item, int);

    /// Reset all settings - for model settings just revert changes.
    /// \param group The group id to reset settings for.
    /// \return True if success, else false.
    bool resetSettingsGroup(int group);

    /// Every setting widget item should connect to this slot to make sure the
    /// system detects a change in the settings.
    void settingsChanged();

    /// Show information about restart needed.
    void restartNeededInformation();

protected:
    /// Override from QDialog.
    void closeEvent(QCloseEvent *event) override;

    /// Override from QDialog.
    void showEvent(QShowEvent *event) override;

private:
    /// Append and conenct a settings widget to the stacked widget.
    /// \param settingsWidget The new settings widget to be connected and
    /// appended.
    void appendConnectSettingsWidget(QMSettingsWidget *settingsWidget);

    /// Create all custom widgets, add and connect them.
    void initStackWidgets();

    /// Will be called after recieving the show event of the dialog.
    void windowLoaded();

    /// Override from QMDialog.
    void saveSettings() override;

    /// Load all settings into the widgets and control elements in one step.
    void loadSettings();

    /// Save the settings for the given group.
    /// \param group The index number of the group to save.
    /// \return True if success, else false.
    bool saveSettingsGroup(int group);

     /// Save the database settings group.
     /// \param group The group name.
     /// \return True if success, else false.
    bool saveSettingsGroupDatabase(int group);

    /// Init the settings in the tree widget.
    void initTreeWidgets();

    Ui::QMSettingsDialog *ui;

    bool changed;
    int currentGroup;
    int lastGroup;
};

#endif // QMSETTINGSDIALOG_H
