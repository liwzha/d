% vectorize a tensor

function v=vec(X)
v = reshape(X, [numel(X), 1]);