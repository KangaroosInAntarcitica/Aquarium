#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

String data = String("");
String data_out = String("");

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
const int SERIAL_WAIT = 1e4; // micros
const int TIMER_DELAY = 1e3; // micros
unsigned int last_serial = 0;
void ICACHE_RAM_ATTR onTimerISR() { timerEvent(); }

// DATA
String wifi_connect_data = String();
int wifi_connect_needed = 0;

int read_networks_needed = 0;
String read_networks_result = String();

char request_type = 0;
String request_url = String();
String request_body = String();
int request_needed = 0;
int response_code = 0;
String response = String();

void setup() {
  // LED
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

  // Serial
  Serial.begin(9600);
  delay(10);

  WiFi.begin();
  delay(10);

  // Timer
  timer1_attachInterrupt(onTimerISR);
  timer1_enable(TIM_DIV16, TIM_EDGE, TIM_SINGLE);
  timer1_write(TIMER_DELAY * TIMER_MULTIPLIER);
}

void loop() {
  wifi_connect();
  read_networks();
  send_request();
}

void timerEvent() {
  if (Serial.available()) {
    digitalWrite(LED_BUILTIN, LOW);
    
    char data_byte = Serial.read();
    data.concat(data_byte);

    last_serial = micros();
  }

  unsigned int time = micros();
  if (last_serial && time - last_serial > SERIAL_WAIT) {
    digitalWrite(LED_BUILTIN, HIGH);

    if (data.length() > 0) {
      process_serial();
      data = String();
    }
  }
  
  timer1_write(TIMER_DELAY * TIMER_MULTIPLIER);
}

void process_serial() {
  char com = data.charAt(0);
  data_out = String();

  switch(com) {
    case WIFI_CONNECT: serial_wifi_connect(); break;
    case IS_CONNECTED: serial_is_connected(); break;
    case WIFI_DISCONNECT: serial_wifi_disconnect(); break;
    case GET_NETWORKS: serial_get_networks(); break;
    case READ_NETWORKS: serial_read_networks(); break;
    case CONFIGURE_REQUEST: serial_configure_request(); break;
    case GET_CONFIGURATION: serial_get_configuration(); break;
    case SEND_REQUEST: serial_send_request(); break;
    case GET_RESPONSE: serial_get_response(); break;
    case GET_RESPONSE_CODE: serial_get_response_code(); break;
    case ECHO: serial_echo(); break;
  }

  if (data_out.length() > 0) {
    Serial.print(data_out);
  }
}

// FUNCTIONS FOR SERIAL RESPONSE

void serial_is_connected() {
  // Output [IS_CONNECTED] [TRUE/FALSE]
  int connected = WiFi.status() == WL_CONNECTED;
  data_out.concat(IS_CONNECTED);
  data_out.concat(connected ? TRUE : FALSE);
}

void serial_get_networks() {
  // Output [GET_NETWORKS] [number] [[TRUE/FALSE - with password] [NETWORK_NAME] [SEPARATOR]]
  int ready = !read_networks_needed && read_networks_result.length() > 0;
  if (ready) {
    data_out = read_networks_result;
  }
  else {
    data_out.concat(GET_NETWORKS);
    data_out.concat((byte) 0);
  }
}

void serial_read_networks() {
  // Output [READ_NETWORKS] [TRUE/FALSE read finised]
  int ready = !read_networks_needed && read_networks_result.length() > 0;
  data_out.concat(READ_NETWORKS);
  data_out.concat(ready ? TRUE : FALSE);
  read_networks_needed = true;
}

void serial_wifi_connect() {
  // Input [ssid] [SEPARATOR] [password] [SEPARATOR]
  // Output [WIFI_CONNECT]
  wifi_connect_data = data;
  wifi_connect_needed = 1;
  
  data_out.concat(WIFI_CONNECT);
}

void serial_wifi_disconnect() {
  // Output [WIFI_DISCONNECT]
  WiFi.persistent(false);
  WiFi.disconnect(true);
  data_out.concat(WIFI_DISCONNECT);
}

void serial_configure_request() {
  // Input [CONFIGURE_REQUEST] [POST / GET] [url]
  // Output [CONFIGURE_REQUEST] [TRUE/FALSE - success]
  data_out.concat(CONFIGURE_REQUEST);
  if (request_needed) {
    data_out.concat(FALSE); return;
  }

  char type = 0;
  if (data.length() > 1)
    type = data.charAt(1);

  if (type != GET && type != POST) {
    data_out.concat(FALSE); return;
  } else {
    request_type = type;
  }

  request_url = String();
  for (int i = 2; i < data.length(); i++) {
    request_url.concat(data.charAt(i));
  }

  data_out.concat(TRUE);
}

