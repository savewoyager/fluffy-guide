#define _CRT_SECURE_NO_WARNINGS
#include<iostream>
#include<string>
#include<sstream>
#include<fstream>
#include<windows.h>
#include<functional>
#include<pugixml.hpp>
#include<filesystem>
#include<iomanip>
#include<ctime>
#include<vector>
#include <algorithm>
using namespace std;
using namespace pugi;

xml_document vfs_doc;

const char* username = getenv("USERNAME");
static int errors = 0;
string current_path = "/";
xml_node find_directory(xml_node root, const string& path);
string normalize_path(const string& path)
{
	vector<string> parts;
	string full_path;

	if (!path.empty() && path[0] == '/')
		full_path = path;
	else if (current_path == "/")
		full_path = "/" + path;
	else
		full_path = current_path + "/" + path;

	stringstream ss(full_path);
	string part;

	while (getline(ss, part, '/'))
	{
		if (part.empty() || part == ".")
			continue;

		if (part == "..")
		{
			if (!parts.empty())
				parts.pop_back();
		}
		else
			parts.push_back(part);
	}

	string result = "/";
	for (size_t i = 0; i < parts.size(); i++)
	{
		if (i > 0)
			result += "/";
		result += parts[i];
	}

	return result;
}

xml_node find_entry(xml_node parent, const string& name)
{
	for (xml_node node : parent.children())
	{
		if (node.type() != node_element)
			continue;

		string type = node.name();
		if ((type == "file" || type == "directory") &&
			string(node.attribute("name").value()) == name)
			return node;
	}

	return xml_node();
}

xml_node find_entry_by_path(const string& path)
{
	string normalized = normalize_path(path);

	if (normalized == "/")
		return vfs_doc.document_element();

	size_t pos = normalized.find_last_of('/');
	string parent_path = normalized.substr(0, pos);
	string name = normalized.substr(pos + 1);

	if (parent_path.empty())
		parent_path = "/";

	xml_node parent = find_directory(
		vfs_doc.document_element(), parent_path);

	if (!parent)
		return xml_node();

	return find_entry(parent, name);
}

xml_node find_parent_by_path(const string& path)
{
	string normalized = normalize_path(path);

	if (normalized == "/")
		return xml_node();

	size_t pos = normalized.find_last_of('/');
	string parent_path = normalized.substr(0, pos);

	if (parent_path.empty())
		parent_path = "/";

	return find_directory(vfs_doc.document_element(), parent_path);
}


bool load_vfs(const string& path)
{
	cout << "[VFS] Загрузка из: " << path << '\n';

	xml_parse_result result = vfs_doc.load_file(path.c_str());
	if (!result)
	{
		cerr << "[VFS ERROR] Не получилось открыть "
			<< "или разобрать XML-файл: "
			<< path << '\n';
		cerr << "[VFS ERROR] Причина: "
			<< result.description() << '\n';
		return false;
	}

	cout << "[VFS] Файл открылся.\n";

	// Проверяем корневой элемент.
	xml_node root = vfs_doc.document_element();

	if (!root || string(root.name()) != "vfs")
	{
		cerr << "[VFS ERROR] Неправильный корневой элемент.\n";
		return false;
	}
	return true;
}
xml_node find_directory(xml_node root, const string& path)
{ // поиск нужной директории
	xml_node current = root;
	if (path == "/")
		return current;
	stringstream ss(path);
	string part;
	while (getline(ss, part, '/'))
	{
		if (part.empty())
			continue;
		xml_node next;
		for (xml_node node : current.children("directory"))
		{
			if (string(node.attribute("name").value()) == part)
			{
				next = node;
				break;
			}
		}
		if (!next)
			return xml_node();
		current = next;
	}
	return current;
}
void command_prompt()
{
	printf("%s@%s:~$ ", username, username);
}

