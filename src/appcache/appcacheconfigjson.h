// appcacheconfigjson.h is part of QualificationMatrix
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

#ifndef QUALIFICATIONMATRIX_APPCACHECONFIGJSON_H
#define QUALIFICATIONMATRIX_APPCACHECONFIGJSON_H

#include "appcacheconfig.h"

/// Add support for configurations written in json format.
/// \author Christian Kr, Copyright 2021
class AppCacheConfigJSON: public AppCacheConfig
{
    Q_OBJECT

public:
    /// Constructor
    /// \param parent
    explicit AppCacheConfigJSON(QObject *parent = nullptr);

    /// Overrides from AppCacheConfig.
    /// \return
    QString getName() override { return "JSON"; }

    /// Overrides from AppCacheConfig.
    /// \param fileName
    /// \return
    bool parse(QString fileName) override;

    /// Overrides from AppCacheConfig.
    /// \return
    QList<QString> getCacheFiles() override;

    /// Overrides from AppCacheConfig.
    /// \return
    QString version() override;
};

#endif //QUALIFICATIONMATRIX_APPCACHECONFIGJSON_H
