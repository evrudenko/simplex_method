#include "..\headers\MainWindow.hpp"

MainWindow::MainWindow(QWidget *parent/* = 0*/) : QMainWindow(parent),
	startRowPos(1), startColPos(0), varNumber(0), checkedVBtnNumber(-1), checkedHBtnNumber(-1),
	minWidth(50), minHeight(50), abm_stepNumber(-1), systemInvalid(false), secureMode(true)
{
	abm_SystemSaver = new ABMSystemSaver(); 
	// ~~~~ Инициализация окна настройки/задания системы ~~~~

	setupWindow = new SetupWindow();
	setupWindow->setWindowModality(Qt::ApplicationModal);
	connect(setupWindow, SIGNAL(applied()), SLOT(setup_completed()));
	connect(setupWindow, SIGNAL(canceled()), SLOT(setup_canceled()));

	wFont = QApplication::font();
	wFont.setPixelSize(18);

	// ~~~~ Инициализация меню ~~~~

	// Меню "Система"
	newSystemActn = new QAction("Новая система");
	connect(newSystemActn, SIGNAL(triggered()), SLOT(new_system()));

	restoreSystemActn = new QAction("Открыть");
	connect(restoreSystemActn, SIGNAL(triggered()), SLOT(restore_system()));

	saveSystemActn = new QAction("Сохранить");
	saveSystemActn->setEnabled(false);
	connect(saveSystemActn, SIGNAL(triggered()), SLOT(save_system()));

	QMenu *systemMenu = menuBar()->addMenu("Система");
	systemMenu->addAction(newSystemActn);
	systemMenu->addAction(restoreSystemActn);
	systemMenu->addAction(saveSystemActn);

	// Меню "Настройка системы"
	editSystemActn = new QAction("Редактировать систему");
	editSystemActn->setEnabled(false);
	connect(editSystemActn, SIGNAL(triggered()), SLOT(edit_system()));

	QMenu *systemSetupMenu = menuBar()->addMenu("Настройка системы");
	systemSetupMenu->addAction(editSystemActn);

	// Меню "Поиск решения"
	findSolutionActn = new QAction("Найти решение");
	findSolutionActn->setEnabled(false);
	connect(findSolutionActn, SIGNAL(triggered()), SLOT(find_solution()));

	QMenu *solutionSearchMenu = menuBar()->addMenu("Поиск решения");
	solutionSearchMenu->addAction(findSolutionActn);

	// Меню "Искусственный базис"
	addArtBasisActn = new QAction("Добавить базис");
	addArtBasisActn->setEnabled(false);
	connect(addArtBasisActn, SIGNAL(triggered()), SLOT(add_art_basis()));

	runArtBasisMethodActn = new QAction("Использовать метод");
	runArtBasisMethodActn->setEnabled(false);
	connect(runArtBasisMethodActn, SIGNAL(triggered()), SLOT(run_art_basis_method()));

	QMenu *artBasisMenu = menuBar()->addMenu("Искусственный базис");
	artBasisMenu->addAction(addArtBasisActn);
	artBasisMenu->addAction(runArtBasisMethodActn);

	// Меню "Справка"
	QAction *helpActn = menuBar()->addAction("Справка");
	connect(helpActn, SIGNAL(triggered()), SLOT(show_help()));

	// ~~~~ Инициализация начальной страницы ~~~~

	display = new QLabel("<h1>Welcome</h1>");
	display->setAlignment(Qt::AlignCenter);

	// ~~~~ Инициализация управляющих кнопок ~~~~

	// Кнопка для шага назад
	backButton = new QPushButton();
	backButton->setShortcut(QKeySequence("left"));
	backButton->setMinimumSize(minWidth, minHeight);
	backButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	if(QFile::exists(backPngFile)) {
		backButton->setIcon(QIcon(backPngFile));
		backButton->setIconSize(QSize(25, 25));
	}
	else
		backButton->setText("BACK");

	connect(backButton, SIGNAL(clicked()), SLOT(__step_back()));

	// Кнопка для шага вперед
	forwardButton = new QPushButton();
	forwardButton->setShortcut(QKeySequence("right"));
	forwardButton->setMinimumSize(minWidth, minHeight);
	forwardButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	if(QFile::exists(forwardPngFile)) {
		forwardButton->setIcon(QIcon(forwardPngFile));
		forwardButton->setIconSize(QSize(25, 25));
	}
	else
		forwardButton->setText("FORWARD");

	connect(forwardButton, SIGNAL(clicked()), SLOT(__step_forward()));
	
	// ~~~~ Инициализация центрального виджета ~~~~

	mainLayout = new QGridLayout();
	mainLayout->addWidget(display, 0, 0);

	QWidget *mainWidget = new QWidget(this);
	mainWidget->setStyleSheet("background-color: ghostwhite;");
	mainWidget->setLayout(mainLayout);

	setCentralWidget(mainWidget);
	resize(800, 600);
}

