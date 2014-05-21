
#ifndef LRKLASIFIKACIJAZVUKOVA_H
#define LRKLASIFIKACIJAZVUKOVA_H
#include <string>
#include <rttools/rttime.h>

#include <boost/shared_ptr.hpp>
#include <alvalue/alvalue.h>
#include <alproxies/almemoryproxy.h>
#include <alaudio/alsoundextractor.h>
#include <iostream>
#include <fstream>
#include <althread/almutex.h>
#include <vector>

#include <boost/multi_array.hpp>

#include "kiss_fft130/kiss_fft.h"

using namespace AL;


class LRKlasifikacijaZvukova : public ALSoundExtractor
{

public:

  LRKlasifikacijaZvukova(boost::shared_ptr<ALBroker> pBroker, std::string pName);
  virtual ~LRKlasifikacijaZvukova();


  //method inherited from almodule that will be called after constructor
  void init();
  
  
  void pocni_klasifikaciju( const AL::ALValue &parametri );
  
  void prekini_klasifikaciju();

public:
  void process(const int & nbOfChannels,
               const int & nbrOfSamplesByChannel,
               const AL_SOUND_FORMAT * buffer,
               const ALValue & timeStamp);
  
  
private:
  
  //2beImplemented...//enum oznakeZnacajki { ASC };
  int klasificiraj( std::vector<int> &signal, //signal; npr. onaj buffer of 370 ms ili vise spojenih buffera
  			//2beImplemented...//std::vector<int> &odabraneZnacajke,
  			//std::vector<float> &izracunateZnacajke, //tu ce se spremat izracunate znacajke
  			const int &nfft, //koliko "point" fourierovu transformaciju radimo; npr. prva veca potencija broja 2, ili signal.size()
  			const int &frekvencija, //frekvencija kojojm je signal sniman 
  			const float &duljinaOkvira ); // duljina okvira u sekundama
  ALMemoryProxy fProxyToALMemory;
  int brojOkviraPoBufferu; // broj okvira u bufferu; 170 ms buffer u npr. 5 okvira znaci da ce jedan okvir biti 170/5 = 34 ms. 
  int brojBufferaKojeKupim;  // nakon sto detektira glasan zvuk, kupit ce npr. 5 buffera sto znaci da mu je jedan uzorak duljine 0.85 sekundi odnosno 5 * 170 = 850 ms.
  bool kupimBuffere; // jel kupim buffere i trazim znacajke (nakon sto je detektiran glasan zvuk) ili gledam glasnocu pojedinog buffera (dok iscekujem glasan zvuk)
  int brojBufferaKojeSamPokupio; //broj buffera koje sam pokupio nakon sto je glasan zvuk detektiran. Nakon sto ovo bude vece od <brojBufferaKojeKupim>, prekida se skupljanje i <kupimBuffere> postaje false.
  int granicaGlasnoce; //ako je sample u bufferu (bilo koji buffer[i]) veci of ovoga, detektiran je "glasan" zvuk i zapocinje skupljanje buffera i racunanje znacajki.
  /*std::vector<float> STE; //za pohranu Short Time Energy
  std::vector<float> ZCR; //za pohranu Zero Crossing Rate
  std::vector<float> ASC; //za pohranu Audio Spectrum Centroid
  std::vector<float> ASS; //za pohranu Audio Spectrum Spread
  std::vector<float> ASF; //za pohranu Audio Spectrum Flatness*/
  //std::vector< std::vector<int> > cijeliZvuk;
  std::vector< std::vector<int> > sviOkviri;
  //std::map<float,int> bazaLSTER;
  std::ofstream soundDat;
  boost::shared_ptr<AL::ALMutex> fCallbackMutex;
  int frekvencija;
  bool parametersSet;
  
  
  
};
#endif


