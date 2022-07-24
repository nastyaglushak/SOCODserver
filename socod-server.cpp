#include "CivetServer.h"
#include <cstring>
#include <iostream>
#include <algorithm>
#include <iterator>

#include "Socod.hpp"
#include "FPGA.hpp"
#include "jsonxx.h"
#include "web_form_map.h"

#include <unistd.h>

#define DOCUMENT_ROOT "/SOCOD_SERVER_FILES"
#define PORT "8000"
bool exitNow = false;


class ChannelFrameNumHandler : public CivetHandler
{
private:
	Socod *socod;
  public:
	ChannelFrameNumHandler(Socod* socod) {
		this->socod = socod;
	}

	bool
	handleGet(CivetServer *server, struct mg_connection *conn)
	{
		jsonxx::Object responseJSON;
		responseJSON << WF_CHANNEL_NUM << (int)socod->getChannelNum();
		responseJSON << WF_FRAME_NUM << (int)socod->getFrameNum();
		mg_printf(conn,
		          "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nConnection: "
		          "close\r\n\r\n");
		mg_printf(conn, responseJSON.json().c_str());
		return true;
	}
};

class AcquireDataHandler : public CivetHandler
{
private:
	Socod *socod;
  public:
	AcquireDataHandler(Socod* socod) {
		this->socod = socod;
	}

	bool
	handlePost(CivetServer *server, struct mg_connection *conn)
	{
		std::string requestText = server->getPostData(conn).c_str();
		jsonxx::Object requestJSON;
		requestJSON.parse(requestText);

		bool forcedTrigger = false;
		if (requestJSON.has<jsonxx::Boolean>(WF_FORCED_TRIGGER)) {
			forcedTrigger = requestJSON.get<jsonxx::Boolean>(WF_FORCED_TRIGGER);
		}

		std::string startupConfigMode = STARTUP_MODE_NO_TRG;
		if (requestJSON.has<jsonxx::String>(WF_STARTUP_CONFIG_MODE)) {
			startupConfigMode = requestJSON.get<jsonxx::String>(WF_STARTUP_CONFIG_MODE);
		}

		int status = socod->acquireData(forcedTrigger, startupConfigMode);

		jsonxx::Object responseJSON;
		if (status < 0)
			responseJSON << WF_DATA_OUT << "";
		else {
			std::stringstream ss;
			std::copy(socod->getAcquiredDataVector()->begin(), socod->getAcquiredDataVector()->end(), std::ostream_iterator<int>(ss, " "));
			std::string s = ss.str();
			s = s.substr(0, s.length()-1);
			responseJSON << WF_DATA_OUT << s;
		}

		mg_printf(conn,
		          "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nConnection: "
		          "close\r\n\r\n");
		mg_printf(conn, responseJSON.json().c_str());
		return true;
	}
};

class WriteRegsHandler : public CivetHandler
{
private:
	Socod *socod;
  public:
	WriteRegsHandler(Socod* socod) {
		this->socod = socod;
	}

	bool
	handlePost(CivetServer *server, struct mg_connection *conn)
	{
		std::string requestText = server->getPostData(conn);
		jsonxx::Object requestJSON;
		requestJSON.parse(requestText);

		if (requestJSON.has<jsonxx::Number>(WF_FRAME_LEN)) {
			int frameLen = (int)requestJSON.get<jsonxx::Number>(WF_FRAME_LEN);
			socod->setFrameLen(frameLen);
		}

		return true;
	}
};

class ReadRegsHandler : public CivetHandler
{
private:
	Socod *socod;
  public:
	ReadRegsHandler(Socod* socod) {
		this->socod = socod;
	}

	bool
	handleGet(CivetServer *server, struct mg_connection *conn)
	{
		jsonxx::Object responseJSON;
		responseJSON << WF_FRAME_LEN << (int)socod->getFrameLen();
		mg_printf(conn,
		          "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nConnection: "
		          "close\r\n\r\n");
		mg_printf(conn, responseJSON.json().c_str());
		return true;
	}
};

class DARegReadHandler : public CivetHandler
{
private:
	Socod *socod;
  public:
	DARegReadHandler(Socod* socod) {
		this->socod = socod;
	}

	bool
	handlePost(CivetServer *server, struct mg_connection *conn)
	{
		std::string requestText = server->getPostData(conn);
		jsonxx::Object requestJSON;
		requestJSON.parse(requestText);

		reg_t dataOut = 0;
		int status = -1;

		if (requestJSON.has<jsonxx::Number>(WF_REG_NUM)) {
			reg_t regNum = (reg_t)requestJSON.get<jsonxx::Number>(WF_REG_NUM);
			dataOut = socod->readReg(regNum);
			status = 0;
		}

		jsonxx::Object responseJSON;
		responseJSON << WF_DATA_OUT << (int)dataOut;
		responseJSON << WF_STATUS << status;

		mg_printf(conn,
		          "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nConnection: "
		          "close\r\n\r\n");
		mg_printf(conn, responseJSON.json().c_str());
		return true;
	}
};

