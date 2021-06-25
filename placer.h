#ifndef _h_ap
#define _h_ap

#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <set>

typedef double dataType;

// location of global point
struct point {
	dataType x;
	dataType y;
};

dataType distance(point a, point b);

template <typename arrayType>
void printArray(arrayType& arr) {
	std::cout << '{';
		for (auto& it : arr) {
			std::cout << it << ',';
		}
		std::cout << '}' << std::endl;
}


void print_point(point& P);

class block {
	int num;
	std::set<int> nets;
	point location;
	dataType disToAnchor; //distance to the anchor point
	char status; //c connected to an anchor, x for available
	char info; //f for fixed block, n for non-fixed, a for anchor blocks
public:
	//default constructor
	block()
	:num(-1),info('x'), status('x'){
		location.x = -1.;
		location.y = -1.;
		disToAnchor = 0.;
	};
	//anchor block constructor
	block(int x, int y, int n)
	:status('x'){
		disToAnchor = 0.;
		num = n;
		location.x = x * 16. + 8.;
		location.y = y * 16. + 8.;
		info = 'a';
	}
	//default destructor
	~block() {};

	void setNum(int newNum);
	void setLocation(dataType xx, dataType yy);
	void setInfo(char newInfo);
	void setNet(int netNo);
	void setStatus(char newstatus);
	void setdis(dataType dis) { disToAnchor = dis; };

	void printInfo();
	void DrawCell(std::string circuit);
	
	dataType getDis() { return disToAnchor; };
	char getStatus() { return status; };
	std::set<int> getNets() { return nets; };
	int getBlockNum() { return num; };
	char getBlockInfo() { return info; };
	point getBlockLocation() { return location; };
};

class net {
	int num;
	std::set<int> connectedBlocks;
	char info; //d for driver, s for sink, x for uninitialised, f for sude net
	int PinNo;
	dataType weight;
	int netColour;
	int DrivenBlockNo;
public:
	net(int newNum) 
		:DrivenBlockNo(-1), info('x'), weight(-1.), PinNo(-1), netColour(0) {
		num = newNum;
	};
	net()
		:DrivenBlockNo(-1), info('x'), weight(-1.), PinNo(-1), netColour(0), num(-1) {};
	~net() {};

	int getNum() { return num; };
	char getInfo() { return info; };
	dataType getWeight() { return weight; };
	std::set<int> getBlocks() { return connectedBlocks; };
	int getTotalPin() { return PinNo; };
	int GetDrivenBlock() { return DrivenBlockNo; };

	void CalParameters(char mode, char weightType, std::map<int, block>& blocks); // for clique model
	//void setNum(int newNum);
	void setInfo(char newInfo);
	void setConnectedBlocks(int blockNo);
	void PrintNetInfo();
	void DrawNets(std::map<int, block>& blocks, char mode);
	void setDrivenBlock(int blockno);
};

// find the label of all unique net number and sort them from smallest  to largest
// return the total number of unit nets
int init_net_vector(std::vector<int>& SortedNets, std::map<int,block>& blocks);

void PrintMatrix(std::vector<std::vector<dataType>>& Matrix);

void QMatrixGen(std::map<int, block>& blocks,
	std::vector<std::vector<dataType>>& Q_Matrix,
	std::vector<int>& NotFixedBlockNo,
	std::map<int, net>& nets,
	char mode);

void BMatrixGen(std::map<int, block>& blocks,
	std::vector<dataType>& b,
	std::vector<int>& NotFixedBlockNo,
	std::vector<int>& FixedBlockNo,
	std::map<int, net>& nets, char axis,
	char mode);


void CompressMatrix(std::vector<std::vector<dataType>>& Q_Matrix,
	std::vector<int>& ap,
	std::vector<int>& ai,
	std::vector<dataType>& ax, int n);

int CommonNet(block* block1, block* block2, std::vector<int>& CommonNetsNo);

void UpdateSolverResults(std::vector<dataType>& x,
	std::vector<dataType>& y,
	std::map<int, block>& blocks,
	std::vector<int>& NotFixedBlockNo);

dataType hpwl(std::map<int, block>& blocks, std::map<int, net>& nets);

void simple_overlap_removal(std::map<int, block>& blocks, 
	std::map<int, net>& nets,
	std::vector<int>& NotFixedBlockNo,
	char weightType,
	std::vector<dataType>& bx,
	std::vector<dataType>& by,
	std::vector<int>& ap,
	std::vector<int>& ai,
	std::vector<dataType>& ax,
	std::vector<dataType>& x,
	std::vector<dataType>& y,
	int NotFixed,
	std::vector<int>& FixedBlockNo);
/*
void Q2QMatrixGen(std::map<int, block>& blocks,
	std::vector<std::vector<dataType>>& Q_Matrix,
	std::vector<int>& NotFixedBlockNo,
	std::map<int, net>& nets,
	std::map<int, int>& NetDrivers);

void Q2BMatrixGen(std::map<int, block>& blocks,
	std::vector<dataType>& b,
	std::vector<int>& NotFixedBlockNo,
	std::vector<int>& FixedBlockNo,
	std::map<int, net>& nets,
	char axis,
	std::map<int, int>& NetDrivers);
	*/
void AlternateNetModel(std::vector<int>& ap,
	std::vector<int>& ai,
	std::vector<dataType>& ax,
	std::vector<dataType>& bx,
	std::vector<dataType>& by,
	std::map<int, block>& blocks,
	std::map<int, net>& nets,
	std::vector<int>& NotFixedBlockNo,
	std::vector<int>& FixedBlockNo,
	std::map<int, int>& NetDrivers);

void writeCSV(std::vector<std::vector<dataType>>& Q_Matrix,
	std::vector<int>& ap,
	std::vector<int>& ai,
	std::vector<dataType>& ax, 
	std::vector<dataType>& bx,
	std::vector<dataType>& by);

#endif