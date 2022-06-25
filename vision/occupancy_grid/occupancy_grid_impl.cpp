
#include <memory>
#include <iostream>

#include <stdlib.h>
#include <arpa/inet.h>

#include <opencv2/opencv.hpp>
#include <jetson-utils/cudaMappedMemory.h>

#include "occupancy_grid.h"

typedef uchar3 pixelType;

class OccupancyGridImpl : public OccupancyGrid
{
private:
    const int og_width = 160;
    const int og_depth = 200;
    const double cam_height = -(104.5 + 1.5 + 24); // why -?
    const double cam_angle = 14.0;
    const double x_grid_size = 800;
    const double z_grid_size = 1000;
    const double grid_side = 5;
    const double img_width = 800;
    const double img_height = 600;
    const double scale_factor = 2.5;

    cv::Mat rvec, tvec, mtx, dist;
    cv::Mat grid_image, grid_points;
    cv::Mat mask_image_bgr, mask_image_rgb, original_image_bgr, original_image_rgb;

    double divide_z_grid_size = (z_grid_size / 100.0);
    double zero_zero_grid_z = (z_grid_size / divide_z_grid_size) + 45;
    double zero_zero_grid_x = -(x_grid_size / 2.0);

    int grid_dims[2];
    std::vector<cv::Point2d> projectedPoints;

    cv::Point3d get_pos_grid(int i, int j)
    {
        cv::Point3d p;
        p.x = zero_zero_grid_x + j * grid_side + grid_side / 2.0;
        p.z = zero_zero_grid_z + i * grid_side + grid_side / 2.0;
        // p.y = 0;
        p.y = -cam_height;
        return p;
    }

    cv::Point3d get_pos_grid_scale(int i, int j)
    {
        cv::Point3d p;
        p.x = (zero_zero_grid_x + j * grid_side + grid_side / 2.0) / scale_factor;
        p.z = (zero_zero_grid_z + i * grid_side + grid_side / 2.0) / scale_factor;
        // p.y = 0;
        p.y = (-cam_height) / scale_factor;
        return p;
    }

    inline long long int double2int(double value)
    {
        long long int cast = (long long int)value;
        if (value - cast > 0.5)
            cast += 1;
        return cast;
    }

    inline double degree2rad(double value)
    {
        return value * (M_PI / 180.0);
    }

    inline cv::Point3d rotate_around_x(double angle, cv::Point3d old_point)
    {
        cv::Point3d new_point;
        new_point.x = old_point.x;
        new_point.y = old_point.y * cos(angle) - old_point.z * sin(angle);
        new_point.z = old_point.y * sin(angle) + old_point.z * cos(angle);
        return new_point;
    }

    void rs2_project_point_to_pixel(double pixel[2], cv::Mat dist, cv::Mat mat, const double point[3])
    {
        double x = point[0] / point[2], y = point[1] / point[2];

        double r2 = x * x + y * y;
        double f = 1 + dist.at<double>(0, 0) * r2 + dist.at<double>(0, 1) * r2 * r2 + dist.at<double>(0, 4) * r2 * r2 * r2;
        x *= f;
        y *= f;
        double dx = x + 2 * dist.at<double>(0, 2) * x * y + dist.at<double>(0, 3) * (r2 + 2 * x * x);
        double dy = y + 2 * dist.at<double>(0, 3) * x * y + dist.at<double>(0, 2) * (r2 + 2 * y * y);
        x = dx;
        y = dy;

        pixel[0] = x * mat.at<double>(0, 0) + mat.at<double>(0, 2);
        pixel[1] = y * mat.at<double>(1, 1) + mat.at<double>(1, 2);
    }

    void rs2_project_point_to_pixel_2(double pixel[2], cv::Mat dist, cv::Mat mat, const double point[3])
    {
        double x = point[0] / point[2], y = point[1] / point[2];

        double r2 = x * x + y * y;
        double f = 1 + dist.at<double>(0, 0) * r2 + dist.at<double>(0, 1) * r2 * r2 + dist.at<double>(0, 4) * r2 * r2 * r2;
        double xf = x * f;
        double yf = y * f;
        double dx = xf + 2 * dist.at<double>(0, 2) * x * y + dist.at<double>(0, 3) * (r2 + 2 * x * x);
        double dy = yf + 2 * dist.at<double>(0, 3) * x * y + dist.at<double>(0, 2) * (r2 + 2 * y * y);

        x = dx;
        y = dy;

        pixel[0] = x * mat.at<double>(0, 0) + mat.at<double>(0, 2);
        pixel[1] = y * mat.at<double>(1, 1) + mat.at<double>(1, 2);
    }

