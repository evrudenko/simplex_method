#include "..\headers\SetupWindow.hpp"

SetupWindow::SetupWindow(QWidget *parent/* = 0*/) :
	QWidget(parent), varNumber(defaultVarNumber)
{
	QFile(QString::fromStdString(spFilename)).remove();

	setStyleSheet("background-color: ghostwhite;");
	display = new QLabel("");
	display->setStyleSheet("color: red;");
	display->setAlignment(Qt::AlignCenter);

	QLabel *nSelectionLabel = new QLabel("Количество переменных:");
	nSelectionLabel->setAlignment(Qt::AlignCenter);

	nSelectionSpinBox = new QSpinBox();
	nSelectionSpinBox->setStyleSheet("background-color: white;");
	nSelectionSpinBox->setValue(varNumber);
	nSelectionSpinBox->setRange(1, 20);
	connect(nSelectionSpinBox, SIGNAL(valueChanged(int)), SLOT(var_number_changed(int)));

	nSelectionLayout = new QHBoxLayout();
	nSelectionLayout->addWidget(nSelectionLabel);
	nSelectionLayout->addWidget(nSelectionSpinBox);
	nSelectionLayout->addStretch(1);
	nSelectionLayout->addWidget(display);
	nSelectionLayout->addStretch(1);

	initialize_function();

	numberValidator = new QIntValidator(-10000, 10000, this);

	functionLayout = new QHBoxLayout();
	initialize_function_layout();

	QPushButton *addEqualityBtn = new QPushButton("Добавить ограничение");
	connect(addEqualityBtn, SIGNAL(clicked()), SLOT(add_equality()));

	systemSetupLayout = new QVBoxLayout();
	systemSetupLayout->addLayout(nSelectionLayout);
	systemSetupLayout->addLayout(functionLayout);
	for(auto layout : equalityLayouts)
		systemSetupLayout->addLayout(layout);
	systemSetupLayout->addWidget(addEqualityBtn);
	systemSetupLayout->addStretch(1);

	QLabel *basisSelectionLabel = new QLabel("Выберите базис:");
	basisSelectionLabel->setAlignment(Qt::AlignCenter);

	basisSelectionButtons = create_buttons();

	basisSelectionLayout = new QVBoxLayout();
	basisSelectionLayout->addWidget(basisSelectionLabel);
	for(auto btn : basisSelectionButtons)
		basisSelectionLayout->addWidget(btn);
	basisSelectionLayout->addStretch(1);

	taskSetupLayout = new QHBoxLayout();
	taskSetupLayout->addLayout(systemSetupLayout);
	taskSetupLayout->addLayout(basisSelectionLayout);

	QPushButton *applyBtn = new QPushButton("Принять");
	connect(applyBtn, SIGNAL(clicked()), SLOT(__applied()));

	QPushButton *cancelBtn = new QPushButton("Отменить");
	connect(cancelBtn, SIGNAL(clicked()), SLOT(__canceled()));

	ctrlBtnsLayout = new QHBoxLayout();
	ctrlBtnsLayout->addStretch(1);
	ctrlBtnsLayout->addWidget(applyBtn);
	ctrlBtnsLayout->addWidget(cancelBtn);

	mainLayout = new QVBoxLayout();
	mainLayout->setSpacing(11);
	mainLayout->addLayout(taskSetupLayout);
	mainLayout->addLayout(ctrlBtnsLayout);

	setLayout(mainLayout);

	resize(600, 400);
}

// ~~~~ Slots:
void SetupWindow::__applied() {
	if(equalitiesMatrix.size() == 0) {
		display->setText("Введите хотя бы одно ограничение!");
	}
	else if(basisVars.size() == equalitiesMatrix.size()) {
		display->setText("");
		save_state();
		emit applied();
	}
	else display->setText("Недостаточно базисных переменных. Выбрано: " +
		QString::number(basisVars.size()) + ", необходимо: " +
		QString::number(equalitiesMatrix.size()) + "!");
}

void SetupWindow::__canceled() {
	display->setText("");
	rollback();
	emit canceled();
}

