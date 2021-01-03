-- Info definition

CREATE TABLE IF NOT EXISTS "Info" (
    "id"        INTEGER,
    "name"      TEXT,
    "value"     TEXT,
    PRIMARY KEY("id")
);

-- Add option in info for file_location, which is necessary for certificates

INSERT INTO "Info" ("name", "value") VALUES("certificate_location", "external");
INSERT INTO "Info" ("name", "value") VALUES("certificate_location_path", "");

-- Certificate definition

CREATE TABLE IF NOT EXISTS "Certificate" (
    "id"            INTEGER,
    "name"          TEXT,
    "type"          TEXT,
    "path"          TEXT,
    "binary"        BLOB,
    "md5_hash"      TEXT,
    "add_date"      TEXT,
    "train_date"    TEXT,
    PRIMARY KEY("id")
);

-- TraiDataCertificate links TrainData and File extries

CREATE TABLE IF NOT EXISTS "TrainDataCertificate" (
    "id"            INTEGER,
    "train_data"    INTEGER,
    "certificate"   INTEGER,
    PRIMARY KEY("id"),
    FOREIGN KEY("train_data")       REFERENCES "TrainData"("id"),
    FOREIGN KEY("certificate")      REFERENCES "Certificate"("id")
);

-- Create view to make handling more easy.

CREATE VIEW IF NOT EXISTS TrainDataCertificateView AS
	SELECT
       TrainDataCertificate.id,
       TrainDataCertificate.train_data,
       TrainDataCertificate.certificate,
       Certificate.name,
       Certificate.md5_hash
	FROM
        TrainDataCertificate
    LEFT JOIN Certificate ON TrainDataCertificate.certificate = Certificate.id;

-- Change Train to implement legally_necessary entries

ALTER TABLE "Train" ADD COLUMN "legally_necessary" INTEGER;

-- Update version of database

UPDATE "Info" SET "value" = "1" WHERE "name" = "MAJOR";
UPDATE "Info" SET "value" = "0" WHERE "name" = "MINOR";

-- Add column to TrainDataState for colors.

ALTER TABLE "TrainDataState" ADD COLUMN "color" TEXT;
