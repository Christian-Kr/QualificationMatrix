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
    , m_metaInfoSource(std::make_unique<QMAppCacheMetaInfo>())
    , m_metaInfoTarget(std::make_unique<QMAppCacheMetaInfo>())
    , m_errorType(AppCacheError::NONE)
{}

QMAppCacheManager::~QMAppCacheManager() = default;

bool QMAppCacheManager::trigger()
{
    readSettings();

    // check that both paths exist and are directories
    auto sourceFileInfo = QFileInfo(m_sourcePath);

    if (!sourceFileInfo.exists())
    {
        m_errorType = AppCacheError::SOURCE_NOT_EXIST;
        return false;
    }

    if (!sourceFileInfo.isDir())
    {
        m_errorType = AppCacheError::SOURCE_NOT_DIR;
        return false;
    }

    auto targetFileInfo = QFileInfo(m_targetPath);

    if (!targetFileInfo.exists())
    {
        m_errorType = AppCacheError::TARGET_NOT_EXIST;
        return false;
    }

    if (!targetFileInfo.isDir())
    {
        m_errorType = AppCacheError::TARGET_NOT_DIR;
        return false;
    }

    // read in the meta file from the source
    if (!readMetaInfo(m_metaInfoSource, m_sourcePath))
    {
        return false;
    }
}

bool QMAppCacheManager::readSettings()
{
    auto &settings = QMApplicationSettings::getInstance();

    m_sourcePath = settings.read("AppCache/SourcePath", "").toString();
    m_targetPath = settings.read("AppCache/TargetPath", "").toString();
}

bool QMAppCacheManager::readMetaInfo(std::unique_ptr<QMAppCacheMetaInfo> &metaInfo,
        const QString &path)
{
    auto metaFile = QFile(path + QDir::separator() + APPCACHE_META);

    if (!metaFile.exists())
    {
        m_errorType = AppCacheError::SOURCE_META_NOT_EXIST;
        return false;
    }

    // start to read meta information
    if (!metaFile.open(QFile::ReadOnly))
    {
        m_errorType = AppCacheError::SOURCE_META_NOT_READABLE;
        return false;
    }

    // open the file and read in all byte data
    auto metaFileByteArray = metaFile.readAll();
    metaFile.close();

    // parse the file content
    auto *parseError = new QJsonParseError();
    QJsonDocument jsonDoc = QJsonDocument::fromJson(metaFileByteArray, parseError);

    // if result is null, then parsing might have failed
    if (jsonDoc.isNull())
    {
        m_errorType = AppCacheError::SOURCE_META_PARSE_FAILED;
        return false;
    }

    // Get general information.
    QJsonObject jsonRootObject = jsonDoc.object();

    if (jsonRootObject.contains("Version"))
    {
        auto jsonAppVersion = jsonRootObject["Version"].toObject();

        if (jsonAppVersion.contains("Minor"))
        {
            metaInfo->versionMinor = jsonAppVersion["Minor"].toInt();
        }

        if (jsonAppVersion.contains("Major"))
        {
            metaInfo->versionMajor = jsonAppVersion["Major"].toInt();
        }
    }

    return true;
}
