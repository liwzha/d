function bc = Barycenter(X)
% X - each row is a signal.  each signal is a probability measure.

[N, L] = size(X);

% cost matrix
[Xgrid,Ygrid] = meshgrid(1:L,1:L);
M = abs(Xgrid - Ygrid);
lambda = 200;
K=exp(-lambda*M);
K(K<1e-100)=1e-100;
U=K.*M;
        
figure(1);
subplot(1,2,1);
plot(X');

% prox-function (for Bregman divergence)
% w = @(x) sum(x.^2);

t0 = 0.2;
t = 1;

a = 1/L*ones(L,1);
% a=X(1,:)';
objec_record = [];

for iter=1:300
    fprintf('iter #%d\n', iter);
    figure(1);
    subplot(1,2,2);
    hold off;
    plot(a);
    drawnow;
    
    gamma = 1/t;
    
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
        [D, ~, u, ~]=sinkhornTransport(a,xi,K,U,lambda,[],[],[],[],0);
        alpha_i = 1/lambda*( log(u)+0.5 );
        
        objec = objec+D;
        
        
        % choose alpha_i whose first coordinate is zero (due to
        % convention).
%         alpha_i = alpha_i - alpha_i(1)*ones(size(alpha_i));
        alpha = alpha + alpha_i;
 
    end
    alpha = alpha / N;
    
    fprintf('loss = %f\n', objec);
    objec_record = [objec_record, objec];
    
   
    % Polyak
    step_size = gamma / norm(alpha);
    while( min(a-step_size*alpha)<0 )
        step_size = step_size*0.8;
    end
    a = a - step_size*alpha;
    a = a / norm(a,1);
    
    t = t+1;

end

bc = a;