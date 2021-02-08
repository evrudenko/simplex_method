#ifndef _CONTROLLER_HPP_
#define _CONTROLLER_HPP_

#include <QtWidgets>

#include "MainWindow.hpp"
#include "Optimizer.hpp"
#include "TableSaver.hpp"
#include "Fraction.hpp"

class Controller : public QObject {
	Q_OBJECT

private:
	int stepNumber;

	MainWindow *ui;
	Optimizer *optimizer;
	TableSaver *tableSaver;

public:
	Controller(MainWindow *window, Optimizer *pOptimizer, TableSaver *saver);	

public slots:
	void set_simplex_table();

	void __abm_canceled();

	void abm_set_simplex_table();

	void set_solution();

	void __run_abm_method(int vNumber, QVector<int>& bVars, QVector<int>& nbVars,
		QVector<Fraction>& func, QVector<QVector<Fraction>>& eMatrix);

	void __step_back();

	void __step_forward(int rowNumber, int colNumber, bool delBasisVar);

	void __abm_last_step(int rowNumber, int colNumber, bool delBasisVar);

};

#endif // _CONTROLLER_HPP_