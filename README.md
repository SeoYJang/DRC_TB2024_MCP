# Caution
Only works on enviornment with CVMFS installed on CentOS7. ( ex). KNU CMS server)

# Init
> source /install/envset.sh
>
> cd build
> ccmake ..

While CCMake, change GEANT4_VIS option ON for visualization

> make -j12
>
> cp DRsim/DRsim ../install/bin/DRsim_TB2024_MCP

Submit condor job using install/1run_simulation_TB2024.sh file

# Analysis

> cd build/analysis
>
> ./analysis <path_to_result_rootfile> <histogram_low_range> <histogram_high_range>
