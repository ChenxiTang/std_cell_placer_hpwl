#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <fstream>
#include <algorithm>
#include <iomanip>
#include <set>
#include <cmath>
#include <functional>
#include "placer.h"
#include "graphics.h"
#include "easygl_constants.h"
#ifdef ECF
#include "solver.h"
#endif

using namespace std;

dataType distance(point a, point b) {
	return sqrt((a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y));
}


void PrintMatrix(vector<vector<dataType>>& Matrix) {
	for (vector<vector<dataType>>::iterator it = Matrix.begin(); 
		it != Matrix.end(); it++)
		printArray(*it);
}

void print_point(point& P) {
	cout << '(' << P.x << ',' << P.y << ')';
}

int CommonNet(block* block1, block* block2, vector<int>& CommonNetsNo) {
	int ExistCommon = 0;
	set<int> b1Nets = block1->getNets();
	set<int> b2Nets = block2->getNets();
	vector<int> temp(1000);
	set<int>::iterator b1it, b2it;
	b1it = b1Nets.begin();
	b2it = b2Nets.begin();
	vector<int>::iterator it;
	//cout << "here0\n";
	it = set_intersection(b1it, b1Nets.end(), b2it, b2Nets.end(), temp.begin());
	//cout << "here1\n";
	temp.resize(it - temp.begin());

	for (vector<int>::iterator iit = temp.begin(); iit != temp.end(); iit++)
		CommonNetsNo.push_back(*iit);

	//cout << "block " << block1->getBlockNum() << " and block " << block2->getBlockNum();
	//cout << " have common nets: " << endl;
	//printArray(CommonNetsNo);

	if (temp.size() != 0)
		return 1;
	else
		return 0;
}


// generate Q matrix
void QMatrixGen(map<int, block>& blocks,
	vector<vector<dataType>>& Q_Matrix,
	vector<int>& NotFixedBlockNo,
	map<int, net>& nets, char mode) {
	int x0 = NotFixedBlockNo[0];
	for (auto&RowNo:NotFixedBlockNo) { 
		block* block1 = &blocks[RowNo];
		//consider the ColNo-th non fixed block
		vector<dataType> RolWeights;
		for (vector<int>::iterator BlockNo = NotFixedBlockNo.begin();
			BlockNo != NotFixedBlockNo.end(); BlockNo++) {
			dataType tempWeight = 0.0;
			//iterate through all the not fixed blocks for connection and net weights
			if (RowNo == *BlockNo) {
				//diagonal elements
				set<int> ConnectedNets = blocks[*BlockNo].getNets();
				for (auto&it:ConnectedNets) {
					//tempWeight += nets[it].getWeight() ;
					if(mode!='2')
						tempWeight += nets[it].getWeight() * (nets[it].getTotalPin() - 1);
					else {//question 2 net model
						if (nets[it].GetDrivenBlock() == RowNo)
							tempWeight += nets[it].getWeight() * (nets[it].getTotalPin() - 1);
						else
							tempWeight += nets[it].getWeight();
					}
				}
			}
			else {
				//off diagnoal elements
				vector<int> CommonNetsNo;
				block* block2 = &blocks[*BlockNo];
				if (CommonNet(block1, block2, CommonNetsNo)) {
					for (auto& CommonNets : CommonNetsNo) {
						//get connected nets weights
						if(mode!='2')
							tempWeight -= nets[CommonNets].getWeight();
						else//question 2 net model
							if (*BlockNo == nets[CommonNets].GetDrivenBlock() || RowNo == nets[CommonNets].GetDrivenBlock())
								tempWeight -= nets[CommonNets].getWeight();
					}
				}
			}
			RolWeights.push_back(tempWeight);
		}
		Q_Matrix[RowNo-x0] = RolWeights;
	}
}

//generate b matrix
void BMatrixGen(map<int, block>& blocks,
	vector<dataType>& b,
	vector<int>& NotFixedBlockNo,
	vector<int>& FixedBlockNo,
	map<int, net>& nets,
	char axis, char mode) {
	for (auto& notFixed : NotFixedBlockNo) {
		block* tempBlock = &blocks[notFixed];
		dataType BValue = 0.;
		for (auto& fixed : FixedBlockNo) {
			vector<int> TempCommonNetsNo;
			block* TempFixedBlock = &blocks[fixed];
			dataType FixedBlockLocation;
			if (axis == 'x') {
				//if we are calculating x-axis
				FixedBlockLocation = TempFixedBlock->getBlockLocation().x;
			}
			else {
				//if we are calculating y-axis
				FixedBlockLocation = TempFixedBlock->getBlockLocation().y;
			}
			if (CommonNet(tempBlock, TempFixedBlock, TempCommonNetsNo)) {
				//if this not fixed block has a common connection with one of the
				// fixed block
					
				for (auto& NetIt : TempCommonNetsNo) {
					if (mode != '2') {
						BValue += nets[NetIt].getWeight() * FixedBlockLocation;
					}
					else
						if(nets[NetIt].GetDrivenBlock() == notFixed || nets[NetIt].GetDrivenBlock() == fixed) {
						BValue += nets[NetIt].getWeight() * FixedBlockLocation;
					}

					//cout << BValue << ' ';
				}
			}
		}
		b.push_back(BValue);
	}
}