void SetupWindow::add_equality() {
	display->setText("");

	QVector<QHBoxLayout *> components;
	QHBoxLayout *pHBoxLayout, *pEqualityLayout;
	QLineEdit *pLineEdit;
	QPushButton *pPushButton;

	int row = equalitiesMatrix.size();

	equalitiesMatrix.push_back({});
	equalitiesMatrix[row].fill(0, varNumber + 1);

	for(int i = 0; i < varNumber; ++i) {
		pLineEdit = new QLineEdit(equalitiesMatrix[row][i].toQString());
		pLineEdit->setStyleSheet("background-color: white;");
		pLineEdit->setObjectName("em" + QString::number(row + 1) +
			" " + QString::number(i + 1));
		pLineEdit->setValidator(numberValidator);
		connect(pLineEdit, SIGNAL(textChanged(const QString &)),
			SLOT(equalities_matrix_changed(const QString &)));

		pHBoxLayout = new QHBoxLayout();
		pHBoxLayout->addStretch(1);
		pHBoxLayout->addWidget(pLineEdit);
		pHBoxLayout->addWidget(new QLabel("*"));
		pHBoxLayout->addWidget(new QLabel("X" + QString::number(i + 1)));
		pHBoxLayout->addStretch(1);

		components.push_back(pHBoxLayout);
	}

	if(varNumber) {
		pPushButton = new QPushButton("Удалить");
		pPushButton->setObjectName("de" + QString::number(row));
		connect(pPushButton, SIGNAL(clicked()), SLOT(delete_equality()));

		pEqualityLayout = new QHBoxLayout();
		pEqualityLayout->addWidget(pPushButton);
		pEqualityLayout->addStretch(1);
		pEqualityLayout->addLayout(components[0]);

		for(int i = 1; i < varNumber; ++i) {
			pEqualityLayout->addWidget(new QLabel("+"));
			pEqualityLayout->addLayout(components[i]);
		}

		pLineEdit = new QLineEdit(equalitiesMatrix[row][varNumber].toQString());
		pLineEdit->setStyleSheet("background-color: white;");
		pLineEdit->setObjectName("em" + QString::number(row + 1) +
			" " + QString::number(varNumber + 1));
		pLineEdit->setValidator(numberValidator);
		connect(pLineEdit, SIGNAL(textChanged(const QString &)),
			SLOT(equalities_matrix_changed(const QString &)));

		pEqualityLayout->addWidget(new QLabel("<h4>=</h4>"));
		pEqualityLayout->addWidget(pLineEdit);
		pEqualityLayout->addStretch(1);

		equalityLayouts.push_back(pEqualityLayout);

		systemSetupLayout->insertLayout(systemSetupLayout->count() - 2, pEqualityLayout);
	}
}

void SetupWindow::var_number_changed(int n) {
	fit_function_layout(n);
	fit_equalities_layouts(n);
	fit_basis_selection_layout(n);

	varNumber = n;
}

void SetupWindow::function_changed(const QString& val) {
	QString objName = sender()->objectName();
	int varIndex = objName.right(objName.size() - 2).toInt() - 1;

	function[varIndex] = val.toInt();
}

void SetupWindow::equalities_matrix_changed(const QString& val) {
	QString objName = sender()->objectName();
	QStringList indicies = (objName.right(objName.size() - 2)).split(' ');

	int i = indicies[0].toInt(), j = indicies[1].toInt();

	equalitiesMatrix[i - 1][j - 1] = val.toInt();
}

void SetupWindow::sw_button_toggled(bool btnOn) {
	QString objName = sender()->objectName();
	int btnIndex = objName.right(objName.size() - 2).toInt();
	QPushButton *btn = basisSelectionButtons[btnIndex - 1];

	if(btnOn) {
		if(!basisVars.contains(btnIndex)) {
			if(basisVars.size() == equalitiesMatrix.size())
				btn->setChecked(false);
			else if(basisVars.size() < equalitiesMatrix.size()) {
				display->setText("");
				basisVars.push_back(btnIndex);
			}
		}
	}
	else basisVars.removeAll(btnIndex);
}

