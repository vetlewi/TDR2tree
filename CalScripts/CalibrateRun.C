#include "CalibrateRing.C"
#include "CalibrateSect.C"
#include "CalibrateBack.C"
#include "CalibrateLaBrL.C"
#include "CalibrateLaBrS.C"
#include "CalibrateClover.C"

void WriteCal(ofstream &outfile, vector<double> values, int br, string keyword)
{
	string line = keyword + " = ";
	char tmp[2048];
	for (size_t i = 0 ; i < values.size() ; ++i){
		if ( i % br == 0 )
			line += "\\\n";
		sprintf(tmp, "%2.6f", values[i]);
		line += tmp;
		line += "\t";
	}
	outfile << line;
	outfile << "\n\n";
	return;
}

void CalAndWrite(const char *rfile, const char *templ, const char *output)
{
	// First we will read the template and make a vector of each line
	vector<string> line_template;
	string line;
	ifstream infile(templ);

	while ( getline(infile, line) ){
		line_template.push_back(line);
	}

	infile.close();

	ofstream outfile(output);

	// Write the template to file
	char tmp[2048];
	for (size_t i = 0 ; i < line_template.size() ; ++i){
		sprintf(tmp, "%s\n", line_template[i].c_str());
		outfile << tmp;
	}

	outfile << "\n\n";

	WriteCal(outfile, CalibrateRing(rfile), 8, "shift_t_ring");
	WriteCal(outfile, CalibrateSect(rfile), 8, "shift_t_sect");
	WriteCal(outfile, CalibrateBack(rfile), 8, "shift_t_back");
	WriteCal(outfile, CalibrateLaBrL(rfile), 10, "shift_t_labrL");
	WriteCal(outfile, CalibrateLaBrS(rfile), 10, "shift_t_labrS");
	WriteCal(outfile, CalibrateClover(rfile), 4, "shift_t_clover");
	outfile.close();
}

void CalibrateRun(){

	vector<int> runs = {18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 40,
						41, 42, 43, 44, 45, 46, 47, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63,
						64, 65,66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77};

	char tmp1[1024], tmp2[1024];
	for (size_t i = 0 ; i < runs.size() ; ++i){
		sprintf(tmp1, "../TCal_files/R%d.root", runs[i]);
		sprintf(tmp2, "../cal_files/R%d.cal", runs[i]);
		CalAndWrite(tmp1, "../cal_files/cal_basic.txt", tmp2);
	}
	return;
}