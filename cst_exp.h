#ifndef CST_EXP_H
#define CST_EXP_H

#include <iostream>
#include <exception>
using namespace std;

class cst_exp : public exception {
	const char* what() const throw() {
		return "construct failed";
	}
};
