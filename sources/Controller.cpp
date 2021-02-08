#include "..\headers\Controller.hpp"

Controller::Controller(MainWindow *window, Optimizer *pOptimizer, TableSaver *saver) :
	ui(window), optimizer(pOptimizer), tableSaver(saver)
{
	connect(ui, SIGNAL(simplex_table_request()), SLOT(set_simplex_table()));
	connect(ui, SIGNAL(abm_simplex_table_request()), SLOT(abm_set_simplex_table()));
	connect(ui, SIGNAL(step_back()), SLOT(__step_back()));
	connect(ui, SIGNAL(step_forward(int, int, bool)), SLOT(__step_forward(int, int, bool)));
	connect(ui, SIGNAL(abm_completed()), SLOT(set_simplex_table()));
	connect(ui, SIGNAL(run_abm_method(int, QVector<int>&, QVector<int>&, QVector<Fraction>&,
		QVector<QVector<Fraction>>&)), SLOT(__run_abm_method(int, QVector<int>&,
		QVector<int>&, QVector<Fraction>&, QVector<QVector<Fraction>>&)));
	connect(ui, SIGNAL(abm_last_step(int, int, bool)), SLOT(__abm_last_step(int, int, bool)));
	connect(ui, SIGNAL(solution_request()), SLOT(set_solution()));
	connect(ui, SIGNAL(abm_canceled()), SLOT(__abm_canceled()));
}

void Controller::set_simplex_table() {
	int varNumber;
	QVector<int> basisVars, nonBasisVars, newBasisVars;
	QVector<Fraction> function;
	QVector<QVector<Fraction>> sTable, equalitiesMatrix;

	stepNumber = -1;
	tableSaver->reset();

	varNumber = ui->get_var_number();
	basisVars = ui->get_basis_vars();
	nonBasisVars = ui->get_nonbasis_vars();
	function = ui->get_function();
	equalitiesMatrix = ui->get_equalities_matrix();

	optimizer->set_system(varNumber, basisVars,
		nonBasisVars, function, equalitiesMatrix);

	if(optimizer->setup_completed()) {
		sTable = optimizer->get_simplex_table();

		stepNumber = 0;
		newBasisVars = optimizer->get_basis_vars();

		tableSaver->save(sTable, newBasisVars, nonBasisVars);

		ui->update_state(sTable, newBasisVars, nonBasisVars);

		if(optimizer->solution_found())
			ui->set_solution_found();
	}
	else
		ui->set_system_invalid();
}

void Controller::__abm_canceled() {
	int varNumber;
	QVector<int> basisVars, nonBasisVars;
	QVector<Fraction> function;
	QVector<QVector<Fraction>> equalitiesMatrix;

	varNumber = ui->get_var_number();
	basisVars = ui->get_basis_vars();
	nonBasisVars = ui->get_nonbasis_vars();
	function = ui->get_function();
	equalitiesMatrix = ui->get_equalities_matrix();

	optimizer->set_system(varNumber, basisVars,
		nonBasisVars, function, equalitiesMatrix);

	if(optimizer->setup_completed()) {
		__step_back();
	}
	else
		ui->set_system_invalid();
}

void Controller::abm_set_simplex_table() {
	int varNumber;
	QVector<int> basisVars, nonBasisVars;
	QVector<Fraction> function;
	QVector<QVector<Fraction>> sTable, equalitiesMatrix;

	varNumber = ui->get_var_number();
	basisVars = ui->get_basis_vars();
	nonBasisVars = ui->get_nonbasis_vars();
	function = ui->get_function();
	equalitiesMatrix = ui->get_equalities_matrix();

	optimizer->set_system(varNumber, basisVars,
		nonBasisVars, function, equalitiesMatrix);

	if(optimizer->setup_completed()) {
		sTable = optimizer->get_simplex_table();
		
		tableSaver->save(sTable, basisVars, nonBasisVars);

		ui->set_simplex_table(sTable);

		++stepNumber;
	}
}

void Controller::set_solution() {
	if(stepNumber != -1) {
		QVector<int> bVars, nbVars;
		QVector<QVector<Fraction>> sTable;

		optimizer->solve_system();

		if(optimizer->solution_found()) {
			bVars = optimizer->get_basis_vars();
			nbVars = optimizer->get_nonbasis_vars();
			sTable = optimizer->get_simplex_table();

			tableSaver->save(sTable, bVars, nbVars);

			ui->update_state(sTable, bVars, nbVars);

			++stepNumber;

			ui->set_solution_found();
		}
		else
			ui->set_no_solution();
	}
}
void Controller::__run_abm_method(int vNumber, QVector<int>& bVars,
	QVector<int>& nbVars, QVector<Fraction>& func, QVector<QVector<Fraction>>& eMatrix)
{
	int varNumber;
	QVector<int> basisVars, nonBasisVars;
	QVector<Fraction> function;
	QVector<QVector<Fraction>> equalitiesMatrix, sTable;

	optimizer->set_system(vNumber, bVars, nbVars, func, eMatrix);

	if(optimizer->setup_completed()) {
		optimizer->abm_solve_system();

		if(optimizer->abm_avail_point_exists()) {
			basisVars = optimizer->get_basis_vars();
			nonBasisVars = optimizer->get_nonbasis_vars();

			varNumber = ui->get_var_number();
			function = ui->get_function();
			equalitiesMatrix = ui->get_equalities_matrix();

			// Вызываемый метод может изменять аргументы
			optimizer->set_system(varNumber, basisVars,
				nonBasisVars, function, equalitiesMatrix);

			// Поэтому здесь мы запрашиваем параметры снова
			basisVars = optimizer->get_basis_vars();
			nonBasisVars = optimizer->get_nonbasis_vars();
			sTable = optimizer->get_simplex_table();

			tableSaver->save(sTable, basisVars, nonBasisVars);

			ui->update_state(sTable, basisVars, nonBasisVars);

			++stepNumber;

			if(optimizer->solution_found())
				ui->set_solution_found();
		}
		else
			ui->set_no_solution();
	}
}

void Controller::__step_back() {
	if(stepNumber > 0) {
		QVector<int> bVars, nbVars;
		QVector<QVector<Fraction>> sTable;

		tableSaver->restore(sTable, bVars, nbVars, --stepNumber);

		optimizer->update_state(sTable, bVars, nbVars);

		ui->update_state(sTable, bVars, nbVars);

		if(optimizer->solution_found())
			ui->set_solution_found();
	}
}

void Controller::__step_forward(int rowNumber, int colNumber, bool delBasisVar) {
	if(stepNumber != -1) {
		QVector<int> bVars, nbVars;
		QVector<QVector<Fraction>> sTable;

		optimizer->step_forward(rowNumber, colNumber, delBasisVar);

		bVars = optimizer->get_basis_vars();
		nbVars = optimizer->get_nonbasis_vars();
		sTable = optimizer->get_simplex_table();

		tableSaver->save(sTable, bVars, nbVars);

		ui->update_state(sTable, bVars, nbVars);

		++stepNumber;

		if(optimizer->solution_found())
			ui->set_solution_found();
	}
}

void Controller::__abm_last_step(int rowNumber, int colNumber, bool delBasisVar) {
	if(!delBasisVar)
		return;

	if(stepNumber != -1) {
		QVector<int> bVars, nbVars;
		QVector<QVector<Fraction>> sTable;

		optimizer->step_forward(rowNumber, colNumber, delBasisVar);

		bVars = optimizer->get_basis_vars();
		nbVars = optimizer->get_nonbasis_vars();
		sTable = optimizer->get_simplex_table();

		ui->update_state(sTable, bVars, nbVars);
	}
}