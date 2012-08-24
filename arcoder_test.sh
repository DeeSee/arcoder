#!/bin/bash

if [ "$1" == "" ]
then
	echo "Path to binary is not provided, exiting"
	exit 1
fi

PATH_TO_BINARY=$1
CURRENT_DIRECTORY=`pwd`
TEST_FILE_NAME=$CURRENT_DIRECTORY"/testfile"
ENCODED_TEST_FILE_NAME=$CURRENT_DIRECTORY"/testfile_encoded"
DECODED_TEST_FILE_NAME=$CURRENT_DIRECTORY"/testfile_decoded"

if [ ! -f $PATH_TO_BINARY ]
then
	echo "Couldn't find binary" $PATH_TO_BINARY
	exit 1
fi

if [ -f $TEST_FILE_NAME ]
then
	echo Error: test file exists
	exit 2
fi

CYCLES_COUNT=100

for (( i = 1 ; i <= CYCLES_COUNT; i++ ))
do
	dd if=/dev/urandom of=$TEST_FILE_NAME count=10 status=noxfer 2>/dev/null
	$PATH_TO_BINARY e $TEST_FILE_NAME $ENCODED_TEST_FILE_NAME
	$PATH_TO_BINARY d $ENCODED_TEST_FILE_NAME $DECODED_TEST_FILE_NAME
	cmp -s $TEST_FILE_NAME $DECODED_TEST_FILE_NAME > /dev/null
	if [ $? -eq 1 ]
	then
		echo Files are not equal, exiting
		rm $TEST_FILE_NAME
		rm $ENCODED_TEST_FILE_NAME
		rm $DECODED_TEST_FILE_NAME
		exit 3
	fi

	echo -ne "\r $i / $CYCLES_COUNT ["
	for (( j = 1 ; j <= i; j++ ))
	do
		echo -n "#"
	done
	for (( j = i + 1; j <= CYCLES_COUNT; j++ ))
	do
		echo -n " "
	done
	echo -n "]"
done

echo

rm $TEST_FILE_NAME
rm $ENCODED_TEST_FILE_NAME
rm $DECODED_TEST_FILE_NAME

exit 0
