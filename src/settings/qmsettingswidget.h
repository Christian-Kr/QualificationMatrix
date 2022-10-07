// qmsettingswidget.cpp is part of QualificationMatrix
//
// QualificationMatrix is free software: you can redistribute it and/or modify it under the terms of the GNU General
// Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option)
// any later version.
//
// QualificationMatrix is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the
// implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
// details.
//
// You should have received a copy of the GNU General Public License along with QualificationMatrix.
// If not, see <http://www.gnu.org/licenses/>.

#ifndef QMSETTINGSWIDGET_H
#define QMSETTINGSWIDGET_H

#include <QWidget>

class QMSettingsWidget: public QWidget
{
Q_OBJECT

public:
    /// Constructor
    /// \param parent The parent object for the qt system.
    /// \param adminAcces True if administrator should be logged in to access this dialog, else false.
    explicit QMSettingsWidget(QWidget *parent = nullptr, bool adminAccess = false);

    // Getter for admin rights.
    bool adminAccessNeeded() const { return admin; }

signals:
    /// A setting in the dialog changed (not in QMApplicationSettings).
    void settingsChanged();

    void settingsApplied();

public slots:
    void emitSettingsChanged();

    /// All settings have to be saved in data.
    virtual void saveSettings() = 0;

    /// Reset settings to data.
    virtual void revertChanges() = 0;

    /// Load settings from appropriate source. (Might be a database or qsettings.)
    virtual void loadSettings() = 0;

protected:
    /// Override from QWidget.
    void showEvent(QShowEvent *event) override;

private:

    // The object itself holds the property of needing admin access. Other parts of the application need to take care
    // of being logged in as an administrator. This dialog will only permit the show the content, if no admin has been
    // logged in.
    bool admin;
};

#endif // QMSETTINGSWIDGET_H
