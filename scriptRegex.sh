count=0
count2=0

for entry in "$1"/*
do
    if test -f "$entry"
    then
	res=$(cat $entry | grep -E "^[A-Z][a-zA-Z0-9 ,]+\.$" | grep -v "si," | grep -v "n[pb]")
	if test -n "$res"
	then
	    echo "$entry" >> "$2"
	fi
    fi

    if test -L "$entry"
    then
        count=`expr $count + 1`
    fi

    if test -d "$entry"
    then
	count2=$(bash "$0" "$entry" "$2")
	count=`expr $count2 + $count`
    fi
done

echo "$count"
