/*
 * Retrievalgitter.h
 *
 *  Created on: 26.05.2010
 *      Author: martin
 */

#ifndef RETRIEVALGITTER_HH_
#define RETRIEVALGITTER_HH_

#include"Gitterpunkt.h"
#include "Ausgewertete_Messung_Limb.h"
#include "Konfiguration.h"

#include <vector>

class Retrievalgitter
{
public:
	//Konstruktoren
	Retrievalgitter();
	Retrievalgitter(const Retrievalgitter &rhs);
	// Überladene Operatoren
	Retrievalgitter &operator =(const Retrievalgitter &rhs);
	//Destruktor
	~Retrievalgitter();
	//Methoden
	void Retrievalgitter_erzeugen(std::vector<Ausgewertete_Messung_Limb> &AM_Limb,
			double Epsilon, Konfiguration &Konf);
	void Alle_Durchstosspunkte_Null_setzen();
	void In_Datei_Ausgeben(std::string Dateiname);

// //TODO löschen nach Implementierung im Raytracing
// // wird bei der Berechnung der AMF Matrix für Zwischenrechnungen benötigt
// void SZA_und_Streuwinkel_NULL_setzen();
// //TODO löschen nach Implementierung im Raytracing
// void SZA_und_Streuwinkel_bestimmen(double Sat_Lat,
//   double Sat_Lon, double Sat_Hoehe,
//   //Richtpunkt (Tangentenpunkt oder Grundpunkt)
//   double RP_Lat,double RP_Lon, double RP_Hoehe,
//   double Erdradius, double Deklination, double Stunde, double Minute);
//   // für jeden Strahlengang neu
// //TODO löschen nach test
// void Retrievalgitter_erzeugen_und_Messrichtung_herausfinden(
//   vector<Ausgewertete_Messung_Limb>& AM_Limb);

	// Hilfsfunktionen
	//Membervariablen
	int m_Anzahl_Hoehen;
	int m_Anzahl_Breiten;
	int m_Anzahl_Punkte;
	Gitterpunkt *m_Gitter;
};
//Hilfsfunktionen
int Get_Index_of_Maximum(std::vector<double> A);
int Get_Index_of_Minimum(std::vector<double> A, int Startindex);


#endif /* RETRIEVALGITTER_HH_ */
