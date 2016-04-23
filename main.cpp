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
#include <stack>
#include <vector>
#include <list>
#include <utility>

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
			
			edge_map(i,j,0) = is_local_maxima?255:0;

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
	
	sobel(0, 0) = -1.0/8.0; sobel(1, 0) = -2.0/8.0; sobel(2, 0) = -1.0/8.0;
	sobel(0, 1) = 0.0; sobel(1, 1) = 0.0; sobel(2, 1) = 0.0;
	sobel(0, 2) = 1.0/8.0; sobel(1, 2) = 2.0/8.0; sobel(2, 2) = 1.0/8.0;
	Gy.convolve(sobel, 1, false);	

	for (int i = 0; i < G.width(); ++i)
	{
		for (int j = 0; j < G.height(); ++j)
		{
			G(i,j,0) = sqrt(Gx(i,j)*Gx(i,j)+Gy(i,j)*Gy(i,j));						
			if (G(i,j,0) > 255) G(i,j,0) = 255;			
			G(i,j,1) = atan2(Gy(i,j), Gx(i,j));
		}		
	}

	return edge_thinning_non_maximum_suppress(G, threshold1, threshold2, range);	
}

CImg<double> get_stroke_width(CImg<double> &G)
{	
	list< list< pair<int, int> > > lines;
	CImg<double> stroke_width(G.width(), G.height(), 1, 1, 255);

	for (int i = 0; i < G.width(); ++i)
	{
		for (int j = 0; j < G.height(); ++j)
		{
			if (G(i,j,0) < 254)
				continue;	

			list< pair<int, int> > points;
			points.push_back(make_pair(i,j));

			double w, direction = G(i,j,1);
			int x, y, prev_x = i, prev_y = j;
			bool found = false;
		
			for (double d = 0.1; ; d += 0.3)
			{	
				x = i + d * cos(direction) + 0.5;			
				y = j + d * sin(direction) + 0.5;	

				if (x < 0 || y < 0 || y >= G.height() || x >= G.width())
					break;

				if (x == prev_x && y == prev_y)
					continue;

				prev_x = x;
				prev_y = y;

				points.push_back(make_pair(x,y));

				if (G(x,y,0) > 254)
				{
					int dir1 = (int)(direction * 180.0 / PI + 0.5 + 360*4) % 360;
					int dir2 = (int)(G(x,y,1) * 180.0 / PI + 0.5 + 360*4) % 360;
					if (dir2 > dir1)
					{
						int temp = dir1;
						dir1 = dir2;
						dir2 = temp;
					}
									
					//if ( abs(dir1-(dir2+180)) < 35)
					if ( abs(dir1-(dir2+180)) < 55)
					{							
						w = sqrt((i-x)*(i-x)+(j-y)*(j-y));
						found = true;						
					}
					break;
				}
			}
		
			if (found == true)
			{						
				for(list< pair<int, int> >::iterator it = points.begin(); it != points.end(); ++it)
				{
					x = (*it).first;
					y = (*it).second;
					stroke_width(x,y) = min(stroke_width(x,y), w);					
				}
				lines.push_back(points);
			}							
		}
	}

	// Second pass
	for(list< list< pair<int, int> > >::iterator it = lines.begin(); it != lines.end(); ++it)
	{
		double avg = 0.0, count = 0.0;
		int x, y;
		for(list< pair<int, int> >::iterator it2 = (*it).begin(); it2 != (*it).end(); ++it2)
		{
			x = (*it2).first;
			y = (*it2).second;
			avg += stroke_width(x,y);
			++count;
		}
		if (count < 0.5)
			avg = 0.0;
		else
			avg = avg / count;

		for(list< pair<int, int> >::iterator it2 = (*it).begin(); it2 != (*it).end(); ++it2)
		{
			x = (*it2).first;
			y = (*it2).second;			
			if (stroke_width(x,y) > avg)
				stroke_width(x,y) = avg;
		}
	}

	return stroke_width;
}

void group_candidates(CImg<double> &G, CImg<bool> &visited, list<pair<int,int> > &group,
														int i, int j, double threshold)
{	
	if (visited(i,j) == true)		
		return;

	stack<pair<int,int> > stck;
	stck.push(make_pair(i,j));

	while (stck.empty() == false)
	{
		i = stck.top().first;
		j = stck.top().second;
		stck.pop();

		visited(i,j) = true;
		group.push_back(make_pair(i,j));

		for (int x = i-1; x <= i+1; ++x)
		{
			if (x < 0 || x >= G.width())
				continue;
			
			for (int y = j-1; y <= j+1; ++y)
			{
				if (y < 0 || y >= G.height())
					continue;
				if (G(x,y) < 1.0 || G(i,j) < 1.0) continue;	
				if (G(x,y) > 254) continue;
				double ratio = G(x,y)>G(i,j)?G(x,y)/G(i,j):G(i,j)/G(x,y);		
				if (ratio < threshold && visited(x,y)==false)					
					stck.push(make_pair(x,y));
			}
		}
	}
}

