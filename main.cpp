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

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QElapsedTimer>
#include <QFileInfo>
#include <QSettings>

void parseArgs(atools::fs::BglReaderOptions& opts, QString& databaseName);
bool checkDir(const QString& path, const QString& msg);
bool checkFile(const QString& path, const QString& msg);

int main(int argc, char *argv[])
{
  // Initialize the resources from atools static library
  Q_INIT_RESOURCE(atools);

  int retval = 0;
  QCoreApplication app(argc, argv);
  Q_UNUSED(app);
  QCoreApplication::setApplicationName("Navdata Reader");
  QCoreApplication::setOrganizationName("ABarthel");
  QCoreApplication::setOrganizationDomain("abarthel.org");
  QCoreApplication::setApplicationVersion("0.5.0");

  atools::logging::LoggingHandler::initialize(atools::settings::Settings::getOverloadedLocalPath(
                                                ":/navdatareader/resources/config/logging.cfg"));

  QString databaseName;
  atools::fs::BglReaderOptions options;
  parseArgs(options, databaseName);
  // -------------------------------------------------------------------------------
  // -------------------------------------------------------------------------------
  // -------------------------------------------------------------------------------
  qInfo() << "Starting ...";
  try
  {
    using atools::fs::scenery::SceneryCfg;
    using atools::sql::SqlDatabase;
    using atools::sql::SqlScript;
    using atools::sql::SqlQuery;

    qInfo() << options;

    SqlDatabase db = SqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(databaseName);
    db.open();

    atools::fs::Navdatabase nd(options, &db);
    nd.create();

    db.close();

  }
  catch(const atools::Exception& e)
  {
    qCritical() << "Caught db::DatabaseException " << e.what();
    retval = 1;
  }
  catch(const std::exception& e)
  {
    qCritical() << "Caught std::exception " << e.what();
    retval = 1;
  }
  catch(...)
  {
    qCritical() << "Caught other";
    retval = 1;
  }

  qInfo() << "done.";
  atools::logging::LoggingHandler::shutdown();
  return retval;
}

void parseArgs(atools::fs::BglReaderOptions& opts, QString& databaseName)
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

  databaseName = parser.value(databaseOpt);
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

  settings.beginGroup("Options");
  opts.setDeletes(settings.value("ProcessDelete", true).toBool());
  opts.setFilterRunways(settings.value("FilterRunways", true).toBool());
  opts.setIncomplete(settings.value("SaveIncomplete", true).toBool());
  settings.endGroup();

  settings.beginGroup("Filter");

  opts.setFilenameFilterInc(settings.value("IncludeFilenames").toStringList());
  opts.setFilenameFilterExcl(settings.value("ExcludeFilenames").toStringList());
  opts.setPathFilterInc(settings.value("IncludePathFilter").toStringList());
  opts.setPathFilterExcl(settings.value("ExcludePathFilter").toStringList());
  opts.setAirportIcaoFilterInc(settings.value("IncludeAirportIcaoFilter").toStringList());
  opts.setAirportIcaoFilterExcl(settings.value("ExcludeAirportIcaoFilter").toStringList());
  opts.setBglObjectFilterInc(settings.value("IncludeBglObjectFilter").toStringList());
  opts.setBglObjectFilterExcl(settings.value("ExcludeBglObjectFilter").toStringList());
  settings.endGroup();
}

bool checkFile(const QString& path, const QString& msg)
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

bool checkDir(const QString& path, const QString& msg)
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
