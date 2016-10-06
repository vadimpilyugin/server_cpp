#include <vector>
#include <string>
using namespace std;

#include "table.h"
#include "excp.h"

vector<string> words = {
	"program",
	"int",
	"string",
	"bool",
	"if",
	"else",
	"read",
	"write",
	"not",
	"and",
	"or",
	"true",
	"false",
	"while",
	"for"
};

vector<string> delims = {
	"=",
	",",
	";",
	"{",
	"}",
	"+",
	"-",
	"*",
	"/",
	"++",
	"--",
	">",
	"<",
	"(",
	")",
	">=",
	"<=",
	"==",
	"!=",
	"$",
};
vector<Variable> idents;
vector<vector<string>> env;

void fill(char **envp)
{
	int i = 0;
	vector<string> new_elem;
	new_elem.resize(2);
	if(!env.empty())
		throw "Заполняете непустой вектор!";
	while(envp[i] != NULL)
	{
		string s(envp[i]);
		string::size_type pos = s.find('=');
		new_elem[0] = s.substr(0, pos);
		new_elem[1] = s.substr(pos + 1);
		env.push_back(new_elem);
		i++;
	}
}

string find_env(string key)
{
	for(auto it:env)
		if(it[0] == key)
			return it[1];
	throw Excp ( Lex(LEX_ERR, string("$") + key), "Указанная переменная окружения не найдена", "Runtime error:");

}

unsigned int look(Lex var)
{
	int i, size = idents.size();
	for(i = 0; i < size; i++)
		if(var.str() == idents[i].name())
			return i;
	return i;
}

string Variable::var_type()
{
	switch(_type)
	{
	case INT_VAR: return "int"; break;
	case STR_VAR: return "string"; break;
	case BOOL_VAR: return "bool"; break;
	default: return "no type exists"; break;
	};
}
