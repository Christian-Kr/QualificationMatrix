-- Info definition

CREATE TABLE "Info" (
    "id"        INTEGER,
    "name"      TEXT,
    "value"     TEXT,
    PRIMARY KEY("id")
);

INSERT INTO "Info" ("name", "value") VALUES("MAJOR", "1");
INSERT INTO "Info" ("name", "value") VALUES("MINOR", "0");
INSERT INTO "Info" ("name", "value") VALUES("file_location", "external");

-- Files definition

CREATE TABLE "File" (
    "id"    INTEGER,
    "name"      TEXT,
    "type"      TEXT,
    "path"      TEXT,
    "binary"    BLOB,
    PRIMARY KEY("id")
);

-- TraiDataFile links TrainData and File extries

CREATE TABLE "TrainDataFile" (
    "id"            INTEGER,
    "train_data"    INTEGER,
    "file"          INTEGER,
    PRIMARY KEY("id"),
    FOREIGN KEY("train_data")   REFERENCES "TrainData"("id"),
    FOREIGN KEY("file")         REFERENCES "File"("id")
);

-- Change Train to implement legally_necessary entries

ALTER TABLE "Train" ADD COLUMN "legally_necessary" INTEGER;
