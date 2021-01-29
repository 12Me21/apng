dir=`mktemp -d`
for i in `seq 0.3 0.1 2.0`; do
	echo "$i" >&2
	curl -s "https://thisanimedoesnotexist.ai/results/psi-$i/seed$1.png" -o "$dir/$i.png" &
	#curl -s "https://thisanimedoesnotexist.ai/results/psi-$i/seed$1.png" -o $dir/$i.png &
done
wait
cat $dir/*.png | ./apng-hack 18 1 15
rm -rf "$dir"
