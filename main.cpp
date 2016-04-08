// B657 final project skeleton code
//
// Compile with: "make"
//
// See assignment handout for command line and project specifications.


//Link to the header file
#include "CImg.h"
#include <ctime>
#include <cmath>
#include <sstream>
#include <iostream>
#include <cstdlib>
#include <string>
#include <vector>

//Use the cimg namespace to access the functions easily
using namespace cimg_library;
using namespace std;

#define PI 3.141592654

void flood_fill(CImg<double> &G, CImg<bool> &visited, double threshold1, int i, int j)
{
	if (visited(i,j) == true)
		return;
	visited(i,j) = true;
	for (int c = i-1; c <= i+1; ++c)
	{
		if (c < 0 || c >= G.width())
			continue;
		for (int r = j-1; r <= j+1; ++r)
		{
			if (r < 0 || r >= G.height())
				continue;

			if (G(c,r,0) > threshold1)
				flood_fill(G, visited, threshold1, c, r);
		}
	}
}

CImg<double> edge_thinning_non_maximum_suppress(CImg<double> &G, const double threshold1,
														const double threshold2, const double range)
{
	CImg<double> edge_map(G.width(), G.height(), 2, 1);

	for (int i = 0; i < G.width(); ++i)	
		for (int j = 0; j < G.height(); ++j)	
			for (int k = 0; k < 2; ++k)					
				edge_map(i,j,k) = G(i,j,k);
			
	for (int i = 0; i < G.width(); ++i)
	{
		for (int j = 0; j < G.height(); ++j)
		{			
			if (G(i,j,0) < threshold1)
			{
				edge_map(i,j,0) = 0;
				continue;
			}

			bool is_local_maxima = true;
			double prev = G(i,j,0);					
			for (double d = 0; d <= range/2.0; ++d)
			{				
				int r = j + d * sin(G(i,j,1)) + 0.5;
				int c = i + d * cos(G(i,j,1)) + 0.5;

				if (r < 0 || c < 0 || r >= G.height() || c >= G.width())
					break;

				if (r == j && c == i)
					continue;		

				if (G(c,r,0) > prev + 0.0001)
				{
					is_local_maxima = false;
					break;
				}			
				prev = G(c,r,0);
			}

			if (is_local_maxima == true)
			{
				prev = G(i,j,0);					
				for (double d = 0; d >= -range/2.0; --d)
				{				
					int r = j + d * sin(G(i,j,1)) + 0.5;
					int c = i + d * cos(G(i,j,1)) + 0.5;

					if (r < 0 || c < 0 || r >= G.height() || c >= G.width())
						break;

					if (r == j && c == i)
						continue;		

					if (G(c,r,0) > prev + 0.0001)
					{
						is_local_maxima = false;
						break;
					}			
					prev = G(c,r,0);
				}
			}

			edge_map(i,j,0) = is_local_maxima?edge_map(i,j,0):0;

		}
	}	
	CImg<bool> visited(G.width(), G.height(), 1, 1, false);
	for (int i = 0; i < G.width(); ++i)
	{
		for (int j = 0; j < G.height(); ++j)
		{			
			if (edge_map(i,j,0) > threshold2)
			{				
				flood_fill(edge_map, visited, threshold1, i, j);
			}
		}
	}

	for (int i = 0; i < G.width(); ++i)	
		for (int j = 0; j < G.height(); ++j)
			if (visited(i,j) == false)
				edge_map(i,j,0) = 0;

	return edge_map;	
}

CImg<double> get_edge_map(CImg<double> &input_image, const double threshold1=11.0,
									const double threshold2=13.0, const double range=3.0)
{
	CImg<double> Gx = input_image.get_RGBtoYCbCr().get_channel(0);
	CImg<double> Gy = input_image.get_RGBtoYCbCr().get_channel(0);
	CImg<double> G(input_image.width(), input_image.height(), 2, 1, 0);
	
	CImg<double> sobel(3, 3, 1, 1);
	sobel(0, 0) = -1.0/8.0; sobel(1, 0) = 0.0; sobel(2, 0) = 1.0/8.0;
	sobel(0, 1) = -2.0/8.0; sobel(1, 1) = 0.0; sobel(2, 1) = 2.0/8.0;
	sobel(0, 2) = -1.0/8.0; sobel(1, 2) = 0.0; sobel(2, 2) = 1.0/8.0;
	Gx.convolve(sobel, 1, false);
	//Gx.save("gx.png");
	sobel(0, 0) = -1.0/8.0; sobel(1, 0) = -2.0/8.0; sobel(2, 0) = -1.0/8.0;
	sobel(0, 1) = 0.0; sobel(1, 1) = 0.0; sobel(2, 1) = 0.0;
	sobel(0, 2) = 1.0/8.0; sobel(1, 2) = 2.0/8.0; sobel(2, 2) = 1.0/8.0;
	Gy.convolve(sobel, 1, false);
	//Gy.save("gy.png");

	for (int i = 0; i < G.width(); ++i)
	{
		for (int j = 0; j < G.height(); ++j)
		{
			G(i,j,0) = sqrt(Gx(i,j)*Gx(i,j)+Gy(i,j)*Gy(i,j));						
			if (G(i,j,0) > 255) G(i,j,0) = 255;			
			if ( abs(G(i,j,0)) < 0.0001)
				G(i,j,1) = PI / 2.0;
			else
				G(i,j,1) = atan(Gy(i,j) / Gx(i,j));
		}		
	}
	G.save("G.png");

	return edge_thinning_non_maximum_suppress(G, threshold1, threshold2, range);	
}

int main(int argc, char **argv)
{
	if(argc < 2)
	{
		cout << "Insufficent number of arguments; correct usage:" << endl;
		cout << "./final <image-name> <threshold1> <threshold2> <range>" << endl;
		return -1;
    }

	//Initializing the variables
	string inputFile = argv[1];
	string threshold1_str = argv[2];
	string threshold2_str = argv[3];
	string range_str = argv[4];
	double threshold1, threshold2, range;
	
	istringstream iss1(threshold1_str);
	iss1 >> threshold1;
	istringstream iss2(threshold2_str);
	iss2 >> threshold2;
	istringstream iss3(range_str);
	iss3 >> range;	

	CImg<double> input_image(inputFile.c_str());

	CImg<double> edge_map = get_edge_map(input_image, threshold1, threshold2, range);
	edge_map.save("edge.png");
}