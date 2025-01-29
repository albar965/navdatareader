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

#include "exception.h"
#include "geo/calculations.h"
#include "gui/consoleapplication.h"
#include "io/fileroller.h"
#include "logging/logginghandler.h"
#include "navdatareader.h"
#include "util/crashhandler.h"
#include "settings/settings.h"

#include <QFileInfo>
#include <QDir>

/*
 *  .Options:
 *  .  -h, --help                               Displays this help.
 *  .  -v, --version                            Displays version information.
 *  .  -f, --flight-simulator <simulator>       Required option. Flight simulator
 *  .                                           type <simulator> or other data
 *  .                                           source. Either FSX, FSXSE, P3DV2,
 *  .                                           P3DV3, P3DV4, P3DV5, P3DV6, XP11,
 *  .                                           XP12, MSFS or DFD.
 *  .  -s, --scenery <scenery>                  Scenery.cfg file <scenery> for FSX
 *  .                                           and P3D. Determined by registry
 *  .                                           entries if not given.
 *  .  -d, --source-database <source-database>  Required option for DFD compilation.
 *  .                                           Source database <source-database> for
 *  .                                           DFD based compilation.
 *  .  -b, --basepath <basepath>                Flight simulator basepath for BGL
 *  .                                           files <basepath>. Determined by
 *  .                                           registry entries if not given.
 *  .  -o, --output <output>                    Output Sqlite database filename
 *  .                                           <output> Default is "navdata.sqlite".
 *  .  -c, --config <config>                    Configuration file <config> for
 *  .                                           Navdatareader. Default is to use
 *  .                                           integrated "navdatareader.cfg".
 */
int main(int argc, char *argv[])
{
  // Initialize the resources from atools static library
  Q_INIT_RESOURCE(atools);
#ifdef SIMCONNECT_BUILD_WIN64
  Q_INIT_RESOURCE(navdata);
#endif
  atools::fs::FsPaths::intitialize();

  atools::geo::registerMetaTypes();

  int retval = 0; // Success is default
  atools::gui::ConsoleApplication app(argc, argv);
  QCoreApplication::setApplicationName("Navdatareader");
  QCoreApplication::setOrganizationName("ABarthel");
  QCoreApplication::setOrganizationDomain("littlenavmap.org");
  QCoreApplication::setApplicationVersion(VERSION_NUMBER_NAVDATAREADER);

  atools::logging::LoggingHandler::initialize(atools::settings::Settings::getOverloadedLocalPath(
                                                ":/navdatareader/resources/config/logging.cfg"));

  // Initialize crashhandler - disable on Linux to get core files
  atools::util::crashhandler::init();
  atools::util::crashhandler::setStackTraceLog("navdatareader-stacktrace.txt");

  NavdataReader reader;

  try
  {
    // Read the scenery.cfg, read all scenery areas and BGL files and store them in the Sqlite database
    reader.run();

    if(reader.getResultFlags().testFlag(atools::fs::COMPILE_BASIC_VALIDATION_ERROR))
      retval = 1;
  }
  catch(const atools::Exception& e)
  {
    qCritical() << "*** Compilation failed";
    qCritical() << "Caught atools::Exception " << e.what();
    retval = 1;
  }
  catch(const std::exception& e)
  {
    qCritical() << "*** Compilation failed";
    qCritical() << "Caught std::exception " << e.what();
    retval = 1;
  }
  catch(...)
  {
    qCritical() << "*** Compilation failed";
    qCritical() << "Caught unknown exception";
    retval = 1;
  }

  if(retval == 1)
  {
    // Failed - rename output file and add "BROKEN" suffix to avoid accidental use
    QFileInfo filename = reader.getDatabaseName();
    QString newName = filename.absolutePath() + QDir::separator() + filename.baseName() + "_BROKEN." + filename.completeSuffix();

    // Create backup copies of broken files
    // little_navmap_navigraph_BROKEN.sqlite
    // little_navmap_navigraph_BROKEN.sqlite.1
    atools::io::FileRoller roller(2);
    roller.rollFile(newName);

    // Rename output file
    qWarning() << "Renaming output file from" << filename << "to" << newName;
    if(!QFile::rename(reader.getDatabaseName(), newName))
      qWarning() << "*** Renaming failed";
  }

  atools::util::crashhandler::deInit();
  qInfo() << "done.";
  atools::logging::LoggingHandler::shutdown();

  return retval;
}
