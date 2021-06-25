#include <iostream>
#include <vector>
#include <fstream>
#include <map>
#include <stdio.h>
#include <string>
#include <queue>

#include "placer.h"
#include "graphics.h"
#include "easygl_constants.h"

#ifdef ECF
#include "solver.h"
#endif


using namespace std;

// graphics 
void drawscreen(void);
void act_on_button_press(float x, float y);
static bool rubber_band_on = false;
static bool have_entered_line, have_rubber_line;
static bool line_entering_demo = false;
static float xx1, yy1, xx2, yy2;
void Blank(void) {};

vector<double> circuit;
string circuitName;

//key is block no, vector<int> stores connected nets
map <int, vector<int>> circuitConnections;
map <int, point> fixedBlocks;
map <int, block> blocks;
map<int, int> NetDrivers;//map<netNo, drivenBy>

// unique net number from smallest to largest
vector<int> SortedNets;
map<int, net> nets;//net no and net object


//initialise nx1 b matrix
vector<dataType> bx;
vector<dataType> by;
vector<int> ap, ai;
vector<dataType> ax;
vector<dataType> x;
vector<dataType> y;
int NotFixed;
char application;

int main() {
	 
	//interactive user interface
	
	cout << "Choose circuit file:\n";
	cout << "1:cct1\n2:cct2\n3:cct3";
	cout << endl;
	char circuitChoice;
	cin >> circuitChoice;
	switch (circuitChoice) {
	case '1': {
		circuitName = "cct1";
		break; }
	case '2': {
		circuitName = "cct2";
		break; }
	case '3': {
		circuitName = "cct3";
		break; }
	default: {
		cout << "WRONG CIRCUIT FILE\n";
		return 1;
	}
	}
	
	cout << endl;
	cout << "Chose an applcation: ";
	cout << endl;
	if(circuitName!="cct1")
		cout << "a: question 1\nb: question 2\nc: question 3\n";
	else
		cout << "a: question 1\nb: question 2\n";
	char application;
	cin >> application;

	//start reading circuit files

	ifstream InFile;
	InFile.open(circuitName, ios::in);

	//read input file
	if (InFile.is_open()) {
		dataType temp;
		cout << "Start reading file" << endl;
		while (InFile) {
			InFile >> temp;
			circuit.push_back(temp);
		}
		InFile.close();
	}
	else {
		cerr << "File not opened\n";
		return 1;
	}

	//parsing circuit info
	int StartSec2 = 0;
	for (auto it = circuit.begin(); it != circuit.end(); it++) {
		if (StartSec2 == 0) {
			int BlockKey = *it;
			it++;
			while (*it != -1) {
				circuitConnections[BlockKey].push_back((int)(*it));
				it++;
			}
			if (*it == -1 && *(it + 1) == -1) {
				StartSec2 = 1;
				it++;
			}
		}
		else if (*it != -1) {
			int BlockKey = *it;
			it++;
			fixedBlocks[BlockKey].x = (dataType)(*it);
			it++;
			fixedBlocks[BlockKey].y = (dataType)(*it);
		}
		else
			break;
	}

	//debug
	/*
	for (map <int, vector<int>>::iterator it = circuitConnections.begin(); it != circuitConnections.end(); it++) {
		vector<int> nets = it->second;
		int BlockNo = it->first;
		cout << "Block " << BlockNo << ": ";
		vector<int>::iterator SmallIt = nets.begin();
		while (SmallIt != nets.end()) {
			cout << *SmallIt << ' ';
			SmallIt++;
		}
		cout << endl;
	}

	for (map <int, point>::iterator it = fixedBlocks.begin(); it != fixedBlocks.end(); it++) {
		cout << "Fixed block: " << it->first << " at point ";
		print_point(it->second);
		cout << endl;
	}
	*/

	//initiate objects

	vector<int> NotFixedBlockNo, FixedBlockNo;
	for (map<int, vector<int>>::iterator IT = circuitConnections.begin(); IT != circuitConnections.end(); IT++) {
		block temp;
		int blockNo = IT->first;
		vector<int> nets = IT->second;

		temp.setNum(blockNo);

		//set up block nets
		for (auto netIt = nets.begin(); netIt != nets.end(); netIt++)
			temp.setNet(*netIt);

		map<int, point>::iterator itFind = fixedBlocks.find(IT->first);
		if (itFind != fixedBlocks.end())
		{
			//cout << "Found a fixed block " << IT->first << endl;
			temp.setLocation(itFind->second.x, itFind->second.y);
			FixedBlockNo.push_back(blockNo);
			temp.setInfo('f');
		} 
		else {
			temp.setInfo('n');
			NotFixedBlockNo.push_back(blockNo);
		}
		blocks[blockNo] = temp;
	}

	//find which cell drives which net, for question2
	for (map<int, block>::iterator blockit = blocks.begin(); blockit != blocks.end(); blockit++) {
		int blockNo = blockit->first;
		set<int> BlockNets = (blockit->second).getNets();
		for (set<int>::iterator netit = BlockNets.begin(); netit != BlockNets.end(); netit++) {
			// check if this net is already been pushed to map netdriver
			if (NetDrivers.find(*netit) == NetDrivers.end()) {
				NetDrivers[*netit] = blockNo;			
			}
		}
	}


	

	NotFixed = NotFixedBlockNo.size();
	int NetNo = init_net_vector(SortedNets, blocks);
	// initiate pin nets 
	//!!!!!!!PROBLEM
	for (vector<int>::iterator i = SortedNets.begin(); i != SortedNets.end(); i++) {
		net temp(*i);
		nets[*i] = temp;
	}
	for (map<int, int>::iterator it = NetDrivers.begin(); it != NetDrivers.end(); it++)
		nets[it->first].setDrivenBlock(it->second);
	//debug
	/*
	for (auto& it : nets) {
		cout << "Net " << it.second.getNum() << " driven by block " << it.second.GetDrivenBlock() << endl;
	}
	*/
	// graph window initialisation
	printf("Analytical Placement and Spreading\n");
	init_graphics("Analytical Placement and Spreading", WHITE);
	//still picture drawing allows user to zoom, etc. 
	if(circuitName=="cct1")
		init_world(0.f, 5.f, 5.f, 0.f);
	else
		init_world(-1.f, 65.0f, 65.0f, -1.f);
	event_loop(act_on_button_press, NULL, NULL, drawscreen);



   //different routines for different parts of the assignment
	char WeightModel = 'c';//default clique model

	switch (application) {
	case 'c': {
		if (circuitName != "cct8")
			cout << "Simple Overlap Removal ENABLED\n";
		else {
			cout << "Spreading is not available for this circuit file\n";
			return 1;
		}
		WeightModel = '3';
	}
	case 'a': {
		//clique model 
		cout << "Start generating clique model\n";
		cout << "Max net size " << nets.max_size() << endl;
		for (map<int, block>::iterator it = blocks.begin(); it != blocks.end(); it++) {
			set<int> temp = (it->second).getNets();
			int Bno = it->first;
			for (auto& netIt : temp)
				nets[netIt].setConnectedBlocks(Bno);
		}
		//calculate weights associated with each net
		for (auto& it : nets)
			(it.second).CalParameters(WeightModel,'x', blocks);
		cout << "Nets init complete\n";
		//debug, print info
		/*
		for (auto& it : blocks) {
			(it.second).printInfo();
		}
		for (auto& it : nets) {
			it.PrintNetInfo();
		}
		*/
		//initialise nxn A matrix
		vector<vector<dataType>> Q_Matrix(NotFixed, vector<dataType>(NotFixed, 0.));

		// generate clique model matrices and compression matrices to pass to the solver
		QMatrixGen(blocks, Q_Matrix, NotFixedBlockNo, nets, '1');
		//PrintMatrix(Q_Matrix);
		BMatrixGen(blocks, bx, NotFixedBlockNo, FixedBlockNo, nets, 'x', '1');
		BMatrixGen(blocks, by, NotFixedBlockNo, FixedBlockNo, nets, 'y', '1');
		// compress matrix

		CompressMatrix(Q_Matrix, ap, ai, ax, NotFixed);
		//debug
		writeCSV(Q_Matrix, ap, ai, ax, bx, by);
		
		/*
		cout << "bx:" << endl;
		printArray(bx);
		cout << "by:" << endl;
		printArray(by);
		if (bx.size() == NotFixed && by.size() == NotFixed)
			cout << "bx and by size correct\n";
		else
			cout << "WRONG SIZE!!!!!!!!!!!!!!\n";
		cout << "ap:" << endl;
		printArray(ap);
		cout << "ai:" << endl;
		printArray(ai);
		cout << "ax:" << endl;
		printArray(ax);
		*/
		break;
	}
	case 'b': {
		WeightModel = '2';
		AlternateNetModel(ap, ai, ax, bx, by, blocks, nets, NotFixedBlockNo, FixedBlockNo, NetDrivers);
		break;
	}
	default: {
		cout << "WRONG INPUT\n";
		break;
	}
	}


#ifdef ECF
	APSolve(ap, ai, ax, bx, x, NotFixed);
	APSolve(ap, ai, ax, by, y, NotFixed);

	//write solved values
	string ofilename = "solvedX";
	ofstream OutFile;
	OutFile.open(ofilename);
	if (OutFile.is_open()) {
		for (auto& it : x)
			OutFile << it << endl;
		OutFile.close();
	}
	ofilename = "solvedY";
	OutFile.open(ofilename);
	if (OutFile.is_open()) {
		for (auto& it : y)
			OutFile << it << endl;
		OutFile.close();
	}
#else
	string ifilename = "solvedX";
	ifstream SolverFile;
	SolverFile.open(ifilename);
	if (SolverFile.is_open()) {
		dataType temp;
		while (SolverFile) {
			SolverFile >> temp;
			x.push_back(temp);
		}
		SolverFile.close();
	}
	else
		cout << "solver file x not found\n";
	ifilename = "solvedY";
	SolverFile.open(ifilename);
	if (SolverFile.is_open()) {
		dataType temp;
		while (SolverFile) {
			SolverFile >> temp;
			y.push_back(temp);
		}
		SolverFile.close();
	}
	else
		cout << "solver file y not found\n";

#endif
	cout << "solved x: " << endl;
	printArray(x);
	cout << "solved y: " << endl;
	printArray(y);

	UpdateSolverResults(x, y, blocks, NotFixedBlockNo);

	event_loop(act_on_button_press, NULL, NULL, drawscreen);

	//HPWL
	dataType SolvedHPWL = hpwl(blocks, nets);
	cout << "HPWL after the solver is " << SolvedHPWL << endl;

	//if overlap removal is needed
	if (application == 'c') {
		cout << "Choose your weight type:\nw:weak weight\nm: medium weight\ns: strong weight\n";
		char weightType;
		cin >> weightType;
		if (weightType == 'w' || weightType == 'm' || weightType == 's') {
			//erase global vector values
			bx.erase(bx.begin(), bx.begin() + bx.size());
			by.erase(by.begin(), by.begin() + by.size());
			ap.erase(ap.begin(), ap.begin() + ap.size());
			ai.erase(ai.begin(), ai.begin() + ai.size());
			ax.erase(ax.begin(), ax.begin() + ax.size());
			x.erase(x.begin(), x.begin() + x.size());
			y.erase(y.begin(), y.begin() + y.size());

			simple_overlap_removal(blocks, nets, NotFixedBlockNo, weightType, bx, by, ap, ai, ax, x, y, NotFixed, FixedBlockNo);
			UpdateSolverResults(x, y, blocks, NotFixedBlockNo);

			for (auto& blockit : blocks) {
				if (blockit.second.getBlockLocation().x > 64.0 || blockit.second.getBlockLocation().y > 64.0) {
					cout << "block no. " << blockit.second.getBlockNum() << " out of bound, this block is connected to pseudo net ";
					for (auto& netit : blockit.second.getNets())
						if (nets[netit].getInfo() == 'f')
							cout << netit << endl;
				}
			}

			event_loop(act_on_button_press, NULL, NULL, drawscreen);
		}
			
		else
			cerr << "WRONG INPUTS!!!\n";

		SolvedHPWL = hpwl(blocks, nets);
		cout << "HPWL after spreading is " << SolvedHPWL << endl;

	}
	
	event_loop(act_on_button_press, NULL, NULL, drawscreen);
	//close up
	close_graphics();

	return 0;
}



