/*
 * Glaetten.h
 *
 *  Created on: 05.11.2010
 *      Author: martin
 */

#ifndef GLAETTEN_HH_
#define GLAETTEN_HH_

void smooth_data(int Datasize, double *Data, int Anzahl_Nachbarn_eine_Seite,
		int Zahl_der_Iterationen);

int my_moving_average(vector<double> &y, int ws);
int my_savitzky_golay(vector<double> &y, int ws);
int my_gauss_blur_1d(vector<double> &y);

double shift_wavelength(double wl);

#endif /* GLAETTEN_HH_ */
