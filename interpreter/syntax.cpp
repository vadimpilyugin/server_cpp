#include <string>
#include <iostream>
#include <stack>
using namespace std;

#include "table.h"
#include "excp.h"
#include "syntax.h"

void Parser::gl()
{
	if(isNext)
	{
		last_lex = curr_lex;
		curr_lex = next_lex;
		isNext = false;
	}
	else
	{
		last_lex = curr_lex;
		curr_lex = scan.get_lex();
		c_str = curr_lex.str();
	}
}
Lex Parser::next()
{
	isNext = true;
	return next_lex = scan.get_lex();
}
bool Parser::eq(vector<string> s, string key)
{
	if(key == "")
		key = c_str;
	for(auto &it:s)
		if(key == it)
			return true;
	return false;
}
void Parser::assert(string l)
{
	if(c_str != l)
		throw Excp( curr_lex, string("Ожидалась лексема \"") + l + "\"");
}
int Parser::check_id()
{
	int i = 0;
	for(auto &it:idents)
	{
		if(c_str == it.name())
			return i;
		i++;
	}
	throw Excp(curr_lex, "Неописанная переменная");
}
void Parser::check_not()
{
	Operand op = st.top();
	st.pop();
	if(op.str() == "bool")
		st.push( Operand (OP_EXPR, "bool") );
	else
		throw Excp(curr_lex, "Операция неприменима к данному типу");
}
void Parser::check_pp(Lex &op)
{
	if(st.top().str() == "int" && st.top().type() == LEX_VAR)
	{
		st.pop();
		st.push( Operand (OP_EXPR, "int") );
	}
	else
		throw Excp ( op, "Операция применима только к целочисленному типу" );
}
void Parser::check_op()
{
	if(st.empty())
		throw string("Стек операндов пуст");
	else
	{
		Operand op2 = st.top();
		st.pop();
		Operand operation = st.top();
		st.pop();
		Operand op1 = st.top();
		st.pop();
		if(operation.str() == "=")
		{
			if(op1.type() == LEX_VAR && op1.str() == op2.str())
				st.push( Operand (OP_EXPR, op1.str()) );
			else if(op1.type() != LEX_VAR)
				throw Excp(op1, "Операнд не является переменной");
			else
				throw Excp(op1, "Не совпадают типы операндов");
		}
		else if( eq ({"and", "or"}, operation.str()) )
		{
			if(op1.str() == op2.str() && op1.str() == "bool")
				st.push( Operand (OP_EXPR, "bool") );
			else if(op1.str() != op2.str())
				throw Excp(op1, "Не совпадают типы операндов");
			else
				throw Excp(op1, "Тип операнда не bool");
		}
		else if( eq ({">", "<", ">=", "<=", "==", "!="}, operation.str()) )
		{
			if(op1.str() == op2.str() && op1.str() != "bool")
				st.push( Operand (OP_EXPR, "bool") );
			else if(op1.str() != op2.str())
				throw Excp(op1, "Не совпадают типы операндов");
			else
				throw Excp(op1, "Операция неприменима к булевскому типу данных");
		}
		else if( eq ({"+", "-"}, operation.str()) )
		{
			if(op1.str() == op2.str() && op1.str() == "int")
				st.push( Operand (OP_EXPR, "int") );
			else if(op1.str() == op2.str() && op1.str() == "string" && operation.str() == "+")
				st.push( Operand (OP_EXPR, "string") );
			else if(op1.str() != op2.str())
				throw Excp(op1, "Не совпадают типы операндов");
			else
				throw Excp(op1, "Операция неприменима к этому типу данных");
		}
		else if( eq ({"*", "/"}, operation.str()) )
		{
			if(op1.str() == op2.str() && op1.str() == "int")
				st.push( Operand (OP_EXPR, "int") );
			else if(op1.str() != op2.str())
				throw Excp(op1, "Не совпадают типы операндов");
			else
				throw Excp(op1, "Операция неприменима к этому типу данных");
		}
		else
			throw Excp(operation, "Странная операция");
		prog.put_lex( Lex(operation.type(), operation.str()) );
	}

}
void Parser::eq_bool()
{
	if(st.top().str() != "bool")
		throw Excp(st.top(), "Ожидалось логическое выражение");
	else
		st.pop();
}

void Parser::analyze()
{
	gl();
	program();
	//prog.print();
}

void Parser::program()
{
	if(c_str == "program")
		gl();
	else
		throw Excp(curr_lex, "Ожидалось слово program");
	if(c_str == "{")
		gl();
	else
		throw Excp(curr_lex, "Ожидалась открывающая фигурная скобка");
	descriptions();
	operators();
	if(c_str == "}")
		gl();
	else
		throw Excp(curr_lex, "Ожидалась закрывающая фигурная скобка");
}

