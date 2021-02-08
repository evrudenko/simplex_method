#include "..\headers\Optimizer.hpp"

Optimizer::Optimizer() : setupSuccessFlag(false), shuffled(false) { }

bool Optimizer::set_system(int varNumber, QVector<int> bVars, QVector<int> nbVars,
	QVector<Fraction> func, QVector<QVector<Fraction>> eMatrix)
{
	// ~ Сброс параметров:
	setupSuccessFlag = false;
	shuffled = false;
	abmCompleted = false;
	basisVars.clear();
	nonBasisVars = nbVars;
	for(auto v : simplexTable)
		v.clear();
	simplexTable.clear();

	// Сохранение параметров
	function = func;
	equalitiesMatrix = eMatrix;

	setupSuccessFlag = create_simplex_table(bVars, func, eMatrix, simplexTable);

	basisVars = bVars;

	return setupSuccessFlag;
}

bool Optimizer::create_simplex_table(QVector<int> &bVars, QVector<Fraction> func,
	QVector<QVector<Fraction>> eMatrix, QVector<QVector<Fraction>> &sTable)
{
	for(auto &row : sTable)
		row.clear();
	sTable.clear();

	// Анализ полученной системы:
	QVector<int> avail, tbVars, chain(bVars.size(), -1);
	QVector<QVector<int>> availFor;
	for(int i = 0; i < bVars.size(); ++i)
		availFor.push_back({});

	// Определение безусловно доступных ограничений для каждой базисной переменной
	// (независимо от других переменных):
	for(int i = 0; i < bVars.size(); ++i)
		for(int j = 0; j < eMatrix.size(); ++j)
			if(eMatrix[j][bVars[i] - 1] != 0)
				availFor[i].push_back(j);

	// Определение условно доступных ограничений для каждой базисной переменной (с учетом
	// условия, что каждой базисной переменной должно соответствовать одно ограничение).
	for(int i = 0; i < availFor.size(); ++i) {
		avail = availFor[i];
		for(int j = i - 1; j >= 0; --j)
			avail.removeOne(chain[j]);

		if(avail.size() == 0) {
			if(i == 0) { return false; }
			else {
				availFor[i - 1].removeOne(chain[i - 1]);
				chain[i - 1] = -1;
				i -= 2;
				continue;
			}
		}

		chain[i] = avail[0];
	}

	for(int i = 0; i < chain.size(); ++i)
		tbVars.push_back(bVars[chain.indexOf(i)]);

	// Выражение базиса.
	// В каждом ограничении обнуляем коэффициенты у базисных переменных, выраженных не через них:
	Fraction tempFactor;
	for(int i = 0; i < eMatrix.size(); ++i) {
		tempFactor = eMatrix[i][tbVars[i] - 1];
		for(int j = 0; j < eMatrix[i].size(); ++j)
			eMatrix[i][j] /= tempFactor;

		for(int j = i + 1; j < eMatrix.size(); ++j) {
			tempFactor = eMatrix[j][tbVars[i] - 1];
			for(int k = 0; k < eMatrix[j].size(); ++k)
				eMatrix[j][k] -= eMatrix[i][k] * tempFactor;
		}
	}

	for(int i = eMatrix.size() - 1; i >= 0; --i) {
		for(int j = i - 1; j >= 0; --j) {
			tempFactor = eMatrix[j][tbVars[i] - 1];
			for(int k = 0; k < eMatrix[j].size(); ++k)
				eMatrix[j][k] -= eMatrix[i][k] * tempFactor;
		}
	}

	func.push_back(0);
	for(int i = 0; i < eMatrix.size(); ++i) {
		tempFactor = func[tbVars[i] - 1];
		for(int j = 0; j < func.size(); ++j)
			func[j] -= eMatrix[i][j] * tempFactor;
	}

	// Составление симплекс-таблицы:
	for(int i = 0; i <= tbVars.size(); ++i)
		sTable.push_back({});

	for(int i = 0; i < eMatrix.size(); ++i)
		for(int j = 0; j < eMatrix[i].size(); ++j)
			if(tbVars.indexOf(j + 1) == -1)
				sTable[i].push_back(eMatrix[i][j]);

	for(int i = 0; i < func.size(); ++i)
		if(tbVars.indexOf(i + 1) == -1)
			sTable[sTable.size() - 1].push_back(func[i]);

	bVars = tbVars;

	return true;
}

void Optimizer::step_forward(int rowNumber, int colNumber, bool delBasisVar) {
	step_forward(simplexTable, basisVars, nonBasisVars, rowNumber, colNumber, delBasisVar);
}