MainWindow::~MainWindow() {
	delete backButton;
	delete forwardButton;
	delete abm_SystemSaver;
}

void MainWindow::set_solution_found() {
	Fraction fValue;
	QVector<Fraction> solution;

	auto lastRowIndex = simplexTable.size() - 1;
	auto lastColIndex = simplexTable[lastRowIndex].size() - 1;
	fValue = -simplexTable[lastRowIndex][lastColIndex];

	solution.fill(0.0, varNumber);
	for(int i = 0; i < basisVars.size(); ++i)
		solution[basisVars[i] - 1] = simplexTable[i][simplexTable[i].size() - 1];

	QString solutionStr = "";
	for(int i = 0; i < solution.size(); ++i)
		solutionStr += solution[i].toQString() + (i == solution.size() - 1 ? "" : ", ");
	solutionStr = "<h3>F(" + solutionStr + ") = " + fValue.toQString() + "</h3>";

	display->setText(solutionStr);
	display->setStyleSheet("color: black;");
	display->show();
}

void MainWindow::set_no_solution() {
	systemInvalid = true;

	if(!addArtBasisActn->isEnabled())
		addArtBasisActn->setEnabled(true);
	if(!runArtBasisMethodActn->isEnabled())
		runArtBasisMethodActn->setEnabled(true);

	pin_warning_message("Система не имеет допустимых решений!");
}

void MainWindow::update_var_number() {
	varNumber = basisVars.size() + nonBasisVars.size();
}

void MainWindow::update_state(const QVector<QVector<Fraction>>& sTable,
	const QVector<int>& bVars, const QVector<int>& nbVars)
{
	if(abm_stepNumber == -1) {
		basisVars = bVars;
		nonBasisVars = nbVars;
		varNumber = basisVars.size() + nonBasisVars.size();
	}
	else {
		abm_basisVars = bVars;
		abm_nonBasisVars = nbVars;
		abm_varNumber = abm_basisVars.size() + abm_nonBasisVars.size();
	}

	simplexTable = sTable;
}

void MainWindow::set_basis_vars(const QVector<int>& bVars) {
	if(abm_stepNumber == -1)
		basisVars = bVars;
	else
		abm_basisVars = bVars;
}

void MainWindow::set_nonbasis_vars(const QVector<int>& nbVars) { nonBasisVars = nbVars; }

void MainWindow::set_simplex_table(const QVector<QVector<Fraction>>& sTable,
	bool needUpdate/* = false*/)
{
	simplexTable = sTable;

	if(needUpdate)
		update_simplex_table();
}

void MainWindow::set_system_invalid() {
	clear_main_layout();

	display->setText(QString("<h3>Некорректная система: невозможно ") +
		QString("выразить выбранные базисные переменные!</h3>"));
	display->setStyleSheet("color: red;");
	display->show();

	systemInvalid = true;
}

int MainWindow::get_var_number() {
	if(abm_stepNumber == -1)
		return varNumber;
	else
		return abm_varNumber;
}

QVector<int> MainWindow::get_basis_vars() {
	if(abm_stepNumber == -1)
		return basisVars;
	else
		return abm_basisVars;
}

QVector<int> MainWindow::get_nonbasis_vars() {
	if(abm_stepNumber == -1)
		return nonBasisVars;
	else
		return abm_nonBasisVars;
}

QVector<Fraction> MainWindow::get_function() {
	if(abm_stepNumber == -1)
		return function;
	else
		return abm_function;
}

QVector<QVector<Fraction>> MainWindow::get_equalities_matrix() {
	if(abm_stepNumber == -1)
		return equalitiesMatrix;
	else
		return abm_equalitiesMatrix;
}

void MainWindow::set_info(const QString& info) {
	display->setText("Info: " + info);
}

