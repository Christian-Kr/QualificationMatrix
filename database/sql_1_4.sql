-- User for Access Management System

CREATE TABLE IF NOT EXISTS "AMSUser" (
    "id"	                INTEGER,
    "name"	                TEXT,
    "username"              TEXT,
    "password"              TEXT,
    "last_login"            TEXT,
    "unsuccess_login_num"   INTEGER DEFAULT 0,
    "active"                INTEGER DEFAULT 0,
    PRIMARY KEY("id")
);

-- Group for Access Management System

CREATE TABLE IF NOT EXISTS "AMSGroup" (
   "id"	        INTEGER,
   "name"	    TEXT,
   "active"     INTEGER DEFAULT 0,
   PRIMARY KEY("id")
);

-- Connects the AMSUser with the AMSGroup

CREATE TABLE IF NOT EXISTS "AMSUserGroup" (
    "id"        INTEGER,
    "user"      INTEGER,
    "group"     INTEGER,
    PRIMARY KEY("id"),
    FOREIGN KEY("user")     REFERENCES "AMSUser"("id"),
    FOREIGN KEY("group")    REFERENCES "AMSGroup"("id")
);

-- Modes of access for group

CREATE TABLE IF NOT EXISTS "AMSAccessMode" (
    "id"            INTEGER,
    "name"          TEXT,
    "value"         INTEGER,
    "description"   TEXT,
    PRIMARY KEY("id")
);

-- Connects the acces modes with the groups

CREATE TABLE IF NOT EXISTS "AMSGroupAccessMode" (
    "id"            INTEGER,
    "group"         INTEGER,
    "access_mode"   INTEGER,
    PRIMARY KEY("id"),
    FOREIGN KEY("group")        REFERENCES "AMSGroup"("id"),
    FOREIGN KEY("access_mode")  REFERENCES "AMSAccessMode"("id")
);

-- General information for access management system
CREATE TABLE IF NOT EXISTS "AMSGeneral" (
    "id"            INTEGER,
    "name"          TEXT,
    "value"         TEXT,
    "description"   TEXT,
    PRIMARY KEY("id")
);

-- Update version of database

UPDATE "Info" SET "value" = "1" WHERE "name" = "MAJOR";
UPDATE "Info" SET "value" = "4" WHERE "name" = "MINOR";
