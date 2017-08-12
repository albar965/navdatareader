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

#include "logging/logginghandler.h"
#include "logging/loggingutil.h"
#include "settings/settings.h"
#include "navdatareader.h"
#include "exception.h"

#include <QDebug>
#include <QCoreApplication>

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
  QCoreApplication::setApplicationVersion("0.9.7.develop");

  using atools::logging::LoggingHandler;
  LoggingHandler::initialize(atools::settings::Settings::getOverloadedLocalPath(
                               ":/navdatareader/resources/config/logging.cfg"));

  qInfo() << "Starting ...";
  try
  {
    // Read the scenery.cfg, read all scenery areas and BGL files and store them in the Sqlite database
    NavdataReader reader;
    reader.run();
  }
  catch(const atools::Exception& e)
  {
    qCritical() << "Caught atools::Exception " << e.what();
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
