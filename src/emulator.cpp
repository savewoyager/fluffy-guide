#define _CRT_SECURE_NO_WARNINGS
#include<iostream>
#include<string>
#include<sstream>
#include<fstream>
#include<windows.h>
#include<vector>
#include<iterator>
#include<pugixml.hpp>
using namespace std;
using namespace pugi;
const char* username = getenv("USERNAME");
static int errors = 0;


bool load_vfs(const string& path)
{
    ifstream vfs(path);
    cout << "[VFS] загрузка из: " << path << '\n';

    if (!vfs.is_open())
    {
        cerr << "[VFS ERROR] Не получилось открыть файл!: "
            << path << '\n';
        return false;
    }

    string xml((istreambuf_iterator<char>(vfs)),
        istreambuf_iterator<char>());

    cout << "[VFS] Файл открылся.\n";

    vector<string> tags;
    int directories = 0;
    int files = 0;
    bool root_found = false;
    bool root_closed = false;

    size_t pos = 0;

    while ((pos = xml.find('<', pos)) != string::npos)
    {
        size_t end = xml.find('>', pos);

        if (end == string::npos)
        {
            cerr << "[VFS ERROR] Invalid XML: missing '>'.\n";
            return false;
        }

        string tag = xml.substr(pos + 1, end - pos - 1);
        pos = end + 1;

        // Пропускаем XML-декларацию и комментарии.
        if (tag.empty() || tag[0] == '?' || tag[0] == '!')
            continue;

        bool closing = tag[0] == '/';
        bool self_closing = !tag.empty() && tag.back() == '/';

        if (closing)
            tag.erase(0, 1);

        if (self_closing)
            tag.pop_back();

        // Отделяем имя тега от атрибутов.
        size_t space = tag.find_first_of(" \t\r\n");
        string name = tag.substr(0, space);

        if (closing)
        {
            if (tags.empty() || tags.back() != name)
            {
                cerr << "[VFS ERROR] Не найден закрывающий тег: "
                    << name << '\n';
                return false;
            }

            tags.pop_back();

            if (name == "vfs")
                root_closed = true;

            continue;
        }

        if (name != "vfs" && name != "directory" && name != "file")
        {
            cerr << "[VFS ERROR] Неподдерживаемый тег: "
                << name << '\n';
            return false;
        }

        if (name == "vfs")
        {
            if (root_found || !tags.empty())
            {
                cerr << "[VFS ERROR] Неправильный корневой элемент.\n";
                return false;
            }

            root_found = true;
        }
        else
        {
            if (tags.empty() || root_closed ||
                tags.back() == "file")
            {
                cerr << "[VFS ERROR] Invalid element placement: "
                    << name << '\n';
                return false;
            }

            if (name == "directory")
                ++directories;
            else
                ++files;

            string attributes =
                space == string::npos ? "" : tag.substr(space + 1);

            if (attributes.find("name=\"") == string::npos)
            {
                cerr << "[VFS ERROR] Missing name attribute in "
                    << name << ".\n";
                return false;
            }

            if (name == "file" &&
                attributes.find("type=\"") == string::npos)
            {
                cerr << "[VFS ERROR] Missing type attribute in file.\n";
                return false;
            }

            if (name == "file")
            {
                size_t type_pos = attributes.find("type=\"");

                if (type_pos != string::npos)
                {
                    type_pos += 6;
                    size_t type_end = attributes.find('"', type_pos);
                    string type = attributes.substr(
                        type_pos, type_end - type_pos);

                    if (type != "text" && type != "base64")
                    {
                        cerr << "[VFS ERROR] Unsupported file type: "
                            << type << '\n';
                        return false;
                    }
                }
            }
        }

        if (!self_closing)
            tags.push_back(name);
    }

    if (!root_found || !root_closed || !tags.empty())
    {
        cerr << "[VFS ERROR] Invalid XML structure: "
            << "missing root or unclosed tags.\n";
        return false;
    }

    cout << "[VFS] Формат XML структуры корректен.\n";
    cout << "[VFS] Каталогов: " << directories << '\n';
    cout << "[VFS] Файлов: " << files << '\n';
    cout << "[VFS] Найдено элементов: "
        << directories + files << '\n';
    cout << "[VFS] Загрузка завершена успешно.\n\n";

    return true;
}


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
    // выполнение проверки загрузки XML
    if (!load_vfs(vfs_path))
        return 1;

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