#pragma once

#include <string>

#include "../Device.h"

namespace micasa {

	class Text final : public Device {

	private:

	public:
		Text( std::shared_ptr<Hardware> hardware_, std::string id_, std::string unit_, std::string name_, std::map<std::string, std::string> settings_ ) : Device( hardware_, id_, unit_, name_, settings_ ) { };

		std::string toString() const;
		void updateValue( std::string newValue_ );

	}; // class Text

}; // namespace micasa
