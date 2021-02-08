#ifndef _ABM_SYSTEM_SAVER_HPP_
#define _ABM_SYSTEM_SAVER_HPP_

#include <fstream>
#include <string>

#include "SystemSaver.hpp"
#include "Fraction.hpp"

using namespace std;

class ABMSystemSaver {
private:
	int systemsCount;

	const string saveFileName = "saved_abm_systems.txt";

	inline void save(const string &strSystems);

public:
	ABMSystemSaver();

	void reset();

	void save_system(const int &varNumber, const QVector<int> &basisVars,
		const QVector<int> &nonBasisVars, const QVector<Fraction> &function,
		const QVector<QVector<Fraction>> &equalitiesMatrix);

	bool restore_system(int &varNumber, QVector<int> &basisVars, QVector<int> &nonBasisVars,
		QVector<Fraction> &function, QVector<QVector<Fraction>> &equalitiesMatrix);

};

#endif // _ABM_SYSTEM_SAVER_HPP_