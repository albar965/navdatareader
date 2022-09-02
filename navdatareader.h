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

#include "fs/navdatabaseflags.h"
#include "fs/navdatabaseoptions.h"
#include "sql/sqldatabase.h"

/*
 * Reads configuration options from the command line, the configuration file and
 * loads all BGLs into the Sqlite database.
 */
class NavdataReader
{
public:
  void run();

  int getNumErrors() const
  {
    return numErrors;
  }

  const atools::fs::ResultFlags& getResultFlags() const
  {
    return resultFlags;
  }

  /* Database file name */
  QString getDatabaseName() const;

private:
  /* Parse command line arguments */
  void parseArgs();

  atools::fs::FsPaths::SimulatorType type = atools::fs::FsPaths::FSX;

  int numErrors = 0;
  atools::sql::SqlDatabase db;
  atools::fs::NavDatabaseOptions opts;
  QString configFile;
  atools::fs::ResultFlags resultFlags = atools::fs::COMPILE_NONE;
};

#endif // NAVDATAREADER_H
