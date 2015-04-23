//////////////////////////////////////////////////////////////////////
//  Robert Kubiś													
//																	
//  Politechnika Poznańska 2015										
//																	
//////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////
//  URUCHAMIANIE													
//																	
//  cat run.sh 														
//  #! /bin/bash													
//  # -g -- debug mode 												
//																	
//  g++ $1 -fopenmp pi.cpp -o pi.out && ./pi.out 					
//////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////
//  Metoda liczenia to metoda calkowania okregu						
//																	
//  http://pl.wikipedia.org/wiki/OpenMP								
//////////////////////////////////////////////////////////////////////



#include <stdio.h>
#include <time.h>
#include <omp.h>
#include <stdlib.h> 
#include <iostream>
#include <fstream>
using namespace std;

long long num_steps = 10000000;
double step;
const int Repeat = 20;
ofstream file;
clock_t start, stop;
double x, pi, suma;



//  Dodanie samego #pragma omp parallel powoduje zwiekrzenie czasu przetwarzania,
	//  bo kazdy watek oblicza wszystko i wspodzieli zmienne wiec wynik jest mniej dokladny

//  Dodanie tylko #pragma omp parallel for powoduje ze i nie jest wspoldzielone i wynik jest nie poprawny   
	//  x i suma sa modyfikowane i z odczytem problemu nie ma ale jest problem z zapisem poniewaz nastepuje brudny odczyt - nie jest to zrealizowane atomowo

void pi1(int iteration, int repeat) {
	file << "PI1 \nWartosci liczby PI ; Czas wynonania : dla iteracji: " << iteration << "\n";
	printf("PI1 \nWartosci liczby PI ; Czas wynonania : dla iteracji: %d \n", iteration);
	for (int h = 0; h < repeat; h++) {
		suma = 0.0;
		step = 1. / (double)iteration;
		start = clock();

		//  Dyrektywa PRIVATE zapewnia że dane wawnątrz równolegle przetwarzanego regionu są prywatne dla każdego wątku,
			//  co oznacza, że każdy wątek posiada lokalną kopię i używa jej jako zmienną tymczasową.
		#pragma omp parallel private(x)
		{
			#pragma omp for
			for (int i = 0; i<iteration; i++)
			{
				x = (i + .5)*step;
			
			//  Dyrektywa ATOMIC zapewnia atomowosc operacji 
					//  Zawarty blok kodu będzie przetwarzany przez tylko jeden wątek w danym czasie a nie jednocześnie przez kilka.
				#pragma omp atomic
				suma += 4.0 / (1. + x*x);	
			}
		}
		
		pi = suma * step;
		
		stop = clock();
		printf("%15.12f  ; %f sekund\n", pi, ((double)(stop - start) / CLOCKS_PER_SEC));
		file << pi << " ; " << ((double)(stop - start) / CLOCKS_PER_SEC) << "sekund\n";
	}
}

void pi2(int iteration, int repeat) {
	file << "PI2 \nWartosci liczby PI ; Czas wynonania : dla iteracji: " << iteration << "\n";
	printf("PI2 \nWartosci liczby PI ; Czas wynonania : dla iteracji: %d \n", iteration);
	for (int h = 0; h < repeat; h++) {
		suma = 0.0;
		step = 1. / (double)iteration;
		start = clock();
		
		//  Dyrektywa REDUCTION powoduje, że zmienna posiada lokalną kopię w każdym wątku, 
			//  lecz wartości lokalnych kopii są podsumowywane (redukowane) do współdzielonej globalnie zmiennej. 
			
			//  Jest to bardzo użyteczne jeśli dana operacja (w naszym przypadku "+") 
			//  pracuje na interaktywnym typie danych takim, 
			//  że jego wartość podczas konkretnej iteracji zależy od jego wartości w poprzedniej. 
			
			//  Ogólnie, kroki, które prowadzą do operacyjnej inkrementacji są zrównoleglane, 
			//  ale wątki gromadzą się i czekają na zaktualizowanie danych, 
			//  dzięki temu inkrementacja następuje w dobrym porządku co zapobiega błędnym stanom.
		#pragma omp parallel for private(x) reduction(+ : suma)
		for (int i = 0; i<iteration; i++)
		{
			x = (i + .5)*step;
			suma += 4.0 / (1. + x*x);
		}
		
		pi = suma * step;
		
		stop = clock();
		printf("%15.12f  ; %f sekund\n", pi, ((double)(stop - start) / CLOCKS_PER_SEC));
		file << pi << " ; " << ((double)(stop - start) / CLOCKS_PER_SEC) << "sekund\n";
	}
}

