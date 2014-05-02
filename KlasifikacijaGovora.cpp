#include "KlasifikacijaGovora.h"
#include <alcommon/alproxy.h>
#include <iostream>
#include <math.h>
#include <fstream>
#include <string>
#include <qi/log.hpp>

#define SGN(x) (x>0)-(x<0) //signum funkcija


KlasifikacijaGovora::KlasifikacijaGovora(boost::shared_ptr<ALBroker> pBroker,
                                     std::string pName) : ALSoundExtractor(pBroker, pName), fProxyToALMemory(getParentBroker())
{
  setModuleDescription("Klasifikacija Govora. Kada cuje glasan zvuk, klasificira ga i dize event \"KlasifikacijaGovoraEvent\" i u njega sprema ALValue strukturu\n"
  		       "[string koji sadrzi vrijednost \"Artikulirano\" ili \"Neartikulirano\",\n"
  		       "[float HZCRR, float LSTER],\n"
  		       "timeStamp]");
  
  functionName("pocniKlasifikaciju", getName(), "Zapocinje detektiranje glasnih zvukova i njihova klasifikacija");
  addParam("granicaGlasnoceParam", "Npr. 5000");
  addParam("brojOkviraPoBufferuParam", "Npr. 5");
  addParam("brojBufferaKojeKupimParam", "Npr. 10");
  addParam("frekvencijaParam", "Npr. 16000 ili 48000");
  addParam("channelIDParam", "Npr. FRONTCHANNEL ili ALLCHANNELS");
  //addParam("deinterleavingParam", "0 ili 1");   Iz nekog razloga nece se buildati ako je broj parametara >= 7 (da, ako bilo koji od tih 7 parametara izbacim, radi, inace nece) pa sam presudio da je ovaj najmanje vazan i uvijek ce biti 0.
  addParam("inputBufferSizeParam", "8192 (=170ms) ili 16384 (=340ms). Citat iz dokumentacije: \"Warning: when speech recognition is running, a buffer size of 8192 is used. Don't change it during the recognition process.\"");
  BIND_METHOD(KlasifikacijaGovora::pocniKlasifikaciju);
  
  functionName("prekiniKlasifikaciju", getName(), "Prekid detektiranja zvukova i njihove klasifikacije.");
  BIND_METHOD(KlasifikacijaGovora::prekiniKlasifikaciju);
}

void KlasifikacijaGovora::init()
{
  
  //brojOkviraPoBufferu = 5;
  //granicaGlasnoce = 5000;
  kupimBuffere = false;
  //brojBufferaKojeKupim = 10;
  brojBufferaKojeSamPokupio = 0;

  
  /*
  //ucitavanje LSTER baze - ne valja, treba ili koristiti trenutno novi implementirani nacin ili snimiti novu bazu
  float lsterVr;
  int klasaVr;
  std::ifstream fajl("//home//larics//Jagodin//naoqi-sdk-1.14.5-linux64//doc//examples//KlasifikacijaGovora//build-mytoolchain//sdk//bin//LSTER.txt");
  while(fajl>>lsterVr>>klasaVr)
  {
  	bazaLSTER[lsterVr] = klasaVr;
  }*/
  
  /*  (I) Ako je ovo odkomentirano i ako buildamo kao local modul, modul ce se izvoditi stalno od trenutka kad se NAO upali. Ovaj dio je prebacen u posebnu metodu (ispod) da bi se izbjeglo nepotrebno mucenje NAO procesora.
  audioDevice->callVoid("setClientPreferences",
                        getName(),                //Name of this module
                        16000,                    //16000 Hz requested
                        (int)FRONTCHANNEL,         //1 Channel requested
                        0                         //Deinterleaving not requested
                        );

  startDetection();*/
}