void SetupWindow::delete_equality(QString objName/* = ""*/) {
	QLayoutItem *child1, *child2, *child3;
	QHBoxLayout *pHBoxLayout;

	if(objName == "") {
		objName = sender()->objectName();

		basisVars.clear();
		for(auto btn : basisSelectionButtons)
			btn->setChecked(false);
	}

	int row = objName.right(objName.size() - 2).toInt();

	// Переименование объектов, следующих за удаляемым:
	for(int i = row + 1; i < equalityLayouts.size(); ++i) {
		pHBoxLayout = equalityLayouts[i];

		pHBoxLayout->itemAt(0)->widget()->setObjectName("de" + QString::number(i - 1));

		for(int j = 0; j < varNumber; ++j) {
			pHBoxLayout->itemAt(2 + j * 2)->layout()->itemAt(1)->widget()->
				setObjectName("em" + QString::number(i) + " " + QString::number(j + 1));
		}

		pHBoxLayout->itemAt(pHBoxLayout->count() - 2)->widget()->
			setObjectName("em" + QString::number(i) + " " + QString::number(varNumber + 1));
	}

	// Удаление объекта:
	equalityLayouts.removeAt(row);
	equalitiesMatrix.removeAt(row);

	child1 = systemSetupLayout->takeAt(row + 2);

	child2 = child1->layout()->takeAt(0);
	delete child2->widget();
	delete child2;

	for(int i = 0; i < varNumber; ++i) {
		child2 = child1->layout()->takeAt(0);
		delete child2->widget();
		delete child2;

		child2 = child1->layout()->takeAt(0);
		while((child3 = child2->layout()->takeAt(0)) != nullptr) {
			delete child3->widget();
			delete child3;
		}

		delete child2;
	}

	while((child2 = child1->layout()->takeAt(0)) != nullptr) {
		delete child2->widget();
		delete child2;
	}

	delete child1;
}

// ~~~~ Private functions:
void SetupWindow::closeEvent(QCloseEvent *e) {
	display->setText("");
	e->accept();
	rollback();
	emit canceled();
}

void SetupWindow::save_state() {
	SystemSaver::save_system(spFilename, varNumber, basisVars, function, equalitiesMatrix);
}

void SetupWindow::rollback() {
	int vNumber;
	QVector<int> bVars;
	QVector<Fraction> func;
	QVector<QVector<Fraction>> eMatrix;

	if(SystemSaver::restore_system(spFilename, vNumber, bVars, func, eMatrix)) {
		nSelectionSpinBox->setValue(vNumber);
		var_number_changed(vNumber);
		
		change_equalities_number(eMatrix.size());
		equalitiesMatrix = eMatrix;

		basisVars = bVars;
		function = func;

		update_basis_selection_layout();
		update_function_layout();
		update_equalities_layouts();
	}
	else
		reset();
}

QVector<QPushButton *> SetupWindow::create_buttons() {
	QVector<QPushButton *> btns;
	QPushButton *pBtn;

	for(int index = 1; index <= varNumber; ++index) {
		pBtn = new QPushButton("X" + QString::number(index));
		pBtn->setObjectName("sw" + QString::number(index));
		pBtn->setCheckable(true);
		pBtn->setChecked(false);

		connect(pBtn, SIGNAL(toggled(bool)), SLOT(sw_button_toggled(bool)));

		btns.push_back(pBtn);
	}

	return btns;
}

/*inline */void SetupWindow::initialize_function() {
	if(varNumber)
		function.fill(0, varNumber);
}

void SetupWindow::initialize_function_layout() {
	QVector<QHBoxLayout *> components;
	QHBoxLayout *pHBoxLayout;
	QLineEdit *pLineEdit;

	for(int i = 0; i < varNumber; ++i) {
		pLineEdit = new QLineEdit(function[i].toQString());
		pLineEdit->setStyleSheet("background-color: white;");
		pLineEdit->setObjectName("fc" + QString::number(i + 1));
		pLineEdit->setValidator(numberValidator);
		connect(pLineEdit, SIGNAL(textChanged(const QString &)),
			SLOT(function_changed(const QString &)));

		pHBoxLayout = new QHBoxLayout();
		pHBoxLayout->addStretch(1);
		pHBoxLayout->addWidget(pLineEdit);
		pHBoxLayout->addWidget(new QLabel("*"));
		pHBoxLayout->addWidget(new QLabel("X" + QString::number(i + 1)));
		pHBoxLayout->addStretch(1);

		components.push_back(pHBoxLayout);
	}

	if(varNumber) {
		functionLayout->addStretch(1);
		functionLayout->addLayout(components[0]);

		for(int i = 1; i < varNumber; ++i) {
			functionLayout->addWidget(new QLabel("+"));
			functionLayout->addLayout(components[i]);
		}

		functionLayout->addWidget(new QLabel("<h4>⟶</h4>"));
		functionLayout->addWidget(new QLabel("<h4>MIN</h4>"));
		functionLayout->addStretch(1);
	}
}

/*inline */void SetupWindow::update_function_layout() {
	for(int i = 1, j = 0; i < functionLayout->count() - 3 && j < varNumber; i += 2, ++j) {
		qobject_cast<QLineEdit *>(functionLayout->itemAt(i)->layout()->itemAt(1)->
			widget())->setText(function[j].toQString());
	}
}

