function bc = Barycenter(X)
% X - each row is a signal.  

[N, L] = size(X);

% cost matrix
[Xgrid,Ygrid] = meshgrid(1:L,1:L);
M = abs(Xgrid - Ygrid);
lambda = 1000;
K=exp(-lambda*M);
K(K<1e-100)=1e-100;
U=K.*M;
        
figure(1);
subplot(1,2,1);
plot(X');
axis([1,L,min(X(:)),max(X(:))]);

subplot(1,2,2);
axis([1,L,0,1]);

% prox-function (for Bregman divergence)
% w = @(x) sum(x.^2);

t0 = 0.2;
t = 1;

a = 1/L*ones(L,1)*mean(sum(X,2));
a_hat = a;
a_tilde = a;

% objec_record = [];

for iter=1:300
    fprintf('iter #%d\n', iter);
    figure(1);
    subplot(1,2,2);
    hold off;
    plot(a);
    drawnow;
    
    beta = (1+t)/2;
    a = (1-1/beta)*a_hat + 1/beta*a_tilde;
    
    objec = 0;
    % form subgradient
    alpha = zeros(L,1);
    for ii=1:N
        fprintf('ii = %d\n', ii);
        
        xi = X(ii,:);
        xi = reshape(xi,[numel(xi),1]);
        
        % dual
%         lpA = sparse([kron(sparse(eye(L)),ones(L,1)), kron(ones(L,1),sparse(eye(L)))]);
%         lpb = M(:);
%         lpf = -[a;xi];
%         opts = optimset('display', 'off');
%         [alpha_beta_i, fval_d] = linprog(lpf,lpA,lpb, [], [], [], [], [], opts);
%         alpha_i = alpha_beta_i(1:L);
        
        % sinkhorn        
%         [D, ~, u, ~]=sinkhornTransport(a,xi,K,U,lambda,[],[],[],[],0);
%         alpha_i = 1/lambda*( log(u)+0.5 );


        % generalized OT
        
%         m = min(min(a), min(xi));
%         a2 = a-m;  xi2 = xi-m;
%         [D, alpha_i] = generalizedOTDistance(a2/sum(a2)*sum(xi2),xi2,K,U,lambda,[],[],[],[],0);
%         alpha_i = alpha_i*sum(xi2)/sum(a2);
%         D = OTDistance_1D(a2/sum(a2)*sum(xi2),xi2);
        
        a_ir = sum(xi)/sum(a);
        [D, alpha_i] = generalizedOTDistance(a_ir*a,xi,K,U,lambda,[],[],[],[],0);
        alpha_i = alpha_i/a_ir;
        D = OTDistance_1D(a_ir*a,xi);
        
        objec = objec+D;
        
        
        % choose alpha_i whose first coordinate is zero (due to
        % convention).
%         alpha_i = alpha_i - alpha_i(1)*ones(size(alpha_i));
        alpha = alpha + alpha_i;
 
    end
    alpha = alpha / N;
    
    fprintf('loss = %f\n', objec);
%     objec_record = [objec_record, objec];
    
    a_tilde = a_tilde.*exp(-t0*beta*alpha);
    a_tilde = a_tilde / norm(a_tilde,1);
    a_hat = (1-1/beta)*a_hat + 1/beta*a_tilde;
    
    t = t+1;

end

bc = a;