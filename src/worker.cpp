#include "worker.h"
#include "function.h"
#include <random>
#include <mpi.h>

worker::worker(int rank, int size) : rank(rank), size(size), window({0, 0, 0, 0, 1})
{
}

Path worker::computePath(float x, float y)
{
    std::vector<Data> ret;
    Point p = {x, y};
    ret.push_back(Data{p.x, p.y, 0.0});
    for (int i = 1; i < 1000; i++)
    {
        if (p.x * p.x + p.y * p.y > 2.0)
        {
            ret.push_back(Data{p.x, p.y, -1.0f});
            break;
        }

        p = function(p, {x, y});

        ret.push_back(Data{p.x, p.y, (float)i});
    }
    return ret;
}

void worker::recieve()
{
    int paramFlag;
    MPI_Status statusV;
    MPI_Iprobe(0, TAG_VIEWING, MPI_COMM_WORLD, &paramFlag, &statusV);
    if (paramFlag)
    {
        MPI_Recv(&window, sizeof(ViewParams), MPI_BYTE, 0, TAG_VIEWING, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        
        float d = window.right - window.left;
        window.left = ((float)rank-1)*d/((float)size - 1) + window.left;
        window.right = ((float)rank)*d/((float)size - 1) + window.left;
        printf("Worker %d received new params\n", rank);
    }

    int endFlag;
    MPI_Status statusE;
    MPI_Iprobe(0, TAG_SHUTDOWN, MPI_COMM_WORLD, &endFlag, &statusE);
    if (endFlag)
    {
        int msg;
        MPI_Recv(&msg, 1, MPI_INT, 0, TAG_SHUTDOWN, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        endWorker = true;
        printf("Ending Worker %d \n", rank);
    }
}

void worker::compute()
{
    std::vector<float> batch;
    int pathCount = 0;

    for (float cx = window.left; cx <= window.right; cx += window.d)
    {
        for (float cy = window.top; cy <= window.bottom; cy += window.d)
        {
            float dx = window.d * ((float)random() / (float)RAND_MAX - 0.5);
            float dy = window.d * ((float)random() / (float)RAND_MAX - 0.5);

            float xi = cx + dx;
            float yi = cy + dy;
            std::vector<Data> orbit = computePath(xi, yi);
            int size = orbit.size();
            if (size == 0)
            {
                continue;
            }
            if (orbit.back().f < 0)
            {
                continue;
            }
            // for(int i = 1; i < size; i++){
            //     orbit.pop_back();
            // }

            batch.push_back((float)orbit.size());
            for (Data d : orbit) {
                batch.push_back(d.x);
                batch.push_back(d.y);
                batch.push_back(d.f);
            }
            pathCount++;
        }
    }

    int batchSize = batch.size();
    MPI_Send(&batchSize, 1, MPI_INT, 0, TAG_PATH_SIZE, MPI_COMM_WORLD);
    MPI_Send(batch.data(), batchSize, MPI_FLOAT, 0, TAG_PATH, MPI_COMM_WORLD);
}

void worker::mainloop()
{
    while (true)
    {
        recieve();
        if (endWorker)
        {
            break;
        }

        compute();
        if (endWorker)
        {
            break;
        }
    }
    return;
}
