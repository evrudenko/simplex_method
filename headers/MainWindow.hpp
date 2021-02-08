#ifndef _MAIN_WINDOW_HPP_
#define _MAIN_WINDOW_HPP_

#include <QApplication>
#include <QtWidgets>

#include <algorithm>

#include "SetupWindow.hpp"
#include "SystemSaver.hpp"
#include "ABMSystemSaver.hpp"
#include "Fraction.hpp"

const QString backPngFile = "icons\\back.png";
const QString forwardPngFile = "icons\\forward.png";

class MainWindow : public QMainWindow {
	Q_OBJECT
private:
	const int startRowPos, startColPos;

	bool systemInvalid, secureMode;

	int varNumber, checkedVBtnNumber, checkedHBtnNumber,
		minWidth, minHeight, stepNumber, abm_varNumber, abm_stepNumber;

	string abmSaveFileName = "saved_abm_systems.txt";

	QVector<int> basisVars, nonBasisVars, artBasisVars,
		abm_basisVars, abm_nonBasisVars;

	QVector<Fraction> function, abm_function;

	QVector<QVector<bool>> availElementsMatrix;

	QVector<QVector<Fraction>> equalitiesMatrix,
		abm_equalitiesMatrix, simplexTable;

	QFont wFont;
	QLabel *display;
	SetupWindow *setupWindow;
	QPushButton *backButton, *forwardButton;
	QAction *newSystemActn, *restoreSystemActn, *saveSystemActn, *editSystemActn,
		*addArtBasisActn, *runArtBasisMethodActn, *findSolutionActn;
	QVector<QPushButton *> vButtons, hButtons;
	QVector<QVector<QLabel *>> simplexTableLabels;

	QGridLayout *mainLayout;

	ABMSystemSaver *abm_SystemSaver;

	QVector<QPushButton *> create_buttons(const QVector<int> &indices);

	void update_components();

	void clear_main_layout();

	void update_simplex_table();

	void update_avail_elements_matrix();

	void remove_display();

	void remove_control_buttons();

	void pin_warning_message(const QString &message);

public:
	MainWindow(QWidget *parent = 0);

	~MainWindow();

	void set_solution_found();

	void set_no_solution();

	void update_var_number();

	void update_state(const QVector<QVector<Fraction>> &sTable, const QVector<int> &bVars,
		const QVector<int> &nbVars);

	void set_basis_vars(const QVector<int> &bVars);

	void set_nonbasis_vars(const QVector<int> &nbVars);

	void set_simplex_table(const QVector<QVector<Fraction>> &sTable, bool needUpdate = false);

	void set_system_invalid();

	int get_var_number();

	QVector<int> get_basis_vars();

	QVector<int> get_nonbasis_vars();

	QVector<Fraction> get_function();

	QVector<QVector<Fraction>> get_equalities_matrix();

	void set_info(const QString &info);

signals:
	void simplex_table_request();

	void abm_simplex_table_request();

	void step_back();

	void step_forward(int rowNumber, int colNumber, bool delBasisVar);

	void abm_completed();

	void abm_canceled();

	void run_abm_method(int vNumber, QVector<int> &bVars, QVector<int> &nbVars,
		QVector<Fraction> &func, QVector<QVector<Fraction>> &eMatrix);

	void abm_last_step(int rowNumber, int colNumber, bool delBasisVar);

	void solution_request();

public slots:
	void new_system();

	void restore_system();

	void save_system();

	void edit_system();

	void show_help();

	void setup_completed();

	void setup_canceled();

	void mw_button_toggled(bool btnOn);

	void __step_back();

	void __step_forward();

	void add_art_basis();

	void run_art_basis_method();

	void find_solution();
};

#endif // _MAIN_WINDOW_HPP_