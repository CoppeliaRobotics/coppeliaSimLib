Building V-REP
==============

Instructions:

 - Clone this repository into an empty folder, e.g. `myBuild`
 - Clone the [`include`](https://github.com/CoppeliaRobotics/include.git), [`common`](https://github.com/CoppeliaRobotics/common.git) and [`v_repMath`](https://github.com/CoppeliaRobotics/v_repMath.git) repositories into `myBuild/programming`
 
 Make sure that following dependencies are present/installed:
 
 - `Lua5.1`
 - The `Boost C++ library`
 - [`QScintilla2`](http://www.coppeliarobotics.com/helpFiles/index.html)
 - [`Qt`](https://www.qt.io/download-qt-for-application-development)5.9 or above
 
 Compile `Lua5.1` (e.g. via `make`) and `QScintilla2` (via `qmake` or with [`Qt Creator`](http://wiki.qt.io/Qt_Creator)). Then adjust the content of `myBuild/v_rep/config.pri`
 
 Then, in order to have V-REP's dynamics and mesh calculation functionality available, you will need to clone and compile following 2 plugins/libraries into folder `myBuild`:
 
 - The [`dynamicsPlugin`](https://github.com/CoppeliaRobotics/dynamicsPlugin.git) (note: the [`dynamicsPlugin` license](https://github.com/CoppeliaRobotics/dynamicsPlugin/blob/master/license.txt) differs from the V-REP license)
 - The [`meshCalculationPlugin`](https://github.com/CoppeliaRobotics/meshCalculationPlugin.git) (note: the [`meshCalculationPlugin` license](https://github.com/CoppeliaRobotics/meshCalculationPlugin/blob/master/license.txt) differs from the V-REP license)
 
Finally, make sure to also compile/use the various V-REP plugins that your projects might need. Those can be found [here](https://github.com/CoppeliaRobotics)
