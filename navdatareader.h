/*****************************************************************************
* Copyright 2015-2020 Alexander Barthel alex@littlenavmap.org
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*****************************************************************************/

#ifndef NAVDATAREADER_H
#define NAVDATAREADER_H

#include "sql/sqldatabase.h"
#include "fs/navdatabaseoptions.h"

/*
 * Reads configuration options from the command line, the configuration file and
 * loads all BGLs into the Sqlite database.
 */
class NavdataReader
{
public:
  NavdataReader();

  void run();

  int getNumErrors() const
  {
    return numErrors;
  }

  bool isBasicValidationError() const
  {
    return foundBasicValidationError;
  }

private:
  /* Parse command line arguments */
  void parseArgs();

  /* Copy all airport files to the path given on the command line */
  void copyFiles();

  atools::fs::FsPaths::SimulatorType type = atools::fs::FsPaths::FSX;

  int numErrors = 0;
  atools::sql::SqlDatabase db;
  atools::fs::NavDatabaseOptions opts;
  QString copyFilePath;
  QString configFile;
  bool foundBasicValidationError = false;
};

#endif // NAVDATAREADER_H
