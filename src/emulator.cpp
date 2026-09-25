#define _CRT_SECURE_NO_WARNINGS
#include<iostream>
#include<string>
#include<sstream>
#include<windows.h>
using namespace std;
const char* username = getenv("USERNAME");
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
void kill(string instr)
{
	int number;
	stringstream ss(instr);
	ss >> instr;
	if (ss >> instr)
	{
		try
		{
			number = stoi(instr);
			cout << "exit " << number << '\n';
			exit(0);
		}
		catch (invalid_argument const& ex)
		{
			cout << "\nbash: exit: ";
			cout << instr;
			while (ss >> instr)
				cout << ' ' << instr;
			cout << " :numeric argument required";
			exit(0);
		}
	}
}
void not_found(string instr)
{
	stringstream ss(instr);
	ss >> instr;
	cout << "\nbash: " << instr;
	while (ss >> instr)
		cout << ' ' << instr;
	cout << ": command not found\n";
}

int main() 
{
	cout << "To run command as administrator (user " << '"' << "root" << '"' \
		<< "), use " << '"' << "sudo <command>" << '"' << ".\nSee " << '"' << "man sudo_root" << '"' << " for details.\n";
	setlocale(LC_ALL, "Russian");
	string command = "";
	string instr = "";
	while (true) 
	{
		command_prompt();
		getline(cin, command);
		if (command == "")
			continue;
		stringstream ss(command);
		ss >> instr;
		if (instr == "ls")
		{
			ls(command);
			continue;
		}
		else if (instr == "cd")
		{
			cd(command);
			continue;
		}
		else if (instr == "exit")
		{
			kill(command);
		}
		else 
		{
			not_found(command);
			continue;
		}
	};
}