void serial_get_configuration() {
  // Output [GET_CONFIGURATION] [POST / GET] [url]
  data_out.concat(GET_CONFIGURATION);
  data_out.concat((char) request_type);

  for (int i = 0; i < request_url.length(); i++) {
    data_out.concat(request_url.charAt(i));
  }
}

void serial_send_request() {
  // Input [SEND_REQUEST] [body]
  // Output [SEND_REQUEST] [TRUE/FALSE success]
  data_out.concat(SEND_REQUEST);
  // if a request is already being sent
  if (request_needed) {
    data_out.concat(FALSE);
    return;
  }
  
  request_body = String();

  for (int i = 1; i < data.length(); i++) {
    request_body.concat(data.charAt(i));
  }

  request_needed = 1;
  response = String();
  response_code = 0;
  data_out.concat(TRUE);
}

void serial_get_response() {
  // Output: [GET_REPONSE] [TRUE/FALSE response present] [response]
  data_out.concat(GET_RESPONSE);
  data_out.concat(response_code == 0 ? FALSE : TRUE);
  data_out.concat(response);
}

void serial_get_response_code() {
  // Output: [CODE as characters, 0 - if none]
  data_out.concat(GET_RESPONSE_CODE);
  data_out.concat(response_code);
}

void serial_echo() {
  // Output: [ECHO]
  data_out.concat(ECHO);
}


// FUNCTIONS FOR EXECUTION INSIDE LOOP

void wifi_connect() {
  if (!wifi_connect_needed) return;
  
  String ssid = String("");
  String password = String("");
  
  int i; // skip first char
  for (i = 1; i < wifi_connect_data.length(); ++i) {
    if (wifi_connect_data.charAt(i) == SEPARATOR) { ++i; break; }
    ssid.concat(wifi_connect_data.charAt(i));
  }
  for (i = i; i < wifi_connect_data.length(); ++i) {
    if (wifi_connect_data.charAt(i) == SEPARATOR) { ++i; break; }
    password.concat(wifi_connect_data.charAt(i));
  }

  const char* c_ssid = ssid.c_str();
  const char* c_password = password.c_str();

  wifi_disconnect();
  // no password
  if (password.length() == 0) {
    WiFi.begin(c_ssid);
  } else {
    WiFi.begin(c_ssid, c_password);
  }
  delay(10);

  wifi_connect_needed = 0;
}

void read_networks() {
  if (!read_networks_needed) return;
  String result = String("");

  wifi_disconnect();
  int n = WiFi.scanNetworks();
  result.concat(GET_NETWORKS);
  result.concat((byte) n);

  for (int i = 0; i < n; i++) {
    // has password or not
    result.concat(WiFi.encryptionType(i) == ENC_TYPE_NONE ? FALSE : TRUE);
    // hotspot name
    String ssid = WiFi.SSID(i);
    result.concat(ssid);
    result.concat(SEPARATOR);
  }

  // reconnect back
  WiFi.begin();

  read_networks_result = result;
  read_networks_needed = 0;
}

void send_request() {
  if (!request_needed) return;
  if (request_url.length() < 1 || (request_type != GET && request_type != POST)) {
    request_needed = 0;
    return;
  }
  
  const char* url = request_url.c_str();
  HTTPClient http;

  http.begin(url);
  if (request_type == GET) {
    response_code = http.GET();
  }
  else if (request_type == POST) {
    const char* body = request_body.c_str();
    response_code = http.POST(body);
  }
  response = http.getString();
  http.end();

  request_needed = 0;
}

// HELPER FUNCIONS FOR SERIAL AND WIFI CONNECTION

void wifi_disconnect() {
  WiFi.persistent(false);
  WiFi.disconnect(true);
}

void wifi_send_data() {
  const char* url = "http://kangaroosinantarctica.pythonanywhere.com/aquarium/data";
  HTTPClient http;

  char message[1000];
  int i;
  
  http.begin(url); //HTTP
  int httpCode = http.POST(message);
  http.end();
}

void serial_get_data() {
  for (int i = 0; i < 1e4 && Serial.available() == 0; ++i) {}
  delay(100);

  data = String("");
  int data_size = Serial.available();
  for (int i = 0; i < data_size; i++) {
     byte new_byte = Serial.read();
     data.concat((char) new_byte);
  }
}
