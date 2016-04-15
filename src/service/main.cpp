#include <sys/timerfd.h>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>

#include "service_server.h"
#include "glog/logging.h"
#include "jemalloc/jemalloc.h"
#include "event_loop.h"
#include "timer_queue.h"
#include "tinyxml2.h"

using namespace marco;
using namespace tinyxml2;

EventLoop* g_loop;
TimerQueue* g_timerQueue;

void write_cb_and_malloc_trim(void *, const char *info) {
	malloc_trim(0);
	LOG(INFO) << info;
}

int GetCpuNum() {
	int cpu_num = sysconf(_SC_NPROCESSORS_CONF);
	if (cpu_num <= 1) {
		cpu_num = 1;
	}
	return cpu_num;
}

int main(int argc, char *argv[]) {
	int cpuNum = GetCpuNum(); 
	if (cpuNum == 1){
		std::cout << "server cpu num not enough" << std::endl;
		return 0;
	}

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
	g_timerQueue->AddTimer(std::bind(malloc_stats_print, &write_cb_and_malloc_trim, nullptr, "abl"), std::time(NULL), 300, true);

	ServiceServer helloServer(g_loop, ip, portInt, cpuNum-1);
	helloServer.start();
	g_loop->Loop();

	google::ShutdownGoogleLogging();
	return 0;
}
