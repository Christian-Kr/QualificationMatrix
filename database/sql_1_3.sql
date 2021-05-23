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
LEFT JOIN TrainGroup ON Train."group" = TrainGroup.id;

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

-- Create train exception view

CREATE VIEW IF NOT EXISTS TrainExceptionView AS
SELECT
    TrainException.id,
    Employee.name,
    Train.name,
    TrainException.explanation
FROM
    TrainException
LEFT JOIN Employee ON TrainException.employee = Employee.id
LEFT JOIN Train ON TrainException.train = Train.id;

-- Create a quali data view

CREATE VIEW IF NOT EXISTS QualiDataView AS
SELECT
    QualiData.id,
    Func.name,
    Train.name,
    QualiState.name
FROM
    QualiData
LEFT JOIN Func ON QualiData.func = Func.id
LEFT JOIN Train ON QualiData.train = Train.id
LEFT JOIN QualiState ON QualiData.qualistate = QualiState.id;

-- Create employee func view model

CREATE VIEW IF NOT EXISTS EmployeeFuncView AS
SELECT
    EmployeeFunc.id,
    Employee.name,
    Func.name
FROM
    EmployeeFunc
LEFT JOIN Employee ON EmployeeFunc.employee = Employee.id
LEFT JOIN Func ON EmployeeFunc.func = Func.id;

-- Create train data view

CREATE VIEW IF NOT EXISTS TrainDataView AS
SELECT
    TrainData.id,
    Employee.name,
    Train.name,
    TrainData.date,
    TrainDataState.name
FROM
    TrainData
LEFT JOIN Employee ON TrainData.employee = Employee.id
LEFT JOIN Train ON TrainData.train = Train.id
LEFT JOIN TrainDataState ON TrainData.state = TrainDataState.id;

-- Update version of database

UPDATE "Info" SET "value" = "1" WHERE "name" = "MAJOR";
UPDATE "Info" SET "value" = "3" WHERE "name" = "MINOR";
