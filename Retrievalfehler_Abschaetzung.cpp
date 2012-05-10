/*
 * Retrievalfehler_Abschaetzung.cpp
 *
 *  Created on: 16.09.2010
 *      Author: martin
 */

#include "Retrievalfehler_Abschaetzung.h"
#include "Konfiguration.h"
#include "MPL_Matrix.h"

using namespace std;

extern "C" {
	void dgesv_(int *N, int *NRHS, double *A, int *LDA, int *IPIV, double *B,
				int *LDB, int *INFO);
	void dgemm_(char *TRANSA, char *TRANSB, int *M, int *N, int *K,
				double *ALPHA, double *A, int *LDA, double *B, int *LDB,
				double *BETA, double *C, int *LDC);
}

int Retrievalfehler_Abschaetzung(MPL_Matrix &S_x,
								 MPL_Matrix &S_x_meas,
								 MPL_Matrix &Averaging_Kernel_Matrix,
								 const MPL_Matrix &S_apriori,
								 const MPL_Matrix &S_y,
								 MPL_Matrix &S_Breite,
								 MPL_Matrix &S_Hoehe,
								 MPL_Matrix &S_letzte_Hoehe,
								 const double &Lambda_Breite,
								 const double &Lambda_Hoehe,
								 MPL_Matrix &AMF,
								 const Konfiguration &Konf)
{
	//TODO Auch hier kann man das Gleichungssystem
	//mit ATLAS/LAPACK FUNKTIONEN LÖSEN

	// Die Formeln für die Matrizen findet man in Marcos Arbeit
	MPL_Matrix AMF_trans = AMF.transponiert();
	MPL_Matrix S_Hoehe_trans = S_Hoehe.transponiert();
	MPL_Matrix S_Breite_trans = S_Breite.transponiert();
	MPL_Matrix S_letzte_Hoehe_trans = S_Breite.transponiert();

	S_x = (AMF_trans * (S_y * AMF)); // hier noch invers, also noch invertieren
	S_x += S_apriori;
	S_x += Lambda_Hoehe * (S_Hoehe_trans * S_Hoehe);
	S_x += Lambda_Breite * (S_Breite_trans * S_Breite);
	Matrix_Invertieren(S_x);
	//Nun noch die Averaging Kernel Matrix bestimmen
	MPL_Matrix D = S_x * AMF_trans * S_y;
	Averaging_Kernel_Matrix = D * AMF;
	MPL_Matrix S_y_inv = S_y;
	Matrix_Invertieren(S_y_inv);
	S_x_meas = D * S_y_inv * D.transponiert();
	return 0;
}
///////////////////////////////////////////////////////
// Funktionsstart Matrix_Invertieren
///////////////////////////////////////////////////////
void Matrix_Invertieren(MPL_Matrix &M)
{
	// Die Funktion wandelt die Matrix M in ihre eigene Inverse um
	// Dafür muss das gehen (M quadratisch und nicht singulär)
	int N = M.m_Zeilenzahl; // M ist quadratisch !!!!
	MPL_Matrix Inverse(N, N); // Inverse als Einheitsmatrix der RHS initialisieren
	int NN = N * N; // spare die Multiplikation
	for (int i = 0; i < NN; i++) {
		Inverse.m_Elemente[i] = 0;
	}
	for (int i = 0; i < N; i++) {
		Inverse.m_Elemente[i + N * i] = 1;
	}
	int *IPIV;
	IPIV = new int[N]; // Lösung des Gleichungssystems M*Inverse=RHS vorbereiten
	for (int i = 0; i < N; i++)  {
		IPIV[i] = 0;
	}
	int NRHS = N;
	int LDA = N;
	int LDB = N;
	int INFO;
	// M in Fortran Matrix umwandeln->M_transponieren
	M = M.transponiert();
	//Solveraufruf
	dgesv_(&N, &NRHS, M.m_Elemente, &LDA, IPIV, Inverse.m_Elemente, &LDB, &INFO);
	// Die Inverse von Fortran in C++ -> Inverse transponieren
	M = Inverse.transponiert();  // Ergebnis in M deponieren
	delete[] IPIV;
}
///////////////////////////////////////////////////////
// ENDE Matrix_Invertieren
///////////////////////////////////////////////////////
