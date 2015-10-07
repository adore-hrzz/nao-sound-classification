#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <algorithm>
#include <cstdio>

using namespace std;

//DEFAULT CONFIGURATION
string temp;

string names[10] = {"Sound library folder: ",
                   "Sound library list: ",
                   "Feature selection string: ",
                   "Sound features data: ",
                   "Learned classes list:",
                   "Classifying model: ",
                   "Robot IP: ",
                   "Robot port: ",
                   "Sound sample length [pow(2, x)]: ",
                   "subSample length [pow(2, x)]: "};

string params[10] = {"Resources/Sounds/",
                    "Resources/List",
                    "abcdefghjkl",
                    "Resources/modelData.csv",
                    "Resources/classList.txt",
                    "Resources/model.xml",
                    "192.168.1.106",
                    "9559",
                    to_string(13),
                    to_string(9)};

string d_params[10] = {"Resources/Sounds/",
                      "Resources/List.txt",
                      "abcdefghjkl",
                      "Resources/modelData.csv",
                      "Resources/classList.txt",
                      "Resources/model.xml",
                      "192.168.1.106",
                      "9559",
                      to_string(13),
                      to_string(9)};


string configPath = "Resources/SC.config";

ifstream configFileIN;
ofstream configFileOUT;

ifstream soundListIN;
ofstream soundListOUT;
vector<string> soundList;

bool cmpVec(const string &i1, const string &i2){
    return i1.substr(i1.find('_')+1) < i2.substr(i2.find('_')+1);
}

void readParams(int argc, char *argv[]){
    if (argc > 1) configPath = argv[1];

    configFileIN.open(configPath);
    for (int k=0; configFileIN >> temp; k++) params[k] = temp;
    configFileIN.close();
}

int main(int argc, char *argv[]){

    readParams(argc, argv);

    char c;
    while(true){
        cout << "#########-CONFIGURATOR-##########" << endl;
        cout << "# Choose one of the options:    #" << endl;
        cout << "# a: Show current configuration #" << endl;
        cout << "# b: Set default configuration  #" << endl;
        cout << "# c: Change one parameter       #" << endl;
        cout << "# s: Sort sound list by class   #" << endl;
        cout << "# x: Save and exit              #" << endl;
        cout << "# q: Exit without save          #" << endl;
        cout << "#################################" << endl;
        cout << endl;
        cin >> c ;
        switch(c){
        case 'a':
            cout << endl;
            for (int k=0; k < 10; k++){
                cout.width(36);
                cout << right << names[k];
                cout << left << params[k] << endl;
            }
            cout << endl;
            break;
        case 'b':
            for (int k=0; k < 10; k++) params[k] = d_params[k];
            break;
        case 'c':
            cout << endl;
            cout << "Choose one of the parameters:" << endl;
            for (int k=0; k < 10; k++) cout << k << ": " << names[k].substr(0,names[k].size()-2) << endl;
            cout << endl;
            cout << "[EXAMPLE] 2 Resources/List.txt" << endl;
            cout << endl;
            int num;
            cin >> num >> temp;
            params[num] = temp;
            break;
        case 's':
            soundListIN.open(params[1], istream::in);
            soundList.clear();
            for (int k=0; soundListIN >> temp; k++) soundList.push_back(temp);
            soundListIN.close();

            sort(soundList.begin(),soundList.end(), cmpVec);

            soundListOUT.open(params[1], ofstream::out | ofstream::trunc);
            for(int k=0; k < soundList.size(); k++){
                string className = soundList[k].substr(soundList[k].find('_')+1);
                string newName = to_string(k+1) + "_" + className;
                soundListOUT << newName << endl;
                rename ((params[0] + soundList[k] + ".wav").c_str(), (params[0] + newName + ".wav").c_str());
            }
            soundListOUT.flush();
            soundListOUT.close();
            break;
        case 'x':
            configFileOUT.open(configPath, ofstream::out | ofstream::trunc);
            for (int k=0; k < 10; k++) configFileOUT << params[k] << endl;
            configFileOUT.flush();
            configFileOUT.close();
        case 'q':
            return 0;
            break;
        }
        }
}
