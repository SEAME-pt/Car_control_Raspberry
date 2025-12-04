#include "CANControllerTest.cpp"
#include "CANProtocolTest.cpp"
#include "CANInitTest.cpp"
#include "CarControlTest.cpp"
#include "JoyTest.cpp"
#include "SocketCANTest.cpp"

int main(int argc, char **argv) {

	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}

/*
Functions tested:

-------- CANController:

CANController(const std::string &interface);
~CANController();
CANController(CANController&& other) noexcept;
CANController& operator=(CANController&& other) noexcept;

void	initialize();
void	cleanup();

void	sendFrame(uint32_t can_id, const int8_t* data, uint8_t len);
void	sendFrameFD(uint32_t can_id, const int8_t* data, size_t len);

bool 				isInitialized() const;
const std::string&	getInterface() const;
int 				getSocket() const;

------- CANProtocol:

inline void sendEmergencyBrake(CANController& can, bool active);
inline void sendDriveCommand(CANController& can, int8_t steering, int8_t throttle);
inline void sendDriveMode(CANController& can, DriveMode mode);

------- socketCAN:

int		check_mtu_support(int s, struct ifreq *ifr);
int		socketCan_init(const char *interface);
int		can_send_frame(int socket, uint32_t can_id, 
					const int8_t *data, uint8_t len);
int		can_send_frame_fd(int socket, uint32_t can_id, 
					const int8_t *data, uint8_t len);
void	can_close(int socket);

------- carControl:

int8_t			joystickSteering(SDL_Joystick* joystick);
int8_t			joystickThrottle(SDL_Joystick* joystick);
std::unique_ptr<CANController>	
				init_can(const std::string &interface);
t_carControl	initCarControl(int argc, char *argv[]);
SDL_Joystick*	initJoystick();
void			cleanExit(SDL_Joystick* joystick);

*/