void MainWindow::new_system() {
	setupWindow->reset();
	setupWindow->show();
}

void MainWindow::restore_system() {
	systemInvalid = false;

	QString openFileName = QFileDialog::getOpenFileName(this, "Открытие", "", "*.txt");

	if(!openFileName.isEmpty()) {
		int vNumber;
		QVector<int> bVars, nbVars;
		QVector<Fraction> func;
		QVector<QVector<Fraction>> eMatrix;

		if(SystemSaver::restore_system(openFileName.toStdString(),
			vNumber, bVars, nbVars, func, eMatrix))
		{
			editSystemActn->setEnabled(true);
			saveSystemActn->setEnabled(true);

			display->hide();
			display->setText("");
			display->setStyleSheet("color: black;");

			clear_main_layout();

			varNumber = vNumber;
			basisVars = bVars;
			nonBasisVars = nbVars;
			function = func;
			equalitiesMatrix = eMatrix;

			setupWindow->set_system(varNumber, basisVars, function, equalitiesMatrix);

			stepNumber = 0;
			abm_stepNumber = -1;

			emit simplex_table_request();

			update_components();
		}
	}
}

void MainWindow::save_system() {
	QString saveFileName = QFileDialog::getSaveFileName(this, "Сохранение", "", "*.txt");

	if(!saveFileName.isEmpty()) {
		SystemSaver::save_system(saveFileName.toStdString(), varNumber,
			basisVars, nonBasisVars, function, equalitiesMatrix);
	}
}

void MainWindow::edit_system() {
	setupWindow->show();
}

void MainWindow::show_help() {
	// TODO
}

void MainWindow::setup_completed() {
	systemInvalid = false;

	setupWindow->hide();

	editSystemActn->setEnabled(true);
	saveSystemActn->setEnabled(true);

	clear_main_layout();

	stepNumber = 0;
	abm_stepNumber = -1;

	display->hide();
	display->setText("");
	display->setStyleSheet("color: black;");

	varNumber = setupWindow->get_var_number();
	basisVars = setupWindow->get_basis_vars();
	function = setupWindow->get_function();
	equalitiesMatrix = setupWindow->get_equalities_matrix();

	nonBasisVars.clear();
	for(int i = 1; i <= varNumber; ++i)
		if(basisVars.indexOf(i) == -1)
			nonBasisVars.push_back(i);

	emit simplex_table_request();

	update_components();
}

void MainWindow::setup_canceled() {
	setupWindow->hide();
}

void MainWindow::update_components() {
	if(systemInvalid)
		return;

	QVector<int> bVars, nbVars;
	if(abm_stepNumber == -1) {
		bVars = basisVars;
		nbVars = nonBasisVars;

		addArtBasisActn->setEnabled(true);
		runArtBasisMethodActn->setEnabled(true);
		findSolutionActn->setEnabled(true);
	}
	else {
		bVars = abm_basisVars;
		nbVars = abm_nonBasisVars;

		addArtBasisActn->setEnabled(false);
		runArtBasisMethodActn->setEnabled(false);
		findSolutionActn->setEnabled(false);
	}

	if(secureMode)
		update_avail_elements_matrix();

	if(stepNumber > 0)
		backButton->setEnabled(true);
	else
		backButton->setEnabled(false);

	forwardButton->setEnabled(false);

	mainLayout->addWidget(display, 0, startColPos, 1, nbVars.size() + 2);
	mainLayout->addWidget(backButton, startRowPos, startColPos);
	mainLayout->addWidget(forwardButton, startRowPos, startColPos +
		nbVars.size() + 1);

	backButton->show();
	forwardButton->show();

	vButtons = create_buttons(bVars);
	hButtons = create_buttons(nbVars);

	for(int i = 0; i < bVars.size(); ++i)
		mainLayout->addWidget(vButtons[i], startRowPos + i + 1, startColPos);

	for(int j = 0; j < nbVars.size(); ++j)
		mainLayout->addWidget(hButtons[j], startRowPos, startColPos + j + 1);

	QLabel *pLabel;
	QString centerStyle = "background-color: azure;",
		basisVarsValuesStyle = "background-color: azure;",
		functionCoefficientsStyle = "background-color: mintcream;",
		functionValueStyle = "background-color: crimson;",
		availElementsStyle = "background-color: lightgreen;";
	for(int i = 0; i < simplexTable.size(); ++i) {
		for(int j = 0; j < simplexTable[i].size(); ++j) {
			pLabel = new QLabel(simplexTable[i][j].toQString());
			pLabel->setMinimumSize(minWidth, minHeight);
			pLabel->setAlignment(Qt::AlignCenter);
			pLabel->setFont(wFont);
			pLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
			if(i == simplexTable.size() - 1) {
				if(j != simplexTable[i].size() - 1)
					pLabel->setStyleSheet(functionCoefficientsStyle);
				else
					pLabel->setStyleSheet(functionValueStyle);
			}
			else if(i != simplexTable.size() - 1 && j == simplexTable[i].size() - 1)
				pLabel->setStyleSheet(basisVarsValuesStyle);
			else {
				if(secureMode && availElementsMatrix[i][j])
					pLabel->setStyleSheet(availElementsStyle);
				else
					pLabel->setStyleSheet(centerStyle);
			}
			mainLayout->addWidget(pLabel, startRowPos + i + 1, startColPos + j + 1);
		}
	}
}

