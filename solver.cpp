#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <set>
#include <stdio.h>
#include "placer.h"
#include "solver.h"

#ifdef ECF
extern "C" {
#include "umfpack.h"
}
#endif

using namespace std;

void APSolve(vector<int>& ArrAp, vector<int>& ArrAi,
	vector<dataType>& ArrAx, vector<dataType>& Arrb,
	vector<dataType>& ArrX, int n) {

	cout << "Calling UMFPACK solver functions\n";
#ifdef ECF
	cout << "Start solver\n";

	int *Ap = new int[ArrAp.size()];
	int *Ai = new int[ArrAi.size()];
	dataType *Ax = new dataType[ArrAx.size()];
	dataType *b = new dataType[Arrb.size()];
	dataType *x = new dataType[n];

	//copy values to array
	for (int i = 0; i < ArrAp.size(); i++)
		Ap[i] = ArrAp[i];
	for (int i = 0; i < ArrAi.size(); i++)
		Ai[i] = ArrAi[i];
	for (int i = 0; i < ArrAx.size(); i++)
		Ax[i] = ArrAx[i];
	for (int i = 0; i < Arrb.size(); i++)
		b[i] = Arrb[i];


	double* null = (double*)NULL;
	int i;
	void *Symbolic, *Numeric;
	(void)umfpack_di_symbolic(n, n, Ap, Ai, Ax, &Symbolic, null, null);
	(void)umfpack_di_numeric(Ap, Ai, Ax, Symbolic, &Numeric, null, null);
	umfpack_di_free_symbolic(&Symbolic);
	(void)umfpack_di_solve(UMFPACK_A, Ap, Ai, Ax, x, b, Numeric, null, null);
	umfpack_di_free_numeric(&Numeric);

	//copy to result array
	for (int i = 0; i < n; i++)
		ArrX.push_back(x[i]);

	//release memory
	delete[] Ap;
	delete[] Ai;
	delete[] Ax;
	delete[] b;
	delete[] x;
#endif
}