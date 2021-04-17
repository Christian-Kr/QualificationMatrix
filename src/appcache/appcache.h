// applcache.h is part of QualificationMatrix
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

#ifndef QUALIFICATIONMATRIX_APPCACHE_H
#define QUALIFICATIONMATRIX_APPCACHE_H

#include <QObject>
#include <memory>

/// Create a local cache of the application. This will increase performance of application usage.
/// \author Christian Kr, Copyright 2021
class AppCache: public QObject
{
    Q_OBJECT

public:
    /// Constructor and destructor
    /// \param parent
    explicit AppCache(QObject *parent = nullptr);

    /// Run an update of the application cache from the source to the target.
    /// \return True if update was successful, else false.
    bool updateCache();

    /// Get the source path.
    /// \return Source path as string.
    QString getSourcePath() { return *sourcePath; }

    /// Set the source path.
    /// \param path The source path to set.
    void setSourcePath(QString path) { *sourcePath = path; }

    /// Get the target path.
    /// \return Target path as string.
    QString getTargetPath() { return *targetPath; }

    /// Set the target path.
    /// \param path The source path to set.
    void setTargetPath(QString path) { *targetPath = path; }

private:
    std::unique_ptr<QString> sourcePath;
    std::unique_ptr<QString> targetPath;
};


#endif // QUALIFICATIONMATRIX_APPCACHE_H
