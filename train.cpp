
#include "CImg.h"
#include <ctime>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <stdlib.h>
#include <utility>
#include <string>
#include <vector>
#include <sys/types.h>
#include <dirent.h>
#include <list>
#include <map>
#include <numeric>
#include <ctime>

//Use the cimg namespace to access the functions easily
using namespace cimg_library;
using namespace std;

// Dataset data structure, set up so that e.g. dataset["C"][3] is
// filename of 4th C image in the dataset
typedef map<string, vector<string> > Dataset;

const string currentDateTime() 
{
    time_t now = time(0);
    struct tm tstruct;
    char buf[80];
    tstruct = *localtime(&now);        
    strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);

    return buf;
}

// Figure out a list of files in a given directory.
//
vector<string> files_in_directory(const string &directory, bool prepend_directory = false)
{
	vector<string> file_list;
	DIR *dir = opendir(directory.c_str());
	if(!dir)
		throw std::string("Can't find directory " + directory);
  
	struct dirent *dirent;
	while ((dirent = readdir(dir))) 
		if(dirent->d_name[0] != '.')
			file_list.push_back((prepend_directory?(directory+"/"):"")+dirent->d_name);

	closedir(dir);
	return file_list;
}

int main(int argc, char **argv)
{  

	if (argc != 2)
	{
		cout << "Usage: ./train opt\nopt=resize, feature, svm" << endl;
	}
	else if (strcmp(argv[1], "resize") == 0)
	{
		string directory = "English/Img/GoodImg/Bmp";
		
		// Scan through the "train" or "test" directory (depending on the
		//  mode) and builds a data structure of the image filenames for each class.
		Dataset filenames; 
		vector<string> class_list = files_in_directory(directory);
		for(vector<string>::const_iterator c = class_list.begin(); c != class_list.end(); ++c)		
			filenames[*c] = files_in_directory(directory + "/" + *c);
		
		
		system("mkdir training_final");

		for (vector<string>::const_iterator it = class_list.begin(); it != class_list.end(); ++it)
		{		
			string class_name = *it;
			class_name = class_name.substr(class_name.length()-3);
			int class_id;
			istringstream iss(class_name);
			iss >> class_id;
			if (class_id <= 10)
			{
				class_name = '0' + class_id - 1;
			}
			else if (class_id <= 36)
			{
				class_name = 'A' + class_id - 11;
			}
			else if (class_id <= 62)
			{
				class_name = 'a' + class_id - 37;
			}
			string cmd = "mkdir training_final/" + class_name;
			system(cmd.c_str());
			for (vector<string>::const_iterator it2 = filenames[*it].begin(); it2 != filenames[*it].end(); ++it2)
			{			
				string loc = directory + "/" + *it + "/" + *it2;		
				CImg<double> img(loc.c_str());
				CImg<double> gray_img;
				if (img.spectrum() > 1)
					gray_img = img.get_RGBtoYCbCr().get_channel(0);
				else
					gray_img = img;	
				if (gray_img.width() > 231)
					gray_img.resize(231, gray_img.height()*gray_img.width()/231);
				if (gray_img.height() > 231)
					gray_img.resize(gray_img.width()*gray_img.height()/231, 231);

				//gray_img.resize(231, 231);
				CImg<double> final_gray_image(231, 231, 1, 1, 255);
				int startx, starty;
				startx = (231-gray_img.width()) / 2;
				starty = (231-gray_img.height()) / 2;
				for (int i = 0; i < gray_img.width(); ++i)
					for (int j = 0; j < gray_img.height(); ++j)
						final_gray_image(startx+i,starty+j,0,0) = gray_img(i,j,0,0);

				
				loc = "training_final/" + class_name + "/" + *it2;			
				cout << loc << endl;
				final_gray_image.save(loc.c_str());	
				break;	
			}
		}
	}
	else if (strcmp(argv[1], "feature") == 0)
	{	
		string directory = "training_final";
		string svm_train_file_name = "deep_svm_train.dat";				
		string feature_file_name = "overfeat_features.dat";
		string overfeat_path = "/../overfeat/bin/linux_64/overfeat";
		
		// Scan through the "train" or "test" directory (depending on the
		//  mode) and builds a data structure of the image filenames for each class.
		Dataset filenames; 
		vector<string> class_list = files_in_directory(directory);
		for(vector<string>::const_iterator c = class_list.begin(); c != class_list.end(); ++c)		
			filenames[*c] = files_in_directory(directory + "/" + *c, true);	

		ofstream ofs;		
		ofs.open(svm_train_file_name.c_str());

		int class_index = 1;

		// Loop through all the image classes
		for(Dataset::const_iterator c_iter = filenames.begin(); c_iter != filenames.end(); ++c_iter)
		{
			string class_name = c_iter->first;
			int class_train_size = c_iter->second.size();
			cout << "Processing deep feature vector for " << class_name << endl;			
			cout << currentDateTime() << endl;
			// Get features from overfeat			
			string cmd = ".";
			cmd += overfeat_path;
			cmd += " -L 21";
					
			// Loop through all the images of the selected class
			for(int i = 0; i < class_train_size; i++)
			{
				string file_name = c_iter->second[i];			
				cmd += " " + file_name;
			}
			cmd += " > ";
			cmd += feature_file_name;

			system(cmd.c_str());							

			ifstream ifs(feature_file_name.c_str());
			if (ifs.is_open() == false)
			{
				cout << "Failed to read file: " << feature_file_name << endl;
				exit(0);
			}

			int nf, w, h;
			double value;
			for (int i = 0; i < class_train_size; ++i)
			{
				ofs << class_index;
				ifs >> nf >> h >> w;
				for (int j = 0; j < nf * w * h; ++j)
				{
					ifs >> value;
					ofs << " " << (j+1) << ":" << value;
				}
				ofs << endl;
			}
			ifs.close();					
			class_index++;			
		}
		ofs.close();
	}
	else
	{
		cout << "Unknown argument: " << argv[1] << endl;
		cout << "Usage: ./train opt\nopt=resize, feature, svm" << endl;
	}
}