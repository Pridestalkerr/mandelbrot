#ifdef cl_khr_fp64
    #pragma OPENCL EXTENSION cl_khr_fp64 : enable
#elif defined(cl_amd_fp64)
    #pragma OPENCL EXTENSION cl_amd_fp64 : enable
#else
    #error "Double precision floating point not supported by OpenCL implementation."
#endif

kernel void mbs_convergence(global const ushort *width,
global const double *map_to_plane, global ushort *convergence_table_buf)
{
    size_t id = get_global_id(0);

    double xsquare = 0.0;
    double ysquare = 0.0;
    double zsquare = 0.0;
    ushort itrCount = 0;

    int idx = id % (*width);
    int idy = id / (*width);

    double px = idx * map_to_plane[0] + map_to_plane[2];
    double py = idy * map_to_plane[1] + map_to_plane[3];

    while(xsquare + ysquare <= 4  &&  itrCount < 255) 
    {
        double x = xsquare - ysquare + px;
        double y = zsquare - xsquare - ysquare + py;
        xsquare = x * x;
        ysquare = y * y;
        zsquare = (x + y) * (x + y);

        ++itrCount;
    }

    convergence_table_buf[id] = itrCount;
}