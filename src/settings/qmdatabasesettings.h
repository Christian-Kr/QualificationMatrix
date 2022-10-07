// qmdatabasesettings.h is part of QualificationMatrix
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

#ifndef QMDATABASESETTINGS_H
#define QMDATABASESETTINGS_H

#include "qmsettings.h"

#include <memory>

class QMInfoModel;

/// \brief only a wrapper to data from an sql table data
class QMDatabaseSettings: public QMSettings
{
Q_OBJECT

public:
    explicit QMDatabaseSettings(std::shared_ptr<QMInfoModel> model, QObject *parent = nullptr);

    /// \brief override of QMSettings::read
    virtual QVariant read(const QString &name) override;

    virtual QVariant read(const QString &name, const QVariant &defValue) override;

    /// \brief override of QMSettings::write
    virtual void write(const QString &name, const QVariant &value) override;

private:
    /// \return -1 if name could not be found
    int getNameRow(const QString &name);

    std::shared_ptr<QMInfoModel> infoModel;
};

#endif // QMDATABASESETTINGS_H
