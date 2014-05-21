#include "KlasifikacijaGovora.h"
#include <alcommon/alproxy.h>
#include <iostream>
#include <math.h>
#include <fstream>
#include <qi/log.hpp>
#include <althread/alcriticalsection.h>
#define SGN(x) (x>=0)-(x<0) //signum funkcija




LRKlasifikacijaZvukova::LRKlasifikacijaZvukova(boost::shared_ptr<ALBroker> pBroker,
                                     std::string pName) : ALSoundExtractor(pBroker, pName),
                                     fProxyToALMemory(getParentBroker()),
                                     fCallbackMutex(AL::ALMutex::createALMutex())
                                     
{
  setModuleDescription("Klasifikacija Zvokova. Kada cuje glasan zvuk, klasificira ga i dize event \"LRKlasifikacijaZvukovaEvent\" i u njega sprema ALValue strukturu\n"
  		       "[string koji sadrzi vrijednost \"Artikulirano\" ili \"Neartikulirano\",\n"
  		       "[float HZCRR, float LSTER],\n"
  		       "timeStamp]");
  
  
  
  
  
  		  
  functionName("pocni_klasifikaciju", getName(), "Zapocinje detektiranje glasnih zvukova i njihova klasifikacija.");
  addParam("parametri", "ALValue lista u formatu [[granicaGlasnoce, brojBufferaKojeKupim, brojOkviraPoBufferu],[frekvencija, channelID, deinterleaving, inputBufferSize]]");
  BIND_METHOD(LRKlasifikacijaZvukova::pocni_klasifikaciju);
  
  functionName("prekini_klasifikaciju", getName(), "Prekida.");
  BIND_METHOD(LRKlasifikacijaZvukova::prekini_klasifikaciju);
  
  /*functionName("process", getName(), "test");
  addParam("nbOfChannels", "Neka lista, ne znam jos kakva.");
  addParam("nbOfSamples", "Neka lista, ne znam jos kakva.");
  addParam("buffer", "Neka lista, ne znam jos kakva.");
  addParam("timeStamp", "Neka lista, ne znam jos kakva.");
  BIND_METHOD(LRKlasifikacijaZvukova::process);*/

}


void LRKlasifikacijaZvukova::pocni_klasifikaciju( const AL::ALValue &parametri )
{
    AL::ALCriticalSection section(fCallbackMutex);
	if(!parametersSet) {
        parametersSet = true;
        //qiLogWarning("LRKLasifikacijaZvuka") << "Postavljam parametre audio obrade\n";
        granicaGlasnoce = (int)parametri[0][0];
        brojBufferaKojeKupim = (int)parametri[0][1];
        brojOkviraPoBufferu = (int)parametri[0][2];

        audioDevice->callVoid("setClientPreferences",
                      getName(),
                      (int)parametri[1][0], //frekvencija
                      (int)parametri[1][1], //channel
                      (int)parametri[1][2]  //deinterleaving
                      );
        //qiLogWarning("LRKLasifikacijaZvuka") << "Pokusat cu postavit input buffer size na " << (int)parametri[1][3] << std::endl;
        audioDevice->callVoid("setParameter", std::string("inputBufferSize"), (int)parametri[1][3]);

    }
	
	startDetection();
	
	frekvencija = (int)parametri[1][0]; //ovo ce trebati kasnije za racunanje "bins" u metodi "process"
	
	//std::cout << parametri << std::endl;
}
		
void LRKlasifikacijaZvukova::prekini_klasifikaciju()
{
	stopDetection();
}				  



void LRKlasifikacijaZvukova::init()
{
 
  /*
  AL::ALValue parametriObrada, parametriSnimanje, parametri;
  parametriObrada.arrayPush(10000); //granica glasnoce
  parametriObrada.arrayPush(5); //broj okvira koje kupim
  parametriObrada.arrayPush(5); //broj buffera po okviru
  parametriSnimanje.arrayPush(16000); //frekvencija snimanja
  parametriSnimanje.arrayPush(FRONTCHANNEL); //mikrofon
  parametriSnimanje.arrayPush(0); //interleaving
  parametriSnimanje.arrayPush(16384); //velicina buffera
  parametri.arrayPush(parametriObrada);
  parametri.arrayPush(parametriSnimanje);
  std::cout << parametri << std::endl;
  pocni_klasifikaciju( parametri );
  std::cout << parametri << std::endl;
  */

  fProxyToALMemory.declareEvent("SoundClassified", "LRKlasifikacijaZvukova");
  parametersSet = false;
  frekvencija = 0;
  brojOkviraPoBufferu = 0;
  brojBufferaKojeKupim = 0;
  kupimBuffere = false;
  brojBufferaKojeSamPokupio = 0;
  granicaGlasnoce = 0;
  
}


