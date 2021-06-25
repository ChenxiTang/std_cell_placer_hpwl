#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <algorithm>
#include "easygl_constants.h"
#include "graphics.h"
#include "placer.h"

using namespace std;

void block::setStatus(char newstatus) {
	if (status == 'x')
		status = newstatus;
	else
		cout << "Reconnect to an anchor, check code\n";
}

void block::setInfo(char newInfo){
	if (info != 'f')
		info = newInfo;
	else
		cout << "This is a fixed block, cannot be changed" << endl;
}

void block::setNum(int newNum) {
	if (num == -1)
		num = newNum;
	else
		cout << "This block is already assigned a number, cannot be reassigned" << endl;
}

void block::setLocation(dataType xx, dataType yy) {
	if (info != 'f') {
		// if it is not a fixed block, it could be assigned a new location
		location.x = xx;
		location.y = yy;
	}
}


void block::setNet(int netNo) {
	nets.insert(netNo);
}

void block::printInfo() {
	if (info == 'f') {
		cout << "Block no: " << num << " is a fixed block at ";
		print_point(location);
		cout << " connected at ";
		set<int>::iterator SmallIt = nets.begin();
		while (SmallIt != nets.end()) {
			cout << *SmallIt << ' ';
			SmallIt++;
		}
		cout << endl;
	}
	else {
		cout << "Block no: " << num << " is not fixed. ";
		cout << " connected at ";
		set<int>::iterator SmallIt = nets.begin();
		while (SmallIt != nets.end()) {
			cout << *SmallIt << ' ';
			SmallIt++;
		}
		cout << endl;
	}
}

void block::DrawCell(string circuit) {
	if (info == 'f')
		setcolor(RED);
	else if (info == 'a')
		setcolor(GREEN);
	else
		setcolor(DARKGREY);
	if(circuit!="cct1")
		fillarc(location.x, location.y, 1.f, 0.0f, 361.f);
	else
		fillarc(location.x, location.y, .05f, 0.0f, 361.f);
	setcolor(BLUE);
	string text;
	text = '(' + to_string(location.x).substr(0,4) + ',' 
		+ to_string(location.y).substr(0, 4) + ')';
	drawtext(location.x, location.y, text.c_str(), 100.0f);
	flushinput();
}

