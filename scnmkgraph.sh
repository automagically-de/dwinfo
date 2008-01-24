
IFS="
"

function get_scn_by_id()
{
	id=$1
	for i in ${index[@]}; do
		name=`echo $i | cut -d: -f 1`
		off=`echo $i | cut -d: -f 2`
		if [ "$id" = "$off" ]; then
			echo $name
			return
		fi
	done
}

# load index
for line in `indexinfo index`; do
	off=`echo "$line" | sed -e 's/^.* //g' -e 's/)$//'`
	name=`echo "$line" | cut -d: -f 1 | sed -e 's/ *$//'`
	index[$off]=$name:$off
done

# iterate over scn files
for scn in `ls -1 *.scn *.gra 2>/dev/null`; do
	for addrline in `scninfo $scn | grep '^OP' | grep ' 01 '`; do
		addr=`echo "$addrline" | cut -d" " -f 4`
		if echo $addr | egrep -q '00000$' ; then
			echo "$scn -> `get_scn_by_id $addr`;"
		fi
	done
done
