#include <stdio.h>
#include <unistd.h>
#include <cstring>
#include <string>
#include <iostream>
#include "reg.h"
#include "init.h"
#include "stop.h"
#include "rep.h"
//#include "elementos.h"

using namespace std;

int main(int argc, char *argv[])
{
	if (argc > 0)
	{
		int opt;
		int currentArg = 0;
		if(string(argv[1]) == "stop") {
			Stop stop;
			stop.borrar(argv[2]);

		} if (string(argv[1]) == "init") {
			// siguen los comandos
			int i =5;
			int ie = 6;
			int oe = 10;
			string n = "evaluator";
			int b = 100;
			int d = 100;
			int s = 100;
			int q = 6;
			currentArg++;
			if (argc > 1) {
				while ((opt = getopt(argc, argv, "i:ie:o:n:b:d:s:q")) != -1)
				{
					switch (opt)
					{
					case 'i':
						currentArg++;
						if (optarg[0] == 'e') {
							cout << "Posiciones entrada " << argv[currentArg + 1] << endl;
							currentArg++;
						}
						else
						{
							cout << "Entradas: " << optarg << endl;
							currentArg++;
						}
						break;
					case 'o':
						currentArg++;
						if (optarg[0] == 'e')
						{
							currentArg++;
							cout << "Espacio Salida : " << argv[currentArg] << endl;
						}
						break;
					case 'n':
						currentArg = currentArg + 2;
						cout << "nombre Segmento: " << optarg << endl;
						break;
					case 'b':
						currentArg = currentArg + 2;
						cout << "Reactivos Sangre:" << optarg << endl;
						break;
					case 'd':
						currentArg = currentArg + 2;
						cout << "Reactivos detritos: " << optarg << endl;
						break;
					case 's':
						currentArg = currentArg + 2;
						cout << "Reactivos skin: " << optarg << endl;
						break;
					case 'q':
						currentArg = currentArg + 2;
						cout << "Tamaño Colas internas: " << optarg << endl;
						break;
					default:
						break;
					}
				}
			}
			//init con parametros
			Init init;
			init.init(i,ie,oe,n,b,d,s,q);
		}
		else if (string(argv[1]) == "reg")
		{
			int argumentoActual = 2;
			string nombreSeg = "evaluator";
			if (string(argv[2]) == "-n") {
				//nombre del segmento de memoria
				cout << "Se ingreso el segmento de memoria" << endl;
				argumentoActual = argumentoActual + 1;
				nombreSeg = argv[argumentoActual];
			}
			for (int s = argumentoActual + 1; s < argc; s++) {
				if (string(argv[s]).find('.') != string::npos) {
					cout << "nombre archivo" << argv[s] << endl;
					//mandar archivo al reg
					Reg reg;
					reg.leerArchivo(argv[s],nombreSeg);
				}
				else if (string(argv[s]).find('-') != string::npos) {
					cout << "Se manda al modo interactivo" << endl;
					//mandar al reg interactivo
					break;
				}
			}
		}
		else if (string(argv[1]) == "ctrl") {
			string nombreSegmento = "evaluador";
			if (argc > 2) {
				if (string(argv[2]) == "-s") {
					nombreSegmento = string(argv[4]);
				}
				//mandar a modo cntrl
			}
			else {
				//mal uso
			}
		}
		else if (string(argv[1]) == "update") {
			//mandar modo interactivo update
		}
		else if (string(argv[1]) == "rep") {
			string nombreSegmento = "evaluator";
			Rep rep;
			rep.report(nombreSegmento,'i',3);
			while ((opt = getopt(argc, argv, "s:i:m")) != -1) {
				switch (opt)
				{
				case 's':
					nombreSegmento = string(optarg);
					break;
				case 'i':
					cout << "interactivo, espera: " << optarg << endl;
					break;
				case 'm':
					cout << "va a obtener: " << optarg << " de examenes" << endl;
					break;
				}
			}
		}
	}
	return 0;
}
