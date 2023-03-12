/*****************************************************************************
* Copyright 2015-2022 Alexander Barthel alex@littlenavmap.org
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

#include "navdatareader.h"

#include "atools.h"
#include "exception.h"
#include "fs/navdatabase.h"
#include "fs/navdatabaseerrors.h"
#include "logging/logginghandler.h"
#include "logging/loggingutil.h"
#include "settings/settings.h"

#include <QCommandLineParser>
#include <QSettings>

using atools::sql::SqlDatabase;
using atools::fs::NavDatabaseErrors;
using atools::fs::FsPaths;

void NavdataReader::run()
{
  atools::logging::LoggingHandler::initialize(atools::settings::Settings::getOverloadedLocalPath(
                                                ":/navdatareader/resources/config/logging.cfg"));

  // Load simulator paths =================================
  FsPaths::loadAllPaths();

  // Fill NavDatabaseOptions from command line
  parseArgs();

  if(!opts.getSourceDatabase().isEmpty())
    qInfo() << "===== Input Database " << QFileInfo(opts.getSourceDatabase()).absoluteFilePath() << "=====";

  qInfo() << "===== Output Database" << QFileInfo(db.databaseName()).absoluteFilePath() << "=====";
  qInfo() << "===== Using source data type" << FsPaths::typeToShortName(type) << "=====";
  qInfo() << "===== Configuration" << configFile << "=====";

  qInfo() << "This software is licensed under the GPL3 or any later version.";
  qInfo() << "See http://www.gnu.org/licenses/gpl-3.0 for more information.";
  qInfo() << "The source code for this application is available at https://github.com/albar965";
  qInfo() << "Copyright 2015-2023 Alexander Barthel (alex@littlenavmap.org).";
  qInfo().nospace().noquote() << "Version " << QCoreApplication::applicationVersion()
                              << " (revision " << GIT_REVISION_NAVDATAREADER << ")";
  qInfo().nospace().noquote() << "atools Version " << atools::version()
                              << " (revision " << atools::gitRevision() << ")";

  using atools::logging::LoggingUtil;
  LoggingUtil::logSystemInformation();

  LoggingUtil::logStandardPaths();

  FsPaths::logAllPaths();

  qInfo() << opts;

  QStringList DATABASE_PRAGMAS({QString("PRAGMA cache_size=-%1").arg(50000),
                                "PRAGMA synchronous=OFF",
                                "PRAGMA journal_mode=TRUNCATE",
                                "PRAGMA page_size=8196"
                                /*, "PRAGMA locking_mode=EXCLUSIVE"*/});

  // FKs don't work currently
  db.open(DATABASE_PRAGMAS);

  atools::fs::NavDatabaseErrors errors;
  atools::fs::NavDatabase nd(&opts, &db, &errors, GIT_REVISION_NAVDATAREADER);
  resultFlags = nd.compileDatabase();
  db.close();

  if(errors.getTotalErrors() > 0)
  {
    qWarning() << "==================================================================";
    qWarning() << "== FOUND ERRORS ==================================================";
    for(const NavDatabaseErrors::SceneryErrors& errs : errors.sceneryErrors)
    {
      qWarning() << "Error in scenery" << errs.scenery;

      for(const NavDatabaseErrors::SceneryFileError& err : errs.fileErrors)
        qWarning() << "Error in file" << err.filepath << "line" << err.lineNum << ":" << err.errorMessage;

      for(const QString& err : errs.sceneryErrorsMessages)
        qWarning() << "Other error:" << err;
    }

    throw atools::Exception("Found errors while compiling. Check log file.");
  }
}

QString NavdataReader::getDatabaseName() const
{
  return db.databaseName();
}

