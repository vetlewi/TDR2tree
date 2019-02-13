

{
	TChain *chain = new TChain("events");
	
	chain->AddFile("/Users/vetlewi/Trees/R60.root");

	TProof *pro = TProof::Open("");
	chain->SetProof();

	chain->Process("selector.C+");
}