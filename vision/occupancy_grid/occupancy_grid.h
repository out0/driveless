#ifndef __OCUPPANCY_GRID_H
#define __OCUPPANCY_GRID_H

class OccupancyGrid
{
public:
   virtual int GetWidth() = 0;
   virtual int GetHeight() = 0;
   virtual char *ComputeOcuppancyGrid(void *frame, int width, int height) = 0;
};

#endif