void MainWindow::clear_main_layout() {
	QVector<int> bVars, nbVars;
	if(abm_stepNumber == -1) {
		bVars = basisVars;
		nbVars = nonBasisVars;
	}
	else {
		bVars = abm_basisVars;
		nbVars = abm_nonBasisVars;
	}

	remove_display();
	remove_control_buttons();

	QLayoutItem *child;
	for(int i = 0; i <= bVars.size() + 1; ++i) {
		for(int j = 0; j <= nbVars.size() + 1; ++j) {
			child = mainLayout->itemAtPosition(startRowPos + i, startColPos + j);
			if(child != nullptr) {
				mainLayout->removeItem(child);
				delete child->widget();
				delete child;
			}
		}
	}
}

void MainWindow::update_simplex_table() {
	if(systemInvalid)
		return;

	QLayoutItem *child;
	for(int i = 0; i < simplexTable.size(); ++i) {
		for(int j = 0; j < simplexTable[i].size(); ++j) {
			child = mainLayout->itemAtPosition(startRowPos + i + 1, startColPos + j + 1);
			if(child != nullptr) {
				qobject_cast<QLabel *>(child->widget())->setText(simplexTable[i][j].toQString());
			}
		}
	}
}

void MainWindow::update_avail_elements_matrix() {
	for(auto &row : availElementsMatrix)
		row.clear();
	availElementsMatrix.clear();

	int rowNumber, colNumber, lastRowIndex = simplexTable.size() - 1;
	Fraction temp, minValue;
	QVector<int> bVars, nbVars, indices;

	if(abm_stepNumber == -1) {
		bVars = basisVars;
		nbVars = nonBasisVars;
	}
	else {
		bVars = abm_basisVars;
		nbVars = abm_nonBasisVars;
	}

	for(int i = 0; i < bVars.size(); ++i)
		availElementsMatrix.push_back({});
	for(auto &row : availElementsMatrix)
		row.fill(false, nbVars.size());

	for(int j = 0; j < nbVars.size(); ++j)
		if(simplexTable[simplexTable.size() - 1][j] < 0)
			indices.push_back(j);

	// Среди элементов подходящих столбцов ищем подходящие опорные элемент 
	for(const auto &i : indices) {
		colNumber = i;
		// Ищем первый положительный элемент в таблице, чтобы взять значение
		// (константа_строки/значение_элемента) за минимум для дальнейших сравнений.
		for(rowNumber = 0; (rowNumber < lastRowIndex) &&
			(simplexTable[rowNumber][colNumber] <= 0); ++rowNumber) { }

		if(rowNumber == lastRowIndex)
			continue;

		minValue = simplexTable[rowNumber][simplexTable[rowNumber].size() - 1] /
			simplexTable[rowNumber][colNumber];

		// Ищем элемент, чье соотношение (константа_строки/значение_элемента)
		// будет минимальным по столбку с номером colNumber.
		for(int k = rowNumber + 1; k < lastRowIndex; ++k) {
			if(simplexTable[k][colNumber] <= 0)
				continue;

			temp = simplexTable[k][simplexTable[k].size() - 1] / simplexTable[k][colNumber];
			if(temp < minValue) {
				minValue = temp;
				rowNumber = k;
			}

		} // for

		// Все элементы с минимальным значением отношения (константа_строки/значение_элемента)
		// помечаем в матрице availElementsMatrix как допустимые.
		for(int k = 0; k < lastRowIndex; ++k) {
			if(simplexTable[k][colNumber] > 0) {
				temp = simplexTable[k][simplexTable[k].size() - 1] / simplexTable[k][colNumber];
				if(temp == minValue)
					availElementsMatrix[k][colNumber] = true;
			}
		}
	} // for
}