int init_net_vector(vector<int>& SortedNets,
	map<int, block>& blocks) {
	int NoNets = 0;
	set<int> UniqueNets;

	for (auto& it:blocks) {
		set<int> TempNets = (it.second).getNets();
		for (auto& iit:TempNets)
			UniqueNets.insert(iit);
	}

	for (auto& it : UniqueNets)
		SortedNets.push_back(it);
	NoNets = SortedNets.size();
	//debug
	/*
	cout << "There are " << NoNets << " unique nets, net numbers are:" << endl;
	for (auto& it : SortedNets)
		cout << it << ' ';
	cout << endl;
	*/
	return NoNets;
}

void CompressMatrix(vector<vector<dataType>>& Q_Matrix,
	vector<int>& ap, vector<int>& ai, vector<dataType>& ax, int n) {
	int count = 0;
	ap.push_back(count);
	for (int col = 0; col < n; col++) {
		for (int row = 0; row < n; row++) {
			dataType temp = Q_Matrix[col][row];
			if (temp != 0.) {
				ax.push_back(temp);
				count++;
				ai.push_back(row);
			}
		}
		ap.push_back(count);
	}
}

//write x,y values to blocks
void UpdateSolverResults(vector<dataType>& x,
	vector<dataType>& y,
	map<int, block>& blocks,
	vector<int>& NotFixedBlockNo) {

	dataType xx, yy;

	for (int i = 0; i < NotFixedBlockNo.size(); i++) {
		xx = x[i];
		yy = y[i];
		int BlockNo = NotFixedBlockNo[i];
		blocks[BlockNo].setLocation(xx, yy);
	}
}

dataType hpwl(map<int, block>& blocks, map<int, net>& nets) {
	dataType wl = 0.;

	for (map<int, net>::iterator netIt = nets.begin(); netIt != nets.end(); netIt++) {
		//iterate through all the nets
		if (netIt->second.getInfo() != 'f') {
			// excluding fake nets
			float singleWL = 0.;
			set<int> blockNo = (netIt->second).getBlocks();
			vector<dataType> xx, yy;
			for (auto& blockIt : blockNo) {
				xx.push_back(blocks[blockIt].getBlockLocation().x);
				yy.push_back(blocks[blockIt].getBlockLocation().y);
			}
			//find the smallest and largest xx and yy
			const vector<dataType>::iterator xxit = xx.begin();
			const vector<dataType>::iterator yyit = yy.begin();
			vector<dataType>::iterator samllestX = min_element(xxit, xxit + xx.size());
			vector<dataType>::iterator largestX = max_element(xxit, xxit + xx.size());
			vector<dataType>::iterator samllestY = min_element(yyit, yyit + yy.size());
			vector<dataType>::iterator largestY = max_element(yyit, yyit + yy.size());

			dataType deltax = abs(*largestX - *samllestX);
			dataType deltay = abs(*largestY - *samllestY);
			singleWL = deltax + deltay;

			//cout << "Single HPWL of net " << netIt->getNum() << ": " << singleWL << endl;

			wl += singleWL;
		}
	}


	return wl;
}


