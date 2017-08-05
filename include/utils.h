#pragma once

#include <iostream>
#include <cstdio>
#include <cerrno>
#include <assert.h>
using namespace std;
void outVector(vector<string> &a)
{
	cout << "Vector:" << endl;
	for(auto &it:a)
		cout << it << endl;
}
void outVector(char **a)
{
	int i = 0;
	while(a[i] != nullptr)
		cout << a[i++] << endl;
}

string strerr() {return strerror(errno);}
