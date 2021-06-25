#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <algorithm>
#include <set>
#include "easygl_constants.h"
#include "graphics.h"
#include "placer.h"

using namespace std;

void net::setInfo(char newInfo) {
	if (info == 'x')
		info = newInfo;
	else
		cout << "Net property redefined" << endl;
}
/*
void net::setNum(int newNum) {
	if (num == -1)
		num = newNum;
	else
		cout << "Net number redefined" << endl;
}
*/

void net::setDrivenBlock(int blockno) {
	if (DrivenBlockNo == -1)
		DrivenBlockNo = blockno;
	else
		cerr << "MULTIPLE CELL DRIVE ONE NET\n";
}
void net::setConnectedBlocks(int blockNo) {
	connectedBlocks.insert(blockNo);
}

void net::CalParameters(char mode, char weightType, map<int,block>& blocks) {
	PinNo = connectedBlocks.size();
	//simple net check
	if (PinNo == 1)
		cerr << "Net "<< num<<" only connected to one logic block" << endl;
	if (mode == 'c')//clicque model weight
		weight = 2.0 / PinNo;
	else if (mode == '2') //question 2 weight
		weight = 1.; //all net weights are 1
	else {
		//question 3 weight
		if (info == 'f') {
			dataType distance = blocks[*connectedBlocks.begin()].getDis();
			//only recalculate weights for sudo nets
			switch (weightType) {
			case 'w': {
				// light weight
				weight = .1 * distance;
				break;
			}
			case 'm': {
				// medium weight
				weight = 1 * distance;
				break;
			}
			case 's': {
				// heavy weight
				weight = 2.5 * distance;
				break;
			}
			default: {
				// a random huge weight to show error in coding.
				weight = 1 * distance;
				break;
			}
			}
		}
		else
			weight = 2.0 / PinNo;
	}
	int ColorRotate[] = { BLUE, GREEN, YELLOW, CYAN, DARKGREEN, MAGENTA };
	if (info != 'f')
		netColour = ColorRotate[num % 6];
	else
		netColour = RED;
}

void net::PrintNetInfo() {
	cout << "Net no. " << num << " is connected to blocks:\n";
	for (auto& it : connectedBlocks)
		cout << it << "  ";
	cout << endl;
	cout << "This is a " << PinNo << "-pin net, weight is " << weight << endl;	
	if (info == 'd')
		cout << "This block is a driver\n";
	else if (info == 's')
		cout << "This block is a sink\n";

	cout << "----------------------------------\n";
}

void net::DrawNets(map<int, block>& blocks, char mode) {

	if (info != 'f') {
		setcolor(netColour);
		setlinestyle(SOLID);
	}
	else {
		setcolor(netColour);
		setlinestyle(DASHED);
	}

	if (mode != 'b') {
		for (set<int>::iterator blockIt = connectedBlocks.begin();
			blockIt != connectedBlocks.end(); blockIt++) {

			point p1, p2;
			p1 = blocks[*blockIt].getBlockLocation();

			for (auto newIt = next(blockIt, 1); newIt != connectedBlocks.end(); newIt++) {
				p2 = blocks[*newIt].getBlockLocation();
				drawline(p1.x, p1.y, p2.x, p2.y);
			}
		}
	}
	else {
		for (set<int>::iterator blockIt = connectedBlocks.begin();
			blockIt != connectedBlocks.end(); blockIt++) {

			point p1, p2;
			p1 = blocks[*blockIt].getBlockLocation();

			for (auto newIt = next(blockIt, 1); newIt != connectedBlocks.end(); newIt++) {
				p2 = blocks[*newIt].getBlockLocation();
				drawline(p1.x, p1.y, p2.x, p2.y);
			}
		}
	}
	
	flushinput();
}
