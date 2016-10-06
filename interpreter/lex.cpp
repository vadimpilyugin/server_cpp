#include <string>
#include <algorithm>
#include <vector>
#include <iostream>
#include <fstream>
using namespace std;

#include "table.h"
#include "lex.h"
#include "excp.h"

string Lex::lex_type()
{
	switch(type())
	{
	case LEX_IDENT: return "Идентификатор"; break;
	case LEX_STR: return "Строковая константа"; break;
	case LEX_NUM: return "Целое число"; break;
	case LEX_DELIM: return "Разделитель"; break;
	case LEX_WORD: return "Служебное слово"; break;
	case LEX_BOOL: return "Булевская константа"; break;
	case LEX_EOF: return "Лексема конца файла"; break;
	case LEX_ERR: return "Ошибочная лексема"; break;
	//Лексический анализ
	case OP_EXPR: return string("Выражение типа ") + data; break;
	case LEX_VAR: return "Переменная"; break;
	case INT_VAR: return "Целочисленная переменная"; break;
	case STR_VAR: return "Строковая переменная"; break;
	case BOOL_VAR: return "Булевская переменная"; break;
	//case BOOL_VAR: return "Переменная " + idents[n]._name + "типа bool"; break;
	//Синтаксический анализ
	case POLIZ_EMPTY: return data; break;
	case POLIZ_FGO: return "Оператор перехода по лжи"; break;
	case POLIZ_TGO: return "Оператор перехода по истине"; break;
	case POLIZ_LABEL: return string("Ссылка на элемент номер ") + toString(n); break;
	case POLIZ_GO: return "Оператор безусловного перехода"; break;
	//Построение ПОЛИЗа
	case BOOL_EXPR: return string("Выражение типа bool"); break;
	case INT_EXPR: return string("Выражение типа int"); break;
	case STR_EXPR: return string("Выражение типа string"); break;
	//Интерпретация
	default: throw Excp (*this, "Неопознанная лексема"); break;
	}
}
string Lex::toString(int n)
{
	stringstream tmp;
	tmp << n;
	string s;
	tmp >> s;
	return s;
}
string Lex::lex_val()
{
	if(_type == LEX_DELIM || _type == LEX_WORD)
		return string("номер в таблице: ") + toString(n);
	else if(_type == LEX_NUM)
		return string("значение: ") + toString(n);
	else
		return "не имеет числового значения";
}

string &Lex::str()
{
	if(_type != LEX_VAR)
		return data;
	return idents[n].str();
}

int &Lex::num()
{
	if(_type != LEX_VAR)
		return n;
	return idents[n].num();
}

type_of_lex &Lex::type(bool forwarding)
{
	if(_type != LEX_VAR || !forwarding)
		return _type;
	return idents[n].type();
}

ostream &operator<< (ostream &out, Lex l)
{
	switch(l._type)
	{
	case LEX_VAR: out << "Лексема \"" << idents[l.n].name() << "\", " << "тип: " << l.lex_type() << endl; break;
	case LEX_NUM: out << "Лексема \"" << Lex::toString(l.num()) << "\", " << "тип: " << l.lex_type() << endl; break;
	case INT_EXPR: out << l.lex_type() << ", значение "<< l.num() << endl; break;
	case STR_EXPR: out << l.lex_type() << ", значение \""<< l.str() << "\"" << endl; break;
	case BOOL_EXPR: out << l.lex_type() << ", значение " << boolalpha << l.num() << noboolalpha << endl; break;
	case OP_EXPR: out << "Операнд: " << l.lex_type() << endl; break;
	case LEX_ERR: out << "Лексема \"" << l.str() << "\", " << "тип: " << l.lex_type() << endl; break;
	case POLIZ_LABEL: out << "Лексема \"" << "-> " << Lex::toString(l.num()) << "\", " << "тип: " << l.lex_type() << endl; break;
	case POLIZ_FGO: out << "Лексема \"" << "!F" << "\", " << "тип: " << l.lex_type() << endl; break;
	case POLIZ_TGO: out << "Лексема \"" << "!!F" << "\", " << "тип: " << l.lex_type() << endl; break;
	case POLIZ_GO: out << "Лексема \"" << "F" << "\", " << "тип: " << l.lex_type() << endl; break;
	default: out << "Лексема \"" << l.data << "\", " << "тип: " << l.lex_type() << endl; break;
	}
	return out;
}

