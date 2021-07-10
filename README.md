# Jet Core Extraction
The Jet Core Extraction tool extracts jet stream core lines from ERA5 data using a predictor corrector algorithm.

## Example Usage

```
./jet_cmd <data_source_path> <output_path> [Optional Parameters]
```
e.g
```
./jet_cmd /home/usr/Desktop/Data /home/usr/Desktop/JetDir -nCorrectorSteps 10 -nPredictorSteps 2 -pMax 1000
```

Under *demo_data* are ERA5 example data for one time step. If no arguments are given, the demo data will be used. Adjust jet_cmd.cpp lines 128-135 if the demo data is not found on your system.

The ERA5 data is courtesy of the European Centre for Medium-Range Weather Forecasts (ECMWF) and is documented here:
  https://confluence.ecmwf.int/display/CKB/ERA5%3A+data+documentation
The data is available under the Copernicus License Agreement:
  https://cds.climate.copernicus.eu/api/v2/terms/static/licence-to-use-copernicus-products.pdf

**Input**

ERA5 data files are used as input with the following name structure: *P<date_time>*. e.g *P20160901_00*. The data needs to be in netcdf format and contain the following fields:
1. U - eastward wind
2. V - northwards wind
3. Omega - lagrangian tendency of air pressure
4. T - air temperature
5. lev - hybrid sigma pressure levels
6. hyam - hybrid A coefficient at layer midpoints
7. hybm - hybrid B coefficient at layer midpoints
8. PS - Surface pressure

**Output**

The default output is a .vtp file containing the jet core lines. This file can be imported for visualization to Paraview. Alternatively the lines can be exported in ASCII format as .txt file. The point coordinates are in the following format: lon: [0, 720) (0.5' longitude), lat: [0, 361) (0.5' latitude), pressure: (10 hPa).


**Optional Parameters:**

`-exportTxt`
Exports the jet core lines in ASCII format to a .txt file.

`-pMin`
[10, 1040][hPa], Default: 190.0, smaller than pMax. Sets the lower pressure level boundary for the region of interest.

`-pMax`
[10, 1040][hPa], Default: 350.0, larger than pMin. Sets the upper pressure level boundary for the region of interest.

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

`-recompute`
Recomputes the core lines and overrides existing ones.
## Installation Linux

1. Install dependencies

    Ubuntu:
    ```
    sudo apt install build-essential gdb cmake libnetcdf-dev g++-10 git git-lfs
    ```
    OpenSUSE Packages with YaST2:
    ```
    cmake-full patterns-devel-C-C++-devel_C_C++ f netcdf-devel gcc10-c++ git git-lfs
    ```
2. Install git-lfs
    ```
    git lfs install
    ```
3. Clone repository

4. Build project:
    ```
    cd jet-core-extraction
    mkdir build
    cd build
    cmake ..
    make
    ```
## Installation Windows

Tested for Visual Studio 2019.

1. Install dependencies

    Install pre-computed binaries for NetCDF-4 64bit from https://www.unidata.ucar.edu/downloads/netcdf/

2. Clone repository

3. Build project:
    ```
    cd jet-core-extraction
    mkdir build
    cd build
    cmake ..
    make
    ```