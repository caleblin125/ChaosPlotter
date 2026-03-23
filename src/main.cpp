
#include <GLFW/glfw3.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <iostream>
#include <vector>
#include <mpi.h>

#include "renderer.h"

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank); // this process's ID
    MPI_Comm_size(MPI_COMM_WORLD, &size); // total number of processes
    printf("Hello from rank %d of %d\n", rank, size);

    if (rank == 0)
    {
        renderer render;
        if (!render.isError())
        {
            render.mainloop();
        }
    }
    else{
        
    }

    MPI_Finalize();
    return 0;
}