//other support funcitons
void drawscreen(void) {
	set_draw_mode(DRAW_NORMAL);
	clearscreen();
	setfontsize(10);
	setlinestyle(DASHED);
	setlinewidth(1);

	setcolor(LIGHTGREY);
	//draw grids
	if (circuitName != "cct1") {
		drawline(0.0f, 0.0f, 64.0f, 0.0f);
		drawline(0.0f, 0.0f, 0.0f, 64.0f);
		drawline(64.0f, 0.0f, 64.0f, 64.0f);
		drawline(0.0f, 64.0f, 64.0f, 64.0f);

		drawline(0.0f, 16.0f, 64.0f, 16.0f);
		drawline(0.0f, 32.0f, 64.0f, 32.0f);
		drawline(0.0f, 48.0f, 64.0f, 48.0f);

		drawline(16.0f, 0.0f, 16.0f, 64.0f);
		drawline(32.0f, 0.0f, 32.0f, 64.0f);
		drawline(48.0f, 0.0f, 48.0f, 64.0f);

		setlinestyle(SOLID);
	}
	else {
		drawline(0.0f, 0.0f, 6.0f, 0.0f);
		drawline(0.0f, 0.0f, 0.0f, 6.0f);
		drawline(6.0f, 0.0f, 6.0f, 6.0f);
		drawline(0.0f, 6.0f, 6.0f, 6.0f);
	}


	for (auto& it : nets)
//if(it.second.getInfo()=='f')
		(it.second).DrawNets(blocks, application);
	setlinestyle(SOLID);
	blocks[81].DrawCell(circuitName);
		
	for (auto& it:blocks) {
		setlinestyle(SOLID);
		(it.second).DrawCell(circuitName);
	}
	

	flushinput();
}

void act_on_button_press(float x, float y) {

	/* Called whenever event_loop gets a button press in the graphics *
	 * area.  Allows the user to do whatever he/she wants with button *
	 * clicks.                                                        */

	printf("User clicked a button at coordinates (%f, %f)\n", x, y);

	if (line_entering_demo) {
		if (rubber_band_on) {
			rubber_band_on = false;
			xx2 = x;
			yy2 = y;
			have_entered_line = true;  // both endpoints clicked on --> consider entered.

			// Redraw screen to show the new line.  Could do incrementally, but this is easier.
			drawscreen();
		}
		else {
			rubber_band_on = true;
			xx1 = x;
			yy1 = y;
			have_entered_line = false;
			have_rubber_line = false;
		}
	}

}
