#include "..\headers\Fraction.hpp"

int Fraction::gcd(int a, int b) { return b == 0 ? a : abs(gcd(b, a % b)); }

Fraction::Fraction(int n/* = 0*/, int d/* = 1*/) {
	if(d == 0)
		d = 1;

	int temp = abs(gcd(n, d));
	numerator = n / temp;
	denominator = d / temp;
}

int Fraction::num() const { return numerator; }

int Fraction::den() const { return denominator; }

Fraction Fraction::operator-() const { return Fraction(-numerator, denominator); }

Fraction &Fraction::operator+=(const Fraction &val) {
	numerator = numerator * val.den() + val.num() * denominator;
	denominator *= val.den();
	int temp = abs(gcd(numerator, denominator));
	numerator /= temp;
	denominator /= temp;
	return *this;
}

Fraction &Fraction::operator-=(const Fraction &val) {
	(*this) += -val;
	return *this;
}

Fraction &Fraction::operator*=(const Fraction &val) {
	int temp = abs(gcd(numerator * val.num(), denominator * val.den()));
	numerator = (numerator * val.num()) / temp;
	denominator = (denominator * val.den()) / temp;
	return *this;
}

Fraction &Fraction::operator/=(const Fraction &val) {
	if(val.num() < 0)
		(*this) *= Fraction(-val.den(), -val.num());
	else { (*this) *= Fraction(val.den(), val.num()); }
	return *this;
}

QString Fraction::toQString() const {
	return QString::fromStdString(to_string(*this));
}

/*static */Fraction Fraction::fromQString(QString str) {
	auto pos = str.indexOf("/");
	if(pos == -1)
		return Fraction(str.toInt());
	else if(pos == str.size() - 1) {
		str.remove(pos, 1);
		return Fraction(str.toInt());
	}
	else {
		auto numbers = str.split('/');
		return Fraction(numbers[0].toInt(), numbers[1].toInt());
	}
}

string to_string(const Fraction &fraction) {
	if(fraction.den() == 1)
		return to_string(fraction.num());
	return to_string(fraction.num()) + "/" + to_string(fraction.den());
}

istream &operator>>(istream &is, Fraction &fraction) {
	char ch;
	int n, d;

	is >> n >> noskipws >> ch >> skipws;

	if(ch == '/')
		is >> d;
	else
		d = 1;

	if(d != 0) {
		int temp = fraction.gcd(n, d);
		fraction.numerator = n / temp;
		fraction.denominator = d / temp;
	}

	return is;
}

ostream &operator<<(ostream &os, const Fraction &fraction) {
	os << to_string(fraction);
	return os;
}

bool operator<(const Fraction &l, const Fraction &r) {
	int ln = l.num() * r.den(), rn = r.num() * l.den();
	return ln < rn;
}

bool operator>(const Fraction &l, const Fraction &r) { return r < l; }

bool operator==(const Fraction &l, const Fraction &r) {
	return (l.num() == r.num() && (l.num() == 0 || l.den() == r.den()));
}

bool operator!=(const Fraction &l, const Fraction &r) { return !(l == r); }

bool operator<=(const Fraction &l, const Fraction &r) { return l < r || l == r; }

bool operator>=(const Fraction &l, const Fraction &r) { return l > r || l == r; }

Fraction operator+(Fraction l, const Fraction &r) { return l += r; }

Fraction operator-(Fraction l, const Fraction &r) { return l -= r; }

Fraction operator*(Fraction l, const Fraction &r) { return l *= r; }

Fraction operator/(Fraction l, const Fraction &r) { return l /= r; }