void partitionBlocks(map<int, block>& blocks, map<int, net>& nets,
	vector<int>& NotFixedBlockNo,
	vector<net>& pseudoNets,
	vector<int>& anchorNos,
	map<int,block>& anchors) {

	//sort distance by ascending order with lambda expression
	typedef function<bool(pair<int, dataType>, pair<int, dataType>)> comparator;
	comparator compFunctor = [](pair<int, dataType> elem1, pair<int, dataType> elem2) {
		return elem1.second < elem2.second;
	};
	static int PseudoNetIndex = nets.rbegin()->first;
	//cout << PseudoNetIndex << endl;
	//partition non-fixed blocks
		//if cannot be equally divided, partition the first partitionNo*16 blocks to the 16 nearest anchors
	int partitionNo = (NotFixedBlockNo.size() - NotFixedBlockNo.size() % 16) / 16;
	for (auto& anchorit : anchorNos) {
		map<int, dataType> nonFixedDis; //store distance info for non fixed blocks
		for (auto& blockNo : NotFixedBlockNo) {
			if (blocks[blockNo].getStatus() == 'x') {
				//if this block is not already assigned an anchor, calculate distance
				nonFixedDis[blockNo] = distance(blocks[blockNo].getBlockLocation(), blocks[anchorit].getBlockLocation());
			}
		}
		set<pair<int, dataType>, comparator> sortedDistances(nonFixedDis.begin(), nonFixedDis.end(), compFunctor);

		//push the nearest neighbours to the anchor and change status
		for (int i = 0; i < partitionNo; i++) {
			set<pair<int, dataType>, comparator>::iterator NearestNeighbour = next(sortedDistances.begin(), i); // return an iterator to ith nearest cell
			int blockno = NearestNeighbour->first;
			dataType dis = NearestNeighbour->second;
			PseudoNetIndex++;
			blocks[blockno].setStatus('c');
			blocks[blockno].setdis(dis);
			blocks[anchorit].setNet(PseudoNetIndex);
			blocks[blockno].setNet(PseudoNetIndex);
			net temp(PseudoNetIndex);
			temp.setConnectedBlocks(blockno);
			temp.setConnectedBlocks(anchorit);
			temp.setInfo('f');//set it to a fake net for different weight calculation
			nets[PseudoNetIndex] = temp;
			cout << PseudoNetIndex << ' ';
		}
		cout << endl;
	}
		//for the remaining ones, group it to its nearest anchor
	if (partitionNo * 16 != NotFixedBlockNo.size()) {
		map<int, block> UnanchoredBlocks;
		int blockCount = 0;
		//search for all not anchored cells
		for (auto& it : blocks)
			if (it.second.getStatus() == 'x' && it.second.getBlockInfo() != 'f' && it.second.getBlockInfo() != 'a') {
				UnanchoredBlocks[blockCount] = it.second;
				blockCount++;
			}

		for (int i = 0; i < blockCount; i++) {
			PseudoNetIndex++;
			map<int, dataType> remainBlockDis;//distance of a block to 16 anchors
			for (auto& anchorit : anchorNos) {
				//if this block is not already assigned an anchor, calculate distance
				remainBlockDis[anchorit] = distance(UnanchoredBlocks[i].getBlockLocation(), blocks[anchorit].getBlockLocation());
			}
			set<pair<int, dataType>, comparator> AnchorToBlockDis(remainBlockDis.begin(), remainBlockDis.end(), compFunctor);
			set<pair<int, dataType>, comparator>::iterator NearestNeighbour = AnchorToBlockDis.begin();
			int anchorBlockNo = NearestNeighbour->first;
			dataType dis = NearestNeighbour->second;

			blocks[UnanchoredBlocks[i].getBlockNum()].setStatus('c');
			blocks[UnanchoredBlocks[i].getBlockNum()].setdis(dis);
			blocks[UnanchoredBlocks[i].getBlockNum()].setNet(PseudoNetIndex);
			blocks[anchorBlockNo].setNet(PseudoNetIndex);
			net temp(PseudoNetIndex);
			temp.setConnectedBlocks(anchorBlockNo);
			temp.setConnectedBlocks(UnanchoredBlocks[i].getBlockNum());
			temp.setInfo('f');//set it to a fake net for different weight calculation
			nets[PseudoNetIndex] = temp;
		}
	}
	/*
		//debug
		for (auto& it : blocks) {
			if (it.second.getBlockInfo() == 'a') {

				cout << "Block " << it.second.getBlockNum() << " is an anchor, connected to ";
				set<int> tempnetinfo = it.second.getNets();
				printArray(tempnetinfo);

				for (auto& iit : it.second.getNets())
					if (nets[iit].getInfo() != 'f')
						cout << "net no." << iit << "is not a pseudo net but connected to block " << it.second.getBlockNum() << endl;
			}
		}
		for (auto& it : nets) {
			if (it.second.getInfo() == 'f') {
				set<int> tempBlockNos = it.second.getBlocks();
				cout << "Pseudo net " << it.second.getNum() << " connected to blocks";
				printArray(tempBlockNos);
				for (auto& iit : tempBlockNos)
					if (blocks[iit].getBlockInfo() == 'f')
						cout << "block " << iit << "connected to pseudo net " << it.second.getNum() << endl;
			}
		}
		*/
}

