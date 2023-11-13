while read line
do
    echo $line | grep -E "^[A-Z][a-zA-Z0-9 ,]+\.$" | grep -v "si," | grep -v "n[pb]"
done

  