double get_mean(CImg<double> &G, list< pair<int, int> > &points)
{
	double avg = 0.0, n = points.size();
	for (list< pair<int, int> >::iterator it = points.begin(); it != points.end(); ++it)
		avg += G((*it).first,(*it).second);
	avg /= n;	
	return avg;
}

double get_variance(CImg<double> &G, list< pair<int, int> > &points)
{
	double avg = 0.0, n = points.size(), sigma = 0.0;
	for (list< pair<int, int> >::iterator it = points.begin(); it != points.end(); ++it)
		avg += G((*it).first,(*it).second);
	avg /= n;
	for (list< pair<int, int> >::iterator it = points.begin(); it != points.end(); ++it)	
		sigma += (G((*it).first,(*it).second) - avg)*(G((*it).first,(*it).second) - avg);	
	sigma /= n;
	return sqrt(sigma);
}

pair<double,double> get_diameter(list< pair<int, int> > &points)
{
	double minx = 9999.9, miny = 9999.9, maxx = -9999.9, maxy = -9999.9;
	for (list< pair<int, int> >::iterator it = points.begin(); it != points.end(); ++it)
	{
		double x = (*it).first;
		double y = (*it).second;
		if (x < minx) minx = x;
		if (x > maxx) maxx = x;
		if (y < miny) miny = y;
		if (y > maxy) maxy = y;
	}
	
	return make_pair(maxx-minx, maxy-miny);	
}

CImg<double> set_bounding_box(CImg<double> &image, list< list< pair<int, int> > > &groups)
{
	CImg<double> result(image.width(), image.height(), 1, 3, 0.0);
	for (int i = 0; i < image.width(); ++i)	
		for (int j = 0; j < image.height(); ++j)			
			for (int p = 0; p < 3; ++p)
				result(i,j,p) = image(i,j);

	const unsigned char color[] = {255, 0, 0};
	for(list< list< pair<int, int> > >::iterator it = groups.begin(); it != groups.end(); it++)
	{
		double minx = 9999.9, miny = 9999.9, maxx = -9999.9, maxy = -9999.9;
		for (list< pair<int, int> >::iterator it2 = (*it).begin(); it2 != (*it).end(); ++it2)
		{
			double x = (*it2).first;
			double y = (*it2).second;
			if (x < minx) minx = x;
			if (x > maxx) maxx = x;
			if (y < miny) miny = y;
			if (y > maxy) maxy = y;
		}
		for (int w = 0; w < 5; ++w)
		{
			result.draw_line(minx-w, miny-w, maxx+w, miny-w, color);
			result.draw_line(minx-w, miny-w, minx-w, maxy+w, color);
			result.draw_line(maxx+w, miny-w, maxx+w, maxy+w, color);
			result.draw_line(minx-w, maxy+w, maxx+w, maxy+w, color);
		}
	}

	return result;

}

