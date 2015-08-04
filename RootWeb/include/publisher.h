#include "rootweb.hh"
#include <string>
class TH1D;
namespace RootWeb
{
	void makePageOne( RootWSite& site );
	TH1D* createPlot();
	void makePageTwo( RootWSite& site, const std::string& inFilename );
	void prepareSiteStuff( RootWSite& site, std::string& directory, const std::string& run );
	void makeDQMmonitor( const std::string& inFilename, std::string& directory, const std::string& run );
}
