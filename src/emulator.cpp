#include<iostream>
#include<string>
#include<sstream>
#include<windows.h>
using namespace std;
void command_prompt()
{
	printf("frisk@frisk:~$ ");
}
int main() 
{
	cout << "To run command as administrator (user " << '"' << "root" << '"' \
		<< "), use " << '"' << "sudo <command>" << '"' << ".\nSee " << '"' << "man sudo_root" << '"' << " for details.\n";
	setlocale(LC_ALL, "Russian");
	string command = "";
	string buffer = "";
	int number;
	while (true) 
	{
		command_prompt();
		getline(cin, command);
		if (command == "")
			continue;
		stringstream ss(command);
		while (ss >> buffer) 
		{
			if (buffer == "ls")
			{
				cout << buffer;
				while (ss >> buffer)
					cout << ' ' << buffer;
				cout << '\n';
				continue;
			}
			else if (buffer == "cd")
			{
				cout << buffer;
				while (ss >> buffer)
					cout << ' ' << buffer;
				cout << '\n';
				continue;
			}
			else if (buffer == "exit")
			{
				if (ss >> buffer)
				{
					try 
					{
						number = stoi(buffer);
						cout << "exit " << number << '\n';
						return 0;
					}
					catch (invalid_argument const& ex)
					{
						cout << "\nbash: exit: ";
						cout << buffer;
						while (ss >> buffer)
							cout << ' ' << buffer;
						cout << " :numeric argument required";
						return 0;
					}
				}
			}
			else 
			{
				
				cout << "\nbash: " << buffer;
				while (ss >> buffer)
					cout << ' ' << buffer;
				cout << ": command not found\n";
				break;
			}
		}
	};
}