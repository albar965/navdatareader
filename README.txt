Navdatareader is a command line tool that uses the atools fs/bgl
and fs/db modules to store a full flight simulator scenery database into
Sqlite, a relational database.

It can read flight simulator databases from FSX, Prepar3D, MSFS, X-Plane 11
and the DFD format.

------------------------------------------------------------------------------

A short manual is included as PDF in directory "help" or available online here:
https://github.com/albar965/navdatareader/wiki

------------------------------------------------------------------------------

A configuration file includes various settings and filter options to
exclude files, directories, facilities or airports. If the file is not given
the simulator paths will be automatically extracted from the registry and
the conversion process will start.
The default configuration file with comments can be found here:
navdatareader/resources/config/navdatareader.cfg

--------------------
A confguration file to modify logging options can be found here:
navdatareader/resources/config/logging.cfg

--------------------
The database schema is documented in the atools project which contains
the SQL files that create all the needed tables:

* atools/resources/sql/fs/db/create_ap_schema.sql:
  Airports, runways, COM, approaches, transitions and other airport related tables.

* atools/resources/sql/fs/db/create_nav_schema.sql:
  VOR, NDB, ILS, waypoints and airways.

* atools/resources/sql/fs/db/create_boundary_schema.sql:
  Airspace boundaries and related frequencies

* atools/resources/sql/fs/db/create_meta_schema.sql:
  Metadata for BGL files and scenery areas.

* atools/resources/sql/fs/db/create_route_schema.sql:
  Tables needed to route calculation. OBSOLETE - not used in LNM as of 2.6.

------------------------------------------------------------------------------
This software is licensed under GPL3 or any later version.

The source code for this application is available at Github:
https://github.com/albar965/atools
https://github.com/albar965/navdatareader

Copyright 2015-2020 Alexander Barthel (alex@littlenavmap.org).

