#pragma once
#include <vector>
#include <cmath>
#include <thread>
#include <cstdint>
#include <immintrin.h>

namespace mbs{

typedef std::uint64_t u64;
typedef std::uint32_t u32;
typedef std::uint8_t u8;
typedef std::vector <std::vector <u32>> u32Matrix;

template <class metricType = double> 
class Mandelbrot
{
	typedef std::pair <metricType, metricType> point; // (x, y)

	u32 width_;
	u32 height_;
	//double xOffset_ = 0;
	//double yOffset_ = 0;
	point ReBound_;
	point ImBound_;
	metricType ReSize_;
	metricType ImSize_;
	std::vector <std::vector <u32>> itrTable_;

public:

	//int lock;
	Mandelbrot(const u32 width, const u32 height, const point &ReBound, const point &ImBound)
		:
		width_(width),
		height_(height),
		ReBound_(ReBound),
		ImBound_(ImBound),
		ReSize_(std::abs(ReBound.second - ReBound.first)),
		ImSize_(std::abs(ImBound.second - ImBound.first)),
		itrTable_(height, std::vector <u32> (width))
	{
		//lock = 0;
		calculateTableThreaded(4);
	};

	void calculateTable()
	{
		for(u32 row = 0; row < height_; ++row)
			calculateRow(row);
	}

	void calculateTableThreaded(const u32 poolSize)
	{
		u32 divSize = height_ / poolSize;
		std::vector <std::thread> pool(poolSize);

		for(u32 id = 0; id < poolSize; ++id)
			pool[id] = std::thread(&Mandelbrot::calculateSlice, this, id * divSize, (id + 1) * divSize);
		for(auto &thread : pool)
			thread.join();

		for(u32 row =  poolSize * divSize; row < height_; ++row)
			calculateRow(row);

	}

	void calculateSlice(const u32 begin, const u32 end)
	{
		for(u32 row = begin; row < end; ++row)
			calculateRow(row);
	}

	void calculateRow(const u32 row)
	{
		/*for(u32 col = 0; col < width_; ++col)
		{
			point point = mapToPlane(std::make_pair(col, row));
			u32 itrCount = checkConvergence(point, 256);
			itrTable_[row][col] = itrCount;
		}*/
		for(int i = 0; i < width_ / 4; ++i)
		{
			//std::vector <point> pts;
			__m256d px = _mm256_setzero_pd(), py = _mm256_setzero_pd();

			double *pxtemp = (double*) &px;
			double *pytemp = (double*) &py;

			for(int j = i * 4; j < i * 4 + 4; ++j)
			{
				//pts.push_back(mapToPlane(std::make_pair(j, row)));
				auto pt = mapToPlane(std::make_pair(j, row));
				pxtemp[j % 4] = pt.first;
				pytemp[j % 4] = pt.second;
			}
			__m256i h = checkConvergenceAVX(px, py, 255);
			u64 *itrs = (u64*) &h;
			for(int j = i * 4; j < i * 4 + 4; ++j)
			{
				itrTable_[row][j] = itrs[j%4];
			}
		}
		for(u32 col = (width_ / 4) * 4; col < width_ ; ++col)
		{
			point point = mapToPlane(std::make_pair(col, row));
			u32 itrCount = checkConvergence(point, 256);
			itrTable_[row][col] = itrCount;
		}
	}

	point mapToPlane(const point &pixel) //pixel{col/x, row/y}
	{
		double x = (pixel.first / width_) * ReSize_ + ReBound_.first;
		double y = (pixel.second / height_) * ImSize_ + ImBound_.first;

		return std::make_pair(x, y);
	};

	u32 checkConvergence(const point &point, const u32 iterations)
	{
		double xsquare = 0.0;
		double ysquare = 0.0;
		double zsquare = 0.0;
		u32 itrCount = 0;

		while(xsquare + ysquare <= 4  &&  itrCount < iterations) 
		{
			double x = xsquare - ysquare + point.first;
			double y = zsquare - xsquare - ysquare + point.second;
			xsquare = x * x;
			ysquare = y * y;
			zsquare = (x + y) * (x + y);

			++itrCount;
		}

		return itrCount;
	};

