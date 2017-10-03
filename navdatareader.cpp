/*****************************************************************************
* Copyright 2015-2017 Alexander Barthel albar965@mailbox.org
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

#include "fs/scenery/scenerycfg.h"
#include "sql/sqldatabase.h"
#include "fs/fspaths.h"
#include "fs/navdatabase.h"
#include "atools.h"
#include "logging/loggingutil.h"
#include "fs/db/databasemeta.h"
#include "fs/navdatabaseerrors.h"

#include <QDebug>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QElapsedTimer>
#include <QFileInfo>
#include <QSettings>
#include <QCoreApplication>
#include <QDir>

using atools::fs::scenery::SceneryCfg;
using atools::sql::SqlDatabase;
using atools::fs::NavDatabaseErrors;

NavdataReader::NavdataReader()
{

}

void NavdataReader::run()
{
  parseArgs();

  qInfo() << "This software is licensed under the GPL3 or any later version.";
  qInfo() << "See http://www.gnu.org/licenses/gpl-3.0 for more information.";
  qInfo() << "The source code for this application is available at https://github.com/albar965";
  qInfo() << "Copyright 2015-2017 Alexander Barthel (albar965@mailbox.org).";
  qInfo().nospace().noquote() << "Version " << QCoreApplication::applicationVersion()
                              << " (revision " << GIT_REVISION << ")";
  qInfo().nospace().noquote() << "atools Version " << atools::version()
                              << " (revision " << atools::gitRevision() << ")";

  using atools::logging::LoggingUtil;
  LoggingUtil::logSystemInformation();

  LoggingUtil::logStandardPaths();

  qInfo() << opts;

  // FKs don't work currently
  db.open( /*{"PRAGMA foreign_keys = ON"}*/);

  atools::fs::NavDatabaseErrors errors;
  atools::fs::NavDatabase nd(&opts, &db, &errors);
  QString sceneryCfgCodec = opts.getSimulatorType() == atools::fs::FsPaths::P3D_V4 ? "UTF-8" : QString();
  nd.create(sceneryCfgCodec);

  // Copy all files containing airport or navaid information to another directory
  // Only for debugging purposes
  if(!copyFilePath.isEmpty())
    copyFiles();

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
  }
}

void NavdataReader::parseArgs()
{
  // Build command line option parser ===================================================
  QCommandLineParser parser;
  parser.setApplicationDescription("Flight Simulator Navdata Database Reader.\n"
                                   "This software is licensed under the GPL3 or any later version.");
  parser.addHelpOption();
  parser.addVersionOption();

  QCommandLineOption verboseOpt("verbose",
                                QObject::tr("Use verbose logging"));
  parser.addOption(verboseOpt);

  QCommandLineOption sceneryOpt({"s", "scenery"},
                                QObject::tr("Scenery.cfg file <scenery>. Not for X-Plane."),
                                QObject::tr("scenery"));
  parser.addOption(sceneryOpt);

  QCommandLineOption fstypeOpt({"f", "flight-simulator"},
                               QObject::tr("Flight simulator type <simulator>. "
                                           "Either FSX, FSXSE, P3DV2, P3DV3, P3DV4 or XP11."),
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
                            QObject::tr("Configuration file <config> for %1").
                            arg(QCoreApplication::applicationName()),
                            QObject::tr("config"));
  parser.addOption(cfgOpt);

  QCommandLineOption copyOpt(QStringList("copy-files"),
                             QObject::tr("Copy all airport files to the given "
                                         "<filepath> (keeping path structure)"),
                             QObject::tr("filepath"));
  parser.addOption(copyOpt);

  // Process the actual command line arguments given by the user
  parser.process(*QCoreApplication::instance());

  // Simulator Type ===================================================
  atools::fs::FsPaths::SimulatorType type = atools::fs::FsPaths::FSX;
  if(parser.isSet(fstypeOpt))
  {
    type = atools::fs::FsPaths::stringToType(parser.value(fstypeOpt));
    opts.setSimulatorType(type);
  }

  qInfo() << "Using simulator type" << atools::fs::FsPaths::typeToShortName(type);

  // Base path ===================================================
  QString basepath = parser.value(basepathOpt);
  if(basepath.isEmpty())
    basepath = atools::fs::FsPaths::getBasePath(type);

  if(!checkDir(basepath, "Base path: "))
    exit(1);
  opts.setBasepath(basepath);

  // Scenery.cfg ===================================================
  if(type != atools::fs::FsPaths::XPLANE11)
  {
    QString sceneryFile = parser.value(sceneryOpt);
    if(sceneryFile.isEmpty())
      sceneryFile = atools::fs::FsPaths::getSceneryLibraryPath(type);
    if(!checkFile(sceneryFile, "Scenery file: "))
      exit(1);

    opts.setSceneryFile(sceneryFile);
  }

  copyFilePath = parser.value(copyOpt);

  // Configuration file ===================================================
  QString configFile = parser.value(cfgOpt);
  if(configFile.isEmpty())
    // Command line overrides resource settings file
    configFile = ":/navdatareader/resources/config/navdatareader.cfg";
  if(!checkFile(configFile, "Config file: "))
    exit(1);

  qInfo() << "===== Configuration" << configFile << "=====";

  QSettings settings(configFile, QSettings::IniFormat);

  opts.loadFromSettings(settings);
  if(parser.isSet(verboseOpt))
    // Let command line override settings file
    opts.setVerbose(parser.isSet(verboseOpt));

  // Create database ===================================================
  db = SqlDatabase(settings, "Database");
  QString databaseName = parser.value(databaseOpt);
  if(!databaseName.isEmpty())
    db.setDatabaseName(databaseName);

  qInfo() << "===== Database" << databaseName << "=====";
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

void NavdataReader::copyFiles()
{
  atools::sql::SqlQuery query(&db);

  query.exec("select filepath from bgl_file");
  while(query.next())
  {
    QString filepath = query.value("filepath").toString();

    if(!filepath.isEmpty())
    {
      QString destPath(filepath);
#if defined(Q_OS_WIN32)
      if(destPath.at(1) == ':')
        destPath.remove(1, 1);
#endif

      QString destFilename = copyFilePath + QDir::separator() + destPath;
      QString destDir = QFileInfo(destFilename).absolutePath();
      if(!QDir(destDir).mkpath(destDir))
        qWarning() << "Error creating directory" << destDir;

      QFile file(filepath);
      qInfo() << "Copying file" << file.fileName() << "to" << destFilename;
      if(!file.copy(destFilename))
        qWarning() << "Error copying file" << file.fileName() << "to" << destFilename;
    }
  }
}