class DARegWriteHandler : public CivetHandler
{
private:
	Socod *socod;
  public:
	DARegWriteHandler(Socod* socod) {
		this->socod = socod;
	}

	bool
	handlePost(CivetServer *server, struct mg_connection *conn)
	{
		std::string requestText = server->getPostData(conn);
		jsonxx::Object requestJSON;
		requestJSON.parse(requestText);

		int status = -1;

		if (requestJSON.has<jsonxx::Number>(WF_REG_NUM) && requestJSON.has<jsonxx::Number>(WF_DATA_IN)) {
			reg_t regNum = (reg_t)requestJSON.get<jsonxx::Number>(WF_REG_NUM);
			reg_t dataIn = (reg_t)requestJSON.get<jsonxx::Number>(WF_DATA_IN);
			status = socod->writeReg(regNum, dataIn);
		}

		jsonxx::Object responseJSON;
		responseJSON << WF_STATUS << status;
		mg_printf(conn,
		          "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nConnection: "
		          "close\r\n\r\n");
		mg_printf(conn, responseJSON.json().c_str());
		return true;
	}
};

class StopHandler : public CivetHandler
{
private:
	Socod *socod;
  public:
	StopHandler(Socod* socod) {
		this->socod = socod;
	}

	bool
	handlePost(CivetServer *server, struct mg_connection *conn)
	{
		int status = -1;
		socod->setStop(true);
		if (socod->isStop())
			status = 0;

		jsonxx::Object responseJSON;
		responseJSON << WF_STATUS << status;
		mg_printf(conn,
		          "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nConnection: "
		          "close\r\n\r\n");
		mg_printf(conn, responseJSON.json().c_str());
		return true;
	}
};

class SetGlodalLimitHandler : public CivetHandler
{
private:
	Socod *socod;
  public:
	SetGlodalLimitHandler(Socod* socod) {
		this->socod = socod;
	}

	bool
	handlePost(CivetServer *server, struct mg_connection *conn)
	{
		std::string requestText = server->getPostData(conn);
		jsonxx::Object requestJSON;
		requestJSON.parse(requestText);

		if (requestJSON.has<jsonxx::Number>(WF_GLIMIT_1)) {
			reg_t glimit1 = (reg_t)requestJSON.get<jsonxx::Number>(WF_GLIMIT_1);
			socod->setGlobalLimit(glimit1, 0x0000);
		}

		if (requestJSON.has<jsonxx::Number>(WF_GLIMIT_2)) {
			reg_t glimit2 = (reg_t)requestJSON.get<jsonxx::Number>(WF_GLIMIT_2);
			socod->setGlobalLimit(glimit2, 0x0001);
		}

		if (requestJSON.has<jsonxx::Number>(WF_GLIMIT_3)) {
			reg_t glimit3 = (reg_t)requestJSON.get<jsonxx::Number>(WF_GLIMIT_3);
			socod->setGlobalLimit(glimit3, 0x0002);
		}

		if (requestJSON.has<jsonxx::Number>(WF_GLIMIT_4)) {
			reg_t glimit4 = (reg_t)requestJSON.get<jsonxx::Number>(WF_GLIMIT_4);
			socod->setGlobalLimit(glimit4, 0x0003);
		}

		if (requestJSON.has<jsonxx::Number>(WF_GLIMIT_5)) {
			reg_t glimit5 = (reg_t)requestJSON.get<jsonxx::Number>(WF_GLIMIT_5);
			socod->setGlobalLimit(glimit5, 0x0004);
		}

		if (requestJSON.has<jsonxx::Number>(WF_GLIMIT_6)) {
			reg_t glimit6 = (reg_t)requestJSON.get<jsonxx::Number>(WF_GLIMIT_6);
			socod->setGlobalLimit(glimit6, 0x0005);
		}

		if (requestJSON.has<jsonxx::Number>(WF_GLIMIT_7)) {
			reg_t glimit7 = (reg_t)requestJSON.get<jsonxx::Number>(WF_GLIMIT_7);
			socod->setGlobalLimit(glimit7, 0x0006);
		}

		if (requestJSON.has<jsonxx::Number>(WF_GLIMIT_8)) {
			reg_t glimit8 = (reg_t)requestJSON.get<jsonxx::Number>(WF_GLIMIT_8);
			socod->setGlobalLimit(glimit8, 0x0007);
		}
		socod->setControlReg(0x0080);
		return true;
	}
};