void MainWindow::remove_display() {
	QVector<int> nbVars;
	if(abm_stepNumber == -1)
		nbVars = nonBasisVars;
	else
		nbVars = abm_nonBasisVars;

	QLayoutItem *child;
	child = mainLayout->itemAtPosition(0, startColPos);
	if(child != nullptr) {
		mainLayout->removeItem(child);
		delete child;
	}

	display->hide();
}

void MainWindow::remove_control_buttons() {
	QVector<int> nbVars;
	if(abm_stepNumber == -1)
		nbVars = nonBasisVars;
	else
		nbVars = abm_nonBasisVars;

	QLayoutItem *child;
	child = mainLayout->itemAtPosition(startRowPos, startColPos);
	if(child != nullptr) {
		mainLayout->removeItem(child);
		delete child;
	}
	child = mainLayout->itemAtPosition(startRowPos, startColPos +
		nbVars.size() + 1);
	if(child != nullptr) {
		mainLayout->removeItem(child);
		delete child;
	}

	backButton->hide();
	forwardButton->hide();
}

void MainWindow::pin_warning_message(const QString& message) {
	display->setText(QString("<h3>") + message + QString("</h3>"));
	display->setStyleSheet("color: red;");
	display->show();

	systemInvalid = true;
}

void MainWindow::mw_button_toggled(bool btnOn) {
	int btnNumber = -1;

	QPushButton *pButton, *pToggledButton = qobject_cast<QPushButton *>(sender());
	QString objName = pToggledButton->objectName();

	// Поиск в кнопках базисных переменных вызвавшую кнопку
	for(int i = 0; i < vButtons.size(); ++i) {
		pButton = qobject_cast<QPushButton *>(mainLayout->itemAtPosition(startRowPos + i + 1,
			startColPos)->widget());
		if(pButton == pToggledButton) {
			btnNumber = i;
			break;
		}
	}

	// Если кнопка была найдена в кнопках базисных переменных
	if(btnNumber != -1) {
		if(btnOn) {
			for(auto btn : vButtons)
				if(btn != pToggledButton)
					btn->setEnabled(false);

			checkedVBtnNumber = btnNumber;
		}
		else {
			for(auto btn : vButtons)
				if(btn != pToggledButton)
					btn->setEnabled(true);

			checkedVBtnNumber = -1;
		}

		if(checkedVBtnNumber != -1 && checkedHBtnNumber != -1)
			forwardButton->setEnabled(true);
		else if(forwardButton->isEnabled())
			forwardButton->setEnabled(false);

		return;
	}

	// Поиск в кнопках небазисных переменных вызвавшую кнопку
	for(int i = 0; i < hButtons.size(); ++i) {
		pButton = qobject_cast<QPushButton *>(mainLayout->itemAtPosition(startRowPos,
			startColPos + i + 1)->widget());
		if(pButton == pToggledButton) {
			btnNumber = i;
			break;
		}
	}

	// Если кнопка была найдена в кнопках небазисных переменных
	if(btnNumber != -1) {
		if(btnOn) {
			for(auto btn : hButtons)
				if(btn != pToggledButton)
					btn->setEnabled(false);

			checkedHBtnNumber = btnNumber;
		}
		else {
			for(auto btn : hButtons)
				if(btn != pToggledButton)
					btn->setEnabled(true);

			checkedHBtnNumber = -1;
		}

		if(checkedVBtnNumber != -1 && checkedHBtnNumber != -1)
			forwardButton->setEnabled(true);
		else if(forwardButton->isEnabled())
			forwardButton->setEnabled(false);

		return;
	}
}

