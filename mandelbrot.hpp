#pragma once
#include <vector>

namespace mbs{

//we only go up to abs(c) < 2

typedef std::uint32_t u32;
typedef std::uint8_t u8;
typedef std::pair <double, double> coordinates; // y, x
typedef std::vector <std::vector <u32>> u32Matrix;

class Mandelbrot{
    u32 height_;
    u32 width_;
    double xOffset_ = 0;
    double yOffset_ = 0;
    coordinates ReBound_; //min, max
    coordinates ImBound_;
    double ReSize_;
    double ImSize_;
    std::vector <std::vector <u32>> itrTable_; 
public:
	Mandelbrot() = default;
    Mandelbrot(const u32 &height, const u32 &width, const coordinates &ImBound, const coordinates &ReBound):
        height_(height),
        width_(width),
        ReBound_(ReBound),
        ImBound_(ImBound),
        ReSize_ (ReBound.second - ReBound.first),
        ImSize_ (ImBound.second - ImBound.first),
        itrTable_(height, std::vector <u32> (width)) {
        calculateTable();
	};

	void calculateTable()
	{
		for(u32 py = 0; py < height_; ++py)
	        for(u32 px = 0; px < width_; ++px)
	        {
	            coordinates c = mapToPlane(std::make_pair(py, px));
	            u32 itr = checkConvergence(c, 255);
	            itrTable_[py][px] = itr;
	        }
	}

    coordinates mapToPlane(const coordinates &pixel) //coordinates
    {
        double y = (pixel.first / height_) * ImSize_ + ImBound_.first;
        double x = (pixel.second / width_) * ReSize_ + ReBound_.first;
        return std::make_pair(y, x);
    };

    u32 checkConvergence(const coordinates &point, const u32 &iterations)
    {
    	double x = 0.0;
        double y = 0.0;
        u32 itr = 0;
        while(x*x + y*y < 4 && itr <= iterations)
        {
            double xtemp = x*x - y*y + point.second;
            y = 2.0*x*y + point.first;
            x = xtemp;
            ++itr;
        }
        return itr;
    };

    /*u8 getRGB(const u32 &itr)
    {
    	u8[0] = itr * sin(itr) % 256;
    	u8[1] = itr * cos(itr) % 256;
    	u8[2] = itr * tan(itr) % 256;
    }*/

    void update(const coordinates &ImBound, const coordinates &ReBound)
    {
    	ReBound_ = ReBound;
        ImBound_ = ImBound;
        ReSize_ = ReBound.second - ReBound.first;
        ImSize_ = ImBound.second - ImBound.first;
        calculateTable();
    }

    void update(const double &y, const double &x, const double &radius)
    {
    	update(std::make_pair(y - radius, y + radius), std::make_pair(x - radius, x + radius));
    }

    void focus(const coordinates &upperCorner, const coordinates &lowerCorner) //focus | u = ymax, xmin; l = ymin, xmax
    {
        auto uc = mapToPlane(upperCorner); //u = ymax, xmin
        auto lc = mapToPlane(lowerCorner); //l = ymin, xmax
        update(std::make_pair(lc.first, uc.first), std::make_pair(uc.second, lc.second));
    }
    void focus(const double &zoom = 0.85)
    {
        update(std::make_pair(ImBound_.first*zoom, ImBound_.second*zoom), std::make_pair(ReBound_.first*zoom, ReBound_.second*zoom));
    }
    void setOffset(const double &y, const double &x)
    {
        yOffset_ += y;
        xOffset_ += x;
        update(std::make_pair(ImBound_.first + ImSize_*0.2, ImBound_.second+ ImSize_*0.2), std::make_pair(ReBound_.first, ReBound_.second));
        calculateTable();
    }

    u32Matrix& getTable()
    {
    	return itrTable_;
    }
};

}