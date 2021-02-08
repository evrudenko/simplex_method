#include "..\headers\ABMSystemSaver.hpp"

ABMSystemSaver::ABMSystemSaver() {
	reset();
}

void ABMSystemSaver::reset() {
	QFile(QString::fromStdString(saveFileName)).remove();
	systemsCount = 0;
}

void ABMSystemSaver::save_system(const int &varNumber, const QVector<int> &basisVars,
	const QVector<int> &nonBasisVars, const QVector<Fraction> &function,
	const QVector<QVector<Fraction>> &equalitiesMatrix)
{
	ofstream fout(saveFileName, ios::app);

	fout << (systemsCount++) << endl;

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

	fout << endl;

	fout.close();
}

bool ABMSystemSaver::restore_system(int &varNumber, QVector<int> &basisVars,
	QVector<int> &nonBasisVars, QVector<Fraction> &function,
	QVector<QVector<Fraction>> &equalitiesMatrix)
{
	ifstream fin(saveFileName);

	if(!fin.is_open())
		return false;

	int sNumber = -1, eCount;
	string line, strSystems = "", data = "";
	stringstream ss;

	while(!fin.eof()) {
		getline(fin, line);
		strSystems += line + "\n";
		ss.str(line);
		ss >> sNumber;
		ss.clear();

		if(sNumber != systemsCount - 1) {
			// Пропускаем 3 строки
			for(int i = 0; i < 3; ++i) {
				getline(fin, line);
				strSystems += line + "\n";
			}

			// Достаем количество ограничений
			getline(fin, line);
			strSystems += line + "\n";
			ss.str(line);
			ss >> eCount;
			ss.clear();

			// Пропускаем ограничения
			for(int i = 0; i < eCount; ++i) {
				getline(fin, line);
				strSystems += line + "\n";
			}

			// Пропускаем разделяющую пустую строку
			getline(fin, line);
			strSystems += line + "\n";
		}
		else {
			// Сохраняем в data 3 строки
			for(int i = 0; i < 3; ++i) {
				getline(fin, line);
				data += line + "\n";
			}

			// Достаем количество ограничений
			getline(fin, line);
			ss.str(line);
			ss >> eCount;
			ss.clear();

			// Сохраняем ограничения
			for(int i = 0; i < eCount; ++i) {
				getline(fin, line);
				data += line + "\n";
			}

			break;
		}
	}

	save(strSystems);

	if(sNumber == systemsCount - 1)
		return SystemSaver::__restore_system(data, varNumber, basisVars,
			nonBasisVars, function, equalitiesMatrix);

	return false;
}

void ABMSystemSaver::save(const string &strSystems) {
	ofstream fout(saveFileName);

	fout << strSystems;
	
	fout.close();
}