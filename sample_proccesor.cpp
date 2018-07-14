#include <iostream>
#include <fstream>
#include <string>

using namespace std;

int main (int argc, char *argv[]){
  if(argc != 4){
    cout << "Usage: filter.exe \"path to sample file\" \"path to output file\" \"discord user id\"" << endl;
  }
  string line;
  string tmp;
  tmp = string(argv[1]);
  ifstream file(tmp);
  tmp = string(argv[2]);
  ofstream new_file(tmp);
  tmp = string(argv[3]);
  tmp.insert(0, " ");
  string search = tmp;
  if(!new_file.is_open()){
    cout << "file error." << endl;
    return 1;
  }
  if(file.is_open()){
    while(getline(file, line)){
      if(line.find(search) != string::npos){
        while(getline(file, line)){
          if(line.find("[") != string::npos){
            break;
          }
          new_file << line << endl;
        }
      }
    }
    file.close();
  }else{

  }
  new_file.close();
}
