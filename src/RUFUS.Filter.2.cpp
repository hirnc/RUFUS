/*By ANDREW FARRELL
 * RUFUS.CheckHashFilter.cpp
 * TODO: describe funciton of file
 */

#include <bitset>
#include <fstream>
#include <ios>
#include <iostream>
#include <math.h>
#include <sstream>
#include <stack>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sys/resource.h>
#include <time.h>
#include <unistd.h>
#include <unordered_map>
#include <vector>

#include "Util.h"

using namespace std;
unsigned long HashToLongLocal( const char* hash, int& len)
{
  bitset<64> HashBits;
  //#pragma omp parellel for                                                                                                                                                                    
  for(int i=0; i<len;i++)
    {
      if (hash[i] == 'A')
        {
          HashBits[i*2] = 0;
          HashBits[i*2+1] = 0;
        }
      else  if (hash[i] == 'C')
        {
          HashBits[i*2] = 0;
          HashBits[i*2+1] = 1;
        }
      else  if (hash[i] == 'G')
        {
          HashBits[i*2] = 1;
          HashBits[i*2+1] = 0;
        }
      else  if (hash[i] == 'T')
        {
          HashBits[i*2] = 1;
          HashBits[i*2+1] = 1;
        }
      else
        {
          cout << "ERROR, invalid character - " << hash[i] << endl;
        }
    }
  //cout <<  HashBits.to_ulong() << "-" << endl;                                                                                                                                                
  return HashBits.to_ulong();
}

