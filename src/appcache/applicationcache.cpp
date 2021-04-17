// applicationcache.cpp is part of QualificationMatrix
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

#include "applicationcache.h"

#include <QString>
#include <QFileInfo>
#include <QDir>

ApplicationCache::ApplicationCache(QObject *parent)
    : QObject(parent)
    , sourcePath(std::make_unique<QString>())
    , targetPath(std::make_unique<QString>())
{}

bool ApplicationCache::updateCache()
{
    if (sourcePath == nullptr || targetPath == nullptr)
    {
        return false;
    }

    QDir sourceDir = QDir(*sourcePath);
    QDir targetDir = QDir(*targetPath);

    if (!sourceDir.exists() || !targetDir.exists())
    {
        return false;
    }

    // TODO: Complete update the cache. Therefore a file format needs to be created, which includes the files that need
    // to be copied to the new place.
}
