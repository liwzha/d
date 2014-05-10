function X = NormalizeRows(X)
% normalize every rows so that each row sum to one
X = X./ repmat(sum(X,2), [1,size(X,2)]);