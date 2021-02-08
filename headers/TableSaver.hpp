#ifndef _TABLE_SAVER_HPP_
#define _TABLE_SAVER_HPP_

#include <QtWidgets>

#include <fstream>
#include <sstream>
#include <string>

#include "Fraction.hpp"

using namespace std;

class TableSaver {
private:
	const string saveFileName = "saved_tables.txt";

	int tableCount;
	QString info;

	string pack_table(const QVector<QVector<Fraction>> &table,
		const QVector<int> &basisVars, const QVector<int> &nonBasisVars, int tableNumber);

	inline void save(const string &strTables);

public:
	TableSaver();

	void reset();
	
	void save(const QVector<QVector<Fraction>> &table, const QVector<int> &basisVars,
		const QVector<int> &nonBasisVars);

	bool restore(QVector<QVector<Fraction>> &table, QVector<int> &basisVars,
		QVector<int> &nonBasisVars, int tableNumber = -1);

};

#endif // _TABLE_SAVER_HPP_