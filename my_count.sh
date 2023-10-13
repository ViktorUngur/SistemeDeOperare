if test $# -ne 2
then
    echo "Apeleaza sub forma: $0 director outputFile"
fi

suma=0

for entry in "$1"/*.txt
do
    echo "$entry" >> $2
    numarCaractere=$(wc -c < $entry)
    echo $numarCaractere >> $2
    suma=`expr $suma + $numarCaractere`
done

echo $suma >> $2

	 
	     
