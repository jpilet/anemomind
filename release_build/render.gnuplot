# Based on the code here: http://superuser.com/a/718977
set terminal pdf
set output 'awa_offset_plot.pdf' 
set hidden3d
set dgrid3d 50,50 qnorm 2
set xlabel "AWS [noeuds]"
set ylabel "AWA [degrées]"
set zlabel "Offset angle [degrées]" rotate by 90
splot 'illustrations_calib_data_polargrid_list.txt' with lines title "Offset angle"