//simple overlap removal routine
void simple_overlap_removal(map<int, block>& blocks, map<int, net>& nets,
	vector<int>& NotFixedBlockNo, char weightType, vector<dataType>& bx,
	vector<dataType>& by, vector<int>& ap, vector<int>& ai, vector<dataType>& ax,
	vector<dataType>& x, vector<dataType>& y, int NotFixed, vector<int>& FixedBlockNo) {
	map<int, block> anchors;
	vector<net> pseudoNets;
	int MaxNetNo = (nets.rbegin())->first;
	// gnerate anchor blocks and sudo nets
	vector<int> anchorNos;
	for (int yaxis = 0; yaxis < 4; yaxis++) {
		for (int xaxis = 0; xaxis < 4; xaxis++) {
			MaxNetNo++;
			int index = blocks.size() + 1;
			blocks[index] = block(xaxis, yaxis, index);
			//blocks[index].setNet(nets.size() + 1);
			anchorNos.push_back(index);
			FixedBlockNo.push_back(index);
		}
	}

	partitionBlocks(blocks, nets, NotFixedBlockNo, pseudoNets, anchorNos, anchors);

	//recalculate weights based on clique model and weight type choice
	for (auto& it : nets) {
		(it.second).CalParameters('3', weightType, blocks);
		if (it.second.getInfo() == 'f')
			cout << "Net no." << (it.first) << "weight is " << (it.second).getWeight() << endl;
	}
		

	//start the placer
	//initialise nxn A matrix
	vector<vector<dataType>> Q_Matrix(NotFixed, vector<dataType>(NotFixed, 0.));

	// generate clique model matrices and compression matrices to pass to the solver
	QMatrixGen(blocks, Q_Matrix, NotFixedBlockNo, nets, '3');
	//PrintMatrix(Q_Matrix);
	BMatrixGen(blocks, bx, NotFixedBlockNo, FixedBlockNo, nets, 'x', '3');
	BMatrixGen(blocks, by, NotFixedBlockNo, FixedBlockNo, nets, 'y', '3');
	// compress matrix

	CompressMatrix(Q_Matrix, ap, ai, ax, NotFixed);
//debug
		writeCSV(Q_Matrix, ap, ai, ax, bx, by);
#ifdef ECF
	APSolve(ap, ai, ax, bx, x, NotFixed);
	APSolve(ap, ai, ax, by, y, NotFixed);
	//cout << "solved x: " << endl;
	//printArray(x);
	//cout << "solved y: " << endl;
	//printArray(y);

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
#endif
}



void writeCSV(vector<vector<dataType>>& Q_Matrix,
	vector<int>& ap,
	vector<int>& ai,
	vector<dataType>& ax,
	vector<dataType>& bx,
	vector<dataType>& by) {
	string fileName;
	ofstream OutFile;

	fileName = "q.csv";
	OutFile.open(fileName);
	if (OutFile.is_open()) {
		for (auto& it : Q_Matrix)
			for (auto& iit : it)
				OutFile << iit << ',';
		OutFile.close();
	}

	fileName = "bx.csv";
	OutFile.open(fileName);
	if (OutFile.is_open()) {
		for (auto& it : bx)
				OutFile << it << ',';
		OutFile.close();
	}

	fileName = "by.csv";
	OutFile.open(fileName);
	if (OutFile.is_open()) {
		for (auto& it : by)
			OutFile << it << ',';
		OutFile.close();
	}
}

