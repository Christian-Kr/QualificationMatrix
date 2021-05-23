-- Create view for functions

CREATE VIEW IF NOT EXISTS FuncView AS
SELECT
    Func.id,
    Func.name,
    FuncGroup.name
FROM
    Func
LEFT JOIN FuncGroup ON
    Func."group" = FuncGroup.id;

-- Create view for function groups

CREATE VIEW IF NOT EXISTS FuncGroupView AS
SELECT
    FuncGroup.id,
    FuncGroup.name,
    FuncGroup.color
FROM
    FuncGroup;

-- Create view for trainings

CREATE VIEW IF NOT EXISTS TrainView AS
SELECT
    Train.id,
    Train.name,
    TrainGroup.name,
    Train.interval,
    Train.legally_necessary,
    Train.content_description
FROM
    Train
LEFT JOIN TrainGroup ON
    Train."group" = TrainGroup.id;

-- Create view for training groups

CREATE VIEW IF NOT EXISTS TrainGroupView AS
SELECT
    TrainGroup.id,
    TrainGroup.name,
    TrainGroup.color
FROM
    TrainGroup;

-- Create train data state view

CREATE VIEW IF NOT EXISTS TrainDataStateView AS
SELECT
    TrainDataState.id,
    TrainDataState.name
FROM
    TrainDataState;

-- Create train data state view

CREATE VIEW IF NOT EXISTS ShiftView AS
SELECT
    Shift.id,
    Shift.name
FROM
    Shift;

-- Update version of database

UPDATE "Info" SET "value" = "1" WHERE "name" = "MAJOR";
UPDATE "Info" SET "value" = "3" WHERE "name" = "MINOR";
