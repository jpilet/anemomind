data0 = load('/tmp/tilestep1_dispatcher_matrix.txt');
codes = data0(1, :);
data = data0(2:end, :);

labels = {'Time (seconds)', 'X (meters)', 'Y (meters)', '[tilegen_Anemomind estimator]', '[tilegen_NMEA2000/c078be002fb00000]', '[tilegen_Simulated Anemomind estimator]', '[groundTruth_CSV imported]', '[tilegen__gpsBearing_Internal GPS]', '[tilegen__gpsBearing_NMEA2000/c07891002fb3645a]', '[tilegen__magHeading_NMEA2000/c050a0012fb3245a]', '[groundTruth__magHeading_CSV imported]'};
%labels = {'Time (seconds)', 'X (meters)', 'Y (meters)', '[tilegen_Anemomind estimator]', '[tilegen_NMEA2000/c078be002fb00000]', '[groundTruth_CSV imported]', '[tilegen__gpsBearing_Internal GPS]', '[tilegen__gpsBearing_NMEA2000/c07891002fb3645a]', '[tilegen__magHeading_NMEA2000/c050a0012fb3245a]', '[groundTruth__magHeading_CSV imported]'};
%labels = {'Time (seconds)', 'X (meters)', 'Y (meters)', '[tilegen_Anemomind estimator]', '[tilegen_NMEA2000/c078be002fb00000]', '[tilegen_Simulated Anemomind estimator]', '[groundTruth_CSV imported]', '[tilegen__gpsBearing_Internal GPS]', '[tilegen__gpsBearing_NMEA2000/c07891002fb3645a]', '[tilegen__magHeading_NMEA2000/c050a0012fb3245a]', '[groundTruth__magHeading_CSV imported]'};
%labels = {'Time (seconds)', 'X (meters)', 'Y (meters)', '[tilegen_Anemomind estimator]', '[tilegen_NMEA2000/c078be002fb00000]', '[groundTruth_CSV imported]', '[tilegen__gpsBearing_Internal GPS]', '[tilegen__gpsBearing_NMEA2000/c07891002fb3645a]', '[tilegen__magHeading_NMEA2000/c050a0012fb3245a]', '[groundTruth__magHeading_CSV imported]'};
%labels = {'Time (seconds)', 'X (meters)', 'Y (meters)', '[tilegen_Anemomind estimator]', '[tilegen_NMEA2000/c078be002fb00000]', '[tilegen_Simulated Anemomind estimator]', '[groundTruth_CSV imported]', '[tilegen__gpsBearing_Internal GPS]', '[tilegen__gpsBearing_NMEA2000/c07891002fb3645a]', '[tilegen__magHeading_NMEA2000/c050a0012fb3245a]', '[groundTruth__gpsBearing_Internal GPS]', '[groundTruth__gpsBearing_NMEA2000/c07891002fb3645a]', '[groundTruth__magHeading_NMEA2000/c050a0012fb3245a]'};

          
assert(numel(labels) == size(data, 2));        
          
total_data_size = size(data, 1)

if 0,
  i = 19;
  k = 1000;
  offset = i*1000 + 1;
  inds = (offset+1):(offset + k);
end

if 0,
  inds = 1:total_data_size;
end

if 1,
  inds = abs(data(:, 1)) < 10*60;
end

data = data(inds, :);

fprintf("For %.3g seconds\n", data(end, 1) - data(1, 1));



time = data(:, 1);
X = data(:, 2);
Y = data(:, 3);

[~, index] = min(abs(time));

values_at_ref = data(index, :);
fprintf("At reference time, we have:\n");
for i = 1:numel(labels),
  fprintf('   - %s: %8.6f\n', labels{i}, values_at_ref(i));
end

if true,
  k = 10000;
  good = abs(X) < k & abs(Y) < k;

  plot(X(good), Y(good), 'b');
  axis xy equal
end

offsets = [0 0 -360 0]

c = 'rgbk'
twa_inds = find(codes == 3);
if true,
  figure;
  k = 1;
  for i = twa_inds,
    plot(time, make_continuous(data(:, i) + offsets(k)), c(k));
    hold on
    k = k + 1;
  end
  hold off
  make_legend(labels, twa_inds);
  title('TWA');
end

if true,
  figure;
  heading_col = 10;
  heading = data(:, heading_col);
  
  subplot(2, 1, 1);
  title('TWDIR');
  k = 1;
  for i = twa_inds,
    twdir = data(:, i) + heading;
    plot(time, make_continuous(twdir), c(k));
    hold on
    k = k + 1;
  end
  hold off
  xlabel('Time (seconds)');
  ylabel('Angle (degrees)');
  make_legend(labels, twa_inds);
  title('TWDIR');
  subplot(2, 1, 2);
  plot(time, make_continuous(heading));
  xlabel('Time (seconds)');
  ylabel('Angle (degrees)');
  title('HEADING');
end