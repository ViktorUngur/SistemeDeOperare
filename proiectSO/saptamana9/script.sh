if test $# -ne 1
then
    echo "Usage: $0 <c>"
fi

countCorrect=0

while read line
do
    validSentence=$(echo $line | grep -E "^[A-Z][a-zA-Z0-9 ,]+(\.|\?|!)$" | grep -v "si," | grep -v "n[pb]")
    if test -n "$validSentence"
    then
        containsMyCharacter=$(echo $validSentence | grep -E "$1")
        if test -n "$containsMyCharacter"
        then
            countCorrect=`expr $countCorrect + 1`
        fi
    fi
done

echo $countCorrect