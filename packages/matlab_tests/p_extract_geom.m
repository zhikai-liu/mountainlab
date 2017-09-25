% mp-declare-processor matlab_tests.extract_geom
% mp-declare-version 0.1
% mp-declare-description Extract a subset of channels from a geom.csv file
% mp-declare-input geom
% mp-declare-output geom_out
% mp-declare-parameter channels
function p_extract_geom(geom,geom_out,params)
G=csvread(geom);
chans=str2num(params.channels);
G=G(chans,:);
csvwrite(geom_out,G);