#ifndef __TABLE_H__
#define __TABLE_H__

#include <vector>
#include <string>
using namespace std;

#include "lex.h"

class Variable
{
	string _name;
	type_of_lex _type;
	int value;
	string content;
public:
	Variable(Lex a, type_of_lex t): _name(a.str()), _type(t) {};
	void set_value(string t) {content = t;}
	void set_value(int a) {value = a;}
	string name() {return _name;}
	type_of_lex &type() {return _type;}
	string var_type();
	int &num () {return value; }
	string &str() {return content; }
};

unsigned int look(Lex var);
void fill(char **envp);
string find_env(string key);
extern vector<string> words, delims;
extern vector<vector<string>> env;
extern vector<Variable> idents;


#endif
