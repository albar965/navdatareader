/*****************************************************************************
* Copyright 2015-2016 Alexander Barthel albar965@mailbox.org
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

#include "fs/bglreaderoptions.h"
#include "fs/scenery/scenerycfg.h"
#include "sql/sqldatabase.h"
#include "sql/sqlscript.h"
#include "sql/sqlquery.h"
#include "fs/writer/datawriter.h"
#include "fs/writer/routeresolver.h"
#include "fs/scenery/sceneryarea.h"
#include "sql/sqlutil.h"
#include "fs/fspaths.h"
#include "logging/loggingdefs.h"
#include "logging/logginghandler.h"
#include "settings/settings.h"
#include "fs/navdatabase.h"
#include "navdatareader.h"
#include "exception.h"

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QElapsedTimer>
#include <QFileInfo>
#include <QSettings>

#include "navdatareader.h"

#include "sql/sqldatabase.h"

using atools::fs::scenery::SceneryCfg;
using atools::sql::SqlDatabase;
using atools::sql::SqlScript;
using atools::sql::SqlQuery;

NavdataReader::NavdataReader()
{

}

void NavdataReader::run()
{
  parseArgs();

  qInfo() << opts;

  db.open();

  atools::fs::Navdatabase nd(opts, &db);
  nd.create();

  db.close();

}

void NavdataReader::parseArgs()
{
  QCommandLineParser parser;
  parser.setApplicationDescription("Flight Simulator Navdata Database Reader");
  parser.addHelpOption();
  parser.addVersionOption();

  QCommandLineOption verboseOpt("verbose",
                                QObject::tr("Use verbose logging"));
  parser.addOption(verboseOpt);

  QCommandLineOption sceneryOpt({"s", "scenery"},
                                QObject::tr("Scenery.cfg file <scenery>"),
                                QObject::tr("scenery"));
  parser.addOption(sceneryOpt);

  QCommandLineOption fstypeOpt({"f", "flight-simulator"},
                               QObject::tr("Flight simulator type <simulator>. "
                                           "Either FSX, FSXSE, P3DV2 or P3DV3."),
                               QObject::tr("simulator"));
  parser.addOption(fstypeOpt);

  QCommandLineOption basepathOpt({"b", "basepath"},
                                 QObject::tr("Flight simulator basepath for BGL files <basepath>"),
                                 QObject::tr("basepath"));
  parser.addOption(basepathOpt);

  QCommandLineOption databaseOpt({"d", "database"},
                                 QObject::tr("Sqlite database filename <database>"),
                                 QObject::tr("database"),
                                 "navdata.sqlite");
  parser.addOption(databaseOpt);

  QCommandLineOption cfgOpt({"c", "config"},
                            QObject::tr("Configuration file for %1").arg(QCoreApplication::applicationName()),
                            QObject::tr("scenery"));
  parser.addOption(cfgOpt);

  // Process the actual command line arguments given by the user
  parser.process(*QCoreApplication::instance());

  bool verbose = parser.isSet(verboseOpt);
  QString sceneryFile = parser.value(sceneryOpt);
  QString basepath = parser.value(basepathOpt);

  QString configFile = parser.value(cfgOpt);
  qInfo() << "Configuration file" << configFile;

  QString databaseName = parser.value(databaseOpt);
  qInfo() << "Database" << databaseName;

  atools::fs::fstype::SimulatorType type = atools::fs::fstype::FSX;
  if(parser.isSet(fstypeOpt))
    type = atools::fs::FsPaths::stringToType(parser.value(fstypeOpt));

  qInfo() << "Using simulator type" << atools::fs::FsPaths::typeToString(type);

  if(sceneryFile.isEmpty())
    sceneryFile = atools::fs::FsPaths::getSceneryLibraryPath(type);

  if(basepath.isEmpty())
    basepath = atools::fs::FsPaths::getBasePath(type);

  if(!checkDir(basepath, "Base path: "))
    exit(1);

  if(!checkFile(sceneryFile, "Scenery file: "))
    exit(1);

  if(!(configFile.isEmpty() || checkFile(configFile, "Config file: ")))
    exit(1);

  opts.setSceneryFile(sceneryFile);
  opts.setBasepath(basepath);
  opts.setVerbose(verbose);

  QSettings settings(configFile, QSettings::IniFormat);

  opts.loadFiltersFromSettings(settings);

  db = SqlDatabase(settings, "Database");
  if(!databaseName.isEmpty())
    db.setDatabaseName(databaseName);
}

bool NavdataReader::checkFile(const QString& path, const QString& msg)
{
  if(path.isEmpty())
  {
    qCritical().noquote().nospace() << msg << " is empty";
    return false;
  }
  else
  {
    QFileInfo fi(path);
    if(!fi.exists())
    {
      qCritical().noquote().nospace() << msg << " file does not exist";
      return false;
    }
    else if(!fi.isFile())
    {
      qCritical().noquote().nospace() << msg << " is not a file";
      return false;
    }
  }
  return true;
}

bool NavdataReader::checkDir(const QString& path, const QString& msg)
{
  if(path.isEmpty())
  {
    qCritical().noquote().nospace() << msg << " is empty";
    return false;
  }
  else
  {
    QFileInfo fi(path);
    if(!fi.exists())
    {
      qCritical().noquote().nospace() << msg << " directory does not exist";
      return false;
    }
    else if(!fi.isDir())
    {
      qCritical().noquote().nospace() << msg << " is not a directory";
      return false;
    }
  }
  return true;
}