void Optimizer::step_forward(QVector<QVector<Fraction>> &sTable, QVector<int> &bVars,
	QVector<int> &nbVars, int rowNumber, int colNumber, bool delBasisVar)
{
	Fraction supElement = sTable[rowNumber][colNumber];

	sTable[rowNumber][colNumber] = 1 / supElement;

	for(int i = 0; i < sTable[rowNumber].size(); ++i)
		if(i != colNumber)
			sTable[rowNumber][i] /= supElement;

	for(int i = 0; i < sTable.size(); ++i)
		for(int j = 0; j < sTable[i].size(); ++j)
			if(i != rowNumber && j != colNumber)
				sTable[i][j] -= sTable[i][colNumber] * sTable[rowNumber][j];

	for(int i = 0; i < sTable.size(); ++i)
		if(i != rowNumber)
			sTable[i][colNumber] /= -supElement;

	int temp = bVars[rowNumber];
	bVars[rowNumber] = nbVars[colNumber];
	nbVars[colNumber] = temp;

	if(delBasisVar) {
		for(auto &row : sTable)
			row.removeAt(colNumber);

		nbVars.removeAt(colNumber);
	}
}

void Optimizer::update_state(const QVector<QVector<Fraction>>& sTable,
	const QVector<int>& bVars, const QVector<int>& nbVars)
{
	if(!setupSuccessFlag)
		setupSuccessFlag = true;
	if(shuffled)
		shuffled = false;

	simplexTable = sTable;
	basisVars = bVars;
	nonBasisVars = nbVars;
}

void Optimizer::abm_solve_system() {
	abmCompleted = abm_solve_system(simplexTable, basisVars, nonBasisVars);
}

bool Optimizer::abm_solve_system(QVector<QVector<Fraction>> &sTable,
	QVector<int> &bVars, QVector<int> &nbVars)
{
	int lastRowIndex, lastColIndex, varsCount, rowNumber, colNumber;
	Fraction temp, minValue;
	QVector<int> indices, artBasisVars = bVars;
	while(!artBasisVars.empty()) {
		rowNumber = -1, colNumber = -1;
		indices.clear();
		lastRowIndex = sTable.size() - 1;
		varsCount = sTable[lastRowIndex].size() - 1;

		// Поиск подходящих столбцов для поиска опорного элемента
		for(int i = 0; i < varsCount; ++i)
			if(sTable[lastRowIndex][i] < 0) {
				indices.push_back(i);
			}

		if(indices.empty())
			break;

		// Среди элементов подходящих столбцов ищем опорный элемент 
		for(const auto &i : indices) {
			colNumber = i;
			// Ищем первый положительный элемент в таблице, чтобы взять значение
			// (константа_строки/значение_элемента) за минимум для дальнейших сравнений.
			for(rowNumber = 0; (rowNumber < lastRowIndex) &&
				(sTable[rowNumber][colNumber] <= 0); ++rowNumber) { }

			if(rowNumber == lastRowIndex)
				continue;

			minValue = sTable[rowNumber][sTable[rowNumber].size() - 1] /
				sTable[rowNumber][colNumber];

			// Ищем элемент, чье соотношение (константа_строки/значение_элемента)
			// будет минимальным по столбку с номером colNumber.
			for(int k = rowNumber + 1; k < lastRowIndex; ++k) {
				if(sTable[k][colNumber] <= 0)
					continue;

				temp = sTable[k][sTable[k].size() - 1] / sTable[k][colNumber];
				if(temp < minValue) {
					minValue = temp;
					rowNumber = k;
				}

			} // for

			break;
		} // for

		if(rowNumber != -1 && colNumber != -1) {
			if(artBasisVars.indexOf(bVars[rowNumber]) == -1)
				step_forward(sTable, bVars, nbVars, rowNumber, colNumber, false);
			else {
				artBasisVars.removeOne(bVars[rowNumber]);
				step_forward(sTable, bVars, nbVars, rowNumber, colNumber, true);
			}
			continue;
		}

		break;
	} // while

	lastRowIndex = sTable.size() - 1;
	lastColIndex = sTable[lastRowIndex].size() - 1;
	if(sTable[lastRowIndex][lastColIndex] != 0)
		return false;

	return true;
}

void Optimizer::solve_system() {
	QVector<int> bVars = basisVars, nbVars = nonBasisVars;
	QVector<Fraction> func;
	QVector<QVector<Fraction>> sTable = simplexTable;

	set_system_abm(sTable, bVars, nbVars);
	abmCompleted = abm_solve_system(sTable, bVars, nbVars);

	sTable.pop_back();

	// Вычисляем коэффициенты исходной функции для нового базиса
	func = simplexTable[simplexTable.size() - 1];

	int index;
	for(int i = 0; i < nonBasisVars.size(); ++i) {
		if((index = bVars.indexOf(nonBasisVars[i])) != -1) {
			for(int j = 0; j < func.size(); ++j)
				if(j != i)
					func[j] -= simplexTable[simplexTable.size() - 1][i] * sTable[index][j];
				else
					func[j] *= -sTable[index][j];
		}
	}

	sTable.push_back(func);

	int lastRowIndex, varsCount, rowNumber, colNumber;
	Fraction temp, minValue;
	QVector<int> indices;
	while(!solution_found(sTable)) {
		if(!solution_exists(sTable))
			break;

		rowNumber = -1, colNumber = -1;
		indices.clear();
		lastRowIndex = sTable.size() - 1;
		varsCount = sTable[lastRowIndex].size() - 1;

		// Поиск подходящих столбцов для поиска опорного элемента
		for(int i = 0; i < varsCount; ++i)
			if(sTable[lastRowIndex][i] < 0)
				indices.push_back(i);

		// Среди элементов подходящих столбцов ищем опорный элемент 
		for(const auto &i : indices) {
			colNumber = i;
			// Ищем первый положительный элемент в таблице, чтобы взять значение
			// (константа_строки/значение_элемента) за минимум для дальнейших сравнений.
			for(rowNumber = 0; (rowNumber < lastRowIndex) &&
				(sTable[rowNumber][colNumber] <= 0); ++rowNumber) { }

			minValue = sTable[rowNumber][sTable[rowNumber].size() - 1] /
				sTable[rowNumber][colNumber];

			// Ищем элемент, чье соотношение (константа_строки/значение_элемента)
			// будет минимальным по столбку с номером colNumber.
			for(int k = rowNumber + 1; k < lastRowIndex; ++k) {
				if(sTable[k][colNumber] <= 0)
					continue;

				temp = sTable[k][sTable[k].size() - 1] / sTable[k][colNumber];
				if(temp < minValue) {
					minValue = temp;
					rowNumber = k;
				}

			} // for

			break;
		} // for
		
		if(rowNumber == -1 || colNumber == -1)
			break;

		step_forward(sTable, bVars, nbVars, rowNumber, colNumber, false);
	} // while

	simplexTable = sTable;
	basisVars = bVars;
	nonBasisVars = nbVars;
}

