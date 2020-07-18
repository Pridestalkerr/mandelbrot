#pragma once
#include <vector>
#include <cmath>
#include <thread>
#include <cstdint>
#include <immintrin.h>
#include <CL/cl2.hpp>
#include <fstream>
#include <iostream>

namespace mbs{


template <typename Metric_T = double>
class Mandelbrot_base
{
protected:

    using Point_T = std::pair <Metric_T, Metric_T>;
    using u16 = std::uint16_t;
    using u32 = std::uint32_t;

    u16 m_width;            // pixel width
    u16 m_height;           // pixel height

    Point_T m_re_domain;    // real axis domain
    Point_T m_im_domain;    // imaginary axis domain

    Metric_T m_re_size;     // size of the real axis
    Metric_T m_im_size;     // size of the imaginary axis


    virtual void calculate_convergence_table() = 0;

    constexpr Point_T map_to_plane(const u16, const u16);

public:

    constexpr Mandelbrot_base(const u16, const u16, const Point_T&,
        const Point_T&);

    void update(const Point_T&, const Point_T&);

    void update(const Metric_T&, const Metric_T&,
        const Metric_T&, const Metric_T&);

    void focus(const u16, const u16, const u16, const u16);

    void focus(const double = 0.85);

    void set_offset(const Metric_T&, const Metric_T&);
};



template <typename Metric_T>
constexpr
Mandelbrot_base <Metric_T>::Mandelbrot_base(const u16 width, const u16 height,
    const Point_T &re_domain, const Point_T &im_domain)
    :
    m_width(width), m_height(height),
    m_re_domain(re_domain), m_im_domain(im_domain),
    m_re_size(std::abs(re_domain.second - re_domain.first)),
    m_im_size(std::abs(im_domain.second - im_domain.first))
{};


template <typename Metric_T>
constexpr typename Mandelbrot_base <Metric_T>::Point_T
Mandelbrot_base <Metric_T>::map_to_plane(const u16 x, const u16 y)
{
    Metric_T xm = (x / m_width) * m_re_size + m_re_domain.first;
    Metric_T ym = (y / m_height) * m_im_size + m_im_domain.first;

    return Point_T(xm, ym);
};

template <typename Metric_T>
void Mandelbrot_base <Metric_T>::update(const Point_T &re_domain, 
    const Point_T &im_domain)
{
    m_re_domain = re_domain;
    m_im_domain = im_domain;

    m_re_size = std::abs(re_domain.second - re_domain.first);
    m_im_size = std::abs(im_domain.second - im_domain.first);

    calculate_convergence_table();
};

template <typename Metric_T>
void Mandelbrot_base <Metric_T>::update(const Metric_T &x, const Metric_T &y,
    const Metric_T &y_radius, const Metric_T &x_radius)
{
    update(
        Point_T(x - x_radius, x + x_radius),
        Point_T(y - y_radius, y + y_radius)
    );
};

template <typename Metric_T>
void Mandelbrot_base <Metric_T>::focus(const u16 x_upper, const u16 y_upper,
    const u16 x_lower, const u16 y_lower)
{
    Point_T upper_corner = map_to_plane(x_upper, y_upper); //u = xmin, ymax
    Point_T lower_corner = map_to_plane(x_lower, y_lower); //l = xmax, ymin

    update(
        Point_T(upper_corner.first, lower_corner.first),
        Point_T(lower_corner.second, upper_corner.second)
    );
};

template <typename Metric_T>
void Mandelbrot_base <Metric_T>::focus(const double zoom)
{
    Metric_T c_re = (m_re_domain.first + m_re_domain.second) / 2;
    Metric_T c_im = (m_im_domain.first + m_im_domain.second) / 2;

    update(
        Point_T(c_re - m_re_size * zoom / 2, c_re + m_re_size * zoom / 2),
        Point_T(c_im - m_im_size * zoom / 2, c_im + m_im_size * zoom / 2)
    );
};

template <typename Metric_T>
void Mandelbrot_base <Metric_T>::set_offset(const Metric_T &x, const Metric_T &y)
{
    update(
        Point_T(m_re_domain.first + m_re_size*x, m_re_domain.second + m_re_size*x),
        Point_T(m_im_domain.first + m_im_size*y, m_im_domain.second+ m_im_size*y)
    );
};



}