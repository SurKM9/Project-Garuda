uint32 packet_id
float32 latitude
float32 longitude
float32 altitude
float32 velocity
float32 roll
float32 pitch
float32 yaw
float32 battery_voltage
uint8 battery_pct
uint8 flight_mode
uint8 flight_state

uint8 MODE_MANUAL=0
uint8 MODE_STABILIZE=1
uint8 MODE_LOITER=2
uint8 MODE_AUTO=3
uint8 MODE_GUIDED=4
uint8 MODE_RTL=5

uint8 STATE_IDLE=0
uint8 STATE_ARMED=1
uint8 STATE_TAKEOFF=2
uint8 STATE_FLYING=3
uint8 STATE_LANDING=4
uint8 STATE_EMERGENCY=5
