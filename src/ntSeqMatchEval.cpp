/*
 * Compares count lists using Fisher's Exact Test per pair and produces a table
 * with the combinations of all pairs
 */

#include <sstream>
#include <string>
#include <vector>
#include <getopt.h>
#include <iostream>
#include <stdlib.h>
#include <limits.h>
#include <assert.h>
#include "config.h"
#include "src/Options.h"
#include "src/Util.h"
#include "CompareCounts.hpp"

#include <omp.h>

using namespace std;

#define PROGRAM "ntsmEval"

void printVersion()
{
	const char VERSION_MESSAGE[] = PROGRAM " (" PACKAGE_NAME ") " GIT_REVISION "\n"
	"Written by Justin Chu <cjustin@ds.dfci.harvard.edu>\n"
	"\n"
	"Copyright 2021 Dana-Farber Cancer Institute\n";
	cerr << VERSION_MESSAGE << endl;
	exit(EXIT_SUCCESS);
}

void printHelpDialog(){
	const string dialog =
	"Usage: " PROGRAM " [FILES...]\n"
	"  -s, --score_thresh = FLOAT Score threshold ["+ to_string(opt::scoreThresh)+"]\n"
	"  -a, --all                  Output results of all tests, not just those that pass\n"
	"                             the threshold.\n"
	"  -p, --pca = STR            Use PCA information to speed up analysis. Input is a\n"
	"                             set of rotational values from a PCA.\n"
	"  -n, --norm = STR           Set of values use to center the data before rotation\n"
	"  -c, --min_cov = INT        Keep only sites with this coverage and above. ["+ to_string(opt::minCov)+"]\n"
	"  -w, --skew = FLOAT         Divides the score by coverage. Formula: (cov1*cov2)^skew\n"
	"                             Set to zero for no skew. ["+ to_string(opt::genomeSize)+"]\n"
	"  -g, --genome_size = INT    Diploid genome size for error rate estimation.\n"
	"                             ["+ to_string(opt::covSkew)+"]\n"
//	"  -t, --threads              Number of threads to run.[1]\n"
	"  -h, --help                 Display this dialog.\n"
	"  -v, --verbose              Display verbose output.\n"
	"      --version              Print version information.\n";

	cerr << dialog << endl;
	exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[])
{
	//switch statement variable
	int c;

	//control variables
	bool die = false;
	int OPT_VERSION = 0;

	//long form arguments
	static struct option long_options[] = { {
		"score_thresh", required_argument, NULL, 's' }, {
		"all", no_argument, NULL, 'a' }, {
		"min_cov", required_argument, NULL, 'c' }, {
		"max_cov", required_argument, NULL, 'm' }, {
		"skew", required_argument, NULL, 'w' }, {
		"genome_size", required_argument, NULL, 'g' }, {
		"threads", required_argument, NULL, 't' }, {
		"help", no_argument, NULL, 'h' }, {
	    "pca", required_argument, NULL, 'p' }, {
		"norm", required_argument, NULL, 'n' }, {
		"version", no_argument, &OPT_VERSION, 1 }, {
		"verbose", no_argument, NULL, 'v' }, {
		NULL, 0, NULL, 0 } };

	int option_index = 0;
	while ((c = getopt_long(argc, argv, "t:vhs:c:m:aw:g:p:n:", long_options,
			&option_index)) != -1)
	{
		istringstream arg(optarg != NULL ? optarg : "");
		switch (c) {
		case 'h': {
			printHelpDialog();
			break;
		}
		case 'a': {
			opt::all = true;
			break;
		}
		case 's': {
			stringstream convert(optarg);
			if (!(convert >> opt::scoreThresh)) {
				cerr << "Error - Invalid parameter s: "
						<< optarg << endl;
				return 0;
			}
			break;
		}
		case 'w': {
			stringstream convert(optarg);
			if (!(convert >> opt::covSkew)) {
				cerr << "Error - Invalid parameter w: "
						<< optarg << endl;
				return 0;
			}
			break;
		}
		case 'c': {
			stringstream convert(optarg);
			if (!(convert >> opt::minCov)) {
				cerr << "Error - Invalid parameter c: "
						<< optarg << endl;
				return 0;
			}
			break;
		}
		case 'm': {
			stringstream convert(optarg);
			if (!(convert >> opt::maxCov)) {
				cerr << "Error - Invalid parameter m: "
						<< optarg << endl;
				return 0;
			}
			break;
		}
		case 'g': {
			stringstream convert(optarg);
			if (!(convert >> opt::genomeSize)) {
				cerr << "Error - Invalid parameter g: "
						<< optarg << endl;
				return 0;
			}
			break;
		}
		case 't': {
			stringstream convert(optarg);
			if (!(convert >> opt::threads)) {
				cerr << "Error - Invalid parameter t: "
						<< optarg << endl;
				return 0;
			}
			break;
		}
		case 'g': {
			stringstream convert(optarg);
			if (!(convert >> opt::genomeSize)) {
				cerr << "Error - Invalid parameter g: "
						<< optarg << endl;
				return 0;
			}
			break;
		}
		case 'p': {
			stringstream convert(optarg);
			if (!(convert >> opt::pca)) {
				cerr << "Error - Invalid parameter p: "
						<< optarg << endl;
				return 0;
			}
			break;
		}
		case 'n': {
			stringstream convert(optarg);
			if (!(convert >> opt::norm)) {
				cerr << "Error - Invalid parameter n: "
						<< optarg << endl;
				return 0;
			}
			break;
		}
		case '?': {
			die = true;
			break;
		}
		}
	}

#if defined(_OPENMP)
	if (opt::threads > 0)
	omp_set_num_threads(opt::threads);
#endif

	if (OPT_VERSION) {
		printVersion();
	}

	vector<string> inputFiles;
	while (optind < argc) {
		inputFiles.emplace_back(argv[optind]);
		assert(Util::fexists(inputFiles.back()));
		optind++;
	}

	//Check needed options
	if (inputFiles.size() == 0) {
		cerr << "Error: Need Input File" << endl;
		die = true;
	}

	if (die) {
		cerr << "Try '--help' for more information.\n";
		exit(EXIT_FAILURE);
	}

	double time = omp_get_wtime();
	CompareCounts comp(inputFiles);
	if(opt::verbose > 1){
		cerr << "Finished loading files. Now comparing all samples." << endl;
	}
	comp.computeScore();

	cerr << "Time: " << omp_get_wtime() - time << " s Memory: "  << Util::getRSS() << " kbytes" << endl;
	return 0;
}




