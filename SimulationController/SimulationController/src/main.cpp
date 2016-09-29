//author: sebastian beyer 

#include <iostream>

#include <boost/array.hpp>
#include <boost/asio.hpp>

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#include <vector>
#include <string>
#include <memory>

#include <opencv2\opencv.hpp>

#include <conio.h>


using boost::asio::ip::udp;
using namespace std;

struct VehicleInput {
	VehicleInput() {
		throttle = 0.0;
		brake = 0.0;
		steering = 0.0;
	}
	VehicleInput(const string &str) {
		fromString(str);
	}

	float throttle; //between 0 and 1
	float brake;	 //between 0 and 1
	float steering; //between -1 and +1
	
	string toString() {
		//converts this struct to a string
		//example: "0.3;0.0;-0.89"
		std::stringstream strstream;
		strstream
			<< throttle << ";"
			<< brake << ";"
			<< steering;

		return strstream.str();
	}
	void fromString(const std::string &input) {
		vector<string> fields;
		boost::algorithm::split(fields, input, boost::is_any_of(";"));
		if (fields.size() != 3) {
			return;
		}
		throttle = boost::lexical_cast<float,string>(fields[0]);
		brake = boost::lexical_cast<float, string>(fields[1]);
		steering = boost::lexical_cast<float, string>(fields[2]);

		return;
	}
};

struct SimulationOutput {
	SimulationOutput() {
	}

	cv::Mat image;

};

class Interface {
public:
	Interface(string ip = "127.0.0.1", int port = 9999) {
		socket.reset(new udp::socket(io_service, udp::endpoint(udp::v4(), 9999)));
		//socket->non_blocking(true);
	}
	void Connect(string remote_ip, int remote_port) {
		remote_endpoint = udp::endpoint(boost::asio::ip::address().from_string(remote_ip), remote_port);
	}
	bool Send(const std::string &message) {
		if (socket.get() == 0) {
			return false;
		}

		size_t bytes_sent = socket->send_to(boost::asio::buffer(message), remote_endpoint);

		if (bytes_sent != message.length()) {
			return false;
		}
	}

private:
	boost::asio::io_service io_service;
	std::unique_ptr<udp::socket> socket;
	udp::endpoint remote_endpoint;
	boost::array<char, 1024> recv_buf;
};

int main() {

	Interface blenderInterface;
	VehicleInput vehicleInput;

	blenderInterface.Connect("127.0.0.1", 9994);

	while (true) {
		char key = _getch();
		
		switch (key) {
		case 'w': 
			vehicleInput.throttle += 0.3;
			vehicleInput.brake = 0.0;
			break;
		case 's': 
			vehicleInput.brake += 0.5;
			vehicleInput.throttle = 0.0;
			break;
		case 'a':
			vehicleInput.steering -= 0.3333;
			break;
		case 'd':
			vehicleInput.steering += 0.3333;
			break;
		case ' ':
			vehicleInput.throttle = 0.0;
			vehicleInput.brake = 0.0;
		default:
			break;
		}

		blenderInterface.Send(vehicleInput.toString());

	}

}

