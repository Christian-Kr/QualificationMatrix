// qmamsgroupsettingswidget.h is part of QualificationMatrix
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

#ifndef QMAMSGROUPSETTINGSWIDGET_H
#define QMAMSGROUPSETTINGSWIDGET_H

#include "settings/qmsettingswidget.h"

class QMAMSGroupModel;
class QMAMSAccessModeModel;
class QMAMSGroupAccessModeModel;
class QSortFilterProxyModel;

namespace Ui
{
    class QMAMSGroupSettingsWidget;
}

/// Settings widget for account management system.
/// \author Christian Kr, Copyright 2020
class QMAMSGroupSettingsWidget: public QMSettingsWidget
{
    Q_OBJECT

public:
    /// Constructor
    /// \param parent The parent object for the qt system.
    explicit QMAMSGroupSettingsWidget(QWidget *parent = nullptr);

    /// Destructor
    ~QMAMSGroupSettingsWidget() override;

    /// This slot has to be implemented by parent class. When calling this
    /// function, it should take a QSettings object or data Model
    /// (bspw. SqlModel) to save every changes to a certificate based system.
    void saveSettings() override;

    /// Override from QMSettingsWidget.
    void revertChanges() override;

    /// Override from QMSettingsWidget.
    void loadSettings() override;

public slots:
    /// Add a new group.
    void addGroup();

    /// Add a new access mode.
    void addAccessMode();

    /// Remove a group.
    void removeGroup();

    /// Remove an access mode.
    void removeAccessMode();

    /// Group selection changed.
    /// \param deselected Item that has been deselected.
    /// \param selected Item that has been selected.
    void groupSelectionChanged(const QModelIndex &selected,
            const QModelIndex &deselected);

    /// Access mode selection changed.
    /// \param deselected Item that has been deselected.
    /// \param selected Item that has been selected.
    void accessModeSelectionChanged(const QModelIndex &selected,
            const QModelIndex &deselected);

    /// Group access mode correlation table selection changed.
    /// \param deselected Item that has been deselected.
    /// \param selected Item that has been selected.
    void groupAccessModeSelectionChanged(const QModelIndex &selected,
            const QModelIndex &deselected);

private:
    /// Update model data.
    void updateData();

    /// Run action for enabling the usergroup list.
    /// \param selRow Selected row.
    void activateGroupAccessModeList(int selRow);

    /// Run action for disabling the usergroup list.
    void deactivateGroupAccessModeList();

    /// Search for an access mode in user group proxy model.
    /// \param name The name of the group to search for.
    /// \return True if minimum one exist, else false.
    bool groupAccessModeProxyContainsGroup(const QString &accessMode);

    Ui::QMAMSGroupSettingsWidget *ui;

    std::unique_ptr<QMAMSAccessModeModel> amsAccessModeModel;
    std::unique_ptr<QMAMSGroupModel> amsGroupModel;
    std::unique_ptr<QMAMSGroupAccessModeModel> amsGroupAccessModeModel;

    std::unique_ptr<QSortFilterProxyModel> amsGroupAccessModeProxyModel;
};

#endif // QMAMSGROUPSETTINGSWIDGET_H
