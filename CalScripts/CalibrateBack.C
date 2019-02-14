
vector<double> CalibrateBack(const char *rname)
{
	vector<double> result;
	TFile *file = TFile::Open(rname);

	TH2 *m = (TH2 *)file->Get("time_back");

	TSpectrum spec;

	char tmp[1024];

	double peakPos[1024];

	const int num = 16;

	for (int i = 0 ; i < num ; ++i ){
		sprintf(tmp, "px_%d", i);
		TH1 *h = m->ProjectionX(tmp, i+1, i+1);
		h->Rebin(5);
		//h->Draw();
		spec.Search(h);
		int n_peaks = spec.GetNPeaks();
		cout << i << ": " << n_peaks << endl;

		double param[3] = {200., spec.GetPositionX()[0], 1.0};
		TF1 *fit = new TF1("total","gaus(0)",spec.GetPositionX()[0]-100.,spec.GetPositionX()[0]+100.);
		
		for (Int_t k=0; k<3; k++) {
        fit->SetParameter(k, param[k]);
    }

		h->Fit("total", "bR");
		peakPos[i] = fit->GetParameter(1);
		//fit->Draw("same");

	}
	printf("shift_t_back =\t");
	for (int i = 0 ; i < num ; ++i){
		if ( i % 8 == 0)
			printf("\\\n");	
		printf("%2.6f\t", -peakPos[i]);
		result.push_back(-peakPos[i]);
	}

	cout << endl;

	file->Close();
	return result;
}
