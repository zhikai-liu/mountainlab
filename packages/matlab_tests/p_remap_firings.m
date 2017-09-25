% mp-declare-processor matlab_tests.remap_firings
% mp-declare-version 0.1
% mp-declare-description Remap the times in a firings file according to input timestamps vector
% mp-declare-input firings
% mp-declare-input timestamps
% mp-declare-output firings_out
function p_remap_firings(firings,timestamps,firings_out,params)
FF=readmda(firings);
TT=readmda(timestamps);
FF(2,:)=TT(floor(FF(2,:)));
writemda64(FF,firings_out);