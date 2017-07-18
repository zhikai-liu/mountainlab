function [phi psi] = blendpythag(t,w)
% BLENDPYTHAG - evaluate complementary blending functions at time points
%
% [phi psi] = blendpythag(t,w)
% Inputs:
%  t = vector of time points. The blending region is [0 w].
%  w = width of blending function in time samples
% Outputs:
%  phi, psi = phi(t) and psi(t) evaluated at the vector t, same sizes as t.
%
%  Pythagorean blending is such that phi(t)^2 + psi(t)^2 = 1, for all t
%  and phi=0 for t<0, psi=0 for t>w.
%  phi and psi are currently only C^1 smooth, should be sufficient for blending
%  of noise.
%  This is a one-way blending function; to make a two-way function centered
%  at t=w, use [phi psi] = blendpythag(t-2*max(0,t-w),w);
%
% Barnett 3/29/17
if nargin==0, test_blendpythag; return; end

t = min(max(t,0),w);  % clip
th = sin((pi/(2*w))*t).^2;   % values in 
phi = sin((pi/2)*th);
psi = cos((pi/2)*th);

%%%%%%
function test_blendpythag
w = 10;
t = -10:20;
[phi psi] = blendpythag(t,w);
figure; plot(t, [phi;psi], '.-'); legend('phi','psi');
