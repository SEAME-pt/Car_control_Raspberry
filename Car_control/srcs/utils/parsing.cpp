#include "carControl.h"

// Convert input "TRUE"/"FALSE" strings into actual booleans
static bool parseBool(const std::string& value, bool defaultValue = false) {
	if (value == "true" || value == "TRUE" || value == "True")
		return true;
	if (value == "false" || value == "FALSE" || value == "False")
		return false;
	
	std::cerr << "  Invalid boolean value '" << value 
			  << "'. Use 'true' or 'false'. Using default: " 
			  << (defaultValue ? "true" : "false") << std::endl;
	return defaultValue;
}

// Parse command line arguments; can override default values
// Mainly used for debugging
int	parsingArgv(int argc, char *argv[], t_carControl *carControl) {

	for (int i = 1; i < argc; i++) {

		std::string arg(argv[i]);
		
		// Parse --joy=true|false
		if (arg.find("--joy=") == 0) {
			std::string value = arg.substr(6);
			carControl->useJoystick = parseBool(value, true);
			
		// Parse --can=INTERFACE
		} else if (arg.find("--can=") == 0) {
			carControl->canInterface = arg.substr(6);
			
		// Parse --debug
		} else if (arg == "--debug") {
			carControl->debug = true;
			
		// Parse --help
		} else if (arg == "--help" || arg == "-h") {
			std::cout << "Usage: " << argv[0] << " [options]\n"
					  << "  --joy=true|false  Enable joystick (default: true)\n"
					  << "  --can=INTERFACE   CAN interface (default: can0)\n"
					  << "  --debug           Enable debug output\n"
					  << "  --help            Show this help\n" << std::endl;
			carControl->exit = true;
			return (0);
			
		} else {
			std::cerr << "  Unknown option: " << arg << std::endl;
			std::cerr << "  Use --help for more information" << std::endl;
			carControl->exit = true;
			return (0);
		}
	}
	return (1);
}