void MainWindow::__step_back() {
	if(stepNumber > 0) {
		display->hide();
		display->setText("");
		display->setStyleSheet("color: black;");

		clear_main_layout();

		--stepNumber;

		if(abm_stepNumber == 0) {
			abm_varNumber = 0;
			abm_basisVars.clear();
			abm_nonBasisVars.clear();
			abm_function.clear();
			for(auto &row : abm_equalitiesMatrix)
				row.clear();
			abm_equalitiesMatrix.clear();

			abm_stepNumber = -1;

			emit abm_canceled();

			update_components();

			if(!addArtBasisActn->isEnabled())
				addArtBasisActn->setEnabled(true);
			if(!runArtBasisMethodActn->isEnabled())
				runArtBasisMethodActn->setEnabled(true);

			return;
		}

		if(abm_stepNumber > 0) {
			abm_SystemSaver->restore_system(abm_varNumber, abm_basisVars,
				abm_nonBasisVars, abm_function, abm_equalitiesMatrix);

			--abm_stepNumber;
		}

		emit step_back();

		update_components();
	}
}

void MainWindow::__step_forward() {
	if(checkedVBtnNumber != -1 && checkedHBtnNumber != -1) {
		if(secureMode) {
			if(availElementsMatrix[checkedVBtnNumber][checkedHBtnNumber] == false) {
				display->setStyleSheet("color: red;");
				display->setText("<h3>Выбран неверный опорный элемент!</h3>");
				display->show();
				return;
			}
		}

		++stepNumber;

		clear_main_layout();

		// Если метод искусственного базиса не был применен
		if(abm_stepNumber == -1) {
			emit step_forward(checkedVBtnNumber, checkedHBtnNumber, false);
		}
		// Если метод искусственного базиса был применен
		else {
			++abm_stepNumber;

			abm_SystemSaver->save_system(abm_varNumber, abm_basisVars,
				abm_nonBasisVars, abm_function, abm_equalitiesMatrix);

			// Если выбранная базисная переменная была введена по методу
			// искусственного базиса
			if(abm_basisVars[checkedVBtnNumber] > varNumber) {
				artBasisVars.removeOne(abm_basisVars[checkedVBtnNumber]);

				// Если на выполняемом шаге из базиса удаляется последняя переменная
				// искусственного базиса
				if(artBasisVars.empty()) {
					// Корректируем векторы базисных и небазисных переменных
					// для задачи, к которой был применен МИБ
					abm_basisVars.removeAt(checkedVBtnNumber);
					abm_basisVars.push_back(abm_nonBasisVars[checkedHBtnNumber]);
					abm_nonBasisVars.removeAt(checkedHBtnNumber);

					// Переопределяем векторы базисных и небазисных переменных
					// для ИСХОДНОЙ задачи
					basisVars = abm_basisVars;
					nonBasisVars = abm_nonBasisVars;

					// Заправшиваем последнюю симплекс-таблицу для системы, к которой
					// применен метод искусственного базиса
					emit abm_last_step(checkedVBtnNumber, checkedHBtnNumber, true);

					abm_varNumber = 0;
					abm_basisVars.clear();
					abm_nonBasisVars.clear();
					abm_function.clear();
					for(auto &row : abm_equalitiesMatrix)
						row.clear();
					abm_equalitiesMatrix.clear();

					abm_stepNumber = -1;

					// Согласно алгоритму МИБ проверяем значение функции результирующей
					// симплекс-таблицы
					auto lastRowIndex = simplexTable.size() - 1;
					auto lastColIndex = simplexTable[lastRowIndex].size() - 1;
					if(simplexTable[lastRowIndex][lastColIndex] != 0) {
						set_no_solution();
						return;
					}

					// Запрашиваем симплекс-таблицу для ИСХОДНОЙ задачи
					emit abm_completed();

					stepNumber = 0;
				}
				// Если на выполняемом шаге из базиса удаляется НЕ последняя переменная
				// искусственного базиса
				else {
					emit step_forward(checkedVBtnNumber, checkedHBtnNumber, true);

					abm_function.removeAt(checkedHBtnNumber);
					for(auto &row : abm_equalitiesMatrix)
						row.removeAt(checkedHBtnNumber);
				}
			}
			// Если выбранная базисная переменная была определена исходной задачей
			else
				emit step_forward(checkedVBtnNumber, checkedHBtnNumber, false);
		}

		update_components();
	}
}

