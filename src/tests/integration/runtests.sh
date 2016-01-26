#!/bin/bash

RUNNER=../../../build/program-tester

while read PRE_QUERIES POST_QUERIES EXPECTED
do
    if [ "$PRE_QUERIES" = "none" ]
    then
	PRE_ARGS=""
    else
	PRE_ARGS="--quiet-pre-run --pre-run-queries=$PRE_QUERIES"
    fi
    
    OUT=$($RUNNER --file="../programs/$1" --program="$2" $PRE_ARGS --post-run-queries="$POST_QUERIES" 2>error.output)

    if [ ! "$EXPECTED" = "none" ]
    then
	if [ ! "$OUT" = "$EXPECTED" ]
	then
	    printf "Test failed!\n"
	    printf " pre queries\t\t%s\n" $PRE_QUERIES
	    printf " post queries\t\t%s\n" $POST_QUERIES
	    printf " test output\t\t%s\n" $OUT
	    printf " expected\t\t%s\n" $EXPECTED
	    SOME_TEST_FAILED="yes"
	fi
    fi
done

if [ -z $SOME_TEST_FAILED ]
then
    echo "All tests passed!"
fi
