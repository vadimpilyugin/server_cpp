#ifndef __EXECUTER_H__
#define __EXECUTER_H__

#include <vector>
#include <string> 
#include <stack>
using namespace std;

#include "lex.h"

class Executer
{
	Lex curr_lex;
	string c_str;
	type_of_lex c_type;
	stack<Lex> st;
	bool eq(int key, vector<int> a);
	bool eq(vector<string> s, string key = "");
public:
	void execute(Poliz &prog);
};


#endif
