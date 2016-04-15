/*
 * Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

#include <iostream>
#include <vector>
#include <fstream>
#include <termios.h>
#include <fcntl.h>

#include <alerror/alerror.h>
#include <alcommon/alproxy.h>
#include <alvalue/alvalue.h>

#include "config.h"

using namespace std;
using namespace AL;

string configPath = "Resources/SC.config";
ifstream configFileIN;
parameters par;

void changemode(int dir)
{
  static struct termios oldt, newt;

  if ( dir == 1 )
  {
    tcgetattr( STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~( ICANON | ECHO );
    tcsetattr( STDIN_FILENO, TCSANOW, &newt);
  }
  else
    tcsetattr( STDIN_FILENO, TCSANOW, &oldt);
}

int kbhit (void){
  struct timeval tv;
  fd_set rdfs;

  tv.tv_sec = 0;
  tv.tv_usec = 0;

  FD_ZERO(&rdfs);
  FD_SET (STDIN_FILENO, &rdfs);

  select(STDIN_FILENO+1, &rdfs, NULL, NULL, &tv);
  return FD_ISSET(STDIN_FILENO, &rdfs);

}

void readParams(int argc, char *argv[]){
    if (argc > 1) configPath = argv[1];

    string temp;
    string params[10];

    configFileIN.open(configPath, ifstream::in);
    for (int k=0; configFileIN >> temp; k++)
        params[k] = temp;
    configFileIN.close();

    par.fill(params);
}

int main(int argc, char *argv[]){

    readParams(argc, argv);

    try {
        boost::shared_ptr<AL::ALProxy> moduleProxy = boost::shared_ptr<AL::ALProxy>(new AL::ALProxy("SCModule", par.robotIP, par.robotPort));
        boost::shared_ptr<AL::ALProxy> memoryProxy = boost::shared_ptr<AL::ALProxy>(new AL::ALProxy("ALMemory", par.robotIP, par.robotPort));

        ALValue oldHistory, newHistory;

        try {oldHistory = memoryProxy->call<ALValue>("getEventHistory", "soundClassified");}
        catch (const AL::ALError& e){}

        moduleProxy->callVoid("initRTClassification", ALValue(par.model), ALValue(par.featParams), ALValue(par.classList));

        char c = 'a';
        while (c != 'n' && c != 'y'){
            cout << "Module ready.\nStart real time classification? (y/n)" << endl;
            cin >> c;
            cin.ignore();
        }
        if (c == 'y'){
            cout << "Press ENTER to end the process" << endl;
            moduleProxy->callVoid("startRTClassification");
            int i = 1;
            while(!kbhit()){
                try {newHistory = memoryProxy->call<ALValue>("getEventHistory", "soundClassified");}
                catch (const AL::ALError& e){}
                for (int k= oldHistory.getSize(); k < newHistory.getSize(); k++)
                    cout << i++ << "\t" + (string)newHistory[k][0] << endl;
                oldHistory = newHistory;
            }
            moduleProxy->callVoid("stopRTClassification");
        }




    }
    catch (const AL::ALError& e) {
        std::cerr << e.what() << std::endl;
    }
}
