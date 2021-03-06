% the function Gf(q) in "Learning Tensors in RKHS with Multilinear Spectral Penalties"
% the order of permutation is adapted to Ryota's mode-k (un)folding

function P = Gf(q, sz)
ne = prod(sz);
psz = [sz(q:end), sz(1:(q-1))];
idx = zeros(1,ne);
for n = 1:ne
    subs = ind2subs(sz, n);
    subs = [subs(q:end), subs(1:(q-1))];
    ind = subs2ind(psz, subs);
    idx(n) = ind;
end
P = sparse(idx, 1:ne, 1.0);