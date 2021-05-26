# Jet Visualization
The Jet Visualization comes with two tools: The commandline tool and the viewer.
With the commandline tool, jet stream core lines can be extracted and the viewer can then be used to visualize the extraced core lines in context of the surrounding athmosphere. 

## Example Usage
**Commandline tool** 

`./jet_cmd <Path to the data> <Output path> [Optional Parameters]`

Under *Demo/Data* are ERA5 example data for two time steps.

Optional Parameters:

`-preprocess`
Preprocessses the data in the source directory for the viewer.

`-export_txt`
Exports the jet core lines in human readable form to a .txt file. By default the lines are exported in binary format.

`-pMin`
[10, 1040][hPa], Default: 190.0, smaller than pMax. Sets the lower pressure level boundary for the region of interest. Only seeds above pMin will be considered. 

`-pMax`
[10, 1040][hPa], Default: 350.0, larger than pMin. Sets the upper pressure level boundary for the region of interest. Only seeds below pMax will be considered.

`-windspeedThreshold`
[0, inf), Default: 40.0, the wind magnitude threshold criteria used to identify jet stream cores.

`-integrationStepsize`
[0, inf), Default: 0.04, the integration step size for numerical integration. Smaller values result in smoother lines with more vertexes.

`-nStepsBelowThreshold`
[0 ... inf)(integer), Default: 100, the number of steps the jet core line is allowed to be below the windspeedThreshold.

`-nPredictorSteps`
[0 ... inf)(integer), Default: 1, the number of predictor steps per iteration.

`-nCorrectorSteps`
[0 ... inf)(integer), Default: 5, the number of corrector steps per iteration.

For example:  `./jet_cmd /home/usr/Desktop/Data /home/usr/Desktop/JetDir -nCorrectorSteps 10 -nPredictorSteps 2 -pMax 1000`

**Viewer** 

Preprocess data and compute jet core lines: `./jet_cmd <Path to the data> <Output path> -preprocess`

Run viewer: `./jet_viewer`

## Installation Linux
The commandline-tool requires VTK-9.0.1 and the viewer additionally requires Qt-5.15.1.

**commandline-tool**

Install dependencies
Ubuntu:
```
sudo apt install build-essential gdb cmake libglu1-mesa-dev freeglut3-dev mesa-common-dev libtbb-dev libnetcdf-dev libnetcdff-dev g++-10
```
OpenSUSE Packages with YaST2: 
```
cmake-full patterns-devel-C-C++-devel_C_C++ freeglut-devel libx11-devel  tbb-devel netcdf-devel gcc10-c++
```
[Download](https://vtk.org/download/) and build VTK

```
cd VTK-9.0.1
mkdir build
cd build
cmake ..
make
```

Build project
```
cd iac-visualization
mkdir build
cd build
cmake -G"Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -DVTK_DIR=<Path_to_VTK>/build -DCMAKE_C_COMPILER=gcc-10 -DCMAKE_CXX_COMPILER=g++-10 ..
make
chmod +x jet_cmd
```
---
**commandline-tool and viewer**

Install dependencies
Ubuntu:
```
sudo apt install build-essential gdb cmake libglu1-mesa-dev freeglut3-dev mesa-common-dev libtbb-dev libnetcdf-dev libnetcdff-dev libxcb-randr0-dev libxcb-xtest0-dev libxcb-xinerama0-dev libxcb-shape0-dev libxcb-xkb-dev g++-10
```
OpenSUSE Packages with YaST2: 
```
cmake-full patterns-devel-C-C++-devel_C_C++ freeglut-devel libx11-devel  tbb-devel netcdf-devel gcc10-c++
```

[Download](https://www.qt.io/download-qt-installer) and Install Qt
* Start qt-unified-linux-x64-3.2.3-online.run
* next
* log in or create Account ->next
* Accept check boxes
* next
* Disable or enable sending statistics ->next
* Name the installation directory "Qt" ->next
* check the box: Qt 5.15.1 -> next
* accept agreement -> next -> install

[Download](https://vtk.org/download/) and build VTK

```
cd VTK-9.0.1
mkdir build
cd build
cmake -DVTK_GROUP_ENABLE_Qt=YES -DQt5_DIR=<Path_to_QT>/5.15.1/gcc_64/lib/cmake/Qt5 -DVTK_Group_Qt=YES -DVTK_MODULE_ENABLE_VTK_GUISupportQt=YES -DCMAKE_BUILD_TYPE=Release -DQTDIR=<Path_to_QT>/5.15.1/gcc_64 ..
make
```

Build project
```
cd iac-visualization
mkdir build
cd build
cmake -G"Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -DVTK_DIR=<Path_to_VTK>/build -DUSE_QT=ON -DQTDIR=<Path_to_QT>/5.15.1/gcc_64 -DCMAKE_C_COMPILER=gcc-10 -DCMAKE_CXX_COMPILER=g++-10 ..
make
chmod +x jet_cmd
chmod +x jet_viewer
cd ../assets
cp map.jpg mapNorth.jpg mapSouth.jpg ../build/
```

Troubleshooting

In case of shading errors when executing the viewer:
`glxinfo | grep "ES profile shading language version string:"`
Note the last number e.g 3.10
`export MESA_GL_VERSION_OVERRIDE=3.10`

## Installation Windows
The commandline-tool requires VTK-9.0.1 and the viewer additionally requires Qt-5.15.1.

[Download](https://www.qt.io/download-qt-installer) and Install Qt
* Start qt-unified-linux-x64-3.2.3-online.run
* next
* log in or create Account ->next
* Accept check boxes
* next
* Disable or enable sending statistics ->next
* Name the installation directory "Qt" ->next
* check the box: Qt 5.15.1 -> next
* accept agreement -> next -> install

[Download](https://vtk.org/download/) and build VTK from source using [CMake GUI](https://cmake.org/download/):
* Configure
* Enable Qt: *VTK_GROUP_ENABLE_Qt = YES*
* Set Qt5 directory: *QT5_Dir = <Path_to_Qt>\5.15.1\msvc2017_64\lib\cmake\Qt5*  
* Generate
* Open Project
* Choose release mode and build

Build the project with CMake GUI
* Configure
* Set VTK directory: *VTK_DIR = <Path_to_VTK>\build* 
* Set use Qt: *USE_QT = checked*
* Set Qt directory: *QTDIR = <Path_to_Qt>\5.15.1\msvc2017_64*
* Generate
* Open Project

Copy the contents of the assets folder to build/viewer

Adjust the data source and preprocessing directory path in settings.txt according to your system.

Use Visual Studio to build and run the project:

Set the path for each project, jet_cmd, jet_core, jet_viewer to include VTK:

*Project->Properties->Configuration Properties->Debugging->Environment*
* Set the environment path to: *PATH=%PATH;<Path_to_VTK>\bin\\$(Configuration)*
* Set *Merge Environments = Yes*

The commandline arguments of jet_cmd can be set under:

*jet_cmd->Properties->Configuration Properties->Debugging->Command Arguments*

Set there the same data source and preprocessing directory path as in settings.txt of the viewer. Add the *-preprocess* flag to preprocess the data for the viewer. The viewer can be used once the preprocessing is done.
