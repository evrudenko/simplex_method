#ifndef _SYSTEM_SAVER_HPP_
#define _SYSTEM_SAVER_HPP_

#include <QtWidgets>

#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "Fraction.hpp"

using namespace std;

class SystemSaver {
private:
	friend class ABMSystemSaver;

	static QLinkedList<string> split_lines(string data);

	static bool __restore_system(const string &data, int &varNumber, QVector<int> &basisVars,
		QVector<Fraction> &function, QVector<QVector<Fraction>> &equalitiesMatrix);

	static bool __restore_system(const string &data, int &varNumber,
		QVector<int> &basisVars, QVector<int> &nonBasisVars, QVector<Fraction> &function,
		QVector<QVector<Fraction>> &equalitiesMatrix);

public:
	SystemSaver();

	static void save_system(const string &filename, const int &varNumber,
		const QVector<int> &basisVars, const QVector<Fraction> &function,
		const QVector<QVector<Fraction>> &equalitiesMatrix);

	static void save_system(const string &filename, const int &varNumber,
		const QVector<int> &basisVars, const QVector<int> &nonBasisVars,
		const QVector<Fraction> &function, const QVector<QVector<Fraction>> &equalitiesMatrix);

	static bool restore_system(const string &filename, int &varNumber, QVector<int> &basisVars,
		QVector<Fraction> &function, QVector<QVector<Fraction>> &equalitiesMatrix);

	static bool restore_system(const string &filename, int &varNumber,
		QVector<int> &basisVars, QVector<int> &nonBasisVars,
		QVector<Fraction> &function, QVector<QVector<Fraction>> &equalitiesMatrix);

};

#endif // _SYSTEM_SAVER_HPP_