LRKlasifikacijaZvukova::~LRKlasifikacijaZvukova()
{
  stopDetection();
}


/// Description: The name of this method should not be modified as this
/// method is automatically called by the AudioDevice Module.
void LRKlasifikacijaZvukova::process(const int & nbOfChannels,
                                const int & nbOfSamplesByChannel,
                                const AL_SOUND_FORMAT * buffer,
                                const ALValue & timeStamp)
{
  /// Computation of the RMS power of the signal delivered by
  /// each microphone on a 170ms buffer
  
  
  AL::ALCriticalSection section(fCallbackMutex);
  
  int brojUzorakaPoOkviru = nbOfSamplesByChannel/brojOkviraPoBufferu;
  int ukupanBrojOkvira = brojOkviraPoBufferu*brojBufferaKojeKupim;
  
  
  if( kupimBuffere == false ) //cekam da dodje dovoljno glasan zvuk
  {
  	for( int i = 0; i < nbOfSamplesByChannel; i++ )
  	{
  		if( buffer[i] > granicaGlasnoce ) // buffer[i] > granicaGlasnoce )
  		{
  		
            //std::cout << "Detektiran glasan zvuk.\n";
  			kupimBuffere = true;
  			//soundDat.open ("/home/larics/Jagodin/naoqi-sdk-1.14.5-linux64/doc/examples/LRKlasifikacijaZvukova/sound.txt", std::ios::trunc);
  			//soundDat.open ("/home/nao/NikolaJagodin/sound.txt", std::ios::trunc);
  			
  			// inicijaliziram vektore u koje cu pokupit STE, ZCR, ili sta vec zelim...
  			
  			//STE = std::vector<float>(ukupanBrojOkvira,0); //za sve okvire iz svih buffera
  			//ZCR = std::vector<float>(ukupanBrojOkvira,0);
  			//ASC = std::vector<float>(ukupanBrojOkvira,0);
  			//ASS = std::vector<float>(ukupanBrojOkvira,0);
  			//ASF = std::vector<float>(ukupanBrojOkvira,0);
  			
  			sviOkviri.resize( ukupanBrojOkvira );
			for( int o = 0; o < ukupanBrojOkvira ; o++ )
			{
				sviOkviri[o].resize( brojUzorakaPoOkviru );
			}
  			
  			//std::cout << sviOkviri.size() << "," << sviOkviri[0].size() << "\n";
  			break;
  		}
  	
  	}
  }
  
  
  if( kupimBuffere == true ) //znaci da je dosao glasan zvuk u ovom bufferu
  {
  	//std::cout << STE << std::endl;
    //std::cout << "Kupim buffer broj " << brojBufferaKojeSamPokupio + 1 << ".\n";
  	//racunam STE, ZCR, sta vec hocu
  	
  	
  	/*for( int i = 0; i < nbOfSamplesByChannel; i++ )
	{
		soundDat << buffer[i] << "\n";
	}*/
	
	for( int o = 0; o < brojOkviraPoBufferu; o++ ) //kupim sve okvire iz buffera na jedno mjesto
	{
		for( int i = 0; i < brojUzorakaPoOkviru; i++ )
		{
			sviOkviri[brojBufferaKojeSamPokupio*brojOkviraPoBufferu + o][i] = buffer[o*brojUzorakaPoOkviru + i];
		}
	}
	
	
  	
  	brojBufferaKojeSamPokupio++;
  	if( brojBufferaKojeSamPokupio == brojBufferaKojeKupim )
  	{
        //std::cout << "Pokupio sam sve buffere.\n";
  		
  		//###################################################################################
		//################# O D R E D J I V A N J E     Z N A C A J K I #####################
		//###################################################################################
  		
  		
  		std::vector<float> ZCR( ukupanBrojOkvira, 0.0 ); // alokacija prostora
  		std::vector<float> STE( ukupanBrojOkvira, 0.0 );
  		
  		for( int o = 0; o < ukupanBrojOkvira; o++ ) //za svaki okvir
  		{
  			//ZCR - Zero Crossing Rate
  			for( int i = 1; i < brojUzorakaPoOkviru; i++ )
  			{
  				if( SGN(sviOkviri[o][i-1]) != SGN(sviOkviri[o][i]) )
  				{
  					ZCR[o] += 1;
  				}
  			}
  			ZCR[o] /= (float)brojUzorakaPoOkviru;
  			
  			//STE - Short Time Enegry
  			for( int i = 0; i < brojUzorakaPoOkviru; i++ )
  			{
  				STE[o] += (float)sviOkviri[o][i]*sviOkviri[o][i]; 
  			}
  			STE[o] /= (4.0 * brojUzorakaPoOkviru * brojUzorakaPoOkviru);
  		}
  		
  		
  		// racunam LSTER, HZCRR, sta vec
  		
  		
  		//LSTER - Low Short Time Energy Ratio
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
  		
  		
  		//HZCRR - High Zero Crossing Rate Ratio
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
  		
	
	
  		// FFT , koristit ce se za racunanje veceg broja znacajki
  		std::vector< kiss_fft_cpx * > FFT; //prostor za FFT transformacije svakog okvira
  		FFT.resize( ukupanBrojOkvira ); //alociram
  		int nfft = brojUzorakaPoOkviru; // radimo n-point FFT, gdje je n = brojUzorakaPoOkviru, dakle NE zaokruzuje se na prvu vecu potenciju broja 2 (a u praksi se zna to radit zato sto se u tom slucaju FFT transformacija moze efikasnije obaviti, a dodatnim nulama ne gubimo informaciju u frekvencijskoj domeni; to je kao da smo dodali extra tisine ( al to je samo ukoliko koristimo Cooley-Tuckey algoritam (valjd se tako pise)))
  		kiss_fft_cfg cfg = kiss_fft_alloc( nfft, 0, NULL, NULL ); //konfiguracija fft transformacije
		kiss_fft_cpx *signalCPX; //mjesto za signal pretvoren u kompleksni broj, parametar za fft()
		signalCPX = (kiss_fft_cpx *)malloc(nfft * sizeof(kiss_fft_cpx)); //alociram prostor ovisno o tome koliki je nfft
  		for( int o = 0; o < ukupanBrojOkvira; o++ )
  		{
  			for( int i = 0; i < brojUzorakaPoOkviru; i++ ) //kopiram signal
  			{
  				signalCPX[i].r = sviOkviri[o][i];
  				signalCPX[i].i = 0.0;
  			}
  			for( int i = brojUzorakaPoOkviru; i < nfft; i++ ) //prosirujem nulama u slucaju da je nfft > brojUzorakaPoOkviru, tj. nfft smo zaokruzili na prvu vecu potenciju broja 2
  			{
  				signalCPX[i].r = 0.0;
  				signalCPX[i].i = 0.0;
  			}
  			FFT[o] = (kiss_fft_cpx *)malloc(nfft * sizeof(kiss_fft_cpx)); //alocira se prostor za 'o'-ti okvir
  			kiss_fft( cfg, signalCPX, FFT[o] ); //provodi se transformacija za o-ti okvir, rezultat spremljen u FFT[o]
  		}
  		free(cfg); //oslobađanje prostora
  		free(signalCPX);
  		
  		
  		
		// bins
		int nyquist = nfft/2; // pola FFT transformacije ne daje novu informaciju, buduci da je simetricno
		std::vector<float> bin(nyquist); //ili (nyquist + 1) ?
		float fr2nfft = (float)frekvencija/(float)nfft; // ovaj omjer racunam tu da ne racunam kod svake iteracije ispod
		for( int n = 0; n < nyquist; n++ )
		{
			bin[n] = n * fr2nfft;
		}
	
		//std::cout << "bin" << AL::ALValue( bin ) << std::endl;
	
	
	
  		
		// power spectrums, trebat ce za vise znacajki
		std::vector< std::vector<float> > PS;// power spectrum; ps[i] = |fft[i]|^2
		PS.resize( ukupanBrojOkvira ); // alocira se prostor
		std::vector<float> sumePS( ukupanBrojOkvira, 0.0 ); // sume PS-ova od svakog okvira, racuna se tu zato sto se kasnije dosta cesto koristi
		for( int o = 0; o < ukupanBrojOkvira; o++ )
		{
			PS[o].resize( bin.size() ); //alocira se prostor za svaki okvir
			for( int i = 0; i < PS[o].size(); i++ )
			{
				PS[o][i] = (FFT[o][i].r * FFT[o][i].r) + (FFT[o][i].i * FFT[o][i].i); // |fft|^2 = sqrt(fft.r^2 + fft.i^2)^2
				sumePS[o] += PS[o][i];
			}
		}


		//std::cout << "\nsumePS: " << AL::ALValue( sumePS ) << std::endl;
	
	
		//ASC - Audio Spectrum Centroid
  		std::vector<float> ASC( ukupanBrojOkvira, 0.0 ); //alociram prostor za ASC
  		for( int o = 0; o < ukupanBrojOkvira; o++ )
  		{
  			for( int i = 0; i < PS[o].size(); i++ )
  			{
  				ASC[o] += bin[i] * PS[o][i];
  				
  				//tu ispod je drugi nacin racunanja, gdje se frekvencije (bins) skaliraju i logaritmiraju, al ovo mi je intuitivnije pa sam ovako ostavio
  				/*
  				if( bin[i] == 0 ) 
  				{
  					ASC[o] += log2( bin[i]/1000 + 0.0001 )*PS[o][i]; // log2(0)=-inf
  				}
  				else
  				{
  					ASC[o] += log2( bin[i]/1000 )*PS[o][i];
  				}
  				*/
  			}
  			ASC[o] /= sumePS[o];
  		}
  		
        //std::cout << "ASC-evi svih okvira : " << AL::ALValue( ASC ) << std::endl;
  		
  		
  		//ASS - Audio Spectrum Spread
  		std::vector<float> ASS( ukupanBrojOkvira, 0.0 ); //alociram prostor
  		//float log2binKroz1000;
  		for( int o = 0; o < ukupanBrojOkvira; o++ )
  		{
  			for( int i = 0; i < PS[o].size(); i++ )
  			{
  				ASS[o] += (bin[i]-ASC[o])*(bin[i]-ASC[o])*PS[o][i];
  				
  				//isto ko i kod AC-a; ako se ASC racunao s logaritmiranjem frekvencija, trebalo bi se i tu ovo dolje koristiti
  				/*
  				std::cout << "NE";
  				if( bin[i] == 0 )
  				{
  					log2binKroz1000 = log2( bin[i]/1000 + 0.0001 );
  				}
  				else
  				{
  					log2binKroz1000 = log2( bin[i]/1000 );
  				}
  				ASS[o] += (log2binKroz1000 - ASC[o]) * (log2binKroz1000 - ASC[o]) * PS[o][i];
  				*/
  			}
  			ASS[o] /= sumePS[o];
  			ASS[o] = sqrtf( ASS[o] );
  		}
  		
  		
        //std::cout <<  "ASS-evi svih okvira : " << AL::ALValue( ASS ) << std::endl;
  		


		//ASF - Audio Spectrum Flatness
		std::vector<float> ASF( ukupanBrojOkvira, 0.0 );
		for( int o = 0; o < ukupanBrojOkvira; o++ )
		{
			for( int i = 0; i < PS[o].size(); i++ )
			{
				ASF[o] += log( PS[o][i] );
			}
			ASF[o] /= (float)PS[o].size();
			ASF[o] = exp( ASF[o] ) / (sumePS[o]/(float)PS[o].size());
		}
	
        //std::cout <<  "ASF-evi svih okvira : " << AL::ALValue( ASF ) << std::endl;
	

  		for( int o = 0; o < FFT.size(); o++ ) //osloadjanje prostora
  		{
  			free(FFT[o]);
  		}
  		
		
		
		
		
		
		
		//###################################################################################
		//############################# K L A S I F I K A T O R #############################
		//###################################################################################
		
		//sa ispravljenim racunanjem STE znacajke, klasifikator radi extra lose. Treba snimit uzorke i novi napravit. Ili nove znacajke dodat. Ili mozda probat iskoristit ASC, ASS i ASF. Ove znacajke se razlikuju od "starih" u tome sto su one izracunate iz frekvencijske domene signala, dok su LSTER i HZCRR racunate u vremenskoj domeni. Dodatno, LSTER i HZCRR opisuju cijeli zvuk (to su 2 broja), dok ASC, ASS i ASF opisuju svaki okvir (to su liste s <brojBufferaKojeKupim> * <brojOkviraPoBufferu> brojeva, isto kao i STE i ZCR).
  		
  		std::string klasa;
  		
  		if( HZCRR < 0.005 )
  		{
  			klasa = "Neartikulirano";
  			std::cout << "Neartikulirano\n";
  		}
  		else
  		{
			//imporvizirano odredjivanje
  			if( LSTER > 0.6 && LSTER < 0.9 )
  			{
  				klasa = "Artikulirano";
  				std::cout << "Artikulirano\n";
  			}
  			else
  			{
  				klasa = "Neartikulirano";
  				std::cout << "Neartikulirano\n";
  			}
  		}
  		
  		//pohrana podataka, ovo se isto moglo direktno u almemory napisat.
  			
		AL::ALValue eventValue;
		eventValue.arrayPush(klasa);
		
		AL::ALValue vektorZnacajki;
		vektorZnacajki.arrayPush(HZCRR);
		vektorZnacajki.arrayPush(LSTER);
		vektorZnacajki.arrayPush(ASC);
		vektorZnacajki.arrayPush(ASS);
		vektorZnacajki.arrayPush(ASF);
		
		eventValue.arrayPush(vektorZnacajki);
		
		eventValue.arrayPush(timeStamp);
		
        fProxyToALMemory.raiseEvent("SoundClassified", eventValue);
        qiLogWarning("LRKlasifikacijaZvukova") << "SoundClassified event raisan\n";
 
  		
  		
  		//std::cout << "Kraj." << std::endl;
	
	
	
	
  		brojBufferaKojeSamPokupio = 0;
  		kupimBuffere = false;
  		//soundDat.close();
  	} 	
  }
}
