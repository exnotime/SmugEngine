COUNTER=0;
for f in *.cpp; do
	filename="${f%.*}";
	BODY="#include \"$filename.h\"
unsigned int $filename::Flag = 1 << $COUNTER;"
	echo "$BODY" > "$f"
	COUNTER=$(($COUNTER+1))
done
echo "Updated component flags"