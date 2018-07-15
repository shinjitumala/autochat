/**
 * autochat.cpp
 *
 * ---------------
 *
 *
 */
#include "Auto_Type_Library.cpp"

#include <fstream>
#include <string>
#include <iostream>
#include <iomanip>
#include <vector>
#include <sstream>
#include <map>
#include <list>

#include <chrono>
#include <random>

using namespace std;

#define TK_START "START_OF_SENTENCE_UD8a6TXfemyfJItNEJR7"
#define TK_END "END_OF_SENTENCE_NKlykNp6QsY4u3XF2V2R"

string SETTINGS_PATH = "settings.txt";
int UPDATE_INTERVAL = 5000;
int DELAY = 50;
int MAX_WINDOWS = 16;

/**
 * A class used in Language. Used to count how many times a word was used after a word.
 */
class Word{
  string word; // a word in a language
  int count; // the count a word is used after a word in a language

public:
  /**
   * Constuctor
   * string s: a word in a language
   */
  Word(string s){
    word = s;
    count = 1;
  }

  Word(){
    word = "dummy";
    count = 0;
  }

  Word(string s, int i){
    word = s;
    count = i;
  }

  /** increases the word count by 1 */
  void add(){
    count++;
  }

  /** get word*/
  string get_word(){
    return word;
  }

  /** get count */
  int get_count(){
    return count;
  }
};


/**
 * Language class is responsible for learning a language from file and generating text.
 * can also input/output a known language to a file for the archives.
 * uses markov's chain in order to learn languages.
 */
class Language{
  map<string, list<Word>> dictionary; // the dictionary.

public:
  /** Constructor */
  Language(){
    list<Word> t_list;
    dictionary[TK_START] = t_list;
  }

  /**
   * Learns a language from a sample text file.
   * string path: path to the sample text file.
   * bool dictionary: will read learning file as a dictionary file if true.
   */
  void learn_file(string path, bool is_dictionary){
    ifstream sample(path);
    string line;
    if(sample.is_open()){
      while(getline(sample, line)){
        if(!is_dictionary){
          if(line != "") learn_sentence(line);
        }else{
          if(line != "") read_dictionary_line(line);
        }
      }
      sample.close();
    }else{
      LOG("Language >> learn_file(): error reading sample file. Closing program...");
      Sleep(5000);
      exit(1);
    }
  }

  /**
   * Will generate a single sentence from the current dictionary.
   * string return: the generated sentence.
   */
  string generate_sentence(){
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::mt19937 generator(seed);
    string word = TK_START;
    string sentence;
    double rnd;
    while(true){
      rnd = (double) generator() / generator.max();
      word = generate_next(word, rnd);
      if(word == TK_END) break;
      sentence.append(word);
      sentence.append(" ");
    }
    LOG("Language >> generate_sentence(): " << sentence);
    sentence.erase(sentence.end(), sentence.end());
    sentence.append("\n");
    return sentence;
  }

  /**
   * will return the dictionary's data as a string.
   * string return: data of the dictionary.
   */
  string show_dictionary(){
    string ret;
    for(auto it = dictionary.begin(); it != dictionary.end(); it++){
      ret.append((*it).first);
      ret.append(":");
      for(auto itr = (*it).second.begin(); itr != (*it).second.end(); itr++){
        ret.append(" ");
        ret.append((*itr).get_word());
        ret.append(" ");
        ret.append(to_string((*itr).get_count()));

      }
      ret.append("\n");
    }

    return ret;
  }

private:
  /**
   * Adds a word into a list.
   * list<Word> *list: the list to add the word.
   * string next_token: the word to be added. this word should be the next token that comes after the owner of the list.
   */
  void list_add_word(list<Word> *list, string next_token){
    for(auto itr = (*list).begin(); true ; itr++){

      if(itr == (*list).end()){ // if the word in a list is NOT found, will add a new Word.
        (*list).push_back(Word(next_token));
        //LOG("Language >> Learn_sentence(): new word");
        break;

      }else if((*itr).get_word() == next_token){ // if the word in a list is found, increment it's count.
        (*itr).add();
        LOG("Language >> Learn_sentence(): add word");
        break;
      }
    }
  }
  /** will also add the count when adding the word */
  void list_add_word(list<Word> *list, string next_token, int count){
    for(auto itr = (*list).begin(); true ; itr++){

      if(itr == (*list).end()){ // if the word in a list is NOT found, will add a new Word.
        (*list).push_back(Word(next_token, count));
        //LOG("Language >> Learn_sentence(): new word");
        break;

      }else if((*itr).get_word() == next_token){ // if the word in a list is found, increment it's count.
        (*itr).add();
        LOG("Language >> Learn_sentence(): add word");
        break;
      }
    }
  }

