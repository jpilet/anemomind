set title "Anemomoind calibration"
set auto x
set xlabel 'Angle difference (degrees)'
set ylabel 'Number of maneuvers'
set style data histogram
set style histogram cluster gap 2
set style fill solid border -1
set boxwidth 0.9
plot '/tmp/calibration_histogram.mat' using 2:xticlabel(1) title 'Anemomind' fillcolor '#00FF00', \
       '/tmp/calibration_histogram.mat' using 3:xticlabel(1) title 'External' fillcolor '#FF0000'
#
