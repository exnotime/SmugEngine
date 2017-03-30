#!/bin/bash
HEADER="#pragma once
#include \"../SubSystem.h\"
class $1 : public SubSystem {
public:
	$1();
	~$1();

	virtual void Startup();
	virtual void Update(const double deltaTime);
	virtual void Shutdown();
private:

};
"

BODY="#include \"$1.h\"
$1::$1(){

}

$1::~$1(){

}

void $1::Startup() {
}

void $1::Update(const double deltaTime) {
}

void $1::Shutdown() {
}
"
echo "$HEADER" > "$1.h"
echo "$BODY" > "$1.cpp"
