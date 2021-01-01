-- TrainDataState definition

CREATE TABLE IF NOT EXISTS "TrainDataState" (
	"id"	INTEGER,
	"name"	TEXT,
	PRIMARY KEY("id")
);

-- Shift definition

CREATE TABLE IF NOT EXISTS "Shift" (
	"id"	INTEGER,
	"name"	TEXT NOT NULL,
	PRIMARY KEY("id")
);

-- QualiState definition

CREATE TABLE IF NOT EXISTS "QualiState" (
	"id"	INTEGER,
	"name"	TEXT,
	PRIMARY KEY("id")
);

-- Info definition

CREATE TABLE IF NOT EXISTS "Info" (
	"id"	INTEGER,
	"name"	TEXT,
	"value"	TEXT,
	PRIMARY KEY("id")
);

INSERT INTO "Info" ("name", "value") VALUES("MAJOR", "0");
INSERT INTO "Info" ("name", "value") VALUES("MINOR", "0");

-- FuncGroup definition

CREATE TABLE IF NOT EXISTS "FuncGroup" (
	"id"	INTEGER,
	"name"	TEXT,
	"color"	TEXT,
	PRIMARY KEY("id")
);

-- TrainGroup definition

CREATE TABLE IF NOT EXISTS "TrainGroup" (
	"id"	INTEGER,
	"name"	TEXT,
	"color"	TEXT,
	PRIMARY KEY("id")
);

-- Employee definition

CREATE TABLE IF NOT EXISTS "Employee" (
	"id"	INTEGER,
	"name"	TEXT NOT NULL,
	"shift"	INTEGER NOT NULL,
	PRIMARY KEY("id"),
	FOREIGN KEY("shift") REFERENCES `Shift`("id")
);

-- Train definition

CREATE TABLE IF NOT EXISTS "Train" (
	"id"	INTEGER,
	"name"	TEXT,
	"group"	INTEGER,
	"interval"	INTEGER,
	PRIMARY KEY("id"),
	FOREIGN KEY("group") REFERENCES "TrainGroup"("id")
);

-- TrainData definition

CREATE TABLE IF NOT EXISTS "TrainData" (
	"id"	INTEGER,
	"employee"	INTEGER,
	"train"	INTEGER,
	"date"	TEXT,
	"state"	INTEGER,
	PRIMARY KEY("id"),
	FOREIGN KEY("employee") REFERENCES "Employee"("id"),
	FOREIGN KEY("state") REFERENCES "TrainDataState"("id"),
	FOREIGN KEY("train") REFERENCES "Train"("id")
);

-- TrainException definition

CREATE TABLE IF NOT EXISTS `TrainException` (
	"id"	INTEGER,
	"employee"	INTEGER,
	"train"	INTEGER,
	"explanation"	TEXT,
	FOREIGN KEY("train") REFERENCES "Train"("id"),
	FOREIGN KEY("employee") REFERENCES "Employee"("id"),
	PRIMARY KEY("id")
);

-- Func definition

CREATE TABLE IF NOT EXISTS "Func" (
	"id"	INTEGER,
	"name"	TEXT,
	"group"	INTEGER,
	FOREIGN KEY("group") REFERENCES "FuncGroup"("id"),
	PRIMARY KEY("id")
);

-- EmployeeFunc definition

CREATE TABLE IF NOT EXISTS "EmployeeFunc" (
	"id"	INTEGER,
	"employee"	INTEGER,
	"func"	INTEGER,
	PRIMARY KEY("id"),
	FOREIGN KEY("employee") REFERENCES "Employee"("id"),
	FOREIGN KEY("func") REFERENCES "Func"("id")
);

-- QualiData definition

CREATE TABLE IF NOT EXISTS "QualiData" (
	"id"	INTEGER,
	"func"	INTEGER,
	"train"	INTEGER,
	"qualistate"	INTEGER,
	PRIMARY KEY("id"),
	FOREIGN KEY("train") REFERENCES "Train"("id"),
	FOREIGN KEY("func") REFERENCES "Func"("id"),
	FOREIGN KEY("qualistate") REFERENCES "QualiState"("id")
);
