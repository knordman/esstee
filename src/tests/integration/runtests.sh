#!/bin/bash

RUNNER=../../../build/program-tester

IFS="!"

while read FILE PROGRAM EXIT_STATUS PRE_QUERIES POST_QUERIES EXPECTED
do
    if [[ "$FILE" =~ \# ]]
    then
	continue
    fi
    
    if [ -n "$1" ]
    then
	DO_TEST=`echo $FILE $PROGRAM $EXIT_STATUS $PRE_QUERIES $POST_QUERIES $EXPECTED | awk "/$1/"`

	if [ -z "$DO_TEST" ]
	then
	    continue
	fi
    fi

    PRE_ARGS=""
    if [ ! "$PRE_QUERIES" = "none" ]
    then
	if [[ $PRE_QUERIES =~ \' ]]
	then
	    PRE_DELIM='"'
	else
	    PRE_DELIM="'"
	fi

	PRE_ARGS="--quiet-pre-run --pre-run-queries=$PRE_DELIM$PRE_QUERIES$PRE_DELIM"
    fi

    POST_ARGS=""
    if [ ! "$POST_QUERIES" = "none" ]
    then
	if [[ $POST_QUERIES =~ \' ]]
	then
	    POST_DELIM='"'
	else
	    POST_DELIM="'"
	fi

	POST_ARGS="--post-run-queries=$POST_DELIM$POST_QUERIES$POST_DELIM"
    fi
    
    if [ "$2" == "gdb" ]
    then
	echo "Writing GDB debug commands..."
	echo file $RUNNER > test.gdb.commands
	echo set args --file="../programs/$FILE" --program="$PROGRAM" $PRE_ARGS $POST_ARGS >> test.gdb.commands
	break
    fi

    BISON_CMD=""
    if [ "$2" == "bison" ]
    then
     	BISON_CMD="--bison-debug"
    fi

    TEST_CMD="$RUNNER --file='"../programs/$FILE"' --program='"$PROGRAM"' "$PRE_ARGS" "$POST_ARGS" "$BISON_CMD" 2>error.output"
    
    if [ -n "$1" ]
    then
	printf "Testing\n"
	printf "$TEST_CMD\n"
    fi

    OUT=$(eval $TEST_CMD)

    TEST_STATUS=$?
    
    TEST_FAILED="no"
    if [ ! "$TEST_STATUS" = "$EXIT_STATUS" ]
    then
	TEST_FAILED="yes"
    fi
    
    if [ "$EXPECTED" = "none" ]
    then
	if [ -n "$OUT" ]
	then
	    TEST_FAILED="yes"
	fi
    else
	if [ ! "$OUT" = "$EXPECTED" ]
	then
	    TEST_FAILED="yes"
	else
	    TEST_PASSED="yes"
	fi
    fi

    if [ "$TEST_FAILED" = "yes" ]
    then
	printf "############################################\n"
	printf "Test failed!\n"
	printf " exit status\t\t%d\n" $TEST_STATUS
	printf " expected exit\t\t%d\n" $EXIT_STATUS
	printf " pre queries\t\t%s\n" $PRE_QUERIES
	printf " post queries\t\t%s\n" $POST_QUERIES
	printf " test output\t\t%s\n" $OUT
	printf " expected\t\t%s\n" $EXPECTED

	SOME_TEST_FAILED="yes"
    elif [ -n "$1" ]
    then
	printf " exit status\t\t%d\n" $TEST_STATUS
	printf " expected exit\t\t%d\n" $EXIT_STATUS
	printf " pre queries\t\t%s\n" $PRE_QUERIES
	printf " post queries\t\t%s\n" $POST_QUERIES
	printf " test output\t\t%s\n" $OUT
	printf " expected\t\t%s\n" $EXPECTED	
    fi
done

if [ -z "$SOME_TEST_FAILED" -a -n "$TEST_PASSED" ]
then
    echo "All tests passed!"
else
    exit 1
fi