QVector<QVector<Fraction>> Optimizer::get_simplex_table() { return simplexTable; }

bool Optimizer::system_shuffled() { return shuffled; }

bool Optimizer::setup_completed() { return setupSuccessFlag; }

QVector<int> Optimizer::get_basis_vars() { return basisVars; }

QVector<int> Optimizer::get_nonbasis_vars() { return nonBasisVars; }

QString Optimizer::get_info() { return info; }

bool Optimizer::solution_found() {
	return solution_found(simplexTable);
}

bool Optimizer::solution_found(const QVector<QVector<Fraction>> &sTable) {
	auto lastRowIndex = sTable.size() - 1;
	auto varsCount = sTable[lastRowIndex].size() - 1;
	for(int i = 0; i < varsCount; ++i)
		if(sTable[lastRowIndex][i] < 0)
			return false;

	return true;
}

bool Optimizer::solution_exists() {
	return solution_exists(simplexTable);
}

bool Optimizer::solution_exists(const QVector<QVector<Fraction>> &sTable) {
	QVector<int> indices;
	auto lastRowIndex = sTable.size() - 1;
	auto varsCount = sTable[lastRowIndex].size() - 1;
	for(int i = 0; i < varsCount; ++i)
		if(sTable[lastRowIndex][i] < 0)
			indices.push_back(i);

	for(const auto &i : indices)
		for(int j = 0; j <= lastRowIndex; ++j)
			if(sTable[j][i] > 0)
				return true;

	return false;
}

bool Optimizer::abm_avail_point_exists() {
	if(!setupSuccessFlag || !abmCompleted)
		return false;

	auto lastRowIndex = simplexTable.size() - 1;
	auto lastColIndex = simplexTable[lastRowIndex].size() - 1;
	if(solution_found() && simplexTable[lastRowIndex][lastColIndex] == 0)
		return true;
	return false;
}

void Optimizer::set_system_abm(QVector<QVector<Fraction>> &sTable,
	QVector<int> &bVars, QVector<int> &nbVars)
{
	int vNumber = bVars.size() + nbVars.size();
	QVector<int> tbVars, tnbVars;
	QVector<Fraction> abm_function;
	QVector<QVector<Fraction>> tsTable, abm_equalitiesMatrix;

	// Формируем векторы базисных и небазисных
	// переменных для задачи с МИБ
	for(int v = 0; v < bVars.size(); ++v)
		tbVars.push_back(vNumber + v + 1);
	tnbVars = bVars;
	tnbVars.append(nbVars);
	std::sort(tnbVars.begin(), tnbVars.end());

	abm_function.clear();
	for(int i = 0; i < vNumber + bVars.size(); ++i) {
		if(i < vNumber)
			abm_function.push_back(0);
		else
			abm_function.push_back(1);
	}

	abm_equalitiesMatrix = equalitiesMatrix;
	// Домножаем на -1 ограничения, в которых константа < 0
	for(int i = 0; i < abm_equalitiesMatrix.size(); ++i)
		if(abm_equalitiesMatrix[i][abm_equalitiesMatrix[i].size() - 1] < 0)
			for(auto &val : abm_equalitiesMatrix[i])
				val *= -1;
	// Добавляем переменные искусственного базиса в ограничения
	for(int i = 0; i < abm_equalitiesMatrix.size(); ++i) {
		for(int j = 0; j < abm_equalitiesMatrix.size(); ++j) {
			if(i == j)
				abm_equalitiesMatrix[i].insert(vNumber + j, 1);
			else
				abm_equalitiesMatrix[i].insert(vNumber + j, 0);
		}
	}

	create_simplex_table(tbVars, abm_function, abm_equalitiesMatrix, tsTable);

	// Копируем сформированные данные
	sTable = tsTable;
	bVars = tbVars;
	nbVars = tnbVars;
}