void cp(string instr)
{
	stringstream ss(instr);
	string command, arg;
	vector<string> args;

	ss >> command;

	bool recursive = false;

	while (ss >> arg)
	{
		if (arg == "-r" || arg == "-R")
			recursive = true;
		else if (!arg.empty() && arg[0] == '-')
		{
			cerr << "cp: неизвестный флаг: " << arg << '\n';
			errors++;
			return;
		}
		else
			args.push_back(arg);
	}

	if (args.size() != 2)
	{
		cerr << "Использование: cp [-r] источник назначение\n";
		errors++;
		return;
	}

	string src_path = normalize_path(args[0]);
	string dst_path = normalize_path(args[1]);

	xml_node src = find_entry_by_path(src_path);

	if (!src || src == vfs_doc.document_element())
	{
		cerr << "cp: источник не найден: " << args[0] << '\n';
		errors++;
		return;
	}

	string type = src.name();

	if (type == "directory" && !recursive)
	{
		cerr << "cp: пропущена директория (не указан -r): "
			<< args[0] << '\n';
		errors++;
		return;
	}

	if (type != "file" && type != "directory")
	{
		cerr << "cp: неподдерживаемый тип объекта\n";
		errors++;
		return;
	}

	xml_node dst = find_entry_by_path(dst_path);
	xml_node target_parent;
	string target_name;

	if (dst)
	{
		if (string(dst.name()) != "directory")
		{
			cerr << "cp: назначение не является директорией: "
				<< args[1] << '\n';
			errors++;
			return;
		}

		target_parent = dst;
		target_name = src.attribute("name").value();
	}
	else
	{
		target_parent = find_parent_by_path(dst_path);

		size_t pos = dst_path.find_last_of('/');
		target_name = dst_path.substr(pos + 1);
	}

	if (!target_parent || target_name.empty())
	{
		cerr << "cp: каталог назначения не найден: "
			<< args[1] << '\n';
		errors++;
		return;
	}

	// Нельзя копировать объект в самого себя или внутрь него.
	string source_prefix = src_path + "/";
	if (dst_path == src_path ||
		(dst_path.size() > source_prefix.size() - 1 &&
			dst_path.compare(0, source_prefix.size(), source_prefix) == 0))
	{
		cerr << "cp: нельзя копировать объект внутрь самого себя\n";
		errors++;
		return;
	}

	if (find_entry(target_parent, target_name))
	{
		cerr << "cp: объект уже существует: "
			<< target_name << '\n';
		errors++;
		return;
	}

	xml_node copy = target_parent.append_copy(src);
	copy.attribute("name").set_value(target_name.c_str());

	cout << "Скопировано: " << args[0]
		<< " -> " << args[1] << '\n';
}