void NavdataReader::parseArgs()
{
  // Build command line option parser ===================================================
  QCommandLineParser parser;

  parser.setApplicationDescription("Flight Simulator Navdata Database Reader.\n"
                                   "This software is licensed under the GPL3 or any later version.");
  parser.addHelpOption();
  parser.addVersionOption();

  QCommandLineOption fstypeOpt({"f", "flight-simulator"},
                               QObject::tr("Required option. Flight simulator type <simulator> or other data source. "
                                           "Either FSX, FSXSE, P3DV2, P3DV3, P3DV4, P3DV5, XP11, XP12, MSFS or DFD."),
                               QObject::tr("simulator"));
  parser.addOption(fstypeOpt);

  QCommandLineOption sceneryOpt({"s", "scenery"},
                                QObject::tr("Scenery.cfg file <scenery> for FSX and P3D. "
                                            "Determined by registry entries if not given."),
                                QObject::tr("scenery"));
  parser.addOption(sceneryOpt);

  QCommandLineOption sourceDbOpt({"d", "source-database"},
                                 QObject::tr("Required option for DFD compilation. "
                                             "Source database <source-database> for DFD based compilation."),
                                 QObject::tr("source-database"));
  parser.addOption(sourceDbOpt);

  QCommandLineOption basepathOpt({"b", "basepath"},
                                 QObject::tr("Flight simulator basepath for BGL files <basepath>. "
                                             "Determined by registry entries if not given."),
                                 QObject::tr("basepath"));
  parser.addOption(basepathOpt);

  QCommandLineOption databaseOpt({"o", "output"},
                                 QObject::tr("Output Sqlite database filename <output> "
                                             "Default is \"navdata.sqlite\"."),
                                 QObject::tr("output"),
                                 "navdata.sqlite");
  parser.addOption(databaseOpt);

  QCommandLineOption cfgOpt({"c", "config"},
                            QObject::tr("Configuration file <config> for %1. "
                                        "Default is to use integrated \"navdatareader.cfg\".").
                            arg(QCoreApplication::applicationName()),
                            QObject::tr("config"));
  parser.addOption(cfgOpt);

  // Process the actual command line arguments given by the user
  parser.process(*QCoreApplication::instance());

  // Simulator Type ===================================================
  if(parser.isSet(fstypeOpt))
  {
    type = FsPaths::stringToType(parser.value(fstypeOpt).toUpper());
    opts.setSimulatorType(type);
  }

  if(type == FsPaths::NONE)
  {
    qCritical().noquote().nospace() << "*** ERROR: Unknown type for option -f." << endl;
    parser.showHelp(1);
  }

  // Check required options ===================================================
  if(!parser.isSet(fstypeOpt))
  {
    qCritical().noquote().nospace() << "*** ERROR: No database type given." << endl;
    parser.showHelp(1);
  }

  if(FsPaths::isAnyXplane(type) && !parser.isSet(basepathOpt))
  {
    qCritical().noquote().nospace() << "*** ERROR: No base path for X-Plane given." << endl;
    parser.showHelp(1);
  }

  if(type == FsPaths::DFD && !parser.isSet(sourceDbOpt))
  {
    qCritical().noquote().nospace() << "*** ERROR: No DFD database given." << endl;
    parser.showHelp(1);
  }

  // Base path ===================================================
  if(type != FsPaths::DFD)
  {
    QString basepath = parser.value(basepathOpt);
    if(basepath.isEmpty())
      basepath = FsPaths::getBasePath(type);

    if(!atools::checkDir(Q_FUNC_INFO, basepath))
      exit(1);
    opts.setBasepath(basepath);
  }

  // Source database ===================================================
  if(type == FsPaths::DFD)
  {
    QString dbFile = parser.value(sourceDbOpt);

    if(!atools::checkFile(Q_FUNC_INFO, dbFile))
      exit(1);
    opts.setSourceDatabase(dbFile);
  }

  // Scenery.cfg only FSX and P3D ===================================================
  if(!FsPaths::isAnyXplane(type) && type != FsPaths::DFD && type != FsPaths::MSFS)
  {
    QString sceneryFile = parser.value(sceneryOpt);
    if(sceneryFile.isEmpty())
      sceneryFile = FsPaths::getSceneryLibraryPath(type);
    if(!atools::checkFile(Q_FUNC_INFO, sceneryFile))
      exit(1);

    opts.setSceneryFile(sceneryFile);
  }

  if(type == FsPaths::MSFS)
  {
    opts.setMsfsCommunityPath(FsPaths::getMsfsCommunityPath(opts.getBasepath()));
    opts.setMsfsOfficialPath(FsPaths::getMsfsOfficialPath(opts.getBasepath()));
  }

  // Configuration file ===================================================
  configFile = parser.value(cfgOpt);
  if(configFile.isEmpty())
    // Command line overrides resource settings file
    configFile = ":/navdatareader/resources/config/navdatareader.cfg";
  if(!atools::checkFile(Q_FUNC_INFO, configFile))
    exit(1);

  QSettings settings(configFile, QSettings::IniFormat);

  opts.loadFromSettings(settings);

  // Create database ===================================================
  db = SqlDatabase(settings, "Database");
  QString databaseName = parser.value(databaseOpt);
  if(!databaseName.isEmpty())
  {
    // Remove any stale files
    QFileInfo file(databaseName);
    if(file.exists() && file.isFile())
    {
      if(QFile::remove(databaseName))
        qInfo() << "Removed" << databaseName;
    }

    db.setDatabaseName(databaseName);
  }
}