void Parser::descriptions()
{
	for(;;)
		if(eq({"int", "string", "bool"}))
		{
			description();
			if(c_str == ";")
				gl();
			else
				throw Excp(curr_lex, "После описания ожидалась точка с запятой");
		}
		else
			return;
}
void Parser::description()
{
	if(eq({"int", "string", "bool"}))
	{
		type_of_lex type;
		if(c_str == "int")
			type = INT_VAR;
		else if(c_str == "string")
			type = STR_VAR;
		else if(c_str == "bool")
			type = BOOL_VAR;
		gl();
		variable(type);
		while(c_str == ",")
		{
			gl();
			variable(type);
		}
	}
}

void Parser::variable(type_of_lex type)
{
	if(curr_lex.type() != LEX_IDENT)
		throw Excp(curr_lex, "Ожидалось имя переменной");
	if(look(curr_lex) == idents.size())
		idents.push_back( Variable (curr_lex, type) );
	else
		throw Excp(curr_lex, "Двойное определение");
	gl();
	if(c_str == "=")
	{
		gl();
		constant(type);
		gl();
	}
}

void Parser::constant(type_of_lex type)
{
	if(eq({"+", "-"}) || curr_lex.type() == LEX_NUM)
	{
		if(type != INT_VAR)
			throw Excp(curr_lex, "Целочисленную константу нельзя присвоить данному типу");
		bool neg = false;
		if(c_str == "+")
			gl();
		else if(c_str == "-")
		{
			neg = true;
			gl();
		}
		if(curr_lex.type() == LEX_NUM)
		{
			if(neg)
				idents.back().set_value(-curr_lex.num());
			else
				idents.back().set_value(curr_lex.num());
		}
		else
			throw Excp(curr_lex, "Ожидалась целочисленная константа");
	}
	else if(curr_lex.type() == LEX_STR)
	{
		if(type != STR_VAR)
			throw Excp(curr_lex, "Строковую константу нельзя присвоить данному типу");
		idents.back().set_value(c_str);
	}
	else if(curr_lex.type() == LEX_BOOL)
	{
		if(type != BOOL_VAR)
			throw Excp(curr_lex, "Булевскую константу нельзя присвоить данному типу");
		idents.back().set_value(curr_lex.num());
	}
	else
		throw Excp(curr_lex, "Не является константой");
}

void Parser::operators()
{
	while(c_str != "}")
		operatr();
}

void Parser::operatr()
{
	if(c_str == "if")
	{
		gl();
		assert("(");
		gl();
		expression();
		eq_bool();
		assert(")");
		gl();
		int p1 = prog.get_free();
		prog.blank();
		prog.put_lex( Lex (POLIZ_FGO) );
		operatr();
		if(c_str != "else")
		{
			prog.put_lex( Lex (POLIZ_LABEL, "", prog.get_free()) , p1);
			//operatr();
		}
		else
		{
			gl();
			int p2 = prog.get_free();
			prog.blank();
			prog.put_lex( Lex (POLIZ_GO) );
			prog.put_lex( Lex (POLIZ_LABEL, "", prog.get_free()) , p1);
			operatr();
			prog.put_lex( Lex (POLIZ_LABEL, "", prog.get_free()) , p2);
		}
	}
	else if(c_str == "for")
	{
		int p1, p2, p3, p4;
		gl();
		assert("(");
		gl();
		if(c_str != ";")
		{
			expression();
			assert(";");
		}
		gl();
		p1 = prog.get_free();
		if(c_str != ";")
		{
			expression();
			eq_bool();
			assert(";");
			gl();
		}
		else
		{
			prog.put_lex( Lex (LEX_BOOL, "true", 1) );
		}
		p2 = prog.get_free();
		prog.blank();
		prog.put_lex( Lex (POLIZ_FGO) );
		p3 = prog.get_free();
		if(c_str != ";")
			expression();
		assert(")");
		gl();
		p4 = prog.get_free();
		operatr();
		prog.change(p3, p4);
		prog.put_lex( Lex (POLIZ_LABEL, "", p1) );
		prog.put_lex( Lex (POLIZ_GO) );
		prog.put_lex( Lex (POLIZ_LABEL, "", prog.get_free()) , p2);
	}
	else if(c_str == "do")
	{
		gl();
		int p1 = prog.get_free();
		operatr();
		assert("while");
		gl();
		assert("(");
		expression();
		eq_bool();
		assert(")");
		gl();
		assert(";");
		gl();
		prog.put_lex( Lex (POLIZ_LABEL, "", p1) );
		prog.put_lex( Lex (POLIZ_TGO) );
	}
	else if(c_str == "while")
	{
		gl();
		assert("(");
		gl();
		int p1 = prog.get_free();
		expression();
		eq_bool();
		assert(")");
		gl();
		int p2 = prog.get_free();
		prog.blank();
		prog.put_lex( Lex (POLIZ_FGO) );
		operatr();
		//assert(";");
		prog.put_lex( Lex (POLIZ_LABEL, "", p1) );
		prog.put_lex( Lex (POLIZ_GO) );
		prog.put_lex( Lex (POLIZ_LABEL, "", prog.get_free()) , p2);
	}
	else if(c_str == "write")
	{
		Lex write = curr_lex;
		gl();
		assert("(");
		gl();
		expression();
		prog.put_lex(write);
		while(c_str == ",")
		{
			gl();
			expression();
			prog.put_lex(write);
		}
		assert(")");
		gl();
		assert(";");
		gl();
	}
	else if(c_str == "read")
	{
		Lex read = curr_lex;
		gl();
		assert("(");
		gl();
		prog.put_lex( Lex (LEX_VAR, c_str, check_id()) );
		prog.put_lex( read );
		gl();
		assert(")");
		gl();
		assert(";");
		gl();
	}
	else if(c_str == "{")
	{
		gl();
		operators();
		assert("}");
		gl();
	}
	else if(c_str == ";")
		return;
	else
		operator_expr();
}

