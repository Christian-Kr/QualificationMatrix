// config.h is part of QualificationMatrix
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

// Define version of software.
#define STRING(n) #n
#define _BUILD(n) STRING(n)
#define BUILD _BUILD(__TIMESTAMP__)
#define RELEASE_STATE "beta"
#define VERSION_MAJOR "1"
#define VERSION_MINOR "0"

// Define the minimum version a db must have, otherwise the software won't work with it - 0, 0 means no version
// table or entry is available, cause the first version doesn't include this information.
#define DB_MIN_MAJOR 1
#define DB_MIN_MINOR 5

// BackupManager system
#define BACKUP_META_FILE_VERSION 1
