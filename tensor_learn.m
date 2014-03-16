addpath('../tensor');

%% input arguments
X = randn(3,4,5); % tensor to approximate
K = cell(3,1); % gram matrix
K{1} = eye(3); K{2} = eye(4); K{3} = eye(5);

RankBound = [3,3,3]; % upperbound of mode-k rank
lambda = 1;

%% parameters



%% main

% handle input
sz = size(X);
nd = ndims(X);
mlrank = RankBound;
F = cell(size(K));
for ii=1:length(K)
    F{ii} = K{ii}^0.5;
end

% initialization
beta = randn(mlrank);
U = cell(nd,1);
for ii=1:nd
    U{ii} = rand(sz(ii), mlrank(ii));
end

% update beta
G = cell(nd,1);
for ii=1:nd
    G{ii} = F{ii}*U{ii};
end

acc1 = X;
for kk=1:nd
    acc1 = tensorkmat(acc1, G{kk}', kk);
end

acc2 = G{end}'*G{end};
for kk=nd-1:-1:1
    acc2 = kron(acc2, G{kk}'*G{kk});
end

LHS=acc2;
RHS=vec(acc1);
beta=LHS\RHS;









