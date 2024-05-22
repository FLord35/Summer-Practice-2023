#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <regex>
#include <string>
#include <fstream>
#include <map>
#include <ctime>
#include <iomanip>
#include <time.h>

struct UserIpType
{
	int nameAttacks;
	std::string userIp;
	std::time_t firstFailedSessionTime;
	std::time_t lastFailedSessionTime;
	int successfulSessions;
};

bool checkDateAndTime(std::string date) //�������� �������� ��� �� �������������
{
	bool leapYear = false;
	bool bigMonth = false;
	bool isFebruary = false;
	bool invalidMonth = false;
	bool invalidHour = false;
	bool invalidMinute = false;

	int validator = std::stoi(date.substr(6, 4)); //validator �������� ������ � ����
	if ((validator % 4 == 0) && (validator % 100 != 0) || (validator % 400 == 0))
		leapYear = true; //���������� ���
	validator = std::stoi(date.substr(3, 2)); //validator �������� ������ � ������
	if ((validator == 1) || (validator == 3) || (validator == 5) || (validator == 7) || (validator == 8) || (validator == 10) || (validator == 12))
		bigMonth = true; //'�������' �����
	if (validator == 2)
		isFebruary = true; //�������
	if ((validator == 0) || (validator > 12))
		invalidMonth = true; //������� �������� �����
	validator = std::stoi(date.substr(0, 2)); //validator �������� ������ � ���
	if ((validator > 31) || ((validator == 31) && !bigMonth) || ((validator > 28) && isFebruary && !leapYear) || ((validator > 29) && isFebruary && leapYear) || invalidMonth)
		return false;

	validator = std::stoi(date.substr(11, 2)); //validator �������� ������ � ����
	if (validator > 23)
		invalidHour = true; //������� �������� ���
	validator = std::stoi(date.substr(14, 2)); //validator �������� ������ � ������
	if (validator > 59)
		invalidMinute = true; //������� �������� ������
	validator = std::stoi(date.substr(17, 2)); //validator �������� ������ � �������
	if ((validator > 59) || invalidHour || invalidMinute)
		return false;
	return true;
}

bool checkFileAndDate(std::string file, std::string sTime, std::string eTime) //�������� ������������ �������� ����� �������������� ����� � ��� ������ � ����� �������
{
	std::regex fileTemplate(R"regex(([A-Z]:\\)?([\w\\-]*\\)?[\w-]+\.log)regex");
	std::regex timeTemplate(R"regex(\d{2}\.\d{2}\.\d{4},\d{2}:\d{2}:\d{2})regex");

	if (std::regex_match(file, fileTemplate) && std::regex_match(sTime, timeTemplate) && std::regex_match(eTime, timeTemplate) && checkDateAndTime(sTime) && checkDateAndTime(eTime))
		return true;
	if (std::regex_match(file, fileTemplate) && (sTime == "None") && (eTime == "None"))
		return true;
	std::cout << "������� �������� ������" << std::endl;
	return false;
}

int convertTextMonthToInt(std::string month)
{
	if (month == "Jan") return 0;
	if (month == "Feb") return 1;
	if (month == "Mar") return 2;
	if (month == "Apr") return 3;
	if (month == "May") return 4;
	if (month == "Jun") return 5;
	if (month == "Jul") return 6;
	if (month == "Aug") return 7;
	if (month == "Sep") return 8;
	if (month == "Oct") return 9;
	if (month == "Nov") return 10;
	if (month == "Dec") return 11;
}

std::time_t convertGivenTimeToSeconds(std::string date)
{
	std::tm tm{};
	tm.tm_year = std::stoi(date.substr(6, 4)) - 1900;
	tm.tm_mon = std::stoi(date.substr(3, 2)) - 1;
	tm.tm_mday = std::stoi(date.substr(0, 2));
	tm.tm_hour = std::stoi(date.substr(11, 2));
	tm.tm_min = std::stoi(date.substr(14, 2));
	tm.tm_sec = std::stoi(date.substr(17, 2));
	return std::mktime(&tm);
}

std::time_t convertLogTimeToSeconds(std::string date)
{
	std::tm tm{};
	tm.tm_year = std::stoi(date.substr(7, 4)) - 1900;
	tm.tm_mon = convertTextMonthToInt(date.substr(3, 3));
	tm.tm_mday = std::stoi(date.substr(0, 2));
	tm.tm_hour = std::stoi(date.substr(12, 2));
	tm.tm_min = std::stoi(date.substr(15, 2));
	tm.tm_sec = std::stoi(date.substr(18, 2));
	return std::mktime(&tm);
}

