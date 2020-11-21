-- Info definition

CREATE TABLE IF NOT EXISTS "Info" (
    "id"        INTEGER,
    "name"      TEXT,
    "value"     TEXT,
    PRIMARY KEY("id")
);

-- Add option in info for file_location, which is necessary for certificates

INSERT INTO "Info" ("name", "value") VALUES("file_location", "external");

-- Files definition

CREATE TABLE IF NOT EXISTS "File" (
    "id"    INTEGER,
    "name"      TEXT,
    "type"      TEXT,
    "path"      TEXT,
    "binary"    BLOB,
    PRIMARY KEY("id")
);

-- TraiDataFile links TrainData and File extries

CREATE TABLE IF NOT EXISTS "TrainDataFile" (
    "id"            INTEGER,
    "train_data"    INTEGER,
    "file"          INTEGER,
    PRIMARY KEY("id"),
    FOREIGN KEY("train_data")   REFERENCES "TrainData"("id"),
    FOREIGN KEY("file")         REFERENCES "File"("id")
);

-- Change Train to implement legally_necessary entries

ALTER TABLE "Train" ADD COLUMN "legally_necessary" INTEGER;

-- Update version of database

UPDATE "Info" SET "value" = "1" WHERE "name" = "MAJOR";
UPDATE "Info" SET "value" = "0" WHERE "name" = "MINOR";
