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

#include "logging/logginghandler.h"
#include "logging/loggingutil.h"
#include "settings/settings.h"
#include "navdatareader.h"
#include "exception.h"
#include "gui/consoleapplication.h"

#include <QDebug>
#include <QCoreApplication>

int main(int argc, char *argv[])
{
  // Initialize the resources from atools static library
  Q_INIT_RESOURCE(atools);

  int retval = 0;
  atools::gui::ConsoleApplication app(argc, argv);
  QCoreApplication::setApplicationName("Navdatareader");
  QCoreApplication::setOrganizationName("ABarthel");
  QCoreApplication::setOrganizationDomain("littlenavmap.org");

  QCoreApplication::setApplicationVersion("1.0.16"); // VERSION_NUMBER

  try
  {
    // Read the scenery.cfg, read all scenery areas and BGL files and store them in the Sqlite database
    NavdataReader reader;
    reader.run();
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

  qInfo() << "done.";
  atools::logging::LoggingHandler::shutdown();
  return retval;
}
