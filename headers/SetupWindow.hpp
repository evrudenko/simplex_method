#ifndef _SETUP_WINDOW_HPP_
#define _SETUP_WINDOW_HPP_

#include <QtWidgets>

#include <algorithm>
#include <fstream>
#include <sstream>
#include <string>
#include <cstdio>

#include "SystemSaver.hpp"
#include "Fraction.hpp"

using namespace std;

class SetupWindow : public QWidget {
	Q_OBJECT

private:
	int defaultVarNumber = 3;
	const string spFilename = "savepoint.txt";

	int varNumber;
	QVector<int> basisVars;
	QVector<Fraction> function;
	QVector<QVector<Fraction>> equalitiesMatrix;
	QIntValidator *numberValidator;

	QSpinBox *nSelectionSpinBox;
	QLabel *display;
	QVector<QPushButton *> basisSelectionButtons;

	QGridLayout *centralLayout;
	QVBoxLayout *systemSetupLayout, *basisSelectionLayout, *mainLayout;
	QVector<QHBoxLayout *> equalityLayouts;
	QHBoxLayout *nSelectionLayout, *ctrlBtnsLayout, *functionLayout, *taskSetupLayout;

	void closeEvent(QCloseEvent *e) override;

	void save_state();

	void rollback();

	QVector<QPushButton *> create_buttons();

	inline void initialize_function();

	void initialize_function_layout();

	inline void update_function_layout();

	void fit_function_layout(int newVarNumber, int oldVarNumber = -1);

	void update_equalities_layouts();

	void fit_equalities_layouts(int newVarNumber, int oldVarNumber = -1);

	inline void update_basis_selection_layout();

	void fit_basis_selection_layout(int newVarNumber); 

	void change_equalities_number(int newEqNumber);

public:
	SetupWindow(QWidget *parent = 0);

	void reset();

	int get_var_number();

	QVector<Fraction> get_function();

	QVector<QVector<Fraction>> get_equalities_matrix();

	QVector<int> get_basis_vars();

	void set_system(int vNumber, const QVector<int>& bVars,
		const QVector<Fraction>& func, const QVector<QVector<Fraction>>& eMatrix);

signals:
	void applied();

	void canceled();

public slots:
	void __applied();

	void __canceled();

	void add_equality();

	void var_number_changed(int n);

	void function_changed(const QString& val);

	void equalities_matrix_changed(const QString& val);

	void sw_button_toggled(bool btnOn);

	void delete_equality(QString objName = "");
};

#endif // _SETUP_WINDOW_HPP_