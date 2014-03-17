function ind = subs2ind(sz, subs)
nd = length(subs);
if nd~=length(sz)
    error('dim of sz does not match dim of subs.');
end

ind = 1;
for kk=1:nd
    ind = ind+(subs(kk)-1)*prod(sz(1:(kk-1)));
end