void Scanner::gc()
{
	c = program.get();
	if(c == '\n')
		line ++;
}

char Scanner::next()
{
	return program.peek();
}

int Scanner::look(string key, vector<string> &words)
{
	int i, size = words.size();
	for(i = 0; i < size; i++)
		if(key == words[i])
			return i;
	return i;
}

int Scanner::line = 1;

Scanner::Scanner(string path)
{
	program.open(path.c_str());
	gc();
}
Lex Scanner::get_lex()
{
	unsigned int d, i;
	CS = H;
	do{
		switch(CS)
		{
		case H:
			if(isspace(c))
				gc();
			else if(isalpha(c))
			{
				id = string();
				id += c;
				gc();
				CS = IDENT;
			}
			else if(isdigit(c))
			{
				d = c - '0';
				id = string();
				id += c;
				gc();
				CS = NUMB;
			}
			else if(c == '>' || c == '<' || c == '=')
			{
				id = string();
				id += c;
				if(next() == '=')
				{
					gc();
					id += c;
				}
				gc();
				i = look(id, delims);
				return Lex (LEX_DELIM, id, i);
			}
			else if((c == '+' && next() == '+') || (c == '-' && next() == '-'))
			{
				id = string();
				id += c;
				gc();
				id += c;
				gc();
				i = look(id, delims);
				return Lex (LEX_DELIM, id, i);
			}
			else if(c == '!')
			{
				id = string();
				id += c;
				gc();
				CS = NOT;
			}
			else if(c == '/' && next() == '/')
				CS = COMM_EOL;
			else if(c == '/' && next() == '*')
			{
				gc();
				gc();
				CS = COMM;
			}
			else if(c == '"')
			{
				gc();
				id = string();
				CS = STR;
			}
			else if(c == '$')
				CS = ENV;
			else if(c == -1)
				return Lex(LEX_EOF, "---Конец программы---");
			else if(c == '#' && next() == '!')
				CS = COMM_EOL;
			else
				CS = DELIM;
			break;
		case IDENT:
			if(isalpha(c) || isdigit(c) || c == '_')
			{
				id += c;
				gc();
			}
			else
			{
				if((i = look(id, words)) != words.size())
				{
					if(id == "true")
						return Lex(LEX_BOOL, id, 1);
					else if(id == "false")
						return Lex(LEX_BOOL, id, 0);
					else
						return Lex(LEX_WORD, id, i);
				}
				else if((i = look(id, delims)) != delims.size())
					return Lex(LEX_DELIM, id, i);
				else
					return Lex(LEX_IDENT, id);
			}
			break;
		case NUMB:
			if(isdigit(c))
			{
				d = d * 10 + (c - '0');
				id += c;
				gc();
			}
			else
				return Lex(LEX_NUM, id, d);

			break;
		case NOT:
			if(c == '=')
			{
				id += c;
				gc();
				i = look(id, delims);
				return Lex(LEX_DELIM, id, i);
			}
			else
			{
				id += c;
				throw Excp(Lex(LEX_ERR, id), "После восклицательного знака ожидался символ '='", "Лексическая ошибка: ");
			}
			break;
		case DELIM:
			if((i = look(string(1, c), delims)) == delims.size())
				throw Excp(Lex(LEX_ERR, string(1, c)), "Некорректный символ", "Лексическая ошибка: ");
			else
			{
				gc();
				return Lex(LEX_DELIM, delims[i], i);
			}
			break;
		case STR:
			if(c == '\\')
			{
				gc();
				if(c == 'n')
					id += '\n';
				else
					id += c;
				gc();
			}
			else if(c != '"')
			{
				id += c;
				gc();
			}
			else
			{
				CS = H;
				gc();
				return Lex(LEX_STR, id);
			}
			break;
		case ENV:
			{
				id = string();
				gc();
				while(isalpha(c) || c == '_' || c == '-')
				{
					id += c;
					gc();
				}
				return Lex (LEX_STR, find_env(id));
			}
		case COMM:
			if(c == '*' && next() == '/')
			{
				gc();
				gc();
				CS = H;
			}
			else
				gc();
			break;
		case COMM_EOL:
			while(c != '\n')
				gc();
			CS = H;
			break;
		}
	} while (true);
}
