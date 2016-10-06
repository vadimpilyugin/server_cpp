#ifndef __POLIZ_H__
#define __POLIZ_H__

#include <vector>
#include <algorithm>
#include <iostream>
using namespace std;

#include "lex.h"

class Poliz
{
	vector<Lex> p;
public:
	void put_lex(Lex l) { p.push_back(l); }
	void put_lex(Lex l, int place) { p[place] = l; }
	void blank() { p.push_back(Lex(POLIZ_EMPTY, "Пустое поле")); }
	int get_free() { return p.size(); }
	Lex &operator[] (int index) { return p.at(index); }
	void print() { for(auto &it:p) { cout << it; }; }
	void change(int p1, int p2)
	{
		int tmp = p.size();
		p.resize( p.size() + p2 - p1);
		auto pos1 = p.begin();
		auto pos2 = p.begin();
		auto end = p.begin();
		pos1 += p1;
		pos2 += p2;
		end += tmp;
		copy (pos1, pos2, end);
		copy (pos2, end, pos1);
		copy (end, end + p2 - p1, end - p2 + p1);
		p.erase (end, end + p2 - p1);
	}
};


#endif
