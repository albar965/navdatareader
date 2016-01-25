#include "fs/bglreaderoptions.h"
#include "fs/scenery/scenerycfg.h"
#include "sql/sqldatabase.h"
#include "sql/sqlscript.h"
#include "sql/sqlquery.h"
#include "fs/writer/datawriter.h"
#include "fs/writer/routeresolver.h"
#include "fs/scenery/sceneryarea.h"
#include "sql/sqlutil.h"

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDateTime>
#include "logging/loggingdefs.h"

void parseArgs(atools::fs::BglReaderOptions& opts, QString& databaseName);

int main(int argc, char *argv[])
{
  // Initialize the resources from atools static library
  Q_INIT_RESOURCE(atools);

  int retval = 0;
  QCoreApplication app(argc, argv);
  QCoreApplication::setApplicationName("Navdata Reader");
  QCoreApplication::setOrganizationName("ABarthel");
  QCoreApplication::setOrganizationDomain("abarthel.org");
  QCoreApplication::setApplicationVersion("0.5.0");

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

    QTime timer;
    timer.start();

    SceneryCfg cfg;
    cfg.read(options.getSceneryFile());

    SqlDatabase db = SqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(databaseName);
    db.open();

    SqlScript script(&db);
    script.executeScript(":/atools/resources/sql/writer/create_nav_schema.sql");
    script.executeScript(":/atools/resources/sql/writer/create_ap_schema.sql");
    script.executeScript(":/atools/resources/sql/writer/create_meta_schema.sql");
    script.executeScript(":/atools/resources/sql/writer/create_views.sql");
    db.commit();

    SqlQuery(db).exec("PRAGMA foreign_keys = ON");
    db.commit();

    atools::fs::writer::DataWriter dataWriter(db, options);

    for(const atools::fs::scenery::SceneryArea& area : cfg.getAreas())
      if(area.isActive())
        dataWriter.writeSceneryArea(area);
    db.commit();

    // atools::fs::writer::RouteResolver resolver(db);
    // resolver.run();
    // db.commit();

    script.executeScript(":/atools/resources/sql/writer/finish_schema.sql");
    db.commit();

    dataWriter.logResults();
    QDebug info(QtInfoMsg);
    atools::sql::SqlUtil(&db).printTableStats(info, true);

    db.close();

    qInfo() << "Time" << timer.elapsed() / 1000 << "seconds";
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

  // addOption("no-delete", 'd', "Do not process airport deletes", noDeletes);
  // addOption("no-filter-runways", 'f', "Do not filter out dummy runways", noFilterRunways);
  // addOption("no-incomplete", 'i', "Do not write incomplete objects", noIncomplete);
  // addOption("file-filter", 0, "Filter files by regular expression", fileFilterRegexpStr);
  // addOption("airport-icao-filter", 0, "Filter airports by regular expression for ICAO",

  // Process the actual command line arguments given by the user
  parser.process(*QCoreApplication::instance());

  bool verbose = parser.isSet(verboseOpt);
  qInfo() << "Verbose" << verbose;
  QString sceneryFile = parser.value(sceneryOpt);
  qInfo() << "Scenery.cfg file" << sceneryFile;
  QString configFile = parser.value(cfgOpt);
  qInfo() << "Configuration file" << configFile;
  QString basepath = parser.value(basepathOpt);
  qInfo() << "Basepath" << basepath;
  databaseName = parser.value(databaseOpt);
  qInfo() << "Database" << databaseName;

  if(sceneryFile.isEmpty())
  {
    qCritical() << "Scenery path not set";
    exit(1);
  }

  if(basepath.isEmpty())
  {
    qCritical() << "Base path not set";
    exit(1);
  }
  opts.setSceneryFile(sceneryFile);
  opts.setBasepath(basepath);
  opts.setVerbose(verbose);
}
