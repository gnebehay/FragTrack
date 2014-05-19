/*
FragTrack - Fragments-based Tracking Code
-----------------------------------------

By: 	Amit Adam

	amita@cs.technion.ac.il
	www.cs.technion.ac.il/~amita

Date:	November 18'th, 2007

-----------------------------------------
*/


// fragtrack_envelope.cpp 
// The console application envelope for running FragTrack on an image
// sequence
//

#include "Fragments_Tracker.h"

//
// ReadImage - reads an image from file. Converts to gray scale
// and returns in a CvMat*
//

CvMat* Read_Image(char* file_name)
{
	IplImage* I = cvLoadImage(file_name,0);   // force it to be gray scale

	CvMat* out_img;
	
	if (I==NULL)
	{
		out_img = NULL;
		return out_img;
	}

	out_img = cvCreateMat(I->height,I->width,CV_8U);
	cvCopy(I, out_img);
	
	cvReleaseImage(&I);

	return out_img;
}

//
// Read_Setup_File - reads a file that contains the location of the image
// sequence, the range of frame numbers to process, and the various
// values with which to initilaize the tracker
//

bool Read_Setup_File(char* fileName, Parameters& params, char* file_name_pfx, int& first_file_num,
					 int& last_file_num, ofstream& log_file)

{
	log_file << endl << "Reading setup file " << fileName << endl << endl;

	ifstream setup_file;
	setup_file.open(fileName,std::ios::in);

	if (!setup_file) 
	{
		log_file << "Setup file not found !!! " << endl << flush;
		return false;
	}

	//
	// Prefix of the frame file names
	//

	setup_file >> file_name_pfx;
	log_file << "File name prefix: " << file_name_pfx << endl;

	//
	// initial and final frame numbers
	//

	setup_file >> first_file_num;
	setup_file >> last_file_num;
	log_file << "First file number: " << first_file_num << endl;
	log_file << "Last file number: " << last_file_num << endl;

	//
	// Read tracker parameters
	//

	//
    // template position: top left corner and bottom right corner
	// (0 based indexing)
	//

	int itly,itlx,ibry,ibrx;
	setup_file >> itly >> itlx >> ibry >> ibrx;
	params.initial_tl_y = itly;
	params.initial_tl_x = itlx;
	params.initial_br_y = ibry;
	params.initial_br_x = ibrx;
	log_file << "Initial template corners (top left y x, bottom right y x): " << (params.initial_tl_y) << " ";
	log_file << (params.initial_tl_x) << " ";
	log_file << (params.initial_br_y) << " ";
	log_file << (params.initial_br_x) << " " << endl;
	
	//
	// search margin
	//
	
	setup_file >> (params.search_margin);
	log_file << "Search margin (pixels): " << (params.search_margin) << endl;

	//
	// number of bins
	//

	setup_file >> (params.B);
	log_file << "Number of bins: " << (params.B) << endl;
	
	//
	// histogram comparison method: 
	// use 1 for Chi-square, 2 for EMD, 3 for Kolmogorov-Smirnov variation
	// which is equivalent to EMD (for one-dimensional data)
	//

	setup_file >> (params.metric_used);
	log_file << "Metric used for comparing histograms (1 = chi square, 2 = EMD, 3 = KS (best choice)) : " << params.metric_used << endl;
	log_file << flush;

	//
	// that's it
	//

	setup_file.close();

	return true;
}

int main( int argc, char** argv )
{
	//
	// set output window
	//

	cout << "Position output window, then press any key ... " << endl << flush;
	cvNamedWindow("FragTrack",0);
	cvWaitKey(0);

	//
	// open log file 
	//

	ofstream log_file;
	log_file.open("FragTrack_log.txt",std::ios::out);

	//
	// Define variables that will hold the setup data and read setup file
	//
	// default file name is "setup.txt"
	//

	Parameters params;
	int first_frame_num,last_frame_num;
	char file_name_pfx[255];

	bool ok = Read_Setup_File("setup.txt",params,file_name_pfx,first_frame_num,last_frame_num,log_file);
	if (!ok)
	{
		log_file << "**** Failed to read setup file " << flush;
		log_file.close();
		return 0;
	}

    //
	// Define the tracker object
	//

	Fragments_Tracker* FT = NULL;

	//
	// now run on the sequences: initialize the tracker after reading the first
	// frame and then process every frame in the sequence
	//

	int frame_number = first_frame_num - 1;
	char curr_fn[255];

	while (frame_number < last_frame_num)
	{
		frame_number = frame_number + 1;

		//
		// build the current file name
		//

		strcpy(curr_fn,"");
		sprintf(curr_fn,"%s%d.jpg",file_name_pfx,frame_number);

		CvMat* curr_img = Read_Image(curr_fn);

		if (curr_img == NULL)
		{
			cout  << endl << frame_number << " not found ! "  << endl << flush;
			log_file << endl << endl << "**** File " << curr_fn << " was not found " << endl << endl << flush;
		}
		else
		{
			if (frame_number == first_frame_num)
			{
				log_file << endl << "Frame size: height = " << curr_img->height << " width = " << curr_img->width << endl;
				FT = new Fragments_Tracker(curr_img,params,log_file);
			}
			else
			{
				FT->Handle_Frame(curr_img,"FragTrack");
				cvWaitKey(1);  // required for refreshing output window
				cout << "Handled frame number " << frame_number  << endl << flush;
			}
			
			cvReleaseMat(&curr_img);
					
		}  // if the file was found
	
	}      // read next file

	if (FT != NULL) delete FT;

	log_file << endl << endl << "Finished running on the sequence, exiting ... " << endl << flush;
	log_file.close();

	cout << endl << "Finished ... press any key to exit ... " << endl << flush;
	cvWaitKey(0);

	return 0;



}


