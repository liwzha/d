% mode-k folding (inverse of mode-k unfolding -- flatten)
% tested? - Yes.
% args:
%   - A: a matrix
%   - sz: size of the tensor to fold into
%   - k: mode of fold
% return:
%   - X: a tensor with size sz.

function X = kfold(A, sz, k)

if sz(k) ~= size(A,1) || prod(sz) ~= numel(A)
    error('size of input matrix A is incompatible with sz (or k)');
end
nd = length(sz);
sz2 = sz([k:nd, 1:k-1]);
X = reshape(A, sz2);
X = permute(X, [(nd-k+2):nd, 1:(nd-k+1)]);