  /**
   * Finds a word and returns the word cout of that word in a list.
   * list<Word> *list: the list to find the word.
   * string token: the search word.
   * int return: will return -1 if not found. otherwise will return the word count.
   */
  int list_find_word(list<Word> *list, string token){
    for(auto itr = (*list).begin(); true ; itr++){
      if(itr == (*list).end()){ // if the word in a list is NOT found, returns -1.
        return -1;
      }else if((*itr).get_word() == token){ // if the word in a list is found, return the string.
        return (*itr).get_count();
      }
    }
  }

  /**
   * Returns the total sum of word counts in this list.
   * list<Word> *list: the list to get the total word count sum.
   * int return: will return the sum of word counts.
   */
  int list_total_count(list<Word> *list){
    int sum = 0;
    for(auto it = (*list).begin(); it != (*list).end(); it++){
      sum += (*it).get_count();
    }
    return sum;
  }

  /**
   * Will generate a single sentence using the current dictionary.
   * string return: the generated sentence.
   */
  string generate_next(string token, double rnd){
    auto it = dictionary.find(token);
    if(it != dictionary.end()){
      int total_count = list_total_count(&((*it).second));
      int r_count = round(total_count * rnd);
      int count = 0;
      list<Word>::iterator itr;
      for(itr = (*it).second.begin(); itr != (*it).second.end(); itr++){
        count += (*itr).get_count();
        if(r_count <= count) break;
      }

      return (*itr).get_word();
    }else{
      LOG("Language >> generate_next() error.");
      exit(1);
    }
  }

  /**
   * Reads one line from a dictionary file.
   * string s: a line.
   */
  void read_dictionary_line(string s){
    stringstream ss(s);
    string token, f_token;
    list<string> tokens;
    int count;
    map<string, list<Word>>::iterator it;

    while(getline(ss, token, ' ')){ // divide string by space.
      tokens.push_back(token);
    }

    f_token = tokens.front();
    tokens.pop_front();
    f_token.erase(f_token.end() - 1);

    list<Word> t_list;
    dictionary[f_token] = t_list;

    while(!tokens.empty()){
      token = tokens.front();
      tokens.pop_front();
      count = atoi(tokens.front().c_str());
      tokens.pop_front();
      it = dictionary.find(f_token);
      list_add_word(&((*it).second), token, count);
    }
  }

  /**
   * Learns a single sentence
   * string s: the sentence to be learned.
   */
  void learn_sentence(string s){
    stringstream ss(s);
    string token;
    list<string> tokens;
    map<string, list<Word>>::iterator it;

    while(getline(ss, token, ' ')){ // divide string by space.
      tokens.push_back(token);
    }

    it = dictionary.find(TK_START);
    list_add_word(&((*it).second), tokens.front());

    while(!tokens.empty()){
      token = tokens.front();
      tokens.pop_front();

      string next_token;
      if(!tokens.empty()){
        next_token = tokens.front();
      }else{
        next_token = TK_END;
      }

      LOG("Language >> Learn_sentence(): " << token << " -> " << next_token);
      it = dictionary.find(token);
      if(it != dictionary.end()){ // if the list is found, proceed to add the word to the list.
        list_add_word(&((*it).second), next_token);

      }else{ // if the list is NOT found, create a new list and add to map.
        LOG("Language >> Learn_sentence(): new list");
        list<Word> t_list;
        t_list.push_back(Word(next_token));
        dictionary[token] = t_list;
      }
    }
  }
};

/**
 * Acts as the central control for all update operations.
 */
class Timer{
  time_t now_c;
  int wait_time; // the time to wait between updates. in milliseconds. recommended to set it above 60000 (60 sec).

public:
  /**
   * Constructor
   */
  Timer(int wait){
    update();
    wait_time = wait;
  }

  /**
   * Get the tm for the current time.
   * tm return value: the tm of the current time.
   */
  tm *getTM(){
    update();
    return localtime(&now_c);
  }

  /**
   * Waits until the next update cycle.
   */
  void waitNext(){
    Sleep(wait_time);
  }
private:
  /**
   * Updates the time with the current time.
   */
  void update(){
    now_c = chrono::system_clock::to_time_t(chrono::system_clock::now());
  }
};

/*
 * Will update when a tm is sent to it then if it is an appropriate time to send
 * a message then it will queue a message using a MessageSender.
 */
