
#for f in /home/shiroshita/mov_data_100212/movie_backup/*.mpeg 
#do
#./TestVideo $f
#done 

#./TestVideo /home/shiroshita/mov_data_100212/for_experiment/DB/A.mpeg

#./TestVideo -f /home/shiroshita/mov_data_100212/for_experiment/DB/A.mpeg -v 4 -h 4 -c -m 5

#./TestVideo /home/shiroshita/mov_data_100212/for_experiment/DB/B.mpeg
#./TestVideo /home/shiroshita/mov_data_100212/for_experiment/DB/M.mpeg
#./TestVideo /home/shiroshita/mov_data_100212/for_experiment/DB/E.mpeg



#./TestVideo -f /home/shiroshita/mov_data_100212/for_experiment/KEY/KEY2.mp*g -v 1 -h 1 -m 5
#./TestVideo -f /home/shiroshita/mov_data_100212/for_experiment/KEY/KEY2.mp*g -v 2 -h 2 -m 5
#./TestVideo -f /home/shiroshita/mov_data_100212/for_experiment/KEY/KEY2.mp*g -v 4 -h 4 -m 3




TIME_A=`date +%s`
for f in /home/shiroshita/mov_data_100212/for_experiment/DB/*.mp*g
do
./TestVideo -f $f -v 1 -h 1 -c -m 5 -L -C
./TestVideo -f $f -v 2 -h 2 -c -m 5 -L
#./TestVideo -f $f -v 3 -h 3 -c -m 8
./TestVideo -f $f -v 4 -h 4 -c -m 5 -L
#./TestVideo -f $f -v 5 -h 5 -c -m 8
done 

for f in /home/shiroshita/mov_data_100212/for_experiment/KEY/*.mp*g
do
./TestVideo -f $f -v 1 -h 1 -m 5 -L -C
./TestVideo -f $f -v 2 -h 2 -m 5 -L
#./TestVideo -f $f -v 3 -h 3 -m 8
./TestVideo -f $f -v 4 -h 4 -m 5 -L
#./TestVideo -f $f -v 5 -h 5 -m 8
done 

TIME_B=`date +%s`   #B
PT=`expr ${TIME_B} - ${TIME_A}`
H=`expr ${PT} / 3600`
PT=`expr ${PT} % 3600`
M=`expr ${PT} / 60`
S=`expr ${PT} % 60`
echo "${H}:${M}:${S}"

: << COMMENTOUT

for f in /home/shiroshita/mov_data_100212/movie_backup/*
do
./TestVideo -f $f -v 1 -h 1 -c -m 5
./TestVideo -f $f -v 2 -h 2 -c -m 5
#./TestVideo -f $f -v 3 -h 3 -c -m 8
./TestVideo -f $f -v 4 -h 4 -c -m 5
#./TestVideo -f $f -v 5 -h 5 -c -m 8
done 
end
COMMENTOUT


