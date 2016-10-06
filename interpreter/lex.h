#ifndef __LEX_H_
#define __LEX_H_

#include <fstream>
#include <vector>
#include <string>
#include <sstream>
using namespace std;

enum type_of_lex
{
	LEX_EOF,
	LEX_ERR,
	LEX_STR,
	LEX_BOOL,
	LEX_NUM,
	LEX_IDENT,
	LEX_VAR,
	INT_VAR,
	STR_VAR,
	BOOL_VAR,
	LEX_DELIM,
	LEX_WORD,
	OP_EXPR,
	BOOL_EXPR,
	INT_EXPR,
	STR_EXPR,
	POLIZ_EMPTY,
	POLIZ_FGO,
	POLIZ_LABEL,
	POLIZ_GO,
	POLIZ_TGO
};


class Lex
{
	type_of_lex _type;
	string data;
	int n;
	string lex_type();
	static string toString(int n);
	string lex_val();
public:
	Lex(type_of_lex t = POLIZ_EMPTY, string d = "", int a = 0): _type(t), data(d), n(a) {}
	friend ostream &operator<< (ostream &out, Lex l);
	type_of_lex &type(bool forwarding = true);
	string &str();
	int &num();
	string content() {return data; }
	string var_type();
};


class Scanner
{
	char c;
	static int line;
	string id;
	enum state {H, IDENT, NUMB, NOT, DELIM, STR, COMM, COMM_EOL, ENV};
	state CS = H;
	ifstream program;

	void gc();

	char next();

	int look(string key, vector<string> &words);
public:
	Scanner(string path);
	Lex get_lex();
	static int get_line() {return line;}
};

#endif
