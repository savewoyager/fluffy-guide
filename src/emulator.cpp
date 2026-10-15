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
namespace fs = std::filesystem;
using namespace std;
using namespace pugi;
const char* username = getenv("USERNAME");
static int errors = 0;
string current_path = "/";
bool load_vfs(const string& path)
{
	cout << "[VFS] Загрузка из: " << path << '\n';

	xml_document doc;
	xml_parse_result result = doc.load_file(path.c_str());

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
	xml_node root = doc.document_element();

	if (!root || string(root.name()) != "vfs")
	{
		cerr << "[VFS ERROR] Неправильный корневой элемент.\n";
		return false;
	}
	return true;
	//int directories = 0;
	//int files = 0;

	//// Рекурсивно проверяем каталоги и файлы
	//function<bool(xml_node)> check_node;

	//check_node = [&](xml_node parent) -> bool
	//{
	//	for (xml_node node : parent.children())
	//	{
	//		// Пропускаем текстовые узлы и комментарии
	//		if (node.type() != node_element)
	//			continue;

	//		string name = node.name();

	//		if (name == "directory")
	//		{
	//			++directories;

	//			if (!node.attribute("name"))
	//			{
	//				cerr << "[VFS ERROR] "
	//					<< "Отсутствует атрибут name в directory.\n";
	//				return false;
	//			}

	//			if (!check_node(node))
	//				return false;
	//		}
	//		else if (name == "file")
	//		{
	//			++files;

	//			if (!node.attribute("name"))
	//			{
	//				cerr << "[VFS ERROR] "
	//					<< "Отсутствует атрибут name в file.\n";
	//				return false;
	//			}

	//			xml_attribute type = node.attribute("type");

	//			if (!type)
	//			{
	//				cerr << "[VFS ERROR] "
	//					<< "Отсутствует атрибут type в file.\n";
	//				return false;
	//			}

	//			string file_type = type.value();

	//			if (file_type != "text" &&
	//				file_type != "base64")
	//			{
	//				cerr << "[VFS ERROR] "
	//					<< "Неподдерживаемый тип файла: "
	//					<< file_type << '\n';
	//				return false;
	//			}

	//			// Файлы не должны содержать другие элементы.
	//			for (xml_node child : node.children())
	//			{
	//				if (child.type() == node_element)
	//				{
	//					cerr << "[VFS ERROR] "
	//						<< "Файл не может содержать "
	//						<< "вложенные элементы.\n";
	//					return false;
	//				}
	//			}
	//		}
	//		else
	//		{
	//			cerr << "[VFS ERROR] Неподдерживаемый тег: "
	//				<< name << '\n';
	//			return false;
	//		}
	//	}

	//	return true;
	//};
	//if (!check_node(root))
	//	return false;
	//cout << "[VFS] Формат XML структуры корректен.\n";
	//cout << "[VFS] Каталогов: " << directories << '\n';
	//cout << "[VFS] Файлов: " << files << '\n';
	//cout << "[VFS] Найдено элементов: "
	//	<< directories + files << '\n';
	//cout << "[VFS] Загрузка завершена успешно.\n\n";

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
void ls(string instr, const string& path)
{
	xml_document doc;
	xml_parse_result result = doc.load_file(path.c_str());
	if (!result)
	{
		cerr << "[VFS ERROR] Не получилось открыть "
			<< "или разобрать XML-файл: "
			<< path << '\n';
		cerr << "[VFS ERROR] Причина: "
			<< result.description() << '\n';
		return;
	}
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
					cerr << "ls: неизвестный флаг: -"\
						<< arg[i] << '\n';
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
	xml_document doc;
	if (!doc.load_file(path.c_str()))
	{
		cerr << "Ошибка загрузки XML\n";
		return;
	}
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
	}
	else
		cerr << "ls: неизвестный флаг: " << instr << endl;
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
	xml_document doc;
	xml_parse_result result = doc.load_file(path.c_str());
	if (!result)
	{
		cerr << "[VFS ERROR] Не получилось открыть "
			<< "или разобрать XML-файл: "
			<< path << '\n';
		cerr << "[VFS ERROR] Причина: "
			<< result.description() << '\n';
		return;
	}
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