#ifndef __SYNTAX_H__
#define __SYNTAX_H__

#include <string>
#include <vector>
#include <stack>
using namespace std;

#include "lex.h"
#include "poliz.h"

class Operand
{
	string _str;
	type_of_lex _type;
public:
	Operand(type_of_lex t, string s): _str(s), _type(t) {}
	string &str() {return _str; }
	type_of_lex type() {return _type; }
};

class Parser
{
	Lex curr_lex, last_lex, next_lex;
	bool isNext = false;
	string c_str;
	Scanner scan;
	stack<Operand> st;

	void program();
	void descriptions();
	void description();
	void variable(type_of_lex type);
	void constant(type_of_lex type);
	void operators();
	void operatr();
	void operator_expr();
	void expression();
	void E1();
	void E2();
	void E3();
	void E4();
	void E5();
	void E6();

	void gl();
	Lex next();
	bool eq(vector<string> s, string key = "");
	void assert(string l);
	int check_id();
	void check_not();
	void check_pp(Lex &op);
	void check_op();
	void eq_bool();
public:
	Parser(string path): scan(path) {}
	void analyze();
	Poliz prog;
};


#endif
