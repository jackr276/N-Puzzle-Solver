# The runner file for Java project 3
#!/bin/bash

NEWLINE=$'\n'


read -p "Enter the size for the N puzzle: " SIZE
read -p "Enter the complexity of the starting configuration: " COMPLEXITY


#erase everything in out
rm -r out/*

#Shove every java file into the sources file
find . -type f -name "*.java" > sources.txt

echo "${NEWLINE}Compiling:${NEWLINE}"
cat sources.txt


echo "${NEWLINE}Using Java Version: "
java --version


#compile and dump into out directory
javac -d ./out/ @sources.txt

if [[ $? -ne 0 ]]; then
	echo "Compilation failed"
	exit 1
fi

echo "${NEWLINE}Compilation Success!"

echo "Attempting to Run${NEWLINE}"

#assume we run the file called "Main.java"
cd out
java Main $SIZE $COMPLEXITY

if [[ $? -ne 0 ]]; then
	echo "${NEWLINE}Runtime Error Encountered"
	exit 0
fi

echo "${NEWLINE}Running Sucessful! Exit code: $? ${NEWLINE}"

