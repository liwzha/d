% multiply a tensor with a matrix on mode k

function Y = tensorkmat(X, A, k)
sz = size(X);
sz(k) = size(A,1);
Y = A*flatten(X, k);
Y = kfold(Y, sz, k);

