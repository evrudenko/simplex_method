#ifndef _FRACTION_HPP_
#define _FRACTION_HPP_

#include <QString>
#include <QStringList>

#include <iostream>
#include <string>
#include <cstdlib>

using namespace std;

class Fraction {
private:
	int numerator, denominator;

	int gcd(int a, int b);
public:
	Fraction(int n = 0, int d = 1);

	int num() const;
	int den() const;

	Fraction operator-() const;
	Fraction &operator+=(const Fraction &val);
	Fraction &operator-=(const Fraction &val);
	Fraction &operator*=(const Fraction &val);
	Fraction &operator/=(const Fraction &val);

	QString toQString() const;

	static Fraction fromQString(QString str);

	friend istream &operator>>(istream &is, Fraction &fraction);
	friend ostream &operator<<(ostream &os, const Fraction &fraction);
};

string to_string(const Fraction &fraction);
istream &operator>>(istream &is, Fraction &fraction);
ostream &operator<<(ostream &os, const Fraction &fraction);

bool operator<(const Fraction &l, const Fraction &r);
bool operator>(const Fraction &l, const Fraction &r);
bool operator==(const Fraction &l, const Fraction &r);
bool operator!=(const Fraction &l, const Fraction &r);
bool operator<=(const Fraction &l, const Fraction &r);
bool operator>=(const Fraction &l, const Fraction &r);

Fraction operator+(Fraction l, const Fraction &r);
Fraction operator-(Fraction l, const Fraction &r);
Fraction operator*(Fraction l, const Fraction &r);
Fraction operator/(Fraction l, const Fraction &r);

#endif // _FRACTION_HPP_