//  Korzystając z tablicy otrzymujemy lepszy czas, 
	//  ale gorszy niż w reduction ponieważ procesory sobie przeszkadzają występuje tak zwany falsesharing.
	//  procesory odwołują się wtedy do tego samego obszaru pamięci i mamy wtedy unieważnienie,
	//  czyli nie zawsze przetwarzenie równoległe daje efekt! musi być jeszcze efektywnie napisane!

	//  Zmienne sobie przeszkadzają jeżeli są w jednej lini.
		//  Nie przeszkadzają sobie jak wątki zapisują do różnych lini pamięci i nie ma wtedy unieważnienia,
		//  ponieważ każdy z wątków pracuje w swojej lini i nie ma unieważnień.

	// Podsumowując:
		// Program działa szybciej gdy dwa wątki piszą do dwóch lini pamięci, a wolniej gdy do tej samej.
void pi3(int iteration, int repeat) {
	file << "PI3 \nWartosci liczby PI ; Czas wynonania : dla iteracji: " << iteration << "\n";
	printf("PI3 \nWartosci liczby PI ; Czas wynonania : dla iteracji: %d \n", iteration);
	const int tab = 20;

	//  Volatile zapobiega wprowadzaniu optymalizacji poprzez kompilator.
	volatile double sumTab[tab];
	
	for (int h = 0; h < repeat; h++) {
		suma = 0.0;
		
		for (int i = 0; i < tab; i++) {
			sumTab[i] = 0,0;
		}
		step = 1. / (double)iteration;
		omp_set_num_threads(2);
		start = clock();
		
		#pragma omp parallel private(x)
		{
			int id = omp_get_thread_num();

			//  Do wyznaczania dlugosci linii PP
			printf("id:%i\n", id);
			if (id != 0) { 
				id = h+1;
				printf("Przesuniecie:%i\n", id);
			}

			
			#pragma omp for 
			for (int i = 0; i<iteration; i++)
			{
				x = (i + .5)*step;
				sumTab[id] += 4.0 / (1. + x*x);
			}
		}
		
		for (int i = 0; i < tab; i++) {
			suma += sumTab[i];
		}

		pi = suma * step;
		stop = clock();

		printf("Wynik: %15.12f  ; %f sekund\n", pi, ((double)(stop - start) / CLOCKS_PER_SEC));
		file << " Wynik : " << pi << " ; " << ((double)(stop - start) / CLOCKS_PER_SEC) << "sekund\n";
	}
}

int main(int argc, char* argv[])
{
	file.open("Wyniki.txt");

	// pi1(num_steps, Repeat);
	// printf("\n");

	// pi2(num_steps, Repeat);
	// printf("\n");

	pi3(num_steps, Repeat);
	// pi3(40000000, 20);
	printf("\n");

	file.close();
	return 0;
}

//////////////////////////////////////////////////////////////////////
//  CZASY
	//	ATOMIC - najwolniejszy 
	//  REDUCTION - najszybszy 
	//  TABLICA - średni 
//////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////
//  CPU INFO
//  model name	: Intel(R) Core(TM) i5-4200U CPU @ 1.60GHz
//  cache_alignment	: 64
//  core  :  2
//////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////
//  Wyznaczenie długości lini PP
//
// type double - 8 Bytes
//
// id:0
// id:1
// Przesuniecie:6
// Wynik:  3.141592653590  ; 0.301129 sekund
// id:0
// id:1
// Przesuniecie:7
// Wynik:  3.141592653590  ; 0.304438 sekund
// id:0
// id:1
// Przesuniecie:8
// Wynik:  3.141592653590  ; 0.210411 sekund
// id:0
// id:1
// Przesuniecie:9
// Wynik:  3.141592653590  ; 0.210555 sekund
//
//  8 * 8B = 64 B
//////////////////////////////////////////////////////////////////////
