clear all; close all; clc;

addpath('../tensor');

%% input arguments
sz_X = [10, 8];
X = randn(sz_X); % tensor to approximate
K = cell(length(sz_X),1); % gram matrix
for kk = 1:length(sz_X)
    K{kk} = eye(sz_X(kk));
end

RankBound = [10,8]; % upperbound of mode-k rank
lambda = 10;

%% parameters
tol = 1e-4;

%% main

% handle input
sz = size(X);
nd = ndims(X);
mlr = RankBound;
F = cell(size(K));
for ii=1:length(K)
    F{ii} = K{ii}^0.5;
end

% initialization
beta = randn(mlr);
U = cell(nd,1);
for ii=1:nd
    U{ii} = randn(sz(ii), mlr(ii));
end

for iter=1:10
    
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
    
    % regularization term
    reg=0;
    for kk=1:nd
        tmp = kron(eye(prod(mlr([1:kk-1,kk+1:end]))),U{kk}'*U{kk});
        reg = reg + lambda*0.5*Gb(kk,mlr)*tmp*Gf(kk,mlr);
    end
    
    LHS=acc2+reg;
    RHS=vec(acc1);
    if norm(LHS*vec(beta) - RHS, 'fro') > tol
        beta=LHS\RHS;
        beta = kfold(beta, mlr, 1);
        
        X_est = beta;
        for kk=1:nd
            X_est = tensorkmat(X_est,F{kk}*U{kk},kk);
        end
%         fprintf('after update beta: %f\n', sqrt(sum((X_est(:)-X(:)).^2)));
    end
    
    % update U
    for kk=1:nd
        
        acc_loc = 1;
        for ll=[nd:-1:(kk+1),(kk-1):-1:1]
            acc_loc = kron(acc_loc, G{ll});
        end
        acc_loc = acc_loc*(kunfold(beta, kk)');
        T = kron( acc_loc, F{kk});
        
        % regularization term
        mqb_loc = kunfold(beta, kk);
        reg = kron(mqb_loc*mqb_loc',  eye(sz(kk)));
        
        for ll=[1:(kk-1),(kk+1):nd]
            acc2 = 1;
            for jj=1:nd
                if jj==kk || jj==ll
                    continue;
                end
                acc2 = acc2*norm(U{jj},'fro')^2;
            end
            reg = reg+acc2*eye(numel(U{kk}));
        end
        
        LHS = T'*T + lambda*reg;
        RHS = T'*vec(kunfold(X,kk));
        if norm(LHS*vec(U{kk}) - RHS, 'fro') > tol
            U{kk} = LHS\RHS;
            
            % normalize U
%             norm_const = norm(U{kk}, 'fro');
%             U{kk} = U{kk} / norm_const;
%             beta = beta*norm_const;
            
            U{kk} = kfold(U{kk}, [sz(kk), mlr(kk)], 1);
            
        end
        
        X_est = beta;
        for k2=1:nd
            X_est = tensorkmat(X_est,F{k2}*U{k2},k2);
        end

        
    end
    
    X_est = beta;
    for kk=1:nd
        X_est = tensorkmat(X_est,F{kk}*U{kk},kk);
    end
    fprintf('error: %f\n', sqrt(sum((X_est(:)-X(:)).^2)));
    
end







