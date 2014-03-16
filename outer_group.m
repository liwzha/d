function Z = outer_group(varargin)

nd = length(varargin);
sz = zeros(1, nd);
nz = zeros(1,nd);
for ii=1:nd
    sz(ii) = size(varargin{ii}, 1);
    nz(ii) = size(varargin{ii}, 2);
end

Z = cell(nz);
for n=1:numel(Z)
    
    
end

X = ones(sz);
for kk=1:nd
    repsz = [sz(1:(kk-1)), 1, sz((kk+1):nd)];
    fibersz = ones(1,nd);
    fibersz(kk) = sz(kk);
    fiber = reshape(varargin{kk}, fibersz);
    X = X.*repmat(fiber, repsz);
end