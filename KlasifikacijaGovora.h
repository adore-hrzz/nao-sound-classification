
#ifndef KLASIFIKACIJAGOVORA_H
#define KLASIFIKACIJAGOVORA_H
#include <string>
#include <rttools/rttime.h>

#include <boost/shared_ptr.hpp>
#include <alvalue/alvalue.h>
#include <alproxies/almemoryproxy.h>
#include <alaudio/alsoundextractor.h>

using namespace AL;

class KlasifikacijaGovora : public ALSoundExtractor
{

public:

  KlasifikacijaGovora(boost::shared_ptr<ALBroker> pBroker, std::string pName);
  virtual ~KlasifikacijaGovora();

  void pocniKlasifikaciju(const int &granicaGlasnoceParam,
			  const int &brojOkviraPoBufferuParam,
			  const int &brojBufferaKojeKupimParam,
			  const int &frekvencijaParam,
			  const ALValue &channelIDParam,
			  //const int &deinterleavingParam,
			  const int &inputBufferSizeParam);
  
  void prekiniKlasifikaciju();


  //method inherited from almodule that will be called after constructor
  void init();

public:
  void process(const int & nbOfChannels,
               const int & nbrOfSamplesByChannel,
               const AL_SOUND_FORMAT * buffer,
               const ALValue & timeStamp);
               
private:

  ALMemoryProxy fProxyToALMemory;
  int brojOkviraPoBufferu; // broj okvira u bufferu; 170 ms buffer u npr. 5 okvira znaci da ce jedan okvir biti 170/5 = 34 ms. 
  int brojBufferaKojeKupim;  // nakon sto detektira glasan zvuk, kupit ce npr. 5 buffera sto znaci da mu je jedan uzorak duljine 0.85 sekundi odnosno 5 * 170 = 850 ms.
  bool kupimBuffere; // jel kupim buffere i trazim znacajke (nakon sto je detektiran glasan zvuk) ili gledam glasnocu pojedinog buffera (dok iscekujem glasan zvuk)
  int brojBufferaKojeSamPokupio; //broj buffera koje sam pokupio nakon sto je glasan zvuk detektiran. Nakon sto ovo bude vece od <brojBufferaKojeKupim>, prekida se skupljanje i <kupimBuffere> postaje false.
  int granicaGlasnoce; //ako je sample u bufferu (bilo koji buffer[i]) veci of ovoga, detektiran je "glasan" zvuk i zapocinje skupljanje buffera i racunanje znacajki.
  std::vector<float> STE; //za pohranu Short Time Energy
  std::vector<float> ZCR; //za pohranu Zero Crossing Rate
  //std::map<float,int> bazaLSTER;
  
};
#endif


