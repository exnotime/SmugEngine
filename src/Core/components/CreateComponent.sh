#!/bin/bash
HEADER="#pragma once
struct $1 {
	static unsigned int Flag;
};
"

BODY="#include \"$1.h\"
unsigned int $1::Flag = 0;
"
echo "$HEADER" > "$1.h"
echo "$BODY" > "$1.cpp"
