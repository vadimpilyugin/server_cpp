#include <string>
#include <iostream>
using namespace std;

#include "excp.h"
#include "executer.h"
#include "syntax.h"
#include "table.h"

class Interpreter
{
	Parser p;
	Executer e;
public:
	Interpreter(string path): p(path) {}
	void interpretation();
};

void Interpreter::interpretation()
{
	p.analyze();
	e.execute(p.prog);
}

int main(int argc, char **argv, char **envp)
{/*
	for(int i = 0; i < argc; i++)
	{
		cout << "Argument #" << i << ": " << argv[i] << endl;
	}
	for(int i = 0; envp[i] != NULL; i++)
	{
		cout << "Environment #" << i << ": " << envp[i] << endl;
	}*/
	try
	{
		fill(envp);
		Interpreter I(argv[1]);
		//Interpreter I("./test.prog");
		I.interpretation();
		return 0;
	}
	catch(Excp &a)
	{
		cout << a.err_type() << " строка " << a.get_line() << endl;
		cout << a.get_lex();
		cout << "В чем ошибка: " << a.what() << endl;
		return 0;
	}
}
