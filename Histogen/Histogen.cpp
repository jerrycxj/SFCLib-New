// Histogen.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include "pch.h"

#include "typedef.h"
#include "Point.h"
#include "Rectangle.h"

#include "SFCConversion.h"
//#include "OutputSchema.h"
#include "QueryBySFC.h"

#include "SFCPipeline.h"

#include "SFCConversion2.h"
#include "OutputSchema2.h"

#include "RandomLOD.h"

//#include "tbb/task_scheduler_init.h"
#include <map>
#include <iostream>
#include <fstream>
using namespace std;


int main(int argc, char* argv[])
{
	int ndims = 2;//dims for decoding
	int mbits = 4;

	int ntotal = 10;//total levels

	//int ndimsR = 0; //dims for other attributes

	/////////////////////
	int nparallel = 0;
	int nitem_num = 5000;

	int nsfc_type = 0;
	int nencode_type = 0;

	char szinput[256] = { 0 };//1.xyz
	char szoutput[256] = { 0 };
	char sztransfile[256] = { 0 };

	bool bstat = false; //control statistics

	for (int i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "-n") == 0)//dimension number
		{
			i++;
			ndims = atoi(argv[i]);
			continue;
		}

		if (strcmp(argv[i], "-m") == 0)//sfc level number
		{
			i++;
			mbits = atoi(argv[i]);
			continue;
		}

		//if (strcmp(argv[i], "-p") == 0)//if parallel: 0 sequential, 1 max parallel
		//{
		//	i++;
		//	nparallel = atoi(argv[i]);
		//	continue;
		//}

		if (strcmp(argv[i], "-i") == 0)//input file path
		{
			i++;
			strcpy_s(szinput, 256, argv[i]);
			continue;
		}

		if (strcmp(argv[i], "-o") == 0)//output file path
		{
			i++;
			strcpy_s(szoutput, 256, argv[i]);
			continue;
		}

		if (strcmp(argv[i], "-s") == 0)//sfc conversion type: 0 morthon, 1 hilbert
		{
			i++;
			nsfc_type = atoi(argv[i]);
			continue;
		}

		if (strcmp(argv[i], "-e") == 0)//output encoding type: 0 number 1 base32 2 base64
		{
			i++;
			nencode_type = atoi(argv[i]);
			continue;
		}

		if (strcmp(argv[i], "-t") == 0)//coordinates transformation file, two lines: translation and scale, comma separated
		{
			i++;
			strcpy_s(sztransfile, 256, argv[i]);
			continue;
		}

		if (strcmp(argv[i], "-b") == 0)// bottom level no
		{
			i++;
			ntotal= atoi(argv[i]);
			continue;
		}
		if (strcmp(argv[i], "-v") == 0)//statistics
		{
			//i++;
			bstat = true;
			continue;
		}
	}

	///////////////////////////////////////////////////
	///get the coordinates transfomration file--one more for lod value
	double delta[DIM_MAX + 1] = { 0 }; // 526000, 4333000, 300
	double  scale[DIM_MAX + 1] = { 1 }; //100, 100, 1000

	for (int i = 1; i < ndims + 1; i++)
	{
		delta[i] = 0;
		scale[i] = 1;
	}

	if (strlen(sztransfile) != 0)
	{
		FILE* input_file = NULL;
		fopen_s(&input_file, sztransfile, "r");
		if (input_file)
		{
			int j;
			char buf[1024];
			char * pch, *lastpos;
			char ele[64];

			//////translation
			memset(buf, 0, 1024);
			fgets(buf, 1024, input_file);

			j = 0;
			lastpos = buf;
			pch = strchr(buf, ',');
			while (pch != NULL)
			{
				memset(ele, 0, 64);
				strncpy_s(ele, lastpos, pch - lastpos);
				//printf("found at %d\n", pch - str + 1);
				delta[j] = atof(ele);
				j++;

				lastpos = pch + 1;
				pch = strchr(lastpos, ',');
			}
			delta[j] = atof(lastpos); //final part

			//////scale
			memset(buf, 0, 1024);
			fgets(buf, 1024, input_file);

			j = 0;
			lastpos = buf;
			pch = strchr(buf, ',');
			while (pch != NULL)
			{
				memset(ele, 0, 64);
				strncpy_s(ele, lastpos, pch - lastpos);
				//printf("found at %d\n", pch - str + 1);
				scale[j] = atof(ele);
				j++;

				lastpos = pch + 1;
				pch = strchr(lastpos, ',');
			}
			scale[j] = atof(lastpos); //final part

			fclose(input_file);
		}//end if input_file
	}//end if strlen

	CoordTransform<double, long> cotrans(ndims);
	if (delta != NULL && scale != NULL)
	{
		cotrans.SetTransform(delta, scale);
	}

	/////////////////////////////////
	////pipeline
	//if (nparallel == 0)
	//{
	//	if (strlen(szoutput) != 0) printf("serial run   "); //if not stdout ,print sth
	//	//tbb::task_scheduler_init init_serial(1);

	//	//run_decode_pipeline<ndims, mbits, ndimsR>(1, szinput, szoutput, nitem_num, nsfc_type, nencode_type, delta, scale);

	//}

	//if (nparallel == 1)
	//{
	//	if (strlen(szoutput) != 0)  printf("parallel run "); //if not stdout ,print sth
	//	//tbb::task_scheduler_init init_parallel(tbb::task_scheduler_init::automatic);

	//	//run_decode_pipeline<ndims, mbits, ndimsR>(init_parallel.default_num_threads(), szinput, szoutput, nitem_num, nsfc_type, \
	//	//	nencode_type, delta, scale);
	//}
	//system("pause");
	///////////////////////////////////////
	FILE* input_file = NULL;
	if (szinput != NULL && strlen(szinput) != 0)
	{
		input_file = fopen(szinput, "r");
		if (!input_file)
		{
			return 0;
		}
	}
	else
	{
		input_file = stdin;
	}

	std::ostream* out_s;
	std::ofstream of;
	if (szoutput != NULL && strlen(szoutput) != 0)
	{
		of.open(szoutput);
		out_s = &of;
	}
	else
	{
		out_s = &cout;
	}

	//////////////////////////////////////////	
	Point<double> inPt(ndims);
	Point<long> outPt(ndims);
	Point<long> pttemp(ndims);

	SFCConversion sfctest(ndims, mbits);// for bottom
	sfc_bigint val;
	map<sfc_bigint, long long>* level_histo = new map<sfc_bigint, long long>[mbits+1];///level--->0, so level+1

	char buf[1024];
	char * pch, *lastpos;
	char ele[64];

	int i, j;
	i = 0;
	while (1) //always true
	{
		//if (i == _size) break; //full, maximum _size;
		memset(buf, 0, 1024);
		fgets(buf, 1024, input_file);

		if (strlen(buf) == 0) break; // no more data

		j = 0;
		lastpos = buf;
		pch = strchr(buf, ',');
		while (pch != NULL)
		{
			memset(ele, 0, 64);
			strncpy(ele, lastpos, pch - lastpos);
			//printf("found at %d\n", pch - str + 1);
			if (strlen(ele) != 0)
			{
				inPt[j] = atof(ele);
				j++;
			}

			lastpos = pch + 1;
			pch = strchr(lastpos, ',');
		}

		if (strlen(lastpos) != 0 && strcmp(lastpos, "\n") != 0)//final part
		{
			inPt[j] = atof(lastpos);
			j++;
		}

		/////////////////this point to histogram cell
		pttemp = cotrans.Transform(inPt); //scale sfc bottom coordinates to pyramid bottom coordinates
		for (int d = 0; d < ndims; d++)
		{
			pttemp[d] = pttemp[d] >> (ntotal- mbits);
		}

		if (nsfc_type == 0)
			val = sfctest.MortonEncode(pttemp);
		else
			val = sfctest.HilbertEncode(pttemp);

		//cout << val <<":" << inPt[0]<< "," << inPt[1] <<endl;

		//if (inPt[0] == 39167.87890625 && inPt[1] == 417436.51953125)//39167.87890625,417436.51953125
		//{
		//	cout << val << endl;
		//}

		if (level_histo[mbits].find(val) == level_histo[mbits].end())
		{
			// not found-- got 1
			level_histo[mbits].insert(pair<sfc_bigint, long long>(val, 1));
		}
		else
		{
			// found- count increment
			level_histo[mbits][val]++;
		}

		i++; //each point ++
	}

	//tbb::task_scheduler_init init(tbb::task_scheduler_init::default_num_threads());
	tbb::tick_count t0 = tbb::tick_count::now();

	////////////////////////////////////////////////////////////////////////
	///the histogram in this level to the next level
	map<sfc_bigint, long long>::iterator it;
	for (int l = mbits; l >0; l--)// check this point in upper level
	{
		SFCConversion sfctest2(ndims, l); //last
		SFCConversion sfctest3(ndims, l-1);//upper

		//cout << level_histo[l].size() << endl;

		for (it = level_histo[l].begin(); it != level_histo[l].end(); ++it)//for each histogram cell
		{
			if (nsfc_type == 0)
			{
				outPt = sfctest2.MortonDecode(it->first);//last key to upper key
				for (int j = 0; j < ndims; j++) outPt[j] = outPt[j] >> 1; //scale 1/2
				val = sfctest3.MortonEncode(outPt);
			}
			else
			{
				outPt = sfctest2.HilbertDecode(it->first);//last key to upper key
				for (int j = 0; j < ndims; j++) outPt[j] = outPt[j] >> 1; //scale 1/2
				val = sfctest3.HilbertEncode(outPt);
			}

			//cout << it->first << "," << val << endl;

			if (level_histo[l - 1].find(val) == level_histo[l - 1].end()) //count
			{
				// not found-- got 1
				level_histo[l - 1].insert(pair<sfc_bigint, long long>(val, it->second));
			}
			else
			{
				// found- count increment
				level_histo[l - 1][val] += it->second;
			}
		}//for each histogram cell		
	}// from bottom to the root +1

	//////////////////////////////////////////////////////////////////////
	//output nd level histogram to file, count first, level by level next
	*out_s << (mbits+1) << endl; //the pyramid levels--- level ---> 0
	for (int l = mbits; l > 0; l--) //from bottom to root---total count
	{
		*out_s << level_histo[l].size();
		*out_s << " ";
	}
	*out_s << level_histo[0].size() << endl; //root

	for (int l = mbits; l >= 0; l--)// histogram in each level
	{
		for (it = level_histo[l].begin(); it != level_histo[l].end(); ++it)
			*out_s << it->first << " " << it->second << endl; //sfc key, count
	}

	delete[] level_histo;

	tbb::tick_count t1 = tbb::tick_count::now();

	if (bstat)
		cout << "histogram time = " << (t1 - t0).seconds() << endl;

	return 0;
}
