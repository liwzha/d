% outerproduct of vectors, returns a tensor
% tested? - Yes.

function X = outer(varargin)

nd = length(varargin);
sz = zeros(1, nd);
for ii=1:nd
    sz(ii) = length(varargin{ii});
end
X = ones(sz);

for kk=1:nd
    repsz = [sz(1:(kk-1)), 1, sz((kk+1):nd)];
    fibersz = ones(1,nd);
    fibersz(kk) = sz(kk);
    fiber = reshape(varargin{kk}, fibersz);
    X = X.*repmat(fiber, repsz);
end