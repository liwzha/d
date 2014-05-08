clear all; close all;


L = 20;
N = 20;

[Xgrid,Ygrid] = meshgrid(1:L,1:L);
D = double(abs(Xgrid-Ygrid));

% template = sin(linspace(0,pi,L/2));
template = sin(linspace(0,2*pi,L/4)) + 3;

X = zeros(N,L);
for ii=1:N
    % random shift
%     start = randi(L-length(template));
    start = 5+randi(3);
    X(ii, (1+start):(1+start+length(template)-1)) = template;
end

% random noise
X = X + 0.1*rand(size(X));
figure(1);
plot(X');

% br = rand(L,1);
% br = 0.5*ones(L,1);
br = mean(X)';

for iter =1:50
    alpha = zeros(L,1);
    for ii=1:N
        xi = X(ii,:);
        xi = reshape(xi,[numel(xi),1]);
        ai = sum(xi) / sum(br);
        aib = ai*br;
        
        % dual
        A = sparse([kron(eye(L),ones(L,1)), kron(ones(L,1),eye(L))]);
        b = D(:);
        f = -[aib;xi];
        alpha_beta_i = linprog(f,A,b);
        alpha = alpha + alpha_beta_i(1:L);
    end
    

    
    alpha = alpha / N;
    
    c = 1;
    while min(br - c*alpha)<0
        c = c*0.9;
    end
    br_new = br-c*alpha;

%     br = br-min(br);
%     br = br/max(abs(br))*10;

    figure(2);
    subplot(1,3,1);
    plot(br);
    subplot(1,3,2);
    plot(alpha);
    subplot(1,3,3);
    plot(br_new);
    drawnow;
    
%     pause;
    
    br = br_new;
end


% % dual
% A = sparse([kron(eye(L),ones(L,1)), kron(ones(L,1),eye(L))]);
% b = D(:);
% f = -[aib;x];
% alpha_beta = linprog(f,A,b);

% % primal
% f = D(:);
% A = -eye(L*L);
% b = zeros(L*L,1);
% Aeq = [kron(ones(1,L), eye(L)); kron(eye(L),ones(1,L))];
% beq = [xi;aib];
% vecP = linprog(f,A,b,Aeq, beq);
% Pi = reshape(vecP, [L,L]);

