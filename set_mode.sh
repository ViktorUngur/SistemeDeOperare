if test $# -ne 2
then
    echo "Apeleaza sub forma: $0 director mod"
fi


mod=$2

for entry in "$1"/*.txt
do
    chmod +$mod $entry
done


for director in "$1"/*
do
    if test -d "$director"
    then
	echo $director
	bash $0 $director $mod
	
    fi
done
