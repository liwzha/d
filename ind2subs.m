function subs = ind2subs(sz, ind)

nd = length(sz);
subs = zeros(1, nd);

for kk=nd:-1:1
    p = prod([1, sz(1:kk-1)]);
    subs(kk) = floor((ind-1)/p)+1;
    ind = ind - (subs(kk)-1)*p;
end