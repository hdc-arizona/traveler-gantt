Traveler Gantt Chart
====================

Traveler Gantt Chart is an under-development OTF2 Viewer for [HPX Parallel
Runtime](https://github.com/STEllAR-GROUP/hpx) with
[APEX](https://khuck.github.io/xpress-apex) support.

Installation
------------
Traveler Gantt depends on:
- [Open Trace Format version 2 1.4+](http://www.vi-hps.org/projects/score-p/)
- [nlohmann's json](http://github.com/nlohmann/json)
- (Install only) [cmake 2.8.9+](http://www.cmake.org/download/)
- (Optional) [Open Trace Format 1.12+](http://tu-dresden.de/die_tu_dresden/zentrale_einrichtungen/zih/forschung/projekte/otf/index_html/document_view?set_language=en)

To install:

    $ git clone https://github.com/hdc-arizona/traveler-gantt.git
    $ mkdir traveler-gantt/build
    $ cd traveler-gantt/build
    $ cmake -DCMAKE_INSTALL_PREFIX=/path/to/install/directory ..
    $ make
    $ make install

If a dependency is not found, add its install directory to the
`CMAKE_PREFIX_PATH` environment variable.

Usage
-----

Navigate to your installation location (alternatively you can run it out of
the build directory) and run Traveler. Use the `-t` option to specify the path
to your `OTF2` file. The file should end in `.otf2`.

```./Traveler -t /path/to/your/OTF2/file```

For tool tips with extra performance information, use the `-e` option:

```./Traveler -e -t /path/to/your/OTF2/file```

This will launch a webpage on `http://localhost:10006`. Navigate there in a
web browser to view the trace.

Authors
-------
Traveler Gantt was written by Kate Isaacs. It was forked from
[Ravel](https://github.com/LLNL/ravel.git).

License
-------
Traveler Gantt is released under the LGPL license. For more details see the LICENSE
file.

