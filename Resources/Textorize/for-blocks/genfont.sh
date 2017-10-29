#!/bin/bash

FONT_PARAM="-f\"Avenir Next Regular\""
SIZE_PARAM="-s128"
OTHER_PARAMS="-cwhite -gtransparent -a2"

STRING="ABCDEFGHIJKLMNOPQRSTUVWXYZ"

generate() 
{ 
  CHAR=$1
  FILE=$2
  command="textorize $SIZE_PARAM $OTHER_PARAMS $FONT_PARAM $CHAR -o\"./out/$FILE.png\""
  echo 'executing command:' $command
  eval $command
} 


mkdir -p out
for (( i=0; i<${#STRING}; i++ )); do
    CHAR=${STRING:$i:1}
    generate $CHAR $CHAR
done

# generate "BS" "backspace"
# generate "CR" "carriage-return"
# generate "," "comma"
# generate "." "period"

