//
// qmsettings.h is part of QualificationMatrix
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

#ifndef QMSETTINGS_H
#define QMSETTINGS_H

#include <QObject>

/// Parent class for settings inside this application. QualificationMatrix needs his own settings
/// implementation. There need to be a separation between settings in Application manner and
/// Database. Both support normal setting situations, except of the source they might read/write
/// in/from.
///
/// This parent class is a generalised object which implements the basic functionality both derived
/// settings objects should implement.
class QMSettings: public QObject
{
Q_OBJECT

public:
    /// Constructor
    /// \param parent The parent object for the qt system.
    explicit QMSettings(QObject *parent = nullptr);

    /// Read a settings value.
    /// \param name The name of the settings.
    /// \param default The default value if no one exist in settings.
    /// \return The value. If initilized with nullptr (isNull()) then the setting does not exist.
    virtual QVariant read(const QString &name) = 0;

    virtual QVariant read(const QString &name, const QVariant &defValue) = 0;

    /// Write a setting value.
    /// \param name The name of the setting.
    /// \param value The value of the setting.
    virtual void write(const QString &name, const QVariant &value) = 0;
};

#endif // QMSETTINGS_H
