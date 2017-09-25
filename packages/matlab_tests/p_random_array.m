% mp-declare-processor matlab_tests.random_array
% mp-declare-version 0.1
% mp-declare-description Create a random multidimensional array
% mp-declare-output output
% mp-declare-parameter dims
% mp-declare-parameter distribution
function [exit_code,errstr]=p_random_array(output,params)
if (nargin==0), run_test; return; end;

dims0=str2num(params.dims);
if (strcmp(params.distribution,'uniform'))
    X=rand(dims0);
elseif (strcmp(params.distribution,'normal'))
    X=randn(dims0);
else
    errstr=sprintf('Unsupported distribution: %s',params.distribution);
    exit_code=-1;
    return;
end;
writemda32(X,output);
errstr='';
exit_code=0;

function run_test
[ecode,estr]=p_random_array('tmp.mda',struct('dims','3,6','distribution','normal'))
X=readmda('tmp.mda');
figure; imagesc(X');