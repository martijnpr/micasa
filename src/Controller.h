#pragma once

#ifdef _DEBUG
#include <cassert>
#endif // _DEBUG

#include <chrono>
#include <mutex>
#include <forward_list>
#include <vector>

#include "WebServer.h"
#include "Database.h"
#include "Worker.h"
#include "Hardware.h"
#include "Event.h"

extern "C" {
	
	#include "lua.h"
	#include "lualib.h"
	#include "lauxlib.h"

} // extern "C"

namespace micasa {

	class Controller final : public Worker, public LoggerInstance, public WebServerResource, public std::enable_shared_from_this<Controller> {

	public:
		Controller();
		~Controller();

		void start();
		void stop();
		std::string toString() const;
		bool handleRequest( std::string resource_, Method method_, std::map<std::string, std::string> &data_ ) { return true; /* not implemented yet */ }

		std::shared_ptr<Hardware> declareHardware( Hardware::HardwareType hardwareType_, std::string unit_, std::string name_, std::map<std::string, std::string> settings_ );
		
	protected:
		std::chrono::milliseconds _work( unsigned long int iteration_ );

	private:
		std::forward_list<std::shared_ptr<Event> > m_eventQueue;
		std::mutex m_eventQueueMutex;

		std::vector<std::shared_ptr<Hardware> > m_hardware;
		std::mutex m_hardwareMutex;

	}; // class Controller

}; // namespace micasa
