#ifndef _OPTIMIZER_HPP_
#define _OPTIMIZER_HPP_

#include <QtWidgets>

#include <algorithm>

#include "Fraction.hpp"

class Optimizer {
private:
	QString info;
	bool setupSuccessFlag, shuffled, abmCompleted;
	QVector<int> basisVars, nonBasisVars;
	QVector<Fraction> function;
	QVector<QVector<Fraction>> simplexTable, equalitiesMatrix;

	void set_system_abm(QVector<QVector<Fraction>> &sTable,
		QVector<int> &bVars, QVector<int> &nbVars);

public:
	Optimizer();

	bool set_system(int varNumber, QVector<int> bVars, QVector<int> nbVars,
		QVector<Fraction> function, QVector<QVector<Fraction>> eMatrix);

	bool create_simplex_table(QVector<int> &bVars, QVector<Fraction> func,
		QVector<QVector<Fraction>> eMatrix, QVector<QVector<Fraction>> &sTable);

	void step_forward(int rowNumber, int colNumber, bool delBasisVar);

	void step_forward(QVector<QVector<Fraction>> &sTable, QVector<int> &bVars,
		QVector<int> &nbVars, int rowNumber, int colNumber, bool delBasisVar);

	void update_state(const QVector<QVector<Fraction>> &sTable, const QVector<int> &bVars,
		const QVector<int> &nbVars);

	void abm_solve_system();

	bool abm_solve_system(QVector<QVector<Fraction>> &sTable,
		QVector<int> &bVars, QVector<int> &nbVars);

	void solve_system();

	bool solution_found();

	bool solution_found(const QVector<QVector<Fraction>> &sTable);

	bool solution_exists();

	bool solution_exists(const QVector<QVector<Fraction>> &sTable);

	bool abm_avail_point_exists();

	QVector<QVector<Fraction>> get_simplex_table();

	bool system_shuffled();

	bool setup_completed();

	QVector<int> get_basis_vars();

	QVector<int> get_nonbasis_vars();

	QString get_info();

};

#endif // _OPTIMIZER_HPP_