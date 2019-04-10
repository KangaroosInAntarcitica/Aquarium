#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_uart.h"
#include <stdlib.h>

typedef enum {
	WIFI_REQUEST_TYPE_GET,
	WIFI_REQUEST_TYPE_POST
} wifi_request_type;

typedef struct {
	UART_HandleTypeDef* huart;
	char* last_connect;
	char* last_configuration;
	char* last_request;
	char* last_response;
} WifiController;


void wifi_init(WifiController* controller, UART_HandleTypeDef* huart);
int wifi_configure_request(WifiController* controller, wifi_request_type request_type, char* url);
int wifi_send_request(WifiController* controller, char* request_body);
char* wifi_get_response(WifiController* controller);

int wifi_is_connected();
int wifi_connect(WifiController* controller, char* uuid, char* password);
