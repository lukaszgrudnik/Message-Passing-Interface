/**
 * @file bptp.c
 * @author Łukasz Grudnik (lukaszgrudnik@github.com)
 * @brief Point to point blocking mode communication between two processes that sends nested structres.
 * @version 0.1
 * @date 2022-06-12
 *
 * @copyright Copyright (c) 2022
 *
 */

#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <openmpi/mpi.h>
#include <stdlib.h>

struct Department
{
    char name[250];
    int staff;
};

struct Hospital
{
    char name[250];
    int staff;
    float rate;
    struct Department department;
};

int main(int argc, char *argv[])
{

    struct Hospital hospital;
    strcpy(hospital.name, "Szpital Specjalistyczny im. Stefana Żeromskiego SP ZOZ w Krakowie");
    hospital.staff = 100;
    hospital.rate = 2.5;

    MPI_Aint lb, extent;

    MPI_Init(&argc, &argv);

    int rank = 0;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    struct Hospital buffer;
    MPI_Status status;

    // Defines new data type:
    MPI_Datatype MPI_DEPARTMENT;
    MPI_Datatype DEPARTMENT_INT_DT[] = {MPI_CHAR, MPI_INT};
    MPI_Aint DEPARTMENT_AINT[2];
    DEPARTMENT_AINT[0] = 0;
    MPI_Type_get_extent(MPI_CHAR, &lb, &extent);
    DEPARTMENT_AINT[1] = 256 * extent;
    int DEPARTMENT_BLOCKS[] = {250, 1};
    MPI_Type_create_struct(2, DEPARTMENT_BLOCKS, DEPARTMENT_AINT, DEPARTMENT_INT_DT, &MPI_DEPARTMENT);
    MPI_Type_commit(&MPI_DEPARTMENT);

    // Defines new data type:
    MPI_Datatype MPI_HOSTPITAL;
    MPI_Datatype HOSPITAL_INT_DT[] = {MPI_CHAR, MPI_INT, MPI_FLOAT, MPI_DEPARTMENT};
    MPI_Aint HOSPITAL_AINT[3];
    HOSPITAL_AINT[0] = 0;
    MPI_Type_get_extent(MPI_CHAR, &lb, &extent);
    HOSPITAL_AINT[1] = 256 * extent;
    MPI_Type_get_extent(MPI_INT, &lb, &extent);
    HOSPITAL_AINT[2] = HOSPITAL_AINT[1] + extent;
    MPI_Type_get_extent(MPI_FLOAT, &lb, &extent);
    HOSPITAL_AINT[3] = HOSPITAL_AINT[2] + extent;

    int HOSPITAL_BLOCKS[] = {250, 1, 1, 1};
    MPI_Type_create_struct(3, HOSPITAL_BLOCKS, HOSPITAL_AINT, HOSPITAL_INT_DT, &MPI_HOSTPITAL);
    MPI_Type_commit(&MPI_HOSTPITAL);

    if (!rank)
    {
        double start = MPI_Wtime();
        MPI_Send(&hospital, 1, MPI_HOSTPITAL, 1, 0, MPI_COMM_WORLD);
        MPI_Recv(&buffer, 1, MPI_HOSTPITAL, 1, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        printf("Rank: %d, Message: %s Rate: %f\n", rank, buffer.name, buffer.rate);
        double end = MPI_Wtime();

        printf("Time: %lf[s]\n", end - start);
    }
    else
    {
        MPI_Recv(&buffer, 1, MPI_HOSTPITAL, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        printf("Rank: %d, Message: %s Rate: %f\n", rank, buffer.name, buffer.rate);
        buffer.rate = 4.0;
        MPI_Send(&buffer, 1, MPI_HOSTPITAL, 0, 1, MPI_COMM_WORLD);
    }

    MPI_Finalize();
}