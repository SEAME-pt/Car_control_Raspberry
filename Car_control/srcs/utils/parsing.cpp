#include "carControl.h"

std::string to_upper(const std::string &s) {
    std::string out = s;
	for (auto &c : out)
		c = std::toupper(c);
    return out;
}

// Convert input "TRUE"/"FALSE" strings into actual booleans
static bool parseBool(std::string& value, bool defaultValue) {
	if (to_upper(value) == "TRUE")
		return (true);
	if (to_upper(value) == "FALSE")
		return (false);
	
	std::cerr << "Invalid boolean value '" << value 
			  << "'. Use 'true' or 'false'. Using default: " 
			  << (defaultValue ? "true" : "false") << std::endl;
	return (defaultValue);
}

// Parse command line arguments; can override default values
// Mainly used for debugging
int	parsingArgv(int argc, char *argv[], t_carControl *carControl) {

	for (int i = 1; i < argc; i++) {

		std::string arg(argv[i]);
		
		// Parse --joy=true|false
		if (arg.find("--manual=") == 0) {
			std::string value = arg.substr(9);
			carControl->manual = parseBool(value, true);
			
		// Parse --can=INTERFACE
		} else if (arg.find("--can=") == 0) {
			carControl->canInterface = arg.substr(6);
			
		// Parse --help
		} else if (arg == "--help" || arg == "-h") {
			std::cout << "Usage: " << argv[0] << " [options]\n"
					  << "  --manual=true|false  Enable manual mode over autonomous (default: true)\n"
					  << "  --can=INTERFACE   CAN interface (default: can0)\n"
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