void SetupWindow::fit_function_layout(int newVarNumber, int oldVarNumber/* = -1*/) {
	if(oldVarNumber == -1)
		oldVarNumber = varNumber;

	if(newVarNumber < oldVarNumber) {
		QLayoutItem *child, *item;
		for(int i = 0; i < oldVarNumber - newVarNumber; ++i) {
			function.pop_back();
			child = functionLayout->takeAt(functionLayout->count() - 5);
			delete child->widget();
			delete child;

			child = functionLayout->takeAt(functionLayout->count() - 4);
			while((item = child->layout()->takeAt(0)) != nullptr) {
				delete item->widget();
				delete item;
			}

			delete child;
		}
	}
	else if(newVarNumber > oldVarNumber) {
		QHBoxLayout *pHBoxLayout;
		QLineEdit *pLineEdit;

		for(int i = oldVarNumber; i < newVarNumber; ++i) {
			function.push_back(0);
			pLineEdit = new QLineEdit(function[i].toQString());
			pLineEdit->setStyleSheet("background-color: white;");
			pLineEdit->setObjectName("fc" + QString::number(i + 1));
			pLineEdit->setValidator(numberValidator);
			connect(pLineEdit, SIGNAL(textChanged(const QString &)),
				SLOT(function_changed(const QString &)));

			pHBoxLayout = new QHBoxLayout();
			pHBoxLayout->addStretch(1);
			pHBoxLayout->addWidget(pLineEdit);
			pHBoxLayout->addWidget(new QLabel("*"));
			pHBoxLayout->addWidget(new QLabel("X" + QString::number(i + 1)));
			pHBoxLayout->addStretch(1);

			functionLayout->insertWidget(functionLayout->count() - 3, new QLabel("+"));
			functionLayout->insertLayout(functionLayout->count() - 3, pHBoxLayout);
		}
	}
}

void SetupWindow::update_equalities_layouts() {
	QHBoxLayout *pHBoxLayout;
	for(int i = 0; i < equalitiesMatrix.size(); ++i) {
		pHBoxLayout = equalityLayouts[i];
		for(int j = 2, k = 0; j < pHBoxLayout->count() - 3 && k < equalitiesMatrix[i].size(); j += 2, ++k) {
			qobject_cast<QLineEdit *>(pHBoxLayout->itemAt(j)->layout()->itemAt(1)->
				widget())->setText(equalitiesMatrix[i][k].toQString());
		}

		qobject_cast<QLineEdit *>(pHBoxLayout->itemAt(pHBoxLayout->count() - 2)->
			widget())->setText(equalitiesMatrix[i][varNumber].toQString());
	}
}

void SetupWindow::fit_equalities_layouts(int newVarNumber, int oldVarNumber/* = -1*/) {
	if(oldVarNumber == -1)
		oldVarNumber = varNumber;

	if(newVarNumber < oldVarNumber) {
		QLayoutItem *child, *item;
		QHBoxLayout *pHBoxLayout;
		for(int i = 0; i < equalityLayouts.size(); ++i) {
			pHBoxLayout = equalityLayouts[i];
			for(int j = 0; j < oldVarNumber - newVarNumber; ++j) {
				equalitiesMatrix[i].removeAt(equalitiesMatrix[i].size() - 2);

				child = pHBoxLayout->takeAt(pHBoxLayout->count() - 5);
				delete child->widget();
				delete child;

				child = pHBoxLayout->takeAt(pHBoxLayout->count() - 4);
				while((item = child->layout()->takeAt(0)) != nullptr) {
					delete item->widget();
					delete item;
				}

				delete child;
			}
			
			pHBoxLayout->itemAt(equalityLayouts[i]->count() - 2)->widget()->
				setObjectName("em" + QString::number(i + 1) + " " + QString::number(newVarNumber + 1));
		}
	}
	else if(newVarNumber > oldVarNumber) {
		QLineEdit *pLineEdit;
		QHBoxLayout *pHBoxLayout;
		for(int i = 0; i < equalitiesMatrix.size(); ++i) {
			for(int j = oldVarNumber; j < newVarNumber; ++j) {
				equalitiesMatrix[i].insert(equalitiesMatrix[i].size() - 1, 0);
				pLineEdit = new QLineEdit(QString::number(0));
				pLineEdit->setStyleSheet("background-color: white;");
				pLineEdit->setObjectName("em" + QString::number(i + 1) + " " +
					QString::number(j + 1));
				pLineEdit->setValidator(numberValidator);
				connect(pLineEdit, SIGNAL(textChanged(const QString &)),
					SLOT(equalities_matrix_changed(const QString &)));

				pHBoxLayout = new QHBoxLayout();
				pHBoxLayout->addStretch(1);
				pHBoxLayout->addWidget(pLineEdit);
				pHBoxLayout->addWidget(new QLabel("*"));
				pHBoxLayout->addWidget(new QLabel("X" + QString::number(j + 1)));
				pHBoxLayout->addStretch(1);

				equalityLayouts[i]->insertWidget(equalityLayouts[i]->count() - 3, new QLabel("+"));
				equalityLayouts[i]->insertLayout(equalityLayouts[i]->count() - 3, pHBoxLayout);
			}

			equalityLayouts[i]->itemAt(equalityLayouts[i]->count() - 2)->widget()->
				setObjectName("em" + QString::number(i + 1) + " " + QString::number(newVarNumber + 1));
		}
	}
}