void AlternateNetModel(vector<int>& ap,
	vector<int>& ai,
	vector<dataType>& ax,
	vector<dataType>& bx,
	vector<dataType>& by,
	map<int, block>& blocks,
	map<int, net>& nets,
	vector<int>& NotFixedBlockNo,
	vector<int>& FixedBlockNo,
	map<int, int>& NetDrivers) {

	//second net model
	cout << "Start generating net model for question 2\n";
	for (map<int, block>::iterator it = blocks.begin(); it != blocks.end(); it++) {
		set<int> temp = (it->second).getNets();
		int Bno = it->first;
		for (auto& netIt : temp)
			nets[netIt].setConnectedBlocks(Bno);
	}
	//calculate weights associated with each net
	for (auto& it : nets)
		(it.second).CalParameters('2', 'x', blocks);
	cout << "Nets init complete\n";

	//initialise nxn A matrix
	vector<vector<dataType>> Q_Matrix(NotFixedBlockNo.size(), vector<dataType>(NotFixedBlockNo.size(), 0.));

	QMatrixGen(blocks, Q_Matrix, NotFixedBlockNo, nets, '2');
	//PrintMatrix(Q_Matrix);
	BMatrixGen(blocks, bx, NotFixedBlockNo, FixedBlockNo, nets, 'x', '2');
	BMatrixGen(blocks, by, NotFixedBlockNo, FixedBlockNo, nets, 'y', '2');
	// compress matrix

	CompressMatrix(Q_Matrix, ap, ai, ax, NotFixedBlockNo.size());

	cout << "bx:" << endl;
printArray(bx);
cout << "by:" << endl;
printArray(by);
if (bx.size() == NotFixedBlockNo.size() && by.size() == NotFixedBlockNo.size())
	cout << "bx and by size correct\n";
else
	cout << "WRONG SIZE!!!!!!!!!!!!!!\n";
cout << "ap:" << endl;
printArray(ap);
cout << "ai:" << endl;
printArray(ai);
cout << "ax:" << endl;
printArray(ax);

	//debug
	writeCSV(Q_Matrix, ap, ai, ax, bx, by);

}
/*
// generate Q matrix for q2
void Q2QMatrixGen(map<int, block>& blocks,
	vector<vector<dataType>>& Q_Matrix,
	vector<int>& NotFixedBlockNo,
	map<int, net>& nets,
	map<int,int>& NetDrivers) {
	int x0 = NotFixedBlockNo[0];
	for (auto& RowNo : NotFixedBlockNo) {
		block* block1 = &blocks[RowNo];
		//consider the ColNo-th non fixed block
		vector<dataType> RolWeights;
		for (vector<int>::iterator BlockNo = NotFixedBlockNo.begin();
			BlockNo != NotFixedBlockNo.end(); BlockNo++) {
			dataType tempWeight = 0.0;
			//iterate through all the not fixed blocks for connection and net weights
			if (RowNo == *BlockNo) {
				//diagonal elements
				set<int> ConnectedNets = blocks[*BlockNo].getNets();
				for (auto& it : ConnectedNets) {
					//tempWeight += nets[it].getWeight() ;
					if(nets[it].GetDrivenBlock()==RowNo)
						tempWeight += nets[it].getWeight() * (nets[it].getTotalPin() - 1);
					else
						tempWeight += nets[it].getWeight() ;
				}
			}
			else {
				//off diagnoal elements
				vector<int> CommonNetsNo;
				block* block2 = &blocks[*BlockNo];
				if (CommonNet(block1, block2, CommonNetsNo)) {
					//cout << "common nets between block " << *BlockNo << " and block " << RowNo << endl;
					//printArray(CommonNetsNo);
					for (auto& CommonNets : CommonNetsNo) {
						
						//get connected nets weights
						if(*BlockNo== nets[CommonNets].GetDrivenBlock() || RowNo == nets[CommonNets].GetDrivenBlock())
							tempWeight -= nets[CommonNets].getWeight();
					}
				}
			}
			RolWeights.push_back(tempWeight);
		}
		Q_Matrix[RowNo - x0] = RolWeights;
	}
}

//generate b matrix
void Q2BMatrixGen(map<int, block>& blocks,
	vector<dataType>& b,
	vector<int>& NotFixedBlockNo,
	vector<int>& FixedBlockNo,
	map<int, net>& nets,
	char axis,
	map<int, int>& NetDrivers) {
	for (auto& notFixed : NotFixedBlockNo) {
		block* tempBlock = &blocks[notFixed];
		dataType BValue = 0.;
		vector<int> usedConnection;
		for (auto& fixed : FixedBlockNo) {
			vector<int> TempCommonNetsNo;
			block* TempFixedBlock = &blocks[fixed];
			dataType FixedBlockLocation;
			
			if (axis == 'x') {
				//if we are calculating x-axis
				FixedBlockLocation = TempFixedBlock->getBlockLocation().x;
			}
			else {
				//if we are calculating y-axis
				FixedBlockLocation = TempFixedBlock->getBlockLocation().y;
			}
			if (CommonNet(tempBlock, TempFixedBlock, TempCommonNetsNo)) {
				cout << "common nets between block " << notFixed << " and block " << fixed << endl;
				printArray(TempCommonNetsNo);
				//if this not fixed block has a common connection with one of the
				// fixed block
				for (auto& NetIt : TempCommonNetsNo) {
					if (nets[NetIt].GetDrivenBlock() == notFixed || nets[NetIt].GetDrivenBlock() == fixed) {
						BValue += nets[NetIt].getWeight() * FixedBlockLocation;
						usedConnection.push_back(NetIt);
					}
					//cout << BValue << ' ';
				}
			}
		}
		cout << "Block " << notFixed << " size " << usedConnection.size() << endl;
		b.push_back(BValue);
	}
}
*/
