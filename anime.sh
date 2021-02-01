[ $1 ] && seed=$1 make -j20
exit

# old code if you want it:
dir=`mktemp -d` || exit
for i in `seq 0.3 0.1 2.0`; do
	echo downloading frame $i >&2
	curl -s https://thisanimedoesnotexist.ai/results/psi-$i/seed$1.png -o $dir/$i.png &
done
wait
echo download finished >&2
cat $dir/*.png | ./apng-hack 18 1 12 >$1.apng
echo done
rm -rf $dir
