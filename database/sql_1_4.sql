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

INSERT INTO "AMSUser" ("amsuser_name", "amsuser_username", "amsuser_password", "amsuser_active")
VALUES
    ("administrator", "administrator",
     "$argon2id$v=19$m=8192,t=100,p=1$w93g/9FGO2nAajwoi02FHw$tHjzDkGoscBHLJOGJoqh699KXvWf+JqmARrR2L4yzxk", 1);

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

-- Add default values for access mode
INSERT INTO "AMSAccessMode" ("amsaccessmode_name", "amsaccessmode_value", "amsaccessmode_description")
VALUES
       ("QM_MODE_READ", 0, "Read access to qualification matrix"),
       ("QM_MODE_WRITE", 1, "Write and read access to qualification matrix"),
       ("QR_MODE_READ", 2, "Read access to qualification result"),
       ("TD_MODE_READ", 3, "Read access to training data including certification management"),
       ("TD_MODE_WRITE", 4, "Write and read access to training data including certification management"),
       ("PER_DATA_CONFIG", 5, "Read and write access to the persistent tables in config");

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

-- Connects employees with the groups

CREATE TABLE IF NOT EXISTS "AMSGroupEmployee" (
    "amsgroupemployee_id"                           INTEGER,
    "amsgroupemployee_group"                        INTEGER,
    "amsgroupemployee_employee"                     INTEGER,
    PRIMARY KEY("amsgroupemployee_id"),
    FOREIGN KEY("amsgroupemployee_group")           REFERENCES
        "AMSGroup" ("amsgroup_id"),
    FOREIGN KEY("amsgroupemployee_employee")        REFERENCES
        "Employee" ("id")
);

-- General information for access management system
CREATE TABLE IF NOT EXISTS "AMSGeneral" (
    "amsgeneral_id"            INTEGER,
    "amsgeneral_name"          TEXT,
    "amsgeneral_value"         TEXT,
    "amsgeneral_description"   TEXT,
    PRIMARY KEY("amsgeneral_id")
);

-- Update version of database

UPDATE "Info" SET "value" = "1" WHERE "name" = "MAJOR";
UPDATE "Info" SET "value" = "4" WHERE "name" = "MINOR";
