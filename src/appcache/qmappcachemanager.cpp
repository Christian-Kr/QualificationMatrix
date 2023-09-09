// qmappcachemanager.cpp is part of QualificationMatrix
//
// QualificationMatrix is free software: you can redistribute it and/or modify it under the terms
// of the GNU General Public License as published by the Free Software Foundation, either version
// 3 of the License, or (at your option) any later version.
//
// QualificationMatrix is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
// without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See
// the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with QualificationMatrix.
// If not, see <http://www.gnu.org/licenses/>.

#include "qmappcachemanager.h"
#include "settings/qmapplicationsettings.h"
#include "config.h"

#include <QDir>
#include <QDateTime>
#include <QCryptographicHash>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

QMAppCacheManager *QMAppCacheManager::instance = nullptr;

QMAppCacheManager::QMAppCacheManager()
    : QObject()
    , m_metaInfo(std::make_unique<QMAppCacheMetaInfo>())
    , m_sourcePath(std::make_unique<QString>())
    , m_targetPath(std::make_unique<QString>())
{}

QMAppCacheManager::~QMAppCacheManager() = default;

bool QMAppCacheManager::trigger()
{
   // todo: start process
}

bool QMAppCacheManager::readSettings()
{
    auto &settings = QMApplicationSettings::getInstance();

    // Read the source app cache folder.
    auto tmpBackupFolder = settings.read("Backup/Folder", "backup").toString();
}

bool QMAppCacheManager::readMetaInfo(QMAppCacheMetaInfo &appCacheMetaInfo)
{
    // Get the file from the given backupMetaInfo and read in all information from the file.
}
