Navdatareader is a command line tool that uses the atools fs/bgl
and fs/writer to store a full flight simulator scenery database into
a relational database like Sqlite or MySql.

All BGL features except scenery objects are supported.
* airports including parking, com, fences, aprons, apron lights, helipads,
  start points, taxiways and all runway information.
* Approaches, transitions and the respective legs including coordinates.
* VOR, NDB, ILS, DME, markers and waypoints
* Metadata like BGL files and scenery areas.
* Routes and route waypoints.
* The delete airport records are handled for addon airports.

------------------------------------------------------------------------------
This software is licensed under GPL3 or any later version.

The source code for this application is available at Github:
https://github.com/albar965/atools
https://github.com/albar965/navdatareader

Copyright 2015-2016 Alexander Barthel (albar965@mailbox.org).

