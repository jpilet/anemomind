m = load('/tmp/calibration.mat');

under8Opt = sum(m(:,2) < 8) ./ size(m,1)
under8Ext = sum(m(:,4) < 8) ./ size(m,1)
under10Opt = sum(m(:,2) < 10) ./ size(m,1)
under10Ext = sum(m(:,4) < 10) ./ size(m,1)
medianOpt = median(m(:,2))
medianExt = median(m(:,4))
q80Opt = quantile(m(:,2), .8)
q80Ext = quantile(m(:,4), .8)

binWidth = 4;
range = binWidth/2:binWidth:30;
[numOpt, binsOpt] = hist(m(:,2), range);
[numExt, binsExt] = hist(m(:,4), range);

histogramData = [binsOpt; numOpt; numExt]';

save '/tmp/calibration_histogram.mat'  histogramData
csvwrite("calibration.csv", histogramData);
disp("calibration.csv saved.");
