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
   */
  void learn_file(string path){
    ifstream sample(path);
    string line;
    if(sample.is_open()){
      while(getline(sample, line)){
        if(line != "") learn_sentence(line);
      }
      sample.close();
    }else{
      LOG("Language >> learn_file(): error reading sample file.");
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
      LOG("word: " << word);
      rnd = (double) generator() / generator.max();
      word = generate_next(word, rnd);
      if(word == TK_END) break;
      sentence.append(word);
      sentence.append(" ");
      LOG("sentence: " << sentence);
    }
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
        LOG("Language >> Learn_sentence(): new word");
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
      LOG(r_count);
      int count = 0;
      list<Word>::iterator itr;
      for(itr = (*it).second.begin(); itr != (*it).second.end(); itr++){
        count += (*itr).get_count();
        LOG((*itr).get_word());
        if(r_count <= count) break;
      }

      return (*itr).get_word();
    }else{
      LOG("Language >> generate_next() error.");
      exit(1);
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

int main (){
  //DEBUG = true;
  Language penis = Language();
  penis.learn_file("filtered\\sia.txt");
  cout << penis.show_dictionary();
  for(int i = 0; i < 20; i++){
    cout << penis.generate_sentence();
    Sleep(100);
  }
}
