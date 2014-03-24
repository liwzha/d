% compute the multilinear-rank of a tensor

function r = mlrank(X)

nd = length(size(X));
r = zeros(1,nd);

for kk = 1:nd
    r(kk) = rank(kunfold(X,kk));
end