class SetLocalLimitHandler : public CivetHandler
{
private:
	Socod *socod;
  public:
	SetLocalLimitHandler(Socod* socod) {
		this->socod = socod;
	}

	bool
	handlePost(CivetServer *server, struct mg_connection *conn)
	{
		std::string requestText = server->getPostData(conn);
		jsonxx::Object requestJSON;
		requestJSON.parse(requestText);

		reg_t numChip;

		if (requestJSON.has<jsonxx::Number>(WF_CHIP_NUM)) {
			numChip = (reg_t)requestJSON.get<jsonxx::Number>(WF_CHIP_NUM);
			//socod->getSelectChip(numChip);
		}

		if (requestJSON.has<jsonxx::Number>(WF_LLIMIT_11)) {
			reg_t ilimit11 = (reg_t)requestJSON.get<jsonxx::Number>(WF_LLIMIT_11);
			socod->setLocalLimit(numChip,1,1,ilimit11);
		}

		if (requestJSON.has<jsonxx::Number>(WF_LLIMIT_12)) {
			reg_t ilimit12 = (reg_t)requestJSON.get<jsonxx::Number>(WF_LLIMIT_12);
			socod->setLocalLimit(numChip,1,2,ilimit12);
		}

		if (requestJSON.has<jsonxx::Number>(WF_LLIMIT_21)) {
			reg_t ilimit21 = (reg_t)requestJSON.get<jsonxx::Number>(WF_LLIMIT_21);
			socod->setLocalLimit(numChip,2,1,ilimit21);
		}

		if (requestJSON.has<jsonxx::Number>(WF_LLIMIT_22)) {
			reg_t ilimit22 = (reg_t)requestJSON.get<jsonxx::Number>(WF_LLIMIT_22);
			socod->setLocalLimit(numChip,2,2, ilimit22);
		}

		if (requestJSON.has<jsonxx::Number>(WF_LLIMIT_31)) {
			reg_t ilimit31 = (reg_t)requestJSON.get<jsonxx::Number>(WF_LLIMIT_31);
			socod->setLocalLimit(numChip,3,1,ilimit31);
		}

		if (requestJSON.has<jsonxx::Number>(WF_LLIMIT_32)) {
			reg_t ilimit32 = (reg_t)requestJSON.get<jsonxx::Number>(WF_LLIMIT_32);
			socod->setLocalLimit(numChip,3,2,ilimit32);
		}

		if (requestJSON.has<jsonxx::Number>(WF_LLIMIT_33)) {
			reg_t ilimit33 = (reg_t)requestJSON.get<jsonxx::Number>(WF_LLIMIT_33);
			socod->setLocalLimit(numChip,3,3,ilimit33);
		}

		if (requestJSON.has<jsonxx::Number>(WF_LLIMIT_34)) {
			reg_t ilimit34 = (reg_t)requestJSON.get<jsonxx::Number>(WF_LLIMIT_34);
			socod->setLocalLimit(numChip,3,4, ilimit34);
		}

		if (requestJSON.has<jsonxx::Number>(WF_LLIMIT_41)) {
			reg_t ilimit41 = (reg_t)requestJSON.get<jsonxx::Number>(WF_LLIMIT_41);
			socod->setLocalLimit(numChip,4,1,ilimit41);
		}

		if (requestJSON.has<jsonxx::Number>(WF_LLIMIT_42)) {
			reg_t ilimit42 = (reg_t)requestJSON.get<jsonxx::Number>(WF_LLIMIT_42);
			socod->setLocalLimit(numChip,4,2,ilimit42);
		}

		if (requestJSON.has<jsonxx::Number>(WF_LLIMIT_43)) {
			reg_t ilimit43 = (reg_t)requestJSON.get<jsonxx::Number>(WF_LLIMIT_43);
			socod->setLocalLimit(numChip,4,3,ilimit43);
		}

		if (requestJSON.has<jsonxx::Number>(WF_LLIMIT_44)) {
			reg_t ilimit44 = (reg_t)requestJSON.get<jsonxx::Number>(WF_LLIMIT_44);
			socod->setLocalLimit(numChip,4,4, ilimit44);
		}

		if (requestJSON.has<jsonxx::Number>(WF_LLIMIT_51)) {
			reg_t ilimit51 = (reg_t)requestJSON.get<jsonxx::Number>(WF_LLIMIT_51);
			socod->setLocalLimit(numChip,5,1,ilimit51);
		}

		if (requestJSON.has<jsonxx::Number>(WF_LLIMIT_52)) {
			reg_t ilimit52 = (reg_t)requestJSON.get<jsonxx::Number>(WF_LLIMIT_52);
			socod->setLocalLimit(numChip,5,2,ilimit52);
		}

		if (requestJSON.has<jsonxx::Number>(WF_LLIMIT_61)) {
			reg_t ilimit61 = (reg_t)requestJSON.get<jsonxx::Number>(WF_LLIMIT_61);
			socod->setLocalLimit(numChip,6,1,ilimit61);
		}

		if (requestJSON.has<jsonxx::Number>(WF_LLIMIT_62)) {
			reg_t ilimit62 = (reg_t)requestJSON.get<jsonxx::Number>(WF_LLIMIT_62);
			socod->setLocalLimit(numChip,6,2,ilimit62);
		}

		if (requestJSON.has<jsonxx::Number>(WF_LLIMIT_71)) {
			reg_t ilimit71 = (reg_t)requestJSON.get<jsonxx::Number>(WF_LLIMIT_71);
			socod->setLocalLimit(numChip,7,1,ilimit71);
		}

		if (requestJSON.has<jsonxx::Number>(WF_LLIMIT_72)) {
			reg_t ilimit72 = (reg_t)requestJSON.get<jsonxx::Number>(WF_LLIMIT_72);
			socod->setLocalLimit(numChip,7,2,ilimit72);
		}

		if (requestJSON.has<jsonxx::Number>(WF_LLIMIT_73)) {
			reg_t ilimit73 = (reg_t)requestJSON.get<jsonxx::Number>(WF_LLIMIT_73);
			socod->setLocalLimit(numChip,7,3,ilimit73);
		}

		if (requestJSON.has<jsonxx::Number>(WF_LLIMIT_74)) {
			reg_t ilimit74 = (reg_t)requestJSON.get<jsonxx::Number>(WF_LLIMIT_74);
			socod->setLocalLimit(numChip,7,4, ilimit74);
		}

		if (requestJSON.has<jsonxx::Number>(WF_LLIMIT_81)) {
			reg_t ilimit81 = (reg_t)requestJSON.get<jsonxx::Number>(WF_LLIMIT_81);
			socod->setLocalLimit(numChip,8,1,ilimit81);
		}

		if (requestJSON.has<jsonxx::Number>(WF_LLIMIT_82)) {
			reg_t ilimit82 = (reg_t)requestJSON.get<jsonxx::Number>(WF_LLIMIT_82);
			socod->setLocalLimit(numChip,8,2,ilimit82);
		}

		if (requestJSON.has<jsonxx::Number>(WF_LLIMIT_83)) {
			reg_t ilimit83 = (reg_t)requestJSON.get<jsonxx::Number>(WF_LLIMIT_83);
			socod->setLocalLimit(numChip,8,3,ilimit83);
		}

		if (requestJSON.has<jsonxx::Number>(WF_LLIMIT_84)) {
			reg_t ilimit84 = (reg_t)requestJSON.get<jsonxx::Number>(WF_LLIMIT_84);
			socod->setLocalLimit(numChip,8,4, ilimit84);
		}
		socod->setControlReg(0x0020);
		return true;
	}
};