/*inline */void SetupWindow::update_basis_selection_layout() {
	QPushButton *btn;
	for(int i = 0; i < basisSelectionButtons.size(); ++i) {
		btn = basisSelectionButtons[i];
		if(basisVars.contains(i + 1))
			btn->setChecked(true);
		else
			btn->setChecked(false);
	}
}

void SetupWindow::fit_basis_selection_layout(int newVarNumber) {
	int currentVarNumber = basisSelectionButtons.size();

	if(newVarNumber < currentVarNumber) {
		QLayoutItem *child;
		for(int i = currentVarNumber - 1; i >= newVarNumber; --i) {
			basisSelectionButtons.pop_back();
			child = basisSelectionLayout->takeAt(basisSelectionLayout->count() - 2);
			if(qobject_cast<QPushButton *>(child->widget())->isChecked())
				basisVars.removeAll(i + 1);
			delete child->widget();
			delete child;
		}
	}
	else if(newVarNumber > currentVarNumber) {
		QPushButton *pBtn;
		for(int i = currentVarNumber; i < newVarNumber; ++i) {
			pBtn = new QPushButton("X" + QString::number(i + 1));
			pBtn->setObjectName("sw" + QString::number(i + 1));
			pBtn->setCheckable(true);
			pBtn->setChecked(false);

			connect(pBtn, SIGNAL(toggled(bool)), SLOT(sw_button_toggled(bool)));

			basisSelectionLayout->insertWidget(basisSelectionLayout->count() - 1, pBtn);
			basisSelectionButtons.push_back(pBtn);
		}
	}
}

void SetupWindow::change_equalities_number(int newEqNumber) {
	QString objName;
	int currEqNumber = equalitiesMatrix.size();

	if(newEqNumber > currEqNumber)
		for(int i = 0; i < newEqNumber - currEqNumber; ++i)
			add_equality();

	if(newEqNumber < currEqNumber)
		for(int i = currEqNumber; i > newEqNumber; --i) {
			objName = "de" + QString::number(i - 1);
			delete_equality(objName);
		}
}

// ~~~~ Public functions:
void SetupWindow::reset() {
	display->setText("");

	basisVars.clear();
	for(auto btn : basisSelectionButtons)
		btn->setChecked(false);

	change_equalities_number(0);

	for(auto v : equalitiesMatrix)
		v.clear();
	equalitiesMatrix.clear();

	nSelectionSpinBox->setValue(defaultVarNumber);
	var_number_changed(defaultVarNumber);

	for(auto &fval : function)
		fval = 0;

	update_function_layout();

	resize(600, 400);
}

int SetupWindow::get_var_number() { return varNumber; }

QVector<Fraction> SetupWindow::get_function() { return function; }

QVector<QVector<Fraction>> SetupWindow::get_equalities_matrix() { return equalitiesMatrix; }

QVector<int> SetupWindow::get_basis_vars() {
	sort(basisVars.begin(), basisVars.end());
	return basisVars;
}

void SetupWindow::set_system(int vNumber, const QVector<int>& bVars, const QVector<Fraction>& func,
	const QVector<QVector<Fraction>>& eMatrix)
{
		nSelectionSpinBox->setValue(vNumber);
		var_number_changed(vNumber);
		
		change_equalities_number(eMatrix.size());
		equalitiesMatrix = eMatrix;

		basisVars = bVars;
		function = func;

		update_basis_selection_layout();
		update_function_layout();
		update_equalities_layouts();

		save_state();
}