void MainWindow::add_art_basis() {
	systemInvalid = false;

	display->hide();
	display->setText("");
	display->setStyleSheet("color: black;");

	if(addArtBasisActn->isEnabled())
		addArtBasisActn->setEnabled(false);
	if(runArtBasisMethodActn->isEnabled())
		runArtBasisMethodActn->setEnabled(false);

	clear_main_layout();

	QFile(QString::fromStdString(abmSaveFileName)).remove();

	int eCount = equalitiesMatrix.size();

	// ~~ Добавляем переменные искусственного базиса
	artBasisVars.clear();
	for(int i = 0; i < eCount; ++i)
		artBasisVars.push_back(varNumber + i + 1);

	// ~~ Формируем данные для системы, к которой применен МИБ
	abm_varNumber = varNumber + eCount;
	abm_basisVars = artBasisVars;
	abm_nonBasisVars = basisVars;
	abm_nonBasisVars.append(nonBasisVars);
	sort(abm_nonBasisVars.begin(), abm_nonBasisVars.end());
	abm_function.clear();
	for(int i = 0; i < abm_varNumber; ++i) {
		if(i < varNumber)
			abm_function.push_back(0);
		else
			abm_function.push_back(1);
	}
	abm_equalitiesMatrix = equalitiesMatrix;
	// Домножаем на -1 ограничения, в которых константа < 0
	for(int i = 0; i < eCount; ++i)
		if(abm_equalitiesMatrix[i][varNumber] < 0)
			for(auto &val : abm_equalitiesMatrix[i])
				val *= -1;
	// Добавляем переменные искусственного базиса в ограничения
	for(int i = 0; i < eCount; ++i) {
		for(int j = 0; j < eCount; ++j) {
			if(i == j)
				abm_equalitiesMatrix[i].insert(varNumber + j, 1);
			else
				abm_equalitiesMatrix[i].insert(varNumber + j, 0);
		}
	}

	abm_SystemSaver->reset();

	abm_stepNumber = 0;
	++stepNumber;

	emit abm_simplex_table_request();

	update_components();

	if(!backButton->isEnabled())
		backButton->setEnabled(true);
}

void MainWindow::run_art_basis_method() {
	clear_main_layout();

	int eCount = equalitiesMatrix.size();

	artBasisVars.clear();
	for(int i = 0; i < eCount; ++i)
		artBasisVars.push_back(varNumber + i + 1);

	abm_varNumber = varNumber + eCount;
	abm_basisVars = artBasisVars;
	abm_nonBasisVars = basisVars;
	abm_nonBasisVars.append(nonBasisVars);
	sort(abm_nonBasisVars.begin(), abm_nonBasisVars.end());
	abm_function.clear();
	for(int i = 0; i < abm_varNumber; ++i) {
		if(i < varNumber)
			abm_function.push_back(0);
		else
			abm_function.push_back(1);
	}
	abm_equalitiesMatrix = equalitiesMatrix;
	for(int i = 0; i < eCount; ++i) {
		for(int j = 0; j < eCount; ++j) {
			if(i == j)
				abm_equalitiesMatrix[i].insert(varNumber + j, 1);
			else
				abm_equalitiesMatrix[i].insert(varNumber + j, 0);
		}
	}

	++stepNumber;

	emit run_abm_method(abm_varNumber, abm_basisVars,
		abm_nonBasisVars, abm_function, abm_equalitiesMatrix);

	abm_varNumber = 0;
	abm_basisVars.clear();
	abm_nonBasisVars.clear();
	abm_function.clear();
	for(auto &row : abm_equalitiesMatrix)
		row.clear();
	abm_equalitiesMatrix.clear();

	update_components();

	if(!backButton->isEnabled())
		backButton->setEnabled(true);
}

QVector<QPushButton *> MainWindow::create_buttons(const QVector<int>& indices) {
	QVector<QPushButton *> buttons;
	QPushButton *pButton;
	QString style = (QString)"QPushButton { background-color: peachpuff; border: none; }" +
		"QPushButton:checked { background-color: sandybrown; }" +
		"QPushButton:hover { background-color: sandybrown; }";

	for(auto index : indices) {
		pButton = new QPushButton("X" + QString::number(index));
		pButton->setObjectName("mw" + QString::number(index));
		pButton->setMinimumSize(minWidth, minHeight);
		pButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
		pButton->setStyleSheet(style);
		pButton->setCheckable(true);
		pButton->setChecked(false);

		connect(pButton, SIGNAL(toggled(bool)), SLOT(mw_button_toggled(bool)));

		buttons.push_back(pButton);
	}

	return buttons;
}

void MainWindow::find_solution() {
	clear_main_layout();

	emit solution_request();

	++stepNumber;
	
	update_components();
}