#include "..\headers\SystemSaver.hpp"

SystemSaver::SystemSaver() { }

/*static */void SystemSaver::save_system(const string &filename,
	const int &varNumber, const QVector<int> &basisVars, const QVector<Fraction> &function,
	const QVector<QVector<Fraction>> &equalitiesMatrix)
{
	ofstream fout(filename);

	fout << varNumber << endl;

	fout << basisVars.size() << " ";
	for(auto val : basisVars)
		fout << val << " ";
	fout << endl;

	for(auto val : function)
		fout << val << " ";
	fout << endl;

	fout << equalitiesMatrix.size() << endl;

	for(auto row : equalitiesMatrix) {
		for(auto val : row)
			fout << val << " ";
		fout << endl;
	}

	fout.close();
}

/*static */void SystemSaver::save_system(const string &filename, const int &varNumber,
	const QVector<int> &basisVars, const QVector<int> &nonBasisVars,
	const QVector<Fraction> &function, const QVector<QVector<Fraction>> &equalitiesMatrix)
{
	ofstream fout(filename);

	fout << varNumber << endl;

	fout << basisVars.size() << " ";
	for(auto val : basisVars)
		fout << val << " ";
	fout << endl;

	fout << nonBasisVars.size() << " ";
	for(auto val : nonBasisVars)
		fout << val << " ";
	fout << endl;

	for(auto val : function)
		fout << val << " ";
	fout << endl;

	fout << equalitiesMatrix.size() << endl;

	for(auto row : equalitiesMatrix) {
		for(auto val : row)
			fout << val << " ";
		fout << endl;
	}

	fout.close();
}

/*static */bool SystemSaver::restore_system(const string &filename, int &varNumber,
	QVector<int> &basisVars, QVector<Fraction> &function, QVector<QVector<Fraction>> &equalitiesMatrix)
{
	ifstream fin(filename);

	if(!fin.is_open())
		return false;

	string line, data = "";
	while(getline(fin, line))
		data += line + "\n";

	fin.close();

	return __restore_system(data, varNumber, basisVars, function, equalitiesMatrix);
}

/*static */bool SystemSaver::restore_system(const string &filename, int &varNumber,
	QVector<int> &basisVars, QVector<int> &nonBasisVars, QVector<Fraction> &function,
	QVector<QVector<Fraction>> &equalitiesMatrix)
{
	ifstream fin(filename);

	if(!fin.is_open())
		return false;

	string line, data = "";
	while(getline(fin, line))
		data += line + "\n";

	fin.close();

	return __restore_system(data, varNumber, basisVars, nonBasisVars, function, equalitiesMatrix);
}

/*static */QLinkedList<string> SystemSaver::split_lines(string data) {
	string::size_type pos;
	QLinkedList<string> res;

	while((pos = data.find("\n")) != string::npos) {
		res.push_back(data.substr(0, pos));
		data.erase(0, pos + 1);
	}

	return res;
}

/*static */bool SystemSaver::__restore_system(const string &data, int &varNumber,
	QVector<int> &basisVars, QVector<Fraction> &function, QVector<QVector<Fraction>> &equalitiesMatrix)
{
	int vNumber, intVBuffer, bVarsCount, eMatrixSize, rowCount, colCount;
	Fraction FractionVBuffer;
	string line;
	stringstream ss;
	QVector<int> bVars;
	QVector<Fraction> func;
	QVector<QVector<Fraction>> eMatrix;

	QLinkedList<string> lines = split_lines(data);

	// Получение сохраненного значения varNumber:
	if(!lines.empty())
		line = lines.takeFirst();
	else { return false; }

	ss.str(line);
	ss >> vNumber;

	ss.clear();

	// Получение сохраненных индексов базисных переменных:
	if(!lines.empty())
		line = lines.takeFirst();
	else { return false; }

	ss.str(line);
	ss >> bVarsCount;

	for(colCount = 0; colCount < bVarsCount; ++colCount) {
		if(ss >> intVBuffer) {
			if(intVBuffer > vNumber)
				return false;
			
			bVars.push_back(intVBuffer);
		}
	}

	if(colCount != bVarsCount)
		return false;

	ss.clear();

	// Получение сохраненных коэффициентов оптимизируемой функции:
	colCount = 0;
	if(!lines.empty())
		line = lines.takeFirst();
	else { return false; }

	ss.str(line);
	for(colCount = 0; colCount <= vNumber && ss >> FractionVBuffer; ++colCount)
		func.push_back(FractionVBuffer);

	if(colCount != vNumber)
		return false;

	ss.clear();

	// Получение сохраненных коэффициентов функций ограничения:
	if(!lines.empty())
		line = lines.takeFirst();
	else { return false; }

	ss.str(line);
	ss >> eMatrixSize;

	ss.clear();

	for(int i = 0; i < eMatrixSize; ++i)
		eMatrix.push_back({});

	rowCount = 0;
	while(!lines.empty()) {
		line = lines.takeFirst();
		ss.str(line);
		for(colCount = 0; (colCount <= vNumber + 1) && (ss >> FractionVBuffer); ++colCount)
			if(rowCount < eMatrixSize)
				eMatrix[rowCount].push_back(FractionVBuffer);

		if(colCount != vNumber + 1)
			return false;

		ss.clear();
		++rowCount;
	}

	if(bVarsCount > eMatrix.size())
		return false;

	varNumber = vNumber;
	basisVars = bVars;
	function = func;
	equalitiesMatrix = eMatrix;

	return true;
}

/*static */bool SystemSaver::__restore_system(const string &data, int &varNumber,
	QVector<int> &basisVars, QVector<int> &nonBasisVars, QVector<Fraction> &function,
	QVector<QVector<Fraction>> &equalitiesMatrix)
{
	QLinkedList<string> lines = split_lines(data);

	int count, nbVarsCount, iValue;
	string line, _data = "";
	stringstream ss;
	QVector<int> nbVars;

	// Пропускаем первые 2 строки
	line = lines.takeFirst();
	_data += line + "\n";
	line = lines.takeFirst();
	_data += line + "\n";

	// Достаем вектор nonBasisVars (полученная ниже строка line не добавляется в _data)
	if(!lines.empty()) {
		line = lines.takeFirst();
		ss.str(line);
		ss >> nbVarsCount;

		for(count = 0; count < nbVarsCount; ++count) {
			if(ss >> iValue)
				nbVars.push_back(iValue);
		}

		ss.clear();

		if(count != nbVarsCount)
			return false;
	}
	else
		return false;

	// Оставшиеся данные оставляем без изменений
	for(auto line : lines)
		_data += line + "\n";

	// Отправляем данные с вырезанной строкой значений nonBasisVars
	// на дальнейшую обработку
	if(!__restore_system(_data, varNumber, basisVars, function, equalitiesMatrix))
		return false;

	nonBasisVars = nbVars;

	return true;
}