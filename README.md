<<<<<<< HEAD
# BIC TB Simulation
Repository for GEANT4 simulation &amp; analysis of BIC TB simulation.

## How-to
### Compile
After fetching the repository, do
    
    cd install
    source envset.sh
    cd ../build
    cmake -DCMAKE_INSTALL_PREFIX=<path_to_install_directory> ..
    make -j4 install

### Analysis

    ./bin/analysis <path_to_root_files> <low_edge_of_hist> <high_edge>

e.g.)

    ./bin/analysis /home/USER/20GeV_ele_data 0 20
=======
# DRC_TB2024_MCP
TB2024 MCP Simulation
>>>>>>> 0d65794fcd4e6b3c8c0d1751eda4cb329014abd1