void Parser::operator_expr()
{
	expression();
	assert(";");
	st.pop();
	gl();
}

void Parser::expression()
{
	E1();
	while(c_str == "=")
	{
		st.push( Operand (LEX_DELIM, "=") );
		gl();
		E1();
		check_op();
	}
}
void Parser::E1()
{
	E2();
	while(c_str == "or")
	{
		st.push( Operand (LEX_WORD, "or") );
		gl();
		E2();
		check_op();
	}
}
void Parser::E2()
{
	E3();
	while(c_str == "and")
	{
		st.push( Operand (LEX_WORD, "and") );
		gl();
		E3();
		check_op();
	}
}
void Parser::E3()
{
	E4();
	while(eq({">", "<", ">=", "<=", "==", "!="}))
	{
		st.push( Operand (LEX_DELIM, c_str) );
		gl();
		E4();
		check_op();
	}
}
void Parser::E4()
{
	E5();
	while(eq({"+", "-"}))
	{
		st.push( Operand (LEX_DELIM, c_str) );
		gl();
		E5();
		check_op();
	}
}
void Parser::E5()
{
	E6();
	while(eq({"*", "/"}))
	{
		st.push( Operand (LEX_DELIM, c_str) );
		gl();
		E6();
		check_op();
	}
}
void Parser::E6()
{
	if(curr_lex.type() == LEX_IDENT)
	{
		int i = check_id();
		st.push( Operand (LEX_VAR, idents[i].var_type()) );
		prog.put_lex( Lex (LEX_VAR, idents[i].var_type(), i) );
		gl();
		if(c_str == "++" || c_str == "--")
		{
			last_lex = Lex (LEX_VAR, "", i);
			E6();
		}
	}
	else if(curr_lex.type() == LEX_NUM)
	{
		st.push( Operand(LEX_NUM, "int") );
		prog.put_lex ( Lex (LEX_NUM, "", curr_lex.num()) );
		gl();
	}
	else if(curr_lex.type() == LEX_BOOL)
	{
		st.push( Operand(LEX_BOOL, "bool") );
		if(c_str == "true")
			prog.put_lex ( Lex(LEX_BOOL, "", 1) );
		else
			prog.put_lex ( Lex(LEX_BOOL, "", 0) );
		gl();
	}
	else if(curr_lex.type() == LEX_STR)
	{
		st.push( Operand(LEX_STR, "string") );
		prog.put_lex( Lex (LEX_STR, c_str) );
		gl();
	}
	else if(c_str == "not")
	{
		Lex tmp = curr_lex;
		gl();
		E6();
		prog.put_lex ( tmp );
		check_not();
	}
	else if(eq ({"++", "--"}) )
	{
		Lex oper = curr_lex;
		if(last_lex.type() == INT_VAR)  //постфиксная форма, если предыдущая лексема была переменной
		{
			check_pp(oper);
			prog.put_lex( Lex (LEX_DELIM, string("postfix") + oper.str()) );
			gl();
		}
		else
		{
			gl();
			E6();
			check_pp(oper);
			prog.put_lex( Lex (LEX_DELIM, string("prefix") + oper.str()) );
		}
	}
	else if(c_str == "(")
	{
		gl();
		expression();
		if(c_str == ")")
			gl();
		else
			throw Excp(curr_lex, "Ожидалась закрывающая скобка");
	}
	else
		throw Excp(curr_lex, "Ожидалась переменная, константа или выражение в скобках");
}/*
int main()
{
	Scanner sc("./test.cpp");
	while(!sc.feof())
		cout << sc.get_lex();
*/
/*
try{
	Parser p("./test1.cpp");
	p.analyze();
}
catch(Excp &a)
{
	cerr << "Синтаксическая ошибка: " << endl;
	cerr << a.get_lex();
	cerr << a.what() << endl;
}
}
*/
