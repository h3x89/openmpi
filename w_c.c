#include <stdio.h>
#include <string.h>
#include <mpi.h>
#include <stdlib.h>
#include <math.h>
#define MSG_SLV 1

int main(int argc, char **argv){

    // mpicc ciag.c
    // mpirun -default-hostfile none -np 3 ./a.out

    int rank, size, zm;  // rank - nr proces  // size-1 - ilosc procesow    // zm - wiadomosc
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    MPI_Status status;

    char *str, *search;
    int i, j, flag;

    int odp;

    // str = "abdbc";
    // search = "bc";
    // search = "b";

    // str = "aattccatcgatggg";
    // search = "atcgat";
    
    str = "aXXxXaattcXcatXcgcatcgatggggaaattccatcgatcaattccatcgatggggaaattccatcgatgggaattccatcgatgggtgggccatcgataataattccatcgatgggtccatcgatggggggaattaattccatcgatgggaattccatcgatgggccatcgatgggaattccatcgatggg";
    // search = "catcgatggggaaattccatcg";
    search = "XXX";

    int str_len = strlen(str);
    int search_len = strlen(search);

    if (rank == 0) {
        for ( i = 0; i <= str_len-search_len; i++){
            MPI_Recv( &odp, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            if (odp >= 0) {
                printf("\n\nSUCCESS\nposition nr: %i\n\n", odp);
                MPI_Finalize();
                exit(0);
            } else {

            }
        }
        printf("\n\nUNSUCCESS\n\n");

    }
    else{

        // po wszystkich znakach w ciagu z ograniczeniem by nie szukac poza tablica
        for ( i = 0; i <= str_len-search_len; i++){
            if ( i%(size-1)+1 == rank ) {
                // a teraz for po szuaknej frazie 
                //dzieki czemu dojedziemy do konca tego co wyzej ograniczylismy

                // przydzielenie do procesu
                // DEBUG
                // printf("rank: %i  ", rank);
                for (j = 0; j<= search_len - 1; j++){
                    // DEBUG
                    // printf("%i : %i ",str[i+j], search[j]);
                    // jezeli dopasowanie znaku jest poprawne
                    if ( str[i+j] == search[j]){
                        // DEBUG
                        // printf("T ");
                        // jezeli tez jest to ostatni znak z ciagu
                        if (search_len -1 == j){
                            // DEBUG
                            // printf("MATCH");
                            // odp = i - search_len;
                            odp = i;
                            MPI_Send( &odp, 1, MPI_INT, 0, MSG_SLV, MPI_COMM_WORLD);
                        }
                    }
                    else {
                        // DEBUG
                        // printf("F ");
                        odp = -1;
                        MPI_Send( &odp, 1, MPI_INT, 0, MSG_SLV, MPI_COMM_WORLD);
                        break;
                    }

                }
            // DEBUG
            // printf("\n");
            }
        }
    }
MPI_Finalize();
}
