/*
 * Limbauswertung.cpp
 *
 *  Created on: 28.04.2010
 *      Author: martin
 */

/*************************************************************************
 Was passiert hier:
 Die Limbdateien werden aus der Orbitliste geladen.

 Die Interpolation des Sonnenspektrums auf das Limbmessungsspektrum muss nur
 einmal erfolgen.  Da das Limbgitter vom Anfang her dem Nadirgitter entspricht,
 geschiet dies insgesamt auch nur einmal. Deshalb muss die
 erste Datei aus der Orbitliste eine Limbdatei sein.

 Die gemessene Intensität ist mit der Zeilendichte linear über den sogenannten
 Emissivitätsfaktor, oder auch g-Factor verbunden.
 *************************************************************************/

#include <vector>
#include <iterator>
#include "Orbitliste.h"
#include "Sonnenspektrum.h"
#include "Speziesfenster.h"
#include "Ausgewertete_Messung_Limb.h"
#include "Messung_Limb.h"
#include "Datei_IO.h"  //ReadL1C_Limb
#include "Messungs_ausschliessende_Tests.h"
#include "NO_emiss.h"

using namespace std;

////////////////////////////////////////////////////////////////////////////////
// Funktionsstart Limb_Auswertung
////////////////////////////////////////////////////////////////////////////////
int Limb_Auswertung(Orbitliste &Orbitlist,
					int l,
					Sonnenspektrum &Solspec,
					vector<Speziesfenster>& Spezies_Fenster,
					int &counter_Nachtmessungen,
					int &counter_NLC_detektiert,
					int &counter_Richtungsvektor_nicht_ok,
					string Arbeitsverzeichnis, string mache_Fit_Plots,
					string limb_meso_thermo,       // "ja" oder "nein"
					vector<Ausgewertete_Messung_Limb>& Ausgewertete_Limbmessung_MgI,
					vector<Ausgewertete_Messung_Limb>& Ausgewertete_Limbmessung_MgII,
					vector<Ausgewertete_Messung_Limb>& Ausgewertete_Limbmessung_unknown,
					vector<Ausgewertete_Messung_Limb>& Ausgewertete_Limbmessung_FeI,
					vector<Ausgewertete_Messung_Limb>& Ausgewertete_Limbmessung_NO,
					Konfiguration &Konf)
{
	unsigned int k;
	static double last_orbit_phase = 0.;
	static double last_latitude_tp = 90.;
	//Einmalig die Rohdaten aus der Datei Laden
	vector<Messung_Limb> Rohdaten;
	// Achtung das ist noch nicht der entgültige Vektor, weil dieser noch um die
	// Speziesparameter ergänzt werden muss
	Messung_Limb Tropo;  //Hier stecken NUR die Intensitäten der
						 // troposphärischen Säule drin....
						 // bzw der tiefsten bei Limb_meso_thermo
	Messung_Limb space; // intensities at ~ 360 km
	Messung_Limb mean_10_20;
	//Hier stecken NUR die Intensitäten der Säulen 10 bis 20 (ca 30-60km) drin
	//cerr<<"Rohdaten einlesen\n";
	if (limb_meso_thermo != "ja") {
		Rohdaten =
			ReadL1C_Limb_mpl_binary(Orbitlist.m_Dateinamen[l], Tropo, mean_10_20);
	} else {
		//Rohdaten =
		//ReadL1C_Limb_meso_thermo_mpl_binary(Orbitlist.m_Dateinamen[l], Tropo);
		int Anzahl_Hoehen = 30;
		Rohdaten =
			ReadL1C_Limb_meso_thermo_mpl_binary_reduziert(Orbitlist.m_Dateinamen[l],
					Tropo, space, Anzahl_Hoehen);
	}
	//cerr<<Orbitlist.m_Dateinamen[l]<<" wird bearbeitet\n";
	//Testen, ob ReadL1C ordentlich gearbeitet hat
	//Rohdaten[0].Ausgabe_in_Datei("CHECKDATA/Rohdaten_erste_Limb_Messung.txt");
	//-> Das geht jetzt

	Solspec.Interpolieren(Rohdaten[0]);
	//Solspec.nicht_interpolieren();

	// Testen, ob die Interpolation erfolgreich war
	//Solspec.Speichern("CHECKDATA/Sonne_interpoliert_auf_826.txt"); ->ok

	//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	// Hier wäre ein guter Ort, um zu prüfen, ob die Rohdaten weiter verwendet
	// werden dürfen
	//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	bool ist_Nachtmessung;
	if (limb_meso_thermo != "ja") {
		ist_Nachtmessung = Test_auf_Nachtmessung_Limb(Tropo, Konf);
	} else {
		ist_Nachtmessung = Test_auf_Nachtmessung_Limb_meso_thermo(Tropo, Konf);
	}
	if (ist_Nachtmessung == true) {
		std::cout << "# night point at lat = " << Rohdaten[0].m_Latitude_TP
			<< ", alt = " << Rohdaten[0].m_Hoehe_TP << std::endl;
		counter_Nachtmessungen++;
		return 1;  //Nachtmessung 1
	}

	if (Test_auf_NLC_Limb(Rohdaten, Konf) == true) {
		std::cout << "# NLC at lat = " << Rohdaten[0].m_Latitude_TP
			<< ", alt = " << Rohdaten[0].m_Hoehe_TP << std::endl;
		counter_NLC_detektiert++;
		return 2;  //NLC 2
	}
	Test_auf_korrekte_geolocations_Limb(Rohdaten, counter_Richtungsvektor_nicht_ok);
	if (test_auf_SAA_limb(space) && test_auf_SAA_limb(*(Rohdaten.end() - 2))) {
		std::cout << "# SAA at lat = " << Rohdaten[0].m_Latitude_TP
			<< ", alt = " << Rohdaten[0].m_Hoehe_TP << std::endl;
		return 1;
	}

	// skip after-pole points
	if (Rohdaten[0].m_Latitude_TP > last_latitude_tp
		&& Rohdaten[0].m_orbit_phase > last_orbit_phase) {
		std::cout << "# after pole at lat = " << Rohdaten[0].m_Latitude_TP
			<< ", alt = " << Rohdaten[0].m_Hoehe_TP << std::endl;
		return 1;
	}
	last_latitude_tp = Rohdaten[0].m_Latitude_TP;
	last_orbit_phase = Rohdaten[0].m_orbit_phase;
	//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	//
	//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

	// Jedes Element von Rohdaten wird nun in seperaten Schritten mit den
	// Speziesdaten aufgefüllt und die Bestimmung der Zeilendichte wird
	// durchgeführt
	vector<Messung_Limb>::iterator mlit;
	vector<Speziesfenster>::iterator sfit;
	vector<Liniendaten>::iterator ldit;
	//Schleife über alle Rohdaten
	for (mlit = Rohdaten.begin(); mlit != Rohdaten.end(); ++mlit) {
		//Übergabe der Rohdaten, sodass diese nicht verändert werden
		// wegen Parallelisierung
		// sin und cos sind langsame Funktionen...
		// werden aber hierbei auch nicht oft eingesetzt
		mlit->Deklinationswinkel_bestimmen(); //Sonnenlatitude
		mlit->Sonnen_Longitude_bestimmen();
		mlit->Intensitaeten_normieren(Solspec.m_Int_interpoliert);
		// m_Intensitaeten enthält nun nichtmehr I sondern I/(piF)
		// Das könnte man auch nur für die Par Fenster durchführen
		//Schleife über alle Spezies wie z.b. Mg oder Mg+
		for (sfit = Spezies_Fenster.begin(); sfit != Spezies_Fenster.end(); ++sfit) {
			//Schleife über alle Linien dieser Spezies
			for (k = 0, ldit = sfit->m_Liniendaten.begin();
					ldit != sfit->m_Liniendaten.end(); k++, ++ldit) {
				// Aus SZA_TP und SAA_TP lässt sich die Polararisation in den
				// Liniendaten des Speziesfensters ermitteln, und damit die
				// Emissivität berechnen
				//Spezfenst.m_Liniendaten[k].m_theta=Messung.m_Streuwinkel;
				//der Streuwinkel muss woanders berechnet werden
				// Die Phasenfunktion, steckt so nun in den Slant Coloumns drin
				ldit->Emissivitaet_ermitteln();
				mlit->Intensitaeten_durch_piF_Gamma_berechnen((*sfit), ldit->m_Gamma);
				// In der Formel ist piF in W/(m^2*Wellenlänge) verlangt..
				// also muss noch mit der Kanalbreite multipliziert werden
				mlit->Intensitaeten_durch_piF_Gamma_mal_Gitterabstand_berechnen((*sfit));

				// Jetzt Zeilendichte und Fehler bestimmen
				// Hmm hier gibts noch Diskussionsbedarf
				if (sfit->m_Spezies_Name != "NO")
					mlit->Zeilendichte_Bestimmen((*sfit), k,
							Arbeitsverzeichnis, mache_Fit_Plots);

				// Ergebnis zusammenfassen
				Ausgewertete_Messung_Limb Ergebnis
					= mlit->Ergebnis_Zusammenfassen();

				// Die braucht man später für die Luftmassenmatrix
				Ergebnis.m_Wellenlaenge
					= ldit->m_Wellenlaenge;
				//Ergebnis.Ausgabe_auf_Bildschirm();
				// Zusammenfassung der Zwischenresultate dem Vektor für die
				// jeweilige Spezies zuordnen
				if (sfit->m_Spezies_Name == "MgI") {
					//TODO negative Werte zulassen
					if (Ergebnis.m_Zeilendichte > 0) {
						Ausgewertete_Limbmessung_MgI.push_back(Ergebnis);
					} else {
						Ergebnis.m_Zeilendichte = 0;
						Ergebnis.m_Fehler_Zeilendichten
							= Ergebnis.m_Fehler_Zeilendichten;
						Ausgewertete_Limbmessung_MgI.push_back(Ergebnis);
					}
				}
				if (sfit->m_Spezies_Name == "MgII") {
					Ausgewertete_Limbmessung_MgII.push_back(Ergebnis);
				}
				if (sfit->m_Spezies_Name == "unknown") {
					Ausgewertete_Limbmessung_unknown.push_back(Ergebnis);
				}
				if (sfit->m_Spezies_Name == "FeI") {
					Ausgewertete_Limbmessung_FeI.push_back(Ergebnis);
				}
				if (sfit->m_Spezies_Name == "NO") {
					// create new object, same transition but modelled temperature
					double temp = mlit->msise_temperature();
					NO_emiss NO_new(sfit->NO_vec.at(k).get_vu(),
							sfit->NO_vec.at(k).get_vl(),
							sfit->NO_vec.at(k).get_vl_abs(),
							temp);
					NO_new.solar = sfit->NO_vec.at(k).solar;
					NO_new.read_luque_data_from_file("Luqueetal.dat");
					NO_new.calc_excitation();
					NO_new.calc_line_emissivities();
					NO_new.scia_convolve(Rohdaten.at(0));
					double wl = NO_new.get_scia_wl_at_max();
					mlit->slant_column_NO(NO_new, mache_Fit_Plots, Solspec, k,
							*sfit, Arbeitsverzeichnis);
					Ergebnis = mlit->Ergebnis_Zusammenfassen();
					Ergebnis.m_Wellenlaenge
						= ldit->m_Wellenlaenge
						= sfit->m_Wellenlaengen.at(k)
						= wl;
					Ausgewertete_Limbmessung_NO.push_back(Ergebnis);
				}

			}//ende k Linie
		}//ende j Spezies_Fenster
	}//ende i Rohdaten

	return 0; // keine Probleme
}
////////////////////////////////////////////////////////////////////////////////
// ENDE Limb_Auswertung
////////////////////////////////////////////////////////////////////////////////