class WordHandler{
  Schedule *m_schedule; // message sending schedule.
  MessageSender *messanger; // messanger for the window.
  int interval_min; // minimum message interval. in milliseconds. 300000 for 5 mins.
  int interval_max; // maximum message interval. in milliseconds. 300000 for 5 mins.
  int counter; // used to count how much time is left for the next message.
  int next_counter; // the randomly generated interval.
  tm previous; // used to store the tm when the previous call happened.
  std::mt19937 generator;


public:
  /**
   * Constructor
   */
  WordHandler(Schedule *t_schedule, MessageSender *ms_tmp, int t_interval1, int t_interval2){
    m_schedule = t_schedule;
    messanger = ms_tmp;
    interval_min = t_interval1;
    LOG("WordHandler >> min: " << interval_min);
    interval_max = t_interval2;
    LOG("WordHandler >> max: " << interval_max);
    counter = 999999999;
    time_t now_c = chrono::system_clock::to_time_t(chrono::system_clock::now());
    previous = *localtime(&now_c);
    SetNextCounter();
  }

  WordHandler(){}

  /**
   * Takes tm of the current time. Then queues a message to the messageSender() if current time in schedule is true.
   * tm *time_tm: the tm for the current time. Can be taken from Timer().
   * string s: the messange to be queued.
   */
  void update(tm *time_tm, string s){
    LOG("WordHandler >> update(), id: " << this);
    if((*m_schedule).getSchedule(time_tm->tm_wday, 4 * time_tm->tm_hour + time_tm->tm_min / 15)){
      if(CheckInterval(time_tm)){
        (*messanger).queueMessage(s);
      }
    }else{
      counter = 99999999; // resets timer when schedule is over.
    }

    previous = *time_tm;
  }

private:
  bool CheckInterval(tm *time_tm){
    int d_day = ((time_tm->tm_wday - (&previous)->tm_wday) % 7 + 7) % 7;
    int d_hour = ((time_tm->tm_hour - (&previous)->tm_hour) % 24 + 24) % 24;
    int d_min = ((time_tm->tm_min - (&previous)->tm_min) % 60 + 60) % 60;
    int d_sec = ((time_tm->tm_sec - (&previous)->tm_sec) % 60 + 60 % 60);

    counter += d_day * 86400000 + d_hour * 3600000 + d_min * 60000 + d_sec * 1000;
    LOG("WordHandler >> counter: " << counter<< ", id: " << this);

    if(counter > next_counter){
      counter = 0;
      SetNextCounter();
      return true;
    }

    return false;
  }

  void SetNextCounter(){
    next_counter = interval_min + generator() % (interval_max - interval_min);
    LOG("WordHandler >> SetNextCounter(): " << next_counter);
  }
};

/*
 * Will open settings.txt to interpret it and set up a MessageSender and Language
 * After setting them up, will create a Timer and begin the operation.
 */
class initializer{
  Language language;
  WordHandler wh;
  bool dump = false;
  bool is_dictionary = false;
  Schedule schedule;
  MessageSender *ms;

public:
  initializer(){
    language = Language();
  }

  /**
   * begin operation.
   * bool dump: will dump the dictionary file if true.
   * string dump_path: dump dictionary to this path.
   * bool dictionary: will read learning file as a dictionary file if true.
   */
  void begin(bool dump, string dump_path, bool dictionary){
    is_dictionary = dictionary;
    cout << "Building dictionary. Please wait..." << endl;
    openSettings(SETTINGS_PATH);
    if(dump) dump_dictionary(dump_path);
    cout << "Done. Starting operation." << endl;
    Timer timer_clock = Timer(UPDATE_INTERVAL);
    while(true){
      timer_clock.waitNext();
      string msg = language.generate_sentence();
      msg.append("\n");
      wh.update(timer_clock.getTM(), msg);
      (*ms).sendToWindow();
    }
  }

private:
  /**
   * Will create a dictionary file at the given file path.
   * string path: file path.
   */
  void dump_dictionary(string path){
    ofstream file(path);
    if(file.is_open()){
      file << language.show_dictionary();
      file.close();
    }else{
      LOG("Language >> dump_dictionary(): Error creating output file. File was not created.");
      return;
    }
  }

