#ifndef CLIENT_HEADER_H
#define CLIENT_HEADER_H

#include <cstdio>
#include <cstdlib>
#include <sys/types.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <sys/msg.h>
#include <cstring>
#include <string>
#include <iostream>
#include <map>

#include "Message.h"
#include "System.h"

using std::string;
using std::map;
using std::pair;

class Client_t {
public:

	explicit Client_t() :
		error_file(NULL), memory_id(0), user_name(" "), user_id(0), error(" "), flag_of_working(true)
	{
		message_size.mtype = 0; message_size.size = 0;
		message_string.mtype = 0; message_string.message[1] = { 0 };
	}
	~Client_t();
	bool Start();

private:
	FILE* error_file;
	bool flag_of_working;
	string error;
	size_t memory_id;
	string user_name;
	long user_id;
	msgbuf_t message_string;
	msgsize_t message_size;
	Message_t Messenger;

	bool Get_Id();
	bool Get_Message();
	bool Send_Message();
};

Client_t::~Client_t() {
	fclose(error_file);
}

bool Client_t::Start() {
	try {
		error_file = fopen(error_log, "w");
		if (error_file == NULL) {
			error = "Error - open error file\n";
			throw error;
		}
		memory_id = Messenger.Create_Message_Thread(error_log, SYSTEM);
		if (memory_id == ERROR) {
			error = "Error - Memory Id\n";
			throw error;
		}
		if (!Get_Id()) {
			error = "Error - Get_Id\n";
			throw error;
		}
		pid_t status = fork();
		switch (status) {
		case -1: //error
			error = "Error - fork \n";
			throw error;
		case 0:  //son
			if (!Send_Message()) {
				error = "Error - Send Message \n";
				throw error;
			}
			break;
		default: //father
			if (!Get_Message()) {
				error = "Error - Get Message \n";
				throw error;
			}
			break;
		}
		return true;
	}
	catch (string error) {
		print_er(error);
		return false;
	}
}

bool Client_t::Send_Message() {
	try {
		while (flag_of_working) {
			string message = "";
			std::getline(std::cin, message);
			if (message == exit_sys) {
				message = message + std::to_string(user_id);
				if (!Messenger.Send_Message(memory_id, &message_string, &message_size, SYSTEM_ID, message, 0)) {
					error = "Error - Send exit\n";
					throw error;
				}
				flag_of_working = false;
			}
			else {
				if (!Messenger.Send_Message(memory_id, &message_string, &message_size, user_id, message, 0)) {
					error = "Error - Send message\n";
					throw error;
				}
			}
		}
		return true;
	}
	catch (string error) {
		print_er(error);
		return false;
	}
}

bool Client_t::Get_Message() {
	try {
		while (flag_of_working) {
			string message = "";
			if (!Messenger.Get_Message_Size(memory_id, &message_size, user_id + 1, 0)) {
				error = "Error - get size message normal\n";
				throw error;
			}
			if (!Messenger.Get_Message_String(memory_id, message, user_id + 1, 0, message_size.size)) {
				error = "Error - get message normal\n";
				throw error;
			}
			if (message == end_sys) {
				printf("Server is disabled\n");
				flag_of_working = false;
			}
			else if (message == exit_sys) {
				printf("Exit\n");
				flag_of_working = false;
			}
			else {
				printf("%s\n", message.c_str());
			}
		}
		return true;
	}
	catch (string error) {
		print_er(error);
		return false;
	}
}

bool Client_t::Get_Id() {
	try {
		printf("Hello, What is your name?\n");
		std::getline(std::cin, user_name);
		string user_name_mes = new_client_sys + user_name;
		if (!Messenger.Send_Message_Size(memory_id, &message_size, SYSTEM_ID, user_name_mes.size(), 0)) {
			error = "Error - Send Size of Message\n";
			throw error;
		}
		if (!Messenger.Send_Message_String(memory_id, &message_string, SYSTEM_ID, user_name_mes, 0)) {
			error = "Error - Send Message\n";
			throw error;
		}
		if (!Messenger.Get_Message_Size(memory_id, &message_size, SYSTEM_ID + 1, 0)) {
			error = "Error - Get Size\n";
			throw error;
		}
		if (!Messenger.Get_Message_String(memory_id, user_name_mes, SYSTEM_ID + 1, 0, message_size.size)) {
			error = "Error - Get Message\n";
			throw error;
		}
		size_t pas = 0;
		user_name_mes = user_name_mes.substr(user_name.size());
		if (!IsItNumber(user_name_mes)) {
			error = "Error - wrong new id\n";
			throw error;
		}
		user_id = stoul(user_name_mes, &pas, 10);
		if (pas != user_name_mes.size()) {
			error = "Error - Get Id\n";
			throw error;
		}
		printf("Your Id: %ld\n", user_id);
		return true;
	}
	catch (string error) {
		print_er(error);
		return false;
	}
}

#endif // CLIENT_HEADER_H