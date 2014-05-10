L = 30;
N = 10;

% template = sin(linspace(0,pi,L/2));
template = sin(linspace(0,2*pi,4/5*L)) + 3;

X = zeros(N,L);
for ii=1:N
    % random shift
%     start = randi(L-length(template));
    start = 2+randi(3);
    X(ii, (1+start):(1+start+length(template)-1)) = template;
end

% random noise
X = X + 0.3*rand(size(X));
X = NormalizeRows(X);

a = X(1,:)';
b = X(end,:)';

L = length(a);
[Xgrid,Ygrid] = meshgrid(1:L,1:L);
M = double(abs(Xgrid-Ygrid));

lambda = 200;
K=exp(-lambda*M);
K(K<1e-100)=1e-100;

U=K.*M;

a = a/norm(a,1);
% b = b/norm(b,1);
b = 1/L*ones(L,1);
[D, L, u, v]=sinkhornTransport(a,b,K,U,lambda,[],[],[],[],1);