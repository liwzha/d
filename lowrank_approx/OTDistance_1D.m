function dist = OTDistance_1D(x,y)
if abs(sum(x) - sum(y)) > 1e-8
    error('sum(x) ~= sum(y) !!\n');
end
dif = abs(cumsum(x-y));
dist = sum(dif);




