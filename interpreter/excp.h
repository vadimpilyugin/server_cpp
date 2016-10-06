#ifndef EXCP_H_INCLUDED
#define EXCP_H_INCLUDED

#include <string>
using namespace std;

#include "lex.h"
#include "syntax.h"

class Excp
{
	Lex _lex;
	string _comment;
	string _err_type;
	int _line;
public:
	Excp(Lex lex, const string &comment = "",  string err_type = "Синтаксическая ошибка: "): _lex(lex), _comment(comment), _err_type(err_type) {_line = Scanner::get_line(); };
	Excp(Operand op, const string &comment = "", string err_type = "Синтаксическая ошибка: "): _comment(comment), _err_type(err_type) { _lex = Lex (op.type(), op.str()); _line = Scanner::get_line(); }
	Excp(const Excp &a): _lex(a._lex), _comment(a._comment) {};
	Lex get_lex() const {return _lex;}
	const string &what() const {return _comment;}
	const string &err_type() const {return _err_type; }
	int get_line() {return _line; }
};

#endif // EXCP_H_INCLUDED