  void getGenerator(string line, Schedule *s){
    stringstream ss(line);
    string dummy, sample_path, window;
    int min, max;
    ss >> dummy >> sample_path >> window >> min >> max;
    language.learn_file(sample_path, is_dictionary);
    ms = new MessageSender(DELAY, MAX_WINDOWS, window);
    wh  = WordHandler(s, ms, min, max);
  }
  /*
   * Opens file to interpret it as a setting file for this program.
   * string path: settings file path.
   */
  void openSettings(string path){
    string line;
    ifstream settings (path);
    if(settings.is_open()){
      Schedule *current_schedule = NULL;
      int current_wday = -1;
      while(getline(settings, line)){
        if(!(line[0] == '/' && line[1] == '/') || line[0] == '\n'){// skips line if it starts with "//" or "\n".
          switch(line[0]){
            case '>':
              current_schedule = new Schedule();
              getGenerator(line, current_schedule);
              break;
            case '+':
              current_wday = getDay(line);
              break;
            case '=':
              setTime(line, current_wday, current_schedule);
              break;
            case 'G':
              setGlobal(line);
              break;
            default:
              LOG("initializer >> unknown line. Skipping...");
              break;
          }
        }
      }
      settings.close();
    }else{
      LOG("initializer() >> unable to open settings file. Closing program...");
      Sleep(5000);
      exit(1);
    }
  }

  /**
   * Reads the given line to change the global variables.
   * string line: given line.
   */
  void setGlobal(string line){
    stringstream ss(line);
    char dummy;
    string str;
    int arg;
    ss >> str >> dummy >> arg;
    if(str == "G_InputDelay"){
      DELAY = arg;
    }else if (str == "G_MaxWindows"){
      MAX_WINDOWS = arg;
    }else if(str == "G_UpdateInterval"){
      UPDATE_INTERVAL = arg;
    }
  }

  /**
   * Reads the given line to modify the given schedule accordingly.
   * string line: given line.
   * Schedule *schedule: the schedule to be modified.
   */
  void setTime(string line, int wday, Schedule *schedule){
    if(wday == -1 || schedule == NULL) error();
    stringstream ss(line);
    int s_h, s_m, e_h, e_m;
    char dummy;
    ss >> dummy >> s_h >> dummy >> s_m >> dummy >> e_h >> dummy >> e_m;
    s_h = s_h % 24; e_h = e_h % 24; s_m = s_m / 15 * 15; e_m = e_m / 15 * 15;

    int start, end;
    start = s_h * 4 + s_m / 15; end = e_h * 4 + e_m / 15;
    for(int i = start; i < end; i++){
      (*schedule).setSchedule(wday, i, true);
    }
  }

  /**
   * Reads the given line to return the tm_wday vaule of the line.
   * string line: given line.
   * int return value: corresponding tm_wday value.
   */
  int getDay(string line){
    if(line == "+ Su"){
      return 0;
    }else if(line == "+ Mo"){
      return 1;
    }else if(line == "+ Tu"){
      return 2;
    }else if(line == "+ We"){
      return 3;
    }else if(line == "+ Th"){
      return 4;
    }else if(line == "+ Fr"){
      return 5;
    }else if(line == "+ Sa"){
      return 6;
    }else{
      LOG("initializer >> error at line: \"" << line << "\". Closing program...");
      Sleep(5000);
      exit(1);
      return -1;
    }
  }

  /**
   * Call this to exit the program when a error occurs while reading settings.txt.
   */
  void error(){
    LOG("initializer >> fatal error. Closing program...");
    Sleep(5000);
    exit(1);
  }
};

int main (int argc, char *argv[]){
  cout << "autochat.exe is running..." << endl;
  cout << "closing this during a alt tab operation is dangerous." << endl;
  cout << "otherwise feel free to close this window or press \"Ctrl + C\" to stop anytime." << endl;

  initializer ass = initializer();

  string dump_path;
  bool dump, is_dictionary;
  if(argc > 1){
    for(int i = 1; i < argc; i++){
      if(strcmp(argv[i], "-d") == 0){
        DEBUG = true;
        LOG("STARTED ON DEBUG MODE...");
      }else if(strcmp(argv[i], "-o") == 0){
        cout << "Creating dictionary file in dictionary\\" << argv[i + 1] << endl;
        dump_path = "dictionary\\";
        dump_path.append(string(argv[i + 1]));
        dump = true;
        i++;
      }else if(strcmp(argv[i], "--dictionary") == 0){
        cout << "Sample file will be read as a dictionary file." << endl;
        is_dictionary = true;
      }else if(i == argc - 1){
        SETTINGS_PATH = argv[i];
        cout << "Settings file path set to: " << SETTINGS_PATH << endl;
      }else{
        cout << "Usage: autochat.exe [options] \"path to specific settings file. will read settings.txt if not set\"" << endl;
        cout << "Options:" << endl;
        cout << "   -d : debugmode on." << endl;
        cout << "   -o \"filepath\" : dump dictionary file to path." << endl;
        cout << "   --dictionary : will read sample file as a dictionary file." << endl;
        return 0;
      }
    }
  }

  ass.begin(dump, dump_path, is_dictionary);

  return 0;
}