int
main(int argc, char *argv[])
{
	Socod *socod = new Socod();

	std::string documentRoot = DOCUMENT_ROOT;
	if (argc > 1)
		documentRoot = argv[1];
	else
		std::cout << "Invalid argument. Using default path: " << documentRoot << std::endl;

	const char *options[] = {
	    "document_root", documentRoot.c_str(), "listening_ports", PORT, 0};

    std::vector<std::string> cpp_options;
    for (int i=0; i<(sizeof(options)/sizeof(options[0])-1); i++) {
        cpp_options.push_back(options[i]);
    }

	CivetServer server(cpp_options); // <-- C++ style start


	ChannelFrameNumHandler h_channel_frame_num(socod);
	server.addHandler("/channel_frame_num", h_channel_frame_num);

	AcquireDataHandler h_acquire_data(socod);
	server.addHandler("/acquire_data", h_acquire_data);

	WriteRegsHandler h_write_regs(socod);
	server.addHandler("/write_regs", h_write_regs);

	ReadRegsHandler h_read_regs(socod);
	server.addHandler("/read_regs", h_read_regs);

	DARegReadHandler h_d_reg_a_read(socod);
	server.addHandler("/d_reg_a_read", h_d_reg_a_read);

	DARegWriteHandler h_d_reg_a_write(socod);
	server.addHandler("/d_reg_a_write", h_d_reg_a_write);

	StopHandler h_stop(socod);
	server.addHandler("/stop", h_stop);

	SetGlodalLimitHandler h_set_glimit(socod);
	server.addHandler("/set_glim", h_set_glimit);

	SetLocalLimitHandler h_set_ilimit(socod);
	server.addHandler("/set_ilim", h_set_ilimit);


	while (!exitNow) {
#ifdef _WIN32
		Sleep(1000);
#else
		sleep(1);
#endif
	}

	printf("Bye!\n");

	return 0;
}