	__m256i checkConvergenceAVX(const __m256d &x0, const __m256d &y0, const u32 iterations)
	{
		__m256d xsquare = _mm256_setzero_pd();
		__m256d ysquare = _mm256_setzero_pd();
		__m256d zsquare = _mm256_setzero_pd();

		__m256d xpysquare = _mm256_setzero_pd();
		double *xpyPt = (double*) &xpysquare;

		__m256d x;
		__m256d y;

		__m256i iterators = _mm256_setzero_si256();
		__m256i increment = _mm256_set1_epi64x(1);
		u64 *incrementPt = (u64*) &increment;

		u32 itrCount = 0;
		u8 ok = 0b1111;
		while(ok != 0b0000 && itrCount < iterations)
		{
			xpysquare = _mm256_add_pd(xsquare, ysquare);
			
			if(ok & 0b1000)
				if(xpyPt[0] > 4.0)
				{
					ok = ok & 0b0111;
					incrementPt[0] = 0;
				}
			if(ok & 0b0100)
				if(xpyPt[1] > 4.0)
				{
					ok = ok & 0b1011;
					incrementPt[1] = 0;
				}
			if(ok & 0b0010)
				if(xpyPt[2] > 4.0)
				{
					ok = ok & 0b1101;
					incrementPt[2] = 0;
				}
			if(ok & 0b0001)
				if(xpyPt[3] > 4.0)
				{
					ok = ok & 0b1110;
					incrementPt[3] = 0;
				}

			x = _mm256_add_pd(_mm256_sub_pd(xsquare, ysquare), x0);
			y = _mm256_add_pd(_mm256_sub_pd(_mm256_sub_pd(zsquare, xsquare), ysquare), y0);

			xsquare = _mm256_mul_pd(x, x);
			ysquare = _mm256_mul_pd(y, y);
			zsquare = _mm256_add_pd(x, y);
			zsquare = _mm256_mul_pd(zsquare, zsquare);

			++itrCount;
			iterators = _mm256_add_epi64(iterators, increment);
		}

		return iterators;
	}

	void update(const point &ReBound, const point &ImBound)
	{
		ReBound_ = ReBound;
		ImBound_ = ImBound;
		ReSize_ = std::abs(ReBound.second - ReBound.first);
		ImSize_ = std::abs(ImBound.second - ImBound.first);

		calculateTableThreaded(4);

		/*lock = 1;
		for(;;)
		{
			if(lock == 0)
				break;
		}*/
	}

	void update(const metricType &y, const metricType &x, const metricType &yRadius, const metricType &xRadius)
	{
		update(std::make_pair(x - xRadius, x + xRadius), std::make_pair(y - yRadius, y + yRadius));
	}

	void focus(const point &upperCorner, const point &lowerCorner) //focus | u = xmin, ymax; l = xmax, ymin
	{
		auto uc = mapToPlane(upperCorner); //u = xmin, ymax
		auto lc = mapToPlane(lowerCorner); //l = xmax, ymin

		update(std::make_pair(uc.first, lc.first), std::make_pair(lc.second, uc.second));
	}
	void focus(const double &zoom = 0.85)
	{
		metricType cr = (ReBound_.first + ReBound_.second) / 2;
		metricType ci = (ImBound_.first + ImBound_.second) / 2;

		update(std::make_pair(cr - ReSize_ * zoom / 2, cr + ReSize_ * zoom / 2), std::make_pair(ci - ImSize_ * zoom / 2, ci + ImSize_ * zoom / 2));
	}
	void setOffset(const metricType &x, const metricType &y)
	{
		update(std::make_pair(ReBound_.first + ReSize_*x, ReBound_.second + ReSize_*x), std::make_pair(ImBound_.first + ImSize_*y, ImBound_.second+ ImSize_*y));
	}

	u32Matrix& data()
	{
		return itrTable_;
	}
};

class RGB
{
public:

	u8 R, G, B;

	RGB(const u8 R, const u8 G, const u8 B)
		:
		R(R),
		G(G),
		B(B)
	{};
};

RGB generateColor(const u32 itrCount)
{
	RGB color(itrCount % 256, itrCount % 256, itrCount % 256);
	
	return color;
}

/*void calc(Mandelbrot <double> *set)
{
	for(;;)
	{
		if(set->lock == 1)
		{
			set->calculateTable();
			set->lock = 0;
		}
	}
}*/

}