void ls(string instr, const string& path)
{
	xml_document& doc = vfs_doc;
	stringstream ss(instr);
	string command, arg;
	ss >> command;
	bool f_l(false), f_a(false), f_h(false);
	while (ss >> arg)
	{
		if (arg.empty() || arg[0] != '-') 
		{
			cerr << "ls: invalid argument: "\
				<< arg << endl;
			errors++;
			return;
		}
		for (size_t i = 1; i < arg.size(); i++) 
		{
			switch (arg[i]) 
			{
				case 'l':
					f_l = true;
					break;
				case 'h':
					f_h = true;
					break;
				case 'a':
					f_a = true;
					break;
				default:
					cerr << "ls: неизвестный флаг: "\
						<< arg << '\n';
					errors++;
					return;
			}
		}
	}
	// по умолчанию текущая директория - vfs
	xml_node root = doc.document_element();
	xml_node directory = find_directory(root, current_path);

	if (!directory) 
	{
		cerr << "ls: текущий каталог не найден\n";
		return;
	}

	for (xml_node node : directory.children())
	{
		if (node.type() != node_element)
			continue;

		string name = node.attribute("name").value();
		string type = node.name();

		if (type != "directory" && type != "file")
			continue;

		if (!f_a && !name.empty() && name[0] == '.')
			continue;

		if (!f_l) 
		{
			if(type == "file")
				cout << name << endl;
			else
				cout << name << endl;
			continue;
		}
		
		if (type == "directory") 
			cout << left << setw(12) <<"drwxrwx--- " \
			<< setw(12) << "<dir>" << name << endl;
		else
		{
			string content = node.child_value();
			size_t size = content.size();
			string final_size;
			if (f_h) 
			{
				if (size >= pow(1024, 2))
					final_size = \
					to_string(round(size / pow(1024.0, 2))) + " Mb";
				else if (size >= 1024) {
					final_size = \
						to_string(round(size / 1024.0 * 100) / 100);
					final_size.erase(4);
					final_size += " Kb";
				}
				else {
					final_size = \
						to_string(size) + " b";
				}
			}
			else
			{
				final_size = \
					to_string(size);
			}
			cout << left << setw(12) << "-rwxrwx---"\
				<< right << setw(10) << final_size << "  "\
				<< left << name << '\n';
		}
	}
}
void cd(string instr, const string& path)
{
	xml_document& doc = vfs_doc;
	stringstream ss(instr);
	string command, arg;
	ss >> command;
	if (!(ss >> arg))
		arg = "/";
	string new_path;
	if (arg == "/")
		new_path = "/";
	else if (arg == "..")
	{
		if (current_path == "/")
			new_path = "/";
		else
		{
			size_t pos = current_path.find_last_of('/');
			if (pos == 0)
				new_path = "/";
			else
				new_path = current_path.substr(0, pos);
		}
	}
	else if (arg[0] == '/')
		new_path = arg;
	else
	{
		if (current_path == "/")
			new_path = "/" + arg;
		else
			new_path = current_path + "/" + arg;
	}
	xml_node root = doc.document_element();
	xml_node target = find_directory(root, new_path);
	if (!target)
	{
		cerr << "cd: каталог не найден: " << arg << '\n';
		errors++;
		return;
	}
	current_path = new_path;
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
void who(string instr) 
{
	stringstream ss(instr);
	ss >> instr;
	if (!(ss >> instr)) {
		time_t t = time(nullptr);
		tm* time_now = localtime(&t);
		printf("NAME\tLINE\tTIME\n\
root\ttty1\t%d-%d-%d %d:%d\n", \
time_now->tm_year + 1900, time_now->tm_mon + 1, time_now->tm_mday, \
time_now->tm_hour, time_now->tm_min);
		printf("%s\tpts/0\t%d-%d-%d %d:%d\n", \
			username, time_now->tm_year + 1900, time_now->tm_mon + 1, time_now->tm_mday, \
			time_now->tm_hour, time_now->tm_min);
		return;
	}
	else
		cerr << "ls: неизвестный флаг: " << instr << endl;
	errors++;
}
void rev(string instr, const string& path) 
{
	string filename;
	stringstream ss(instr);
	ss >> filename;
	if (!(ss >> filename)) 
	{
		cerr << "Укажите файл\n";
		return;
	}
	xml_document& doc = vfs_doc;
	xml_node root = doc.document_element();		
	xml_node directory = find_directory(root, current_path);
	xml_node find_file;
	for (xml_node node : directory.children())
	{
		if (node.type() != node_element)
			continue;

		string name = node.attribute("name").value();
		string type = node.name();

		if (type != "directory" && type != "file")
			continue;
		if (name[0] == '.')
			continue;
		if (name == filename)
			find_file = node;
	}
	if (!find_file) 
	{
		cerr << "Файл не найден\n";
		return;
	}
	string content = find_file.child_value();
	reverse(content.begin(), content.end());
	cout << content << endl;
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
bool execute(string command, const string& path)
{
	string inst;
	stringstream ss(command);
	if (!(ss >> inst))
		return true;
	if (inst == "ls")
		ls(command, path);
	else if (inst == "cd")
		cd(command, path);
	else if (inst == "who")
		who(command);
	else if (inst == "rev")
		rev(command, path);
	else if (inst == "cp")
		cp(command);
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

		if (!execute(line, vfs_path))
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
		if (!execute(line, vfs_path))
			break;
	}
}