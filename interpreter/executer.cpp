#include <algorithm>
#include <ctype.h>
using namespace std;

#include "syntax.h"
#include "table.h"
#include "excp.h"
#include "executer.h"

bool Executer::eq(int key, vector<int> a)
{
	for(auto it:a)
		if(key == it)
			return true;
	return false;
}

bool Executer::eq(vector<string> s, string key)
{
	if(key == "")
		key = c_str;
	for(auto &it:s)
		if(key == it)
			return true;
	return false;
}

void Executer::execute(Poliz &prog)
{
	int size = prog.get_free();
	for(int index = 0; index < size; )
	{
		curr_lex = prog[index];
		c_str = curr_lex.str();
		c_type = curr_lex.type(false);
		if(eq(c_type, {LEX_BOOL, LEX_NUM, LEX_STR, LEX_VAR, POLIZ_LABEL}))
		{
			st.push( curr_lex );
			index ++;
			//continue;
		}
		else if(c_str == "not")
		{
			st.top().num() = !st.top().num();
			st.top().type() = BOOL_EXPR;
			st.top().str() = "";
			/*
			if(st.top().type() == LEX_VAR)
			{
				bool tmp = !var().flag();
				st.pop();
				st.push( Lex (BOOL_EXPR, "", 0, tmp) );
			}
			else
				st.top().flag() = !st.top().flag();
			//continue;*/
			index ++;
		}
		else if(c_str == "and" || c_str == "or")
		{
			int tmp1, tmp2;
			tmp1 = st.top().num();
			st.pop();
			tmp2 = st.top().num();
			st.pop();
			if(c_str == "and")
				st.push( Lex (BOOL_EXPR, "", tmp1 && tmp2) );
			else
				st.push( Lex (BOOL_EXPR, "", tmp1 || tmp2) );
			/*
			if(st.top().type() == LEX_VAR)
				tmp1 = var().flag();
			else
				tmp1 = st.top().flag();
			st.pop();
			if(st.top().type() == LEX_VAR)
				tmp2 = var().flag();
			else
				tmp2 = st.top().flag();
			st.pop();
			if(c_str == "and")
				st.push( Lex (BOOL_EXPR, "", 0, tmp1 && tmp2) );
			else
				st.push( Lex (BOOL_EXPR, "", 0, tmp1 || tmp2) );
			//continue;*/
			index ++;
		}
		else if(c_type == POLIZ_GO)
		{
			index = st.top().num();
			st.pop();
		}
		else if(c_type == POLIZ_FGO || c_type == POLIZ_TGO)
		{
			int label = st.top().num();
			st.pop();
			int flag = st.top().num();
			st.pop();
			if(c_type == POLIZ_FGO)
				if(!flag)
					index = label;
				else
					index ++;
			else
				if(flag)
					index = label;
				else
					index ++;
			/*

			bool flag;
			st.pop();
			if(st.top().type() == LEX_VAR)
				flag = var().flag();
			else
				flag = st.top().flag();
			st.pop();
			if(c_type == POLIZ_FGO)
				if(!flag)
					index = label.num();
				else
					index ++;
			else
				if(flag)
					index = label.num();
				else
					index ++;*/
		}
		else if(c_str == "write")
		{
			Lex expr = st.top();
			st.pop();
			if(eq (expr.type(), {LEX_NUM, INT_EXPR, INT_VAR}) )
				cout << expr.num();
			else if(eq (expr.type(), {LEX_STR, STR_EXPR, STR_VAR}) )
				cout << expr.str();
			else
			{
				if(expr.num())
					cout << "True";
				else
					cout << "False";
			}
			index ++;
			/*
			if(st.top().type() == LEX_VAR)
			{
				if(var().type() == "int")
					cout << var().num();
				else if(var().type() == "string")
					cout << var().str();
				else if(var().type() == "bool")
				{
					if(var().flag())
						cout << "True";
					else
						cout << "False";
				}
			}
			else
			{
				if(st.top().type() == LEX_NUM || st.top().type() == INT_EXPR)
					cout << st.top().num();
				else if(st.top().type() == LEX_STR || st.top().type() == STR_EXPR)
					cout << var().str();
				else if(st.top().type() == LEX_BOOL || st.top().type() == BOOL_EXPR)
				{
					if(st.top().flag())
						cout << "True";
					else
						cout << "False";
				}
			}
			st.pop();
			index ++;*/
		}
		else if(c_str == "read")
		{
			Lex op = st.top();
			st.pop();
			if(op.type() == INT_VAR)
				cin >> op.num();
			else if(op.type() == STR_VAR)
				cin >> op.str();
			else
			{
				string flag;
				cin >> flag;
				transform(flag.begin(), flag.end(), flag.begin(), ::tolower);
				if(flag == "true" || flag == "1")
					op.num() = 1;
				else if(flag == "false" || flag == "0")
					op.num() = 0;
				else
					throw Excp( Lex (LEX_ERR, flag), "Не удалось ввести булевское значение" );
			}
			index ++;
		}
		else if(c_str == "prefix++" || c_str == "prefix--")
		{
			Lex op = st.top();
			st.pop();
			if(c_str == "prefix++")
				st.push( Lex (INT_EXPR, "", ++ op.num()) );
			else
				st.push( Lex (INT_EXPR, "", -- op.num()) );
			index ++;
		}
		else if(c_str == "postfix++" || c_str == "postfix--")
		{
			Lex op = st.top();
			st.pop();
			if(c_str == "postfix++")
				st.push( Lex (INT_EXPR, "", op.num() ++) );
			else
				st.push( Lex (INT_EXPR, "", op.num() --) );
			index ++;
		}
		else if(c_str == "+" || c_str == "*" || c_str == "/" || c_str == "-")
		{
			Lex op1, op2;
			op2 = st.top();
			st.pop();
			op1 = st.top();
			st.pop();
			if(eq (op1.type(), {LEX_NUM, INT_EXPR, INT_VAR}) )
			{
				int res = op1.num();
				if(c_str == "+")
					res += op2.num();
				else if(c_str == "-")
					res -= op2.num();
				else if(c_str == "*")
					res *= op2.num();
				else if(c_str == "/")
					res /= op2.num();
				st.push ( Lex (INT_EXPR, "", res) );
			}
			else if(eq (op1.type(), {LEX_STR, STR_EXPR, STR_VAR}) )
			{
				string res = op1.str();
				res += op2.str();
				st.push( Lex (STR_EXPR, res) );
			}
			index ++;
		}
		else if(c_str == "=")
		{
			Lex expr = st.top();
			st.pop();
			Lex var = st.top();
			st.pop();
			if(var.type() == INT_VAR)
			{
				var.num() = expr.num();
				st.push (Lex (INT_EXPR, "", var.num()) );
			}
			else if(var.type() == STR_VAR)
			{
				var.str() = expr.str();
				st.push (Lex (STR_EXPR, var.str()) );
			}
			else if(var.type() == BOOL_VAR)
			{
				var.num() = expr.num();
				st.push (Lex (BOOL_EXPR, "", var.num()) );
			}
			index ++;
		}
		else if(eq ({">", "<", ">=", "<=", "==", "!="}) )
		{
			Lex op2 = st.top();
			st.pop();
			Lex op1 = st.top();
			st.pop();
			if( eq (op1.type(), {INT_EXPR, INT_VAR, LEX_NUM}) )
			{
				if(c_str == ">")
					st.push( Lex (BOOL_EXPR, "", op1.num() > op2.num()) );
				else if(c_str == "<")
					st.push( Lex (BOOL_EXPR, "", op1.num() < op2.num()) );
				else if(c_str == ">=")
					st.push( Lex (BOOL_EXPR, "", op1.num() >= op2.num()) );
				else if(c_str == "<=")
					st.push( Lex (BOOL_EXPR, "", op1.num() <= op2.num()) );
				else if(c_str == "==")
					st.push( Lex (BOOL_EXPR, "", op1.num() == op2.num()) );
				else if(c_str == "!=")
					st.push( Lex (BOOL_EXPR, "", op1.num() != op2.num()) );
			}
			else
			{
				if(c_str == ">")
					st.push( Lex (BOOL_EXPR, "", op1.str() > op2.str()) );
				else if(c_str == "<")
					st.push( Lex (BOOL_EXPR, "", op1.str() < op2.str()) );
				else if(c_str == ">=")
					st.push( Lex (BOOL_EXPR, "", op1.str() >= op2.str()) );
				else if(c_str == "<=")
					st.push( Lex (BOOL_EXPR, "", op1.str() <= op2.str()) );
				else if(c_str == "==")
					st.push( Lex (BOOL_EXPR, "", op1.str() == op2.str()) );
				else if(c_str == "!=")
					st.push( Lex (BOOL_EXPR, "", op1.str() != op2.str()) );
			}
			index ++;
		}
		else
			throw Excp ( curr_lex, "POLIZ: unexpected element", "Ошибка выполнения: ");
	}
}