void KlasifikacijaGovora::pocniKlasifikaciju( const int &granicaGlasnoceParam = 5000,
					      const int &brojOkviraPoBufferuParam = 5,
					      const int &brojBufferaKojeKupimParam = 10,
					      const int &frekvencijaParam = 16000,
					      const ALValue &channelIDParam = FRONTCHANNEL,
					      //const int &deinterleavingParam,// = 0, 
					      const int &inputBufferSizeParam = 8192) // 8192 (=170ms) ili 16384 (=340ms). Citat iz dokumentacije: "Warning: when speech recognition is running, a buffer size of 8192 is used. Don't change it during the recognition process."
{   
  audioDevice->callVoid("setClientPreferences",
                        getName(),                //Name of this module
                        frekvencijaParam,         //16000 Hz requested
                        (int)channelIDParam,      //1 Channel requested
                        0 //deinterleavingParam       //Deinterleaving not requested
                        );


  try //ukoliko pozovete modul nekim modulom pa prekinete npr. sa ctrl+c i onda opet pozovete, nece radit zato sto se opet poziva "setParameter", a vec je bio pozvan. Ako hocete  promijeniti bufferSize, restartajte naoqi pa pozovite nanovo. 
  {
    audioDevice->callVoid("setParameter", std::string("inputBufferSize") , inputBufferSizeParam);
  }
  catch(const AL::ALError&) // no object name given to avoid warning
  {
    qiLogInfo("KlasifikacijaGovora") << "Buffer size je " << audioDevice->call<int>("getParameter", std::string("inputBufferSize")) <<
    " i takav ce biti sve dok se naoqi ne restarta." << std::endl;
  }
  
  granicaGlasnoce = granicaGlasnoceParam;
  brojOkviraPoBufferu = brojOkviraPoBufferuParam;
  brojBufferaKojeKupim = brojBufferaKojeKupimParam;

  startDetection();
}

void KlasifikacijaGovora::prekiniKlasifikaciju()
{
  stopDetection();
}

KlasifikacijaGovora::~KlasifikacijaGovora()
{
  stopDetection();
}