void anlogMain(std::string filePath, std::string startTimestamp, std::string endTimestamp) //���������� ������� anlog
{
	if (checkFileAndDate(filePath, startTimestamp, endTimestamp))
	{
		std::ifstream log;
		log.open(filePath);
		if (log.fail())
			std::cout << "������ ��� ������ �����. ���������, ������� �� ����� ��� ����� � ���� � ����" << std::endl;
		else
		{
			std::map<std::string, std::string> idToIp;
			std::map<std::string, UserIpType> database;
			std::time_t startTime;
			std::time_t endTime;
			std::time_t lineTime;
			std::string lineId;
			std::string lineIp;
			char definingSymbol;
			char line[1023];

			if ((startTimestamp != "None") && (endTimestamp != "None"))
			{
				startTime = convertGivenTimeToSeconds(startTimestamp);
				endTime = convertGivenTimeToSeconds(endTimestamp);
			}

			while (!log.eof()) //���� �� ����� �����
			{
				log.getline(line, 1023);
				std::string strLine = line;

				if (strLine.empty())
					continue;

				definingSymbol = strLine[0];
				lineTime = convertLogTimeToSeconds(strLine.substr(2, 20));

				if ((startTimestamp != "None") && (endTimestamp != "None"))
				{
					if (std::difftime(endTime, lineTime) < 0) //���� �������� ������������� ����� ����� ��������� ������ ������� �������� ������, ��������� �����������
						break;
					if (std::difftime(startTime, lineTime) > 0) //���� �������� ������������� ����� ������ ��������� ������ ������� �������� ������, ��������� �� ����������
						continue;
				}

				lineId = strLine.substr(25, 11); //ID ������
				if (definingSymbol == '*')
				{
					std::match_results<const char*> m;
					std::regex reg(R"regex(\[\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3}\])regex");
					std::regex_search(strLine.c_str(), m, reg);
					if (m.size() == 0)
						continue;
					lineIp = m[0].str();
					lineIp = lineIp.substr(1, lineIp.size() - 2); //IP ������

					if (strLine.find("Session started") != -1)
					{
						idToIp.insert(std::pair<std::string, std::string>(lineId, lineIp));
					}

					if (strLine.find("Session finished") != -1)
					{
						idToIp.erase(lineId);
					}
				}

				if ((definingSymbol == '!') && (strLine.find("Authorization failed") != -1))
				{
					std::string ip = idToIp[lineId];
					if (ip.empty())
						continue;

					if (database.find(ip) == database.end())
						database.insert(std::pair<std::string, UserIpType>(ip, { 1, ip, lineTime, lineTime, 0 }));
					else 
					{
						UserIpType& temp = database[ip];
						++temp.nameAttacks;
						if (temp.firstFailedSessionTime == -1)
							temp.firstFailedSessionTime = lineTime;
						temp.lastFailedSessionTime = lineTime;
					}
				}

				if ((definingSymbol == '+') && (strLine.find("Sender") != -1))
				{
					std::string ip = idToIp[lineId];
					if (ip.empty())
						continue;

					if (database.find(ip) == database.end())
						database.insert(std::pair<std::string, UserIpType>(ip, { 0, ip, -1, -1, 1 }));
					else
						++database[ip].successfulSessions;
				}
			}
			std::multimap<int, UserIpType, std::greater<int>> sorted;
			for(auto& it: database)
				sorted.insert(std::pair<int, UserIpType>(it.second.nameAttacks, it.second));

			for (auto& it : sorted)
			{
				if (!it.second.nameAttacks) //���� ��� ����, ������� �� �������
					break;

				std::cout << std::setiosflags(std::ios::left);
				std::cout << std::setw(6) << it.second.nameAttacks << ' ' << std::setw(18) << it.second.userIp << ' ';
				std::cout << std::resetiosflags(std::ios::left);
				std::string temp = std::ctime(&it.second.firstFailedSessionTime);
				temp.pop_back();
				std::cout << std::setw(24) << temp << ' ';
				temp = std::ctime(&it.second.lastFailedSessionTime);
				temp.pop_back();
				std::cout << std::setw(26) << temp << ' ' << std::setw(4) << it.second.successfulSessions << std::endl;
			}
		}		
	}
}

int main(int argc, char *argv[])
{
	std::setlocale(LC_ALL, "RUS"); //��������� ������� ��������� ��� �������

	if ((argc == 3) || (argc > 4))
		std::cout << "��� ������������� ��������� ���������� ������ ���� 1 �������� (���� � ������� SMTP-�������), ���� 3 (����, ���� ������ �������, ���� ����� �������)" << std::endl;
	else if (argc == 2)
		anlogMain(argv[1], "None", "None");
	else anlogMain(argv[1], argv[2], argv[3]);
}