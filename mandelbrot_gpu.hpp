#pragma once

#include <vector>
#include <cmath>
#include <thread>
#include <cstdint>
#include <immintrin.h>

#include "mandelbrot_base.hpp"



namespace mbs
{

constexpr const char *kernel_source = R"cl(
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
)cl";

constexpr const int kernel_source_length = std::strlen(kernel_source);


//

template <int precision = 24>
class Mandelbrot : public Mandelbrot_base <double>    // Metric_T
{
    using Metric_T = double;

    cl::Platform m_platform;
    cl::Device m_device;
    cl::Context m_context;
    cl::Program::Sources m_sources;
    cl::CommandQueue m_queue;

    cl::Buffer m_width_buf;
    // m_re_size_over_w, m_im_size_over_w, m_re_lower, m_im_lower
    cl::Buffer m_map_to_plane_buf;
    cl::Buffer m_convergence_table_buf;
    u32 m_size;

    cl::Program m_program;
    cl::Kernel m_kernel;

    u16 *m_convergence_table;

    void init_gpu();

    void calculate_convergence_table();


public:

    Mandelbrot(const u16, const u16, const Point_T&, const Point_T&);

    ~Mandelbrot() noexcept = default;

    const u16* data() const;

};



template <int precision>
Mandelbrot <precision>::Mandelbrot(const u16 width, const u16 height,
    const Point_T &re_domain, const Point_T &im_domain)
    :
    m_size(width * height),
    m_convergence_table(nullptr),
    Mandelbrot_base(width, height, re_domain, im_domain)
{
    init_gpu();
    calculate_convergence_table();    // called by base
};

template <int precision>
Mandelbrot <precision>::~Mandelbrot()
{
    m_queue.enqueueUnmapMemObject(m_convergence_table_buf, m_convergence_table);
};



template <int precision>
void Mandelbrot <precision>::calculate_convergence_table()
{
    Metric_T buf[4] = {m_re_size / m_width, m_im_size / m_height,
        m_re_domain.first, m_im_domain.first};

    m_queue.enqueueWriteBuffer(m_width_buf, CL_TRUE, 0, sizeof(u16), &m_width);
    m_queue.enqueueWriteBuffer(m_map_to_plane_buf, CL_TRUE, 0,
        sizeof(Metric_T) * 4, buf);

    m_queue.enqueueNDRangeKernel(m_kernel, cl::NullRange, cl::NDRange(m_size),
        cl::NullRange);

    m_convergence_table = (u16*) m_queue.enqueueMapBuffer(
        m_convergence_table_buf, CL_TRUE, CL_MAP_READ, 0, sizeof(u16) * m_size);
};




template <int precision>
void Mandelbrot <precision>::init_gpu()
{
    std::vector<cl::Platform> platforms;
    cl::Platform::get(&platforms);

    m_platform = platforms.at(0);
    std::cout << "Using platform: "
        << m_platform.getInfo <CL_PLATFORM_NAME>() << std::endl;

    std::vector<cl::Device> devices;
    m_platform.getDevices(CL_DEVICE_TYPE_ALL, &devices);

    m_device = devices.at(0);
    std::cout << "Using device: "
        << m_device.getInfo <CL_DEVICE_NAME>() << std::endl;

    m_context = cl::Context({m_device});

    // compile the kernel
    m_sources.push_back({mbs::kernel_source, mbs::kernel_source_length});

    m_program = cl::Program(m_context, m_sources);
    if (m_program.build({m_device}) != CL_SUCCESS)
    {
        std::cout << "Error building: "
            << m_program.getBuildInfo <CL_PROGRAM_BUILD_LOG>(m_device)
            << std::endl;
        
        std::exit(1);
    }

    m_kernel = cl::Kernel(m_program, "mbs_convergence");

    // allocate the needed buffers
    m_width_buf = cl::Buffer(m_context, CL_MEM_READ_WRITE, sizeof(u16));
    m_map_to_plane_buf = cl::Buffer(m_context, CL_MEM_READ_WRITE,
        sizeof(Metric_T) * 4);
    m_convergence_table_buf = cl::Buffer(m_context, CL_MEM_READ_WRITE,
        sizeof(u16) * m_size);

    // initialize the queue
    m_queue = cl::CommandQueue(m_context, m_device);

    // set the arguments for the kernel
    m_kernel.setArg(0, m_width_buf);
    m_kernel.setArg(1, m_map_to_plane_buf);
    m_kernel.setArg(2, m_convergence_table_buf);
};


template <int precision>
const typename Mandelbrot <precision>::u16* Mandelbrot <precision>::data() const
{
    return m_convergence_table;
};

//



};