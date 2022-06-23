#ifndef __OCUPPANCY_GRID_H
#define __OCUPPANCY_GRID_H

class OccupancyGrid
{
public:
   virtual char *ComputeOcuppancyGrid(void *frame, int2 &maskSize) = 0;
};

#endif