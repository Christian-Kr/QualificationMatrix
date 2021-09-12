-- User for Access Management System

CREATE TABLE IF NOT EXISTS "AMSUser" (
    "amsuser_id"	                    INTEGER,
    "amsuser_name"	                    TEXT,
    "amsuser_username"                  TEXT,
    "amsuser_password"                  TEXT,
    "amsuser_last_login"                TEXT,
    "amsuser_unsuccess_login_num"       INTEGER DEFAULT 0,
    "amsuser_active"                    INTEGER DEFAULT 0,
    PRIMARY KEY("amsuser_id")
);

-- Group for Access Management System

CREATE TABLE IF NOT EXISTS "AMSGroup" (
   "amsgroup_id"	                    INTEGER,
   "amsgroup_name"	                    TEXT,
   "amsgroup_active"                    INTEGER DEFAULT 0,
   PRIMARY KEY("amsgroup_id")
);

-- Connects the AMSUser with the AMSGroup

CREATE TABLE IF NOT EXISTS "AMSUserGroup" (
    "amsusergroup_id"                   INTEGER,
    "amsusergroup_user"                 INTEGER,
    "amsusergroup_group"                INTEGER,
    PRIMARY KEY("amsusergroup_id"),
    FOREIGN KEY("amsusergroup_user")    REFERENCES "AMSUser"("amsuser_id"),
    FOREIGN KEY("amsusergroup_group")   REFERENCES "AMSGroup"("amsgroup_id")
);

-- Modes of access for group

CREATE TABLE IF NOT EXISTS "AMSAccessMode" (
    "amsaccessmode_id"                  INTEGER,
    "amsaccessmode_name"                TEXT,
    "amsaccessmode_value"               INTEGER,
    "amsaccessmode_description"         TEXT,
    PRIMARY KEY("amsaccessmode_id")
);

-- Connects the acces modes with the groups

CREATE TABLE IF NOT EXISTS "AMSGroupAccessMode" (
    "amsgroupaccessmode_id"                         INTEGER,
    "amsgroupaccessmode_group"                      INTEGER,
    "amsgroupaccessmode_access_mode"                INTEGER,
    PRIMARY KEY("amsgroupaccessmode_id"),
    FOREIGN KEY("amsgroupaccessmode_group")         REFERENCES
        "AMSGroup" ("amsgroup_id"),
    FOREIGN KEY("amsgroupaccessmode_access_mode")   REFERENCES
        "AMSAccessMode"("amsaccessmode_id")
);

-- General information for access management system
CREATE TABLE IF NOT EXISTS "AMSGeneral" (
    "amsgeneral_id"            INTEGER,
    "amsgeneral_name"          TEXT,
    "amsgeneral_value"         TEXT,
    "amsgeneral_description"   TEXT,
    PRIMARY KEY("id")
);

-- Update version of database

UPDATE "Info" SET "value" = "1" WHERE "name" = "MAJOR";
UPDATE "Info" SET "value" = "4" WHERE "name" = "MINOR";
