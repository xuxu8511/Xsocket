#include <sys/timerfd.h>
#include <stdio.h>
#include <stdlib.h>

#include "customer_client.h"
#include "glog/logging.h"
#include "event_loop.h"
#include "timer_queue.h"
#include "tinyxml2.h"

using namespace marco;
using namespace tinyxml2;

const uint32_t client_size = 10000;

EventLoop* g_loop;
TimerQueue* g_timerQueue;
CustomerClient* g_client[client_size];

void Fun() {
	LOG(INFO) << "timer queue functor, now time: " << std::time(NULL);
	srand((unsigned)time(0));
	for (uint32_t i = 0; i < client_size; i++) {
		//int r = rand() % 2;
		int r = 1;
		if (r == 1 && g_client[i]->IsConnected()) {
			std::string msg("hello");
			g_client[i]->Write(msg);
		}
	}
}

int GetCpuNum() {
	int cpu_num = sysconf(_SC_NPROCESSORS_CONF);
	if (cpu_num <= 1) {
		cpu_num = 1;
	}
	return cpu_num;
}

int main(int argc, char *argv[]) {
	if (argc < 2) {
		std::cout << "input param count less than 2" << std::endl;
		return 0;
	}

	XMLDocument doc;
	XMLError err = doc.LoadFile(argv[1]);
	if (err != XML_SUCCESS) {
		std::cout << "load file config xml failed, path:" << argv[1] << std::endl;
		return 0;
	}

	XMLElement* root = doc.RootElement();
	if (root == NULL) {
		std::cout << "xml not find root element" << std::endl;
		return 0;
	}
	const char *path = root->FirstChildElement("log")->FirstChildElement("path")->GetText();
	const char *level = root->FirstChildElement("log")->FirstChildElement("level")->GetText();
	int levelInt = atoi(level);
	const char *ip = root->FirstChildElement("server")->FirstChildElement("ip")->GetText();
	const char *port = root->FirstChildElement("server")->FirstChildElement("port")->GetText();
	int portInt = atoi(port);

	google::InitGoogleLogging("");
	google::SetLogDestination(levelInt, path);
	FLAGS_logbufsecs = 0; //延时刷新
	FLAGS_max_log_size = 20; //MB
	FLAGS_stderrthreshold = google::FATAL; 

	EventLoop loop;
	g_loop = &loop;

	TimerQueue timerQueue(g_loop);
	g_timerQueue = &timerQueue;
	timerQueue.AddTimer(std::bind(&Fun), std::time(NULL), 10, true);

	for (uint32_t i = 0; i < client_size; i++) {
		LOG(INFO) << "create tcp client i:" << i;
		CustomerClient *client = new CustomerClient(g_loop, ip, portInt);
		g_client[i] = client;
		client->Start();
		usleep(2000);
	}
	g_loop->Loop();

	LOG(INFO) << "STOP SERVER";
	google::ShutdownGoogleLogging();

	return 0;
}
