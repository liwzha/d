% mode-k unfolding of a tensor
%-------------------------------
% This is a modified version of function flatten in tensor package.
% Difference: the order of mode in unfolding.
% Order in flatten: k, k+1, ..., K, 1, 2, ..., k-1
% Order in kunfold: k, 1, 2, 3, ..., k-1, k+1, ..., K
%-------------------------------

function Z=kunfold(X,ind)
nd=ndims(X);
ind = {ind, 1:ind-1, ind+1:nd };
X=permute(X,cell2mat(ind));
Z=X(:,:);
