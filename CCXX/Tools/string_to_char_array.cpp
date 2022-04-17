#include <iostream>
#include <fstream>
#include <string>

using namespace std;

int main()
{
	string strLine;
	ifstream fIn("in.txt");
	if (!fIn) {
		cout << "file open error..." << endl;
		goto EXIT;
	}

	while (getline(fIn, strLine))
	{
		string strConverted = "char sz";
		strConverted.append(strLine).append("[] = {");

		for (int i = 0; i < strLine.size(); i++, strConverted.append(", "))
		{
			strConverted.append("'");
			strConverted.push_back(strLine.at(i));
			strConverted.append("'");
		}

		strConverted.append("0x00};");
		cout << strConverted << endl;
	}

	fIn.close();
EXIT:
	system("pause");
	return 0;
}