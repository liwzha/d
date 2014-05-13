function [dist, grad] = generalizedOTDistance(a,b,K,U,lambda,stoppingCriterion,p_norm,tolerance,maxIter,VERBOSE)

pos = (a-b).*((a-b)>0);
neg = -(a-b).*((a-b)<0);
C = sum(pos);

[D, ~, u, v]=sinkhornTransport(pos/C,neg/C,K,U,lambda,stoppingCriterion,p_norm,tolerance,maxIter,VERBOSE);

dist = D*C;

alpha = 1/lambda*( log(u)+0.5 );
beta = 1/lambda*( log(v)+0.5);
alpha(isinf(alpha))=0;
beta(isinf(beta))=0;
grad = C*alpha - C*beta;

% C = sum(a);
% [D, ~, u, v] = sinkhornTransport(a/C,b/C,K,U,lambda,stoppingCriterion,p_norm,tolerance,maxIter,VERBOSE);
% dist = D*C;
% alpha = 1/lambda*(log(u)+0.5 );
% grad = C*alpha;