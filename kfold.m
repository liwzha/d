%% Old version:
% % mode-k folding (inverse of mode-k unfolding -- flatten)
% % tested? - Yes.
% % args:
% %   - A: a matrix
% %   - sz: size of the tensor to fold into
% %   - k: mode of fold
% % return:
% %   - X: a tensor with size sz.
% 
% function X = kfold(A, sz, k)
% 
% if prod(sz) ~= numel(A)
%     error('size of input matrix A does not match sz');
% end
% nd = length(sz);
% sz2 = sz([k:nd, 1:k-1]);
% X = reshape(A, sz2);
% X = permute(X, [(nd-k+2):nd, 1:(nd-k+1)]);


%% New version:
% mode-k folding (inverse of mode-k unfolding -- kunfold)
% tested? - No.
% args:
%   - A: a matrix
%   - sz: size of the tensor to fold into
%   - k: mode of fold
% return:
%   - X: a tensor with size sz.

function X = kfold(A, sz, k)

if prod(sz) ~= numel(A)
    error('size of input matrix A does not match sz');
end
nd = length(sz);
sz2 = sz([k, 1:k-1, (k+1):nd]);
X = reshape(A, sz2);
X = permute(X, [2:k,1,(k+1):nd]);