CImg<double> find_letter_candidates(CImg<double> &G, double threshold1=5.0, int threshold2=500, double threshold3=3.0)
{
	CImg<double> letter_candidates(G.width(), G.height(), 1, 1, 255);
	for (int i = 0; i < G.width(); ++i)	
		for (int j = 0; j < G.height(); ++j)
			letter_candidates(i,j) = G(i,j);

	double gray_color = 220;
	double ratio = G.width() / 1500.0;

	list< list< pair<int, int> > > groups;	
	CImg<bool> visited(G.width(), G.height(), 1, 1, false);	
	
	// Group together
	for (int i = 0; i < G.width(); ++i)
	{
		for (int j = 0; j < G.height(); ++j)
		{
			if (G(i,j) > 254)
				continue;
			if (visited(i,j) == true)
				continue;

			list<pair<int,int> > group;			
			group_candidates(G, visited, group, i, j, threshold1);
			groups.push_back(group);
		}
	}

	// filter with variance
	for(list< list< pair<int, int> > >::iterator it = groups.begin(); it != groups.end(); )
	{		
		double mean_width = get_mean(G, *it);
		if (get_variance(G, *it) > threshold3)
		{
			for(list< pair<int, int> >::iterator it2 = (*it).begin(); it2 != (*it).end(); ++it2)				
				letter_candidates((*it2).first, (*it2).second) = gray_color;
			it = groups.erase(it);
		}
		else
			++it;
	}
	letter_candidates.save("letter_candidates_1.png");

	// filter with group size
	for(list< list< pair<int, int> > >::iterator it = groups.begin(); it != groups.end(); )
	{
		int group_size = (*it).size();		
		if ( group_size < threshold2 * ratio)
		{
			for(list< pair<int, int> >::iterator it2 = (*it).begin(); it2 != (*it).end(); ++it2)				
				letter_candidates((*it2).first, (*it2).second) = gray_color;
			it = groups.erase(it);
		}
		else
			++it;
	}
	letter_candidates.save("letter_candidates_2.png");

	// filter with mean_width
	for(list< list< pair<int, int> > >::iterator it = groups.begin(); it != groups.end(); )
	{
		int group_size = (*it).size();
		double mean_width = get_mean(G, *it);
		if (mean_width < 5 * ratio)
		{
			for(list< pair<int, int> >::iterator it2 = (*it).begin(); it2 != (*it).end(); ++it2)				
				letter_candidates((*it2).first, (*it2).second) = gray_color;
			it = groups.erase(it);
		}
		else
			++it;
	}
	letter_candidates.save("letter_candidates_3.png");

	// filter with width/height ratio
	for(list< list< pair<int, int> > >::iterator it = groups.begin(); it != groups.end(); )
	{		
		pair<double,double> diameter = get_diameter(*it);
		double r = diameter.first > diameter.second? diameter.first / diameter.second : diameter.second / diameter.first;
		if (r > 5 || diameter.first / diameter.second > 1.0)
		{
			for(list< pair<int, int> >::iterator it2 = (*it).begin(); it2 != (*it).end(); ++it2)				
				letter_candidates((*it2).first, (*it2).second) = gray_color;
			it = groups.erase(it);
		}
		else
			++it;
	}
	letter_candidates.save("letter_candidates_4.png");

	// filter with max size / min size
	for(list< list< pair<int, int> > >::iterator it = groups.begin(); it != groups.end(); )
	{		
		pair<double,double> diameter = get_diameter(*it);
		if (diameter.first > 100 * ratio || diameter.second < 8 * ratio)
		{
			for(list< pair<int, int> >::iterator it2 = (*it).begin(); it2 != (*it).end(); ++it2)				
				letter_candidates((*it2).first, (*it2).second) = gray_color;
			it = groups.erase(it);
		}
		else
			++it;
	}
	letter_candidates.save("letter_candidates_5.png");

	// filter with area size
	for(list< list< pair<int, int> > >::iterator it = groups.begin(); it != groups.end(); )
	{		
		pair<double,double> diameter = get_diameter(*it);
		if (diameter.first * diameter.second < 600 * ratio || diameter.first * diameter.second > 3800 * ratio)
		{
			for(list< pair<int, int> >::iterator it2 = (*it).begin(); it2 != (*it).end(); ++it2)				
				letter_candidates((*it2).first, (*it2).second) = gray_color;
			it = groups.erase(it);
		}
		else
			++it;
	}
	letter_candidates.save("letter_candidates_6.png");

	// filter with diameter / width ratio
	for(list< list< pair<int, int> > >::iterator it = groups.begin(); it != groups.end(); )
	{		
		pair<double,double> diameter = get_diameter(*it);
		double mean_width = get_mean(G, *it);
		if (diameter.first * diameter.second / mean_width < 200)
		{
			for(list< pair<int, int> >::iterator it2 = (*it).begin(); it2 != (*it).end(); ++it2)				
				letter_candidates((*it2).first, (*it2).second) = gray_color;
			it = groups.erase(it);
		}
		else
			++it;
	}
	letter_candidates.save("letter_candidates_7.png");

	set_bounding_box(letter_candidates, groups).save("boxed_letters.png");

	return letter_candidates;
}

CImg<double> get_negative(CImg<double> &G)
{
	CImg<double> negative_image(G.width(), G.height());
	for (int i = 0; i < G.width(); ++i)	
		for (int j = 0; j < G.height(); ++j)			
			negative_image(i,j) = 255-G(i,j);
	return negative_image;
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
	/*string threshold1_str = argv[2];
	string threshold2_str = argv[3];	
	string threshold3_str = argv[4];
	double threshold1, threshold2, threshold3, range;
	
	istringstream iss1(threshold1_str);
	iss1 >> threshold1;
	istringstream iss2(threshold2_str);
	iss2 >> threshold2;
	istringstream iss3(threshold3_str);
	iss3 >> threshold3;*/

	CImg<double> input_image(inputFile.c_str());

	CImg<double> edge_map = get_edge_map(input_image);

	get_negative(edge_map).save("edge.png");

	CImg<double> stroke_width = get_stroke_width(edge_map);	
	stroke_width.save("stroke_width.png");
	
	CImg<double> letter_candidates = find_letter_candidates(stroke_width);//, threshold1, threshold2, threshold3);	
	letter_candidates.save("letter_candidates.png");	
}