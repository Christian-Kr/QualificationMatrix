// qmamsloginwidget.h is part of QualificationMatrix
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

#ifndef QMAMSLOGINWIDGET_H
#define QMAMSLOGINWIDGET_H

#include "settings/qmsettingswidget.h"

namespace Ui
{
    class QMAMSLoginWidget;
}

/// Login widget for account management system.
/// \author Christian Kr, Copyright 2020
class QMAMSLoginWidget: public QMSettingsWidget
{
    Q_OBJECT

public:
    /// Constructor
    /// \param parent The parent object for the qt system.
    explicit QMAMSLoginWidget(QWidget *parent = nullptr);

    /// Destructor
    ~QMAMSLoginWidget() override;

    /// Set the user name to log in. If user is set, it is not changeable by user. To reset, recreate the widget.
    /// \param username The username to log in
    void setUsername(QString name);

public slots:
    /// Login width credentials clicked.
    void login();

    /// Cancel login process.
    void cancel();

private:
    Ui::QMAMSLoginWidget *ui;
};

#endif // QMAMSLOGINWIDGET_H
