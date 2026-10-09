#define _CRT_SECURE_NO_WARNINGS
#include<iostream>
#include<string>
#include<sstream>
#include<fstream>
#include<windows.h>
using namespace std;
const char* username = getenv("USERNAME");
static int errors = 0;
void command_prompt()
{
	printf("%s@%s:~$ ", username, username);
}
void ls(string instr)
{
	stringstream ss(instr);
	ss >> instr;
	cout << instr;
	while (ss >> instr)
		cout << ' ' << instr;
	cout << '\n';

}
void cd(string instr)
{
	stringstream ss(instr);
	ss >> instr;
	cout << instr;
	while (ss >> instr)
		cout << ' ' << instr;
	cout << '\n';
}
bool kill(string instr)
{
	int number;
	stringstream ss(instr);
	ss >> instr;
	if (ss >> instr)
	{
		try
		{
			number = stoi(instr);
			cout << "exit code: " << number << '\n';
			return false;
		}
		catch (invalid_argument const& ex)
		{
			cout << "\nbash: exit: ";
			cout << instr;
			while (ss >> instr)
				cout << ' ' << instr;
			cout << " :numeric argument required\n";
			errors++;
			return false;
		}
	}
	return false;
}
void not_found(string instr)
{
	stringstream ss(instr);
	ss >> instr;
	cout << "bash: " << instr;
	while (ss >> instr)
		cout << ' ' << instr;
	cout << ": command not found\n";
	errors++;
}

bool execute(string command)
{
	string inst;
	stringstream ss(command);
	if (!(ss >> inst))
		return true;
	if (inst == "ls")
		ls(command);
	else if (inst == "cd")
		cd(command);
	else if (inst == "exit")
	{
		kill(command);
		return false;
	}
	else 
	{
		not_found(command);
		errors++;
		return true;
	}

	return true;
}


int main(int argc, char* argv[])
{
	cout << "To run command as administrator (user " << '"' << "root" << '"' \
		<< "), use " << '"' << "sudo <command>" << '"' << ".\nSee " \
		<< '"' << "man sudo_root" << '"' << " for details.\n";
	setlocale(LC_ALL, "Russian");

	if (argc != 3) 
	{
		cerr << "Usage: emulator.exe <VFS_path> <startup_script>\n";
		return 1;
	}

	string vfs_path = argv[1];
	string script_path = argv[2];
	cout << "VFS path: " << vfs_path << endl;
	cout << "Startup script: " << script_path << "\n\n";

	ifstream file(script_path);
	if (!file.is_open()) 
	{
		cerr << "Error: cannot open script: " << script_path << endl;
		return 1;
	}

	string line = "";

	while (getline(file, line))
	{	// Пропускаем пустые строки и комменты
		size_t pos = line.find_first_not_of(" \t\r\n");
		if (pos == string::npos || line[pos] == '#')
			continue;
		// Показываем команду как будто ее ввел пользователь
		command_prompt();
		cout << line << '\n';

		if (!execute(line)) 
		{
			file.close();
			if (errors > 0)
				cout << "В ходе работы программы были обнаружены ошибки\n";
		}
			
	}
	file.close();
	if (errors > 0) 
		cout << "В ходе работы программы были обнаружены ошибки\n";
	while (true) 
	{
		command_prompt();
		if (!getline(cin, line))
			break; 
		if (!execute(line))
			break;
	}
}