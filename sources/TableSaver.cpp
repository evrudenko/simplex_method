#include "..\headers\TableSaver.hpp"

TableSaver::TableSaver() {
	reset();
}

void TableSaver::reset() {
	QFile(QString::fromStdString(saveFileName)).remove();
	tableCount = 0;
}

void TableSaver::save(const QVector<QVector<Fraction>> &table, const QVector<int> &basisVars,
	const QVector<int> &nonBasisVars)
{
	string strTable = pack_table(table, basisVars, nonBasisVars, tableCount++);

	ofstream fout(saveFileName, ios::app);

	fout << strTable;

	fout.close();
}

bool TableSaver::restore(QVector<QVector<Fraction>> &table, QVector<int> &basisVars,
	QVector<int> &nonBasisVars, int tableNumber/* = -1*/)
{
	if(tableNumber < 0)
		tableNumber = tableCount - 1;

	ifstream fin(saveFileName);

	if(!fin.is_open())
		return false;

	string buffer;
	stringstream ss;
	int iVal, count, tNumber = -1, rowCount = -1, colCount = -1, tCount = 0;
	Fraction dVal;
	string strTables = "";
	QVector<int> bVars, nbVars;
	QVector<QVector<Fraction>> tableBuffer;

	while(tNumber != tableNumber && getline(fin, buffer)) {
		// Чтение данных о таблице
		ss.clear();
		ss.str(buffer);

		ss >> tNumber >> rowCount >> colCount;

		// Чтение вектора базисных индексов
		if(!getline(fin, buffer)) {
			fin.close();
			return false;
		}

		ss.clear();
		ss.str(buffer);

		bVars.clear();
		while(ss >> iVal)
			bVars.push_back(iVal);

		// Чтение вектора небазисных индексов
		if(!getline(fin, buffer)) {
			fin.close();
			return false;
		}

		ss.clear();
		ss.str(buffer);

		nbVars.clear();
		while(ss >> iVal)
			nbVars.push_back(iVal);

		// Чтение симплекс таблицы
		for(auto row : tableBuffer)
			row.clear();
		tableBuffer.clear();

		for(int i = 0; i < rowCount; ++i)
			tableBuffer.push_back({});

		for(int i = 0; i < rowCount; ++i) {
			if(getline(fin, buffer)) {
				count = 0;

				ss.clear();
				ss.str(buffer);
				while(ss >> dVal) {
					tableBuffer[i].push_back(dVal);
					++count;
				}

				if(count != colCount) {
					fin.close();
					return false;
				}
			}
			else {
				fin.close();
				return false;
			}
		}

		// Дочитываем пустую строку 
		getline(fin, buffer);

		if(tNumber <= tableNumber) {
			strTables += pack_table(tableBuffer, bVars, nbVars, tNumber);
			++tCount;
		}
	}

	fin.close();

	save(strTables);
	tableCount = tCount;

	// Копируем параметры
	basisVars = bVars;
	nonBasisVars = nbVars;
	table = tableBuffer;

	return true;
}

string TableSaver::pack_table(const QVector<QVector<Fraction>> &table,
	const QVector<int> &basisVars, const QVector<int> &nonBasisVars, int tableNumber)
{
	string res = "";

	if(table.size() > 0) {
		res += to_string(tableNumber) + " " + to_string(table.size()) + " " +
			to_string(table[0].size()) + "\n";

		for(const auto &var : basisVars)
			res += to_string(var) + " ";
		res += "\n";

		for(const auto &var : nonBasisVars)
			res += to_string(var) + " ";
		res += "\n";

		for(const auto &row : table) {
			for(const auto &val : row)
				res += to_string(val) + " ";
			res += "\n";
		}

		res += "\n";
	}

	return res;
}

/*inline */void TableSaver::save(const string &strTables) {
	ofstream fout(saveFileName);

	fout << strTables;

	fout.close();
}