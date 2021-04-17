// appcacheconfig.h is part of QualificationMatrix
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

#ifndef QUALIFICATIONMATRIX_APPCACHECONFIG_H
#define QUALIFICATIONMATRIX_APPCACHECONFIG_H

#include <QObject>

class AppCacheConfig: public QObject
{
    Q_OBJECT

public:
    /// Constructor
    /// \param parent
    explicit AppCacheConfig(QObject *parent = nullptr);

    /// Parse file.
    /// \param fileName The file to open and parse.
    /// \return False if something went wrong, else true.
    virtual bool parse(QString fileName) = 0;

    /// Get a list of all files relative to the source path that should be copied to the local cache folder.
    /// \return QList of all files.
    virtual QList<QString> getCacheFiles() = 0;

    /// Get the version of the software release, to know if a new version is available and should be cached.
    /// \return The version of the release.
    virtual QString version() = 0;
};


#endif //QUALIFICATIONMATRIX_APPCACHECONFIG_H
