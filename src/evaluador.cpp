#include <stdio.h>
#include <unistd.h>
#include <cstring>
#include <string>
#include <iostream>
#include "reg.h"
#include "init.h"
#include "stop.h"
#include "rep.h"
#include "control.h"

using namespace std;
static Init init;

int main(int argc, char *argv[])
{

	if (argc > 0)
	{
		int opt;
		int currentArg = 0;
		if (string(argv[1]) == "stop")
		{
			string nombreSegmento = "evaluator";
			if (argc > 2)
			{
				if (string(argv[2]) == "-n")
				{
					nombreSegmento = string(argv[3]);
				}
			}
			Stop stop;
			stop.borrar(nombreSegmento);
		}
		if (string(argv[1]) == "init"){
			int i = 5;
			int ie = 6;
			int oe = 10;
			string n = "evaluator";
			int b = 100;
			int d = 100;
			int s = 100;
			int q = 6;
			currentArg++;
			if (argc > 1)
			{
				while ((opt = getopt(argc, argv, "i:ie:o:n:b:d:s:q:")) != -1)
				{
					switch (opt)
					{
					case 'i':
						currentArg++;
						if (optarg[0] == 'e')
						{
							ie = atoi(argv[currentArg + 1]);
							currentArg++;
						}
						else
						{
							i = atoi(optarg);
							currentArg++;
						}
						break;
					case 'o':
						currentArg++;
						if (optarg[0] == 'e')
						{
							currentArg++;
							oe = atoi(argv[currentArg]);
						}
						break;
					case 'n':
						currentArg = currentArg + 2;
						n = string(optarg);
						break;
					case 'b':
						currentArg = currentArg + 2;
						b = atoi(optarg);
						break;
					case 'd':
						currentArg = currentArg + 2;
						d = atoi(optarg);
						break;
					case 's':
						currentArg = currentArg + 2;
						s = atoi(optarg);
						break;
					case 'q':
						currentArg = currentArg + 2;
						q = atoi(optarg);
						break;
					default:
						break;
					}
				}
			}
			init.init(i, ie, oe, n, b, d, s, q);
		}else if (string(argv[1]) == "reg"){
			int argumentoActual = 2;
			string nombreSeg = "evaluator";
			if (string(argv[2]) == "-n")
			{
				argumentoActual = argumentoActual + 1;
				nombreSeg = argv[argumentoActual];
				argumentoActual = argumentoActual + 1;
			}
			for (int s = argumentoActual; s < argc; s++){
				Reg reg;
				if (string(argv[s]).find('-') != string::npos){
					reg.interactivo(nombreSeg);
				}else{
					reg.leerArchivo(argv[s], nombreSeg);
				}
			}
		}else if (string(argv[1]) == "ctrl"){
			string nombreSegmento = "evaluator";
			if (argc > 2){
				if (string(argv[2]) == "-n"){
					nombreSegmento = string(argv[3]);
				}
			}
			Ctrl control;
			control.control(nombreSegmento);
		}else if (string(argv[1]) == "rep"){
			string nombreSegmento = "evaluator";
			char caso;
			int cantidad = 0;
			while ((opt = getopt(argc, argv, "s:i:m:")) != -1)
			{
				switch (opt)
				{
				case 's':
					nombreSegmento = string(optarg);
					break;
				case 'i':
					caso = 'i';
					cantidad = atoi(optarg);
					break;
				case 'm':
					caso = 'm';
					cantidad = atoi(optarg);
					break;
				}
			}
			Rep rep;
			rep.report(nombreSegmento,caso, cantidad);
		}
	}
	return 0;
}
