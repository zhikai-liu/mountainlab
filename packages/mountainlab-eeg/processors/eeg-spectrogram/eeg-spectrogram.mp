{
        "processors":[
                {
                        "name":"eeg-spectrogram",
                        "version":"0.11",
                        "description":"Create a spectrogram from timeseries eeg data",
                        "inputs":[{"name":"timeseries"}],
                        "outputs":[{"name":"spectrogram_out"}],
                        "parameters":[],
                        "exe_command":"$(basepath)/run_in_octave.sh 'addpath('\\''$(basepath)/octave'\\''); opts=struct; eeg_spectrogram('\\''$timeseries$'\\'','\\''$spectrogram_out$'\\'',opts);'",
                        "misc_info":"Escape single quotes in JSON octave command using '\\''"
                }
        ]
}
