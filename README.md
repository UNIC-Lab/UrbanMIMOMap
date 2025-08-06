# UrbanMIMOMap
This repository provides access to the dataset described in the paper "UrbanMIMOMap: A Ray-Traced MIMO CSI Dataset with Precoding-Aware Maps and Benchmarks", along with simulation scripts for reproducibility. It includes instructions for dataset acquisition, code for channel modeling, and benchmarking tools used in the study.

## Citation
Our paper, "UrbanMIMOMap: A Ray-Traced MIMO CSI Dataset with Precoding-Aware Maps and Benchmarks," has been accepted by the **IEEE Global Communications Conference (GlobeCom) 2025**.

The full citation will be provided here once the paper is published in the conference proceedings. Please stay tuned for updates.

```
[Citation details will be updated here]
```

## Code Description
The simulation and analysis code is located in the `UrbanMIMOMap_Code` folder. Due to size limitations, this repository only contains a few sample files. The complete code package can be downloaded from the links below.

**Download Links for Complete Code:**
* [Link 1: TBD]
* [Link 2: TBD]

The code folder is organized as follows:

### 1. `DatasetSimulation`
This directory contains the C++ project used to generate the ray-tracing dataset.

```
UrbanMIMOMap_Code/
└── DatasetSimulation/
    ├── antenna_txt/          # Sample antenna coordinate files
    ├── odb_files/            # Sample simulation files in .odb format
    ├── CMakeLists.txt
    ├── matrix2npz.exe
    ├── outdoor_propagation_runms_mimo.cpp
    └── outdoor_propagation_runms_mimo.h
```
* `antenna_txt/`: Contains coordinate files for antenna placements.
* `odb_files/`: Stores the environment models in `.odb` format for the simulation.
* The other files are part of the C++ project for running the simulation and processing the output.

### 2. `UrbanMIMOMap_case1`
This directory contains the Python simulation code for "Case 1" (RM Generation and Analysis) as discussed in the paper. It includes the implementation of the RadioUNet benchmark.

```
UrbanMIMOMap_Code/
└── UrbanMIMOMap_case1/
    ├── antennas/             # Sample antenna data
    ├── env_diffH_withcars/   # Sample environment data
    ├── gain/                 # Sample gain data
    └── RadioUnetMIMO/        # Python project files for the RadioUNet model
```
* `antennas/`, `env_diffH_withcars/`, `gain/`: These folders contain sample data files required to run the Python simulation.
* `RadioUnetMIMO/`: Contains the Python source code for the deep learning model and analysis scripts.

## Dataset Description
The complete channel state information (CSI) dataset is stored in `.npz` format. The dataset is organized into 350 scenarios (maps), with each scenario containing data for 120 transmitter-receiver configurations.

Due to the large size of the dataset, this repository only contains a small subset of the files as examples. The complete dataset can be downloaded from the following links:

**Download Links for Complete Dataset:**
* [Link 3: TBD]
* [Link 4: TBD]

### Directory Structure
The dataset is structured as follows:

```
channelMatrix/
├── resu_npz_map_0/
│   ├── 0_0_0.npz
│   ├── 0_3_120.npz
│   └── ... (120 .npz files in total)
├── resu_npz_map_1/
│   └── ...
├── ...
└── resu_npz_map_349/
    └── ...
```
* There are 350 folders, from `resu_npz_map_0` to `resu_npz_map_349`.
* Each folder contains 120 `.npz` files, where each file represents a specific configuration and contains the corresponding 4x4 complex MIMO channel matrix **H**.
