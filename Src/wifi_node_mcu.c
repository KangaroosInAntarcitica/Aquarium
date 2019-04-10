#include "wifi_node_mcu.h"

// MESSAGE PARTS
const char TRUE = '1';
const char FALSE = '0';
const char SEPARATOR = ' ';
const char GET = 'G';
const char POST = 'P';

// COMMANDS
const char WIFI_CONNECT = 'a';
const char IS_CONNECTED = 'c';
const char WIFI_DISCONNECT = 'd';

const char GET_NETWORKS = 'g';
const char READ_NETWORKS = 'r';

const char CONFIGURE_REQUEST = 'u';
const char GET_CONFIGURATION = 'v';
const char SEND_REQUEST = 's';
const char GET_RESPONSE = 't';
const char GET_RESPONSE_CODE = 'o';

const char ECHO = 'z';

// FOR TIMER
const int TIMER_MULTIPLIER = 50;
const int SERIAL_WAIT = 1e5; // micros
const int TIMER_DELAY = 1e3; // micros

const int timeout = 100;

static size_t c_str_size(char *c_str) {
	size_t i = 0;
	while (c_str[i] != '\0') {
		++i;
	}
	return i;
}

static void str_copy(char* source, char* target, size_t length) {
	for (size_t i = 0; i < length; i++) {
		target[i] = source[i];
	}
}

void wifi_init(WifiController* controller, UART_HandleTypeDef* huart) {
	controller->huart = huart;
	controller->last_configuration = NULL;
	controller->last_request = NULL;
	controller->last_response = NULL;
	controller->last_connect = NULL;
}

static int transmit(WifiController* controller, char* command, size_t command_length) {
	HAL_StatusTypeDef status = HAL_UART_Transmit(controller->huart, command, command_length, timeout);
	return status == HAL_OK;
}

static int receive(WifiController* controller, char* response, size_t response_length) {
	int index;
	HAL_StatusTypeDef status = HAL_ERROR;
	for (index = 0; index < response_length; index++) {
		status = HAL_UART_Receive(controller->huart, &response[index], 1, SERIAL_WAIT / 100);
		if (status != HAL_OK) break;
	}

	if (status == HAL_OK || status == HAL_TIMEOUT) {
		return index;
	} else {
		return 0;
	}
}

static int transmit_receive_process(WifiController* controller, char* command, size_t command_size, char* response, size_t response_size) {
	int transmit_result = transmit(controller, command, command_size);
	if (!transmit_result)
		return transmit_result;

	int receive_length = receive(controller, response, response_size);
	if (receive_length < 1)
		return 0;

	// first char of command and response should match
	if (command[0] != response[0])
		return 0;
	return receive_length;
}

int wifi_is_connected(WifiController* controller) {
	char command[1] = {IS_CONNECTED};
	char response[2];
	int response_length = transmit_receive_process(controller, command, sizeof(command), response, sizeof(response));

	return response_length == 2 && response[1] == TRUE;
}

int wifi_connect(WifiController* controller, char* uuid, char* password) {
	free(controller->last_connect);
	controller->last_connect = NULL;

	size_t uuid_length = c_str_size(uuid);
	size_t password_length = c_str_size(password);
	size_t command_length = uuid_length + password_length + 3;
	char* command = malloc(command_length + 1);
	if (controller == NULL) return 0;
	command[0] = WIFI_CONNECT;
	str_copy(uuid, &command[1], uuid_length);
	command[uuid_length + 1] = SEPARATOR;
	str_copy(password, &command[uuid_length + 2], password_length);
	command[uuid_length + password_length + 2] = SEPARATOR;
	command[uuid_length + password_length + 3] = '\0';

	controller->last_connect = command;

	char response[1];
	size_t response_length = transmit_receive_process(controller, command, command_length, response, sizeof(response));
	return response_length == 1;
}

int wifi_configure_request(WifiController* controller, wifi_request_type request_type, char* url) {
	char request_type_code;
	if (request_type == WIFI_REQUEST_TYPE_GET) {
		request_type_code = GET;
	} else if (request_type == WIFI_REQUEST_TYPE_POST) {
		request_type_code = POST;
	} else {
		return 0;
	}

	free(controller->last_configuration);
	controller->last_configuration = 0;

	size_t url_length = c_str_size(url);
	char* command = malloc(url_length + 3);
	if (command == NULL) return 0;
	command[0] = CONFIGURE_REQUEST;
	command[1] = request_type_code;
	str_copy(url, command + 2, url_length + 1);
	size_t command_length = url_length + 2;

	controller->last_configuration = command;

	char response[2];

	size_t response_length = transmit_receive_process(controller, command, command_length, response, sizeof(response));

	return response_length == 2 && response[1] == TRUE;
}

int wifi_send_request(WifiController* controller, char* request_body) {
	free(controller->last_request);
	free(controller->last_response);
	controller->last_request = 0;
	controller->last_response = 0;

	size_t body_length = c_str_size(request_body);
	char* command = malloc(body_length + 2);
	if (command == NULL) return 0;
	command[0] = SEND_REQUEST;
	str_copy(request_body, command + 1, body_length + 1);
	size_t command_length = body_length + 1;

	controller->last_request = command;
	char response[2];

	size_t response_length = transmit_receive_process(controller, command, command_length, response, sizeof(response));

	return response_length == 2 && response[1] == TRUE;
}

char* wifi_get_response(WifiController* controller) {
	free(controller->last_response);
	controller->last_response = 0;

	char command[] = {GET_RESPONSE};

	// allocate a lot of memory
	size_t response_length = 200;
	char* response = malloc(response_length);
	if (response == NULL) return 0;

	response_length = transmit_receive_process(controller, command, 1, response, response_length);

	if (response_length < 1) return 0;

	controller->last_response = malloc(response_length + 1);
	if (controller->last_response == NULL) return 0;
	str_copy(response, controller->last_response, response_length);
	response[response_length] = '\0';

	if (response[1] != TRUE) {
		return 0;
	}

	return controller->last_response + 2;
}