/// Description: The name of this method should not be modified as this
/// method is automatically called by the AudioDevice Module.
void KlasifikacijaGovora::process(const int & nbOfChannels,
                                const int & nbOfSamplesByChannel,
                                const AL_SOUND_FORMAT * buffer,
                                const ALValue & timeStamp)
{
  /// Computation of the RMS power of the signal delivered by
  /// each microphone on a 170ms buffer
   
  
  
  int brojUzorakaPoOkviru = nbOfSamplesByChannel/brojOkviraPoBufferu;
  int ukupanBrojOkvira = brojOkviraPoBufferu*brojBufferaKojeKupim;
  
  if( kupimBuffere == false ) //cekam da dodje dovoljno glasan zvuk
  {
  	for( int i = 0; i < nbOfSamplesByChannel; i++ )
  	{
  		if( buffer[i] > granicaGlasnoce )
  		{
  			//std::cout << "Detektiran glasan zvuk.\n";
  			kupimBuffere = true;
  			
  			// inicijaliziram vektore u koje cu pokupit STE, ZCR, ili sta vec zelim...
  			
  			STE = std::vector<float>(ukupanBrojOkvira,0); //za sve okvire iz svih buffera
  			ZCR = std::vector<float>(ukupanBrojOkvira,0);
  			
  			break;
  		}
  	
  	}
  }
  
  if( kupimBuffere == true ) //znaci da je dosao glasan zvuk u ovom bufferu
  {
  	//std::cout << STE << std::endl;
  	//std::cout << "Kupim buffer broj " << brojBufferaKojeSamPokupio + 1 << ".\n";
  	//racunam STE, ZCR, sta vec hocu
  	
  	//STE
  	for( int i = 0; i < nbOfSamplesByChannel; i++ )
	{
		STE[ i/brojUzorakaPoOkviru + brojBufferaKojeSamPokupio*brojOkviraPoBufferu ] += (float)buffer[i]
	  					*(float)buffer[i];
	}
	for( int o = 0; o < brojOkviraPoBufferu; o++ )
	{
		STE[o + brojBufferaKojeSamPokupio*brojOkviraPoBufferu] /= (float)nbOfSamplesByChannel/brojOkviraPoBufferu;
		STE[o + brojBufferaKojeSamPokupio*brojOkviraPoBufferu] = sqrtf(STE[o + brojBufferaKojeSamPokupio*brojOkviraPoBufferu]);
	}
	
	
	//ZCR
	for( int i = 1; i < nbOfSamplesByChannel; i++ )
	{
		if( SGN(buffer[i-1]) != SGN(buffer[i]) )
		{ 
			ZCR[ i/brojUzorakaPoOkviru + brojBufferaKojeSamPokupio*brojOkviraPoBufferu ] += 1;
		}
	}
  		
  	
  	brojBufferaKojeSamPokupio++;
  	if( brojBufferaKojeSamPokupio == brojBufferaKojeKupim )
  	{
  		//std::cout << "Pokupio sam sve buffere.\n";
  		
  		
  		// racunam LSTER, HZCRR, sta vec
  		
  		
  		//LSTER
  		float avgSTE = 0;
  		for( int okv = 0; okv < ukupanBrojOkvira; okv++ )
  		{
  			avgSTE += STE[okv];
  		}
  		avgSTE /= ukupanBrojOkvira;
  		float _05avgSTE = avgSTE * 0.5;
  		float LSTER = 0;
  		for( int okv = 0; okv < ukupanBrojOkvira; okv++ )
  		{
  			if( STE[okv] < _05avgSTE )
  			{
  				LSTER++;
  			}
  		}
  		LSTER /= ukupanBrojOkvira;
  		
  		
  		//HZCRR
  		float avgZCR = 0;
  		for( int okv = 0; okv < ukupanBrojOkvira; okv++ )
  		{
  			avgZCR += ZCR[okv];
  		}
  		avgZCR /= ukupanBrojOkvira;
  		float _15avgZCR = avgZCR * 1.5;
  		float HZCRR = 0;
  		for( int okv = 0; okv < ukupanBrojOkvira; okv++ )
  		{
  			if( ZCR[okv] > _15avgZCR )
  			{
  				HZCRR++;
  			}
  		}
  		HZCRR /= ukupanBrojOkvira;
  		
  		//std::cout << "HZCRR : " << HZCRR << std::endl;
  		//std::cout << "LSTER : " << LSTER << std::endl;
  		
  		
  		//klasifikacija...
  		
  		std::string klasa;
  		
  		if( HZCRR < 0.005 )
  		{
  			klasa = "Neartikulirano";
  			std::cout << "Neartikulirano\n";
  		}
  		else
  		{
  		
  			/*
  			//knn - govorni LSTER se kreće oko 0.3, dok se na onoj baz staroj kreto oko 0.65. Treba valjd novu bazu napravit...
  			std::map<float, int> udaljenostiMapa; // udaljenostiMapa[abs(moj_lster - baza_lster_i)] = baza_klasa_i //teoretski je moguce da budu 2 iste udaljenosti pa ce se overwrajtat, ali buduci da radim so nekih 10 decimala, sanse su >>>jako<<< male, a i ako se dogodi, nece bit nist, osim sto ce >>>mozda<<< tada pogresno klasificirat.
	  		//std::vector<float> udaljenostiMapa
	  		
	  		for( std::map<float,int>::iterator it = bazaLSTER.begin(); it != bazaLSTER.end(); ++it )
			{
				udaljenostiMapa[ fabs(it->first - LSTER) ] = bazaLSTER[it->first];
			}
			
			for( std::map<float,int>::iterator it = udaljenostiMapa.begin(); it != udaljenostiMapa.end(); ++it )
			{
				std::cout << it->first << "\t" << udaljenostiMapa[it->first] << std::endl; 
			}
			*/
			
			//imporvizirano odredjivanje buduci da je baza losa
  			if( LSTER > 0.13 && LSTER < 0.45 )
  			{
  				klasa = "Artikulirano";
  				//std::cout << "Artikulirano\n";
  			}
  			else
  			{
  				klasa = "Neartikulirano";
  				//std::cout << "Neartikulirano\n";
  			}
  			
  			//pohrana podataka, ovo se isto moglo direktno u almemory napisat.
  			
  			AL::ALValue eventValue;
  			eventValue.arrayPush(klasa);
  			
  			AL::ALValue vektorZnacajki;
  			vektorZnacajki.arrayPush(HZCRR);
  			vektorZnacajki.arrayPush(LSTER);
  			
  			eventValue.arrayPush(vektorZnacajki);
  			
  			eventValue.arrayPush(timeStamp);
  			
  			fProxyToALMemory.raiseEvent("KlasifikacijaGovoraEvent", eventValue);
  			
  			/* Zapisivanje u datoteku; ne treba vise valjda.
  			std::ofstream logDat;
			logDat.open ("/home/nao/NikolaJagodin/KlasifikacijaGovora_log.txt", std::ios_base::app);
			logDat << timeStamp << " " << klasa << " (" << HZCRR << ", " << LSTER << ")\n";
			logDat.close();
  			*/
  		
  		
  		}
  		
  		
  		//std::cout << "Kraj." << std::endl;
	
  		brojBufferaKojeSamPokupio = 0;
  		kupimBuffere = false;
  	} 	
  }
}

