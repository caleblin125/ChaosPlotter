#include "worker.h"
#include "function.h"
#include <random>
#include <mpi.h>

worker::worker(int rank, int size) : rank(rank), size(size), window({0, 0, 0, 0, 1})
{
}

Path worker::computePath(double x, double y)
{
    std::vector<Data> ret;
    Point p = {x, y};
    ret.push_back(Data{p.x, p.y, 0.0});
    for (int i = 1; i < ITERATIONS; i++)
    {
        if (p.x * p.x + p.y * p.y > BOUND)
        {
            ret.push_back(Data{p.x, p.y, -1.0f});
            break;
        }

        p = function(p, {x, y});

        ret.push_back(Data{p.x, p.y, (double)i});
    }
    return ret;
}

void worker::recieve()
{
    int paramFlag;
    MPI_Status statusV;
    MPI_Iprobe(0, TAG_VIEWING, MPI_COMM_WORLD, &paramFlag, &statusV);
    while (paramFlag)
    {
        MPI_Recv(&window, sizeof(ViewParams), MPI_BYTE, 0, TAG_VIEWING, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        // double d = window.right - window.left;
        // window.left = ((double)rank-1)*d/((double)size - 1) + window.left;
        // window.right = ((double)rank)*d/((double)size - 1) + window.left;

        printf("Worker %d received new params: x[%e, %e] y[%e, %e] %e\n", rank, window.left, window.right, window.top, window.bottom, window.d);
        paramFlag = 0;
        MPI_Iprobe(0, TAG_VIEWING, MPI_COMM_WORLD, &paramFlag, &statusV);
    }

    int endFlag;
    MPI_Status statusE;
    MPI_Iprobe(0, TAG_SHUTDOWN, MPI_COMM_WORLD, &endFlag, &statusE);
    if (endFlag)
    {
        int msg;
        MPI_Recv(&msg, 1, MPI_INT, 0, TAG_SHUTDOWN, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        printf("Recieved End Worker %d \n", rank);
        MPI_Send(&msg, 1, MPI_INT, 0, TAG_SHUTDOWN, MPI_COMM_WORLD);
        endWorker = true;
    }
}

void worker::compute()
{
    std::vector<double> batch;
    int pathCount = 0;

    for (double cx = window.left; cx <= window.right; cx += window.d)
    {
        for (double cy = window.top; cy <= window.bottom; cy += window.d)
        {
            // double dx = window.d * ((double)random() / (double)RAND_MAX - 0.5);
            double dx = (window.d / ((double)size - 1)) * ((double)random() / (double)RAND_MAX) + window.d * ((double)rank - 1) / ((double)size - 1);
            double dy = window.d * ((double)random() / (double)RAND_MAX - 0.5);

            double xi = cx + dx;
            double yi = cy + dy;
            std::vector<Data> orbit = computePath(xi, yi);
            int size = orbit.size();
            if (size == 0)
            {
                continue;
            }
            // if (orbit.back().f < 0)
            // {
            //     continue;
            // }

            for (int i = 1; i < size; i++)
            {
                orbit.pop_back();
            }
            orbit[0].f = size;

            batch.push_back((double)orbit.size());
            for (Data d : orbit)
            {
                batch.push_back(d.x);
                batch.push_back(d.y);
                batch.push_back(d.f);
            }
            pathCount++;
        }
    }

    int batchSize = batch.size();
    MPI_Send(&batchSize, 1, MPI_INT, 0, TAG_PATH_SIZE, MPI_COMM_WORLD);
    MPI_Send(batch.data(), batchSize, MPI_DOUBLE, 0, TAG_PATH, MPI_COMM_WORLD);
}

void worker::mainloop()
{
    while (true)
    {
        recieve();
        if (endWorker)
        {
            printf("Ending Worker %d \n", rank);
            return;
        }

        compute();
    }
    return;
}