    void initializeParams()
    {
        mtx.at<double>(0, 0) = 669.56360147;
        mtx.at<double>(0, 2) = 417.80137854;
        mtx.at<double>(1, 1) = 678.49127745;
        mtx.at<double>(1, 2) = 269.62459559;
        mtx.at<double>(2, 2) = 1.0;
        dist.at<double>(0, 0) = 0.10876619;
        dist.at<double>(0, 1) = -0.50646342;
        dist.at<double>(0, 2) = -0.01617755;
        dist.at<double>(0, 3) = 0.00968778;
        dist.at<double>(0, 4) = 0.77699546;
    }

    void computeGridProjectedPoints()
    {
        std::vector<cv::Point3d> grid_points;

        for (int i = 0; i < grid_dims[0]; i++)
        {
            for (int j = 0; j < grid_dims[1]; j++)
            {
                // cv::Point3d p = get_pos_grid(i,j);
                cv::Point3d p = get_pos_grid_scale(i, j);
                grid_points.push_back(p);
            }
        }

        cv::Point3d point3d;
        double point3d_array[3];
        double point2d_array[2];
        double rotate_x_rad = degree2rad(cam_angle);
        for (auto it = grid_points.begin(); it != grid_points.end(); ++it)
        {
            point3d = *it;
            // std::cout << "point3d" <<std::endl;
            // std::cout << point3d <<std::endl;
            // rotate
            point3d = rotate_around_x(rotate_x_rad, point3d);
            // std::cout << point3d <<std::endl;
            point3d_array[0] = point3d.x;
            point3d_array[1] = point3d.y;
            point3d_array[2] = point3d.z;

            // project - which?
            rs2_project_point_to_pixel(point2d_array, dist, mtx, point3d_array);
            projectedPoints.push_back(cv::Point2d(point2d_array[0], point2d_array[1]));
        }
    }

public:
    OccupancyGridImpl()
    {
        rvec = cv::Mat::zeros(1, 3, CV_64F);
        tvec = cv::Mat::zeros(1, 3, CV_64F);
        mtx = cv::Mat::zeros(3, 3, CV_64F);
        dist = cv::Mat::zeros(1, 5, CV_64F);

        grid_dims[0] = (int)(z_grid_size / grid_side);
        grid_dims[1] = (int)(x_grid_size / grid_side);

        int grid_image_dims[] = {(int)(z_grid_size / grid_side), (int)(x_grid_size / grid_side)}; // HxW
        grid_image = cv::Mat::zeros(2, grid_image_dims, CV_8UC3);

        initializeParams();
        computeGridProjectedPoints();
    }

    char *ComputeOcuppancyGrid(void *frame_input, int2 &maskSize) override
    {
        pixelType *frame = (pixelType *)frame_input;

        char *grid = (char *)malloc(sizeof(char) * og_depth * og_width);
        memset(grid, 0, og_width);

        for (int i = 0; i < grid_dims[0]; i++)
        {
            for (int j = 0; j < grid_dims[1]; j++)
            {
                long long int pixel_w = double2int((projectedPoints[i * grid_dims[1] + j].x) / 2.0);
                long long int pixel_h = double2int((projectedPoints[i * grid_dims[1] + j].y) / 2.0);

                if (pixel_w >= maskSize.x || pixel_w < 0 || pixel_h >= maskSize.y || pixel_h < 0)
                    continue;
                else
                {
                    char *uc = reinterpret_cast<char *>(&frame[maskSize.x * pixel_h + pixel_w]);
                    uint r, g, b;
                    r = *uc;
                    g = *(++uc);
                    b = *(++uc);

                    int mem_pos = og_width * (grid_dims[0] - 1 - i) + j;

                    if (r == 0 && g == 0 && b == 0)
                    {
                        grid[mem_pos] = 0;
                    }
                    else
                    {
                        grid[mem_pos] = 1;
                    }
                }
            }
        }
        return grid;
    }
};

OccupancyGrid *NewOccupancyGridImplInstance() { return new OccupancyGridImpl(); }