int main(int argc, char *argv[]) {
  cout << "Call is PreBuiltMutHash Mutant.fq  firstpassfile hashsize MinQ "
          "HashCountThreshold window threads "
       << endl;
  double vm, rss, MAXvm, MAXrss;
  MAXvm = 0;
  MAXrss = 0;
  Util::process_mem_usage(vm, rss, MAXvm, MAXrss);
  cout << "VM: " << vm << "; RSS: " << rss << endl;

  //int BufferSize = 28;

  cout << "Paramaters are:\n  PreBuiltMutHash = " << argv[1]
       << "\n  Mutant.fq = " << argv[2] << "\n  out stub = " << argv[3]
       << "\n  HashSize = " << argv[4] << "\n  MinQ = " << argv[5]
       << "\n  HashCountThreshold = " << argv[6] << "\n  Window = " << argv[7]
       << "\n  Threads = " << argv[8] << endl;
  // Read in file passed to the program on the command line
	cout << "YAAAAAY" << endl; 
  string temp = argv[4];
  int HashSize = atoi(temp.c_str());
  temp = argv[5];
  int MinQ = atoi(temp.c_str());
  temp = argv[6];
  int HashCountThreshold = atoi(temp.c_str());
  temp = argv[7];
  int Window = atoi(temp.c_str());
  temp = argv[8];
  int Threads = atoi(temp.c_str());
  int BufferSize = Threads*4;
  int hm1 = HashSize - 1;  
  char MinQC = char (33+MinQ); 
  cout << "MinQ = " << MinQ << " so char value is " << 33+MinQ << " so char is " << MinQ << endl;
  ifstream MutHashFile;
  MutHashFile.open(argv[1]);
  if (MutHashFile.is_open()) {
    cout << "Parent File open - " << argv[1] << endl;
  }  // cout << "##File Opend\n";
  else {
    cout << "Error, ParentHashFile could not be opened";
    return 0;
  }

  string filename = argv[2];
  ifstream MutFile;
  if (filename == "stdin") {
    cout << "MutFile is STDIN" << endl;
    MutFile.open("/dev/stdin");
  } else {
    cout << "MutFile is " << argv[2] << endl;
    MutFile.open(argv[2]);
  }
  if (MutFile.is_open()) {
    cout << "##File Opend\n";
  } else {
    cout << "Error, MutFile could not be opened";
    return 0;
  }

  ofstream MutOutFile;
  string FirstPassFile = argv[3];
  FirstPassFile += ".Mutations.fastq";
  MutOutFile.open(FirstPassFile.c_str());
  if (MutOutFile.is_open()) {
  } else {
    cout << "ERROR, Output file could not be opened -" << argv[3] << endl;
    return 0;
  }

  string line;
  unordered_map<unsigned long, int> Mutations;
  cout << "Reading in pre-built hash talbe\n";
  int lines = 0;
  string L1;
  string L2;
  string L3;
  string L4;
  unsigned long LongHash;
  bool notdone = true;
  cout << "starting " << endl;
  cout << "  Reading in MutHashFile" << endl;

  while (getline(MutHashFile, L1)) {
    vector<string> temp;
    temp = Util::Split(L1, '\t');

    if (temp.size() == 2) {
      unsigned long b = Util::HashToLong(temp[0]);
      unsigned long revb = Util::HashToLong(Util::RevComp(temp[0]));
      Mutations.insert(pair<unsigned long, int>(b, 0));
      Mutations.insert(pair<unsigned long, int>(revb, 0));
    } else if (temp.size() == 4) {
      unsigned long b = Util::HashToLong(temp[3]);
      unsigned long revb = Util::HashToLong(Util::RevComp(temp[3]));
      Mutations.insert(pair<unsigned long, int>(b, 0));
      Mutations.insert(pair<unsigned long, int>(revb, 0));
    }
    if (temp.size() == 1) {
      temp = Util::Split(L1, ' ');
      unsigned long b = Util::HashToLong(temp[0]);
      unsigned long revb = Util::HashToLong(Util::RevComp(temp[0]));
      Mutations.insert(pair<unsigned long, int>(b, 0));
      Mutations.insert(pair<unsigned long, int>(revb, 0));
    }
  }

  MutHashFile.close();
  cout << "\nDone Hash Files\n";
  cout << "   Mutations Hash size is " << (int)Mutations.size() << endl;
  int who = RUSAGE_SELF;
  struct rusage usage;
  int b = getrusage(RUSAGE_SELF, &usage);
  cout << "I am using " << usage.ru_maxrss << endl;

  cout << "VM: " << vm << "; RSS: " << rss << "; maxVM: " << MAXvm
       << "; maxRSS: " << MAXrss << endl;
  cout << "Starting Search " << endl;
  clock_t St, Et;
  St = clock();
  int found = 0;
  lines = 0;
  string Buffer[2400];

  while (getline(MutFile, L1)) {
    lines++;
    Buffer[0] = L1;
    getline(MutFile, Buffer[1]);
    getline(MutFile, Buffer[2]);
    getline(MutFile, Buffer[3]);

    if (lines % 10000 > 1 && (lines % (10000 + Threads) < Threads)) {
      Et = clock();
      float Dt = ((double)(Et - St)) * CLOCKS_PER_SEC;
      cout << "Read in " << lines << " lines: Found " << found
           << " Reads per sec = " << (float)lines / (float)Dt << " \r";
    }

    int pos = 4;

    while (getline(MutFile, Buffer[pos])) {
      pos++;
      if (pos == BufferSize) {
	break;
      }
    }
#pragma omp parallel for shared(MutOutFile) num_threads(Threads)
    for (int BuffCount = 0; BuffCount < pos; BuffCount += 4) 
    {
      const char* qual=Buffer[BuffCount + 3].c_str(); 
      const char* seq=Buffer[BuffCount + 1].c_str(); 
      int MutHashesFound = 0;
      int streak = 0; 
      for (int i = 0; i < Buffer[BuffCount + 1].length() ; i++) {
       	if (qual[i] < (char) MinQC){ //(((int)Buffer[BuffCount + 3].c_str()[i] - 33) < MinQ){ /// or (int)Buffer[BuffCount + 1].c_str()[i] == 78) {
	  streak = 0;
        }
	else
		streak++;
        if (streak > hm1 ) 
	{
          const char* hash = seq+i-hm1; 
          //cout << "i = " << i << " streek = " << streak << " hash - " << string(hash, HashSize)<< endl;  
	  if (Mutations.count(HashToLongLocal(hash, HashSize )) > 0) {
            MutHashesFound++;
          }
        }
      }
      if (MutHashesFound > 0)
	{
		#pragma omp critical(MutWrite)
                {
                       MutOutFile << Buffer[BuffCount] << ":MH" << MutHashesFound << endl
                       << Buffer[BuffCount + 1] << endl
                       << Buffer[BuffCount + 2] << endl
                       << Buffer[BuffCount + 3] << endl;
                        found++;
                }
	}
    }
  }
  MutFile.close();
  MutOutFile.close();
  cout << "\nDone running RUFUS.Filter.cpp\n";
}
