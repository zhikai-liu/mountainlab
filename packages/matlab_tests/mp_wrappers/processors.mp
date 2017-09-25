{
    "processors": [
        {
            "name": "matlab_tests.extract_geom",
            "version": "0.1",
            "description": "Extract a subset of channels from a geom.csv file",
            "inputs": [
                {
                    "name": "geom"
                }
            ],
            "outputs": [
                {
                    "name": "geom_out"
                }
            ],
            "parameters": [
                {
                    "name": "channels"
                }
            ],
            "exe_command": "$(basepath)/run_matlab.sh $(basepath)/.. \"addpath('$(basepath)'); params=struct('channels','$channels$'); p_extract_geom('$geom$','$geom_out$',params);\""
        },
        {
            "name": "matlab_tests.random_array",
            "version": "0.1",
            "description": "Create a random multidimensional array",
            "inputs": [],
            "outputs": [
                {
                    "name": "output"
                }
            ],
            "parameters": [
                {
                    "name": "dims"
                },
                {
                    "name": "distribution"
                }
            ],
            "exe_command": "$(basepath)/run_matlab.sh $(basepath)/.. \"addpath('$(basepath)'); params=struct('dims','$dims$','distribution','$distribution$'); p_random_array('$output$',params);\""
        },
        {
            "name": "matlab_tests.remap_firings",
            "version": "0.1",
            "description": "Remap the times in a firings file according to input timestamps vector",
            "inputs": [
                {
                    "name": "firings"
                },
                {
                    "name": "timestamps"
                }
            ],
            "outputs": [
                {
                    "name": "firings_out"
                }
            ],
            "parameters": [],
            "exe_command": "$(basepath)/run_matlab.sh $(basepath)/.. \"addpath('$(basepath)'); params=struct(); p_remap_firings('$firings$','$timestamps$','$firings_out$',params);\""
        }
    ]
}