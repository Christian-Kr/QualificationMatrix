-- TrainGroup: Add a column to specify whether the train group should
-- be part of qualification result.

ALTER TABLE "TrainGroup" ADD COLUMN "qmresult_ignore" INTEGER default 0;

-- Update version of database

UPDATE "Info" SET "value" = "1" WHERE "name" = "MAJOR";
UPDATE "Info" SET "value" = "9" WHERE "name" = "MINOR";
