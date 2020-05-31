#include "FS.h"
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <PubSubClient.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>

//#include "cfg_file.h"
void send_field_uint(const __FlashStringHelper *label,
                     const __FlashStringHelper *name,
                     void *vval, uint8_t mlen, bool hr);
void send_field_str(const __FlashStringHelper *label,
                    const __FlashStringHelper *name,
                    void *vval, uint8_t mlen, bool hr);
void send_field_8ui(const __FlashStringHelper *label,
                    const __FlashStringHelper *name,
                    void *vval, uint8_t mlen, bool hr);
void send_field_hex(const __FlashStringHelper *label,
                    const __FlashStringHelper *name,
                    void *vval, uint8_t mlen, bool hr);
bool read_bytes_wrapper(File &f, void *vval, uint8_t mlen);
bool read_str_wrapper(File &f, void *vval, uint8_t mlen);
bool read_uint16_wrapper(File &f, void *vval, uint8_t mlen);
bool write_bytes_wrapper(File &f, void *vval, uint8_t mlen);
bool write_str_wrapper(File &f, void *vval, uint8_t mlen);
bool write_uint16_wrapper(File &f, void *vval, uint8_t mlen);
bool get_field_str(void *vval, const String &str, uint8_t mlen);
bool get_field_ui16(void *vval, const String &str, uint8_t mlen);

// Use Analog-to-Digital converter for input voltage
ADC_MODE(ADC_VCC);

// a bit of debug facilities
#define DEBUG_ON
#ifdef DEBUG_ON
#define DEBUG_BEGIN do {Serial.begin(115200);Serial.setDebugOutput(true);} while(0);
//#define DEBUG_BEGIN Serial.begin(921600);
#define DEBUG_PRINTLN(x) Serial.println(x)
#define DEBUG_PRINT(x) Serial.print(x)
#else
#define DEBUG_BEGIN
#define DEBUG_PRINTLN(x)
#define DEBUG_PRINT(x)
#endif //DEBUG_ON

/**********************************************************************
  Setup variables and constants
**********************************************************************/

const uint16_t WiFiSetupChannel= 1;
const uint16_t DnsPort= 53;

//Control of MOFSET
const int PwrCtrlPin= D1;

const char emptyStr[]="";
char* ssid     = (char*)emptyStr;
char* password = (char*)emptyStr;
char* mqtt_serv= (char*)emptyStr;
int   mqtt_port= 1883;
char* mqtt_devc= (char*)emptyStr;
char* mqtt_topc= (char*)emptyStr;
char* mqtt_user= (char*)emptyStr;
char* mqtt_pswd= (char*)emptyStr;
char* setup_ssid= (char*)emptyStr;
char* setup_pssw= (char*)emptyStr;
uint8_t wifi_channel= 0;
uint8_t wifi_ap_mac[6]= {0,0,0,0,0,0}; // mark it unset

bool is_wifi_data_set()
{
  for(uint8_t i= 0; i < 6; i++)
    if (wifi_ap_mac[i])
      return true;
  return false;
}

struct VarDesc
{
  const char *label;
  const char *name;
  void *var;
  void (*out_processor)(const __FlashStringHelper *label, const __FlashStringHelper *name, void* param, uint8_t mlen, bool hr);
  bool (*read_cfg)(File &f, void *vval, uint8_t mlen);
  bool (*write_cfg)(File &f, void *vval, uint8_t mlen);
  bool (*in_processor)(void *vval, const String &str, uint8_t mlen);
  uint8_t group;
  uint8_t max_len;
};

const char br_tag[]         PROGMEM= "<br>";
const char ssid_l[]         PROGMEM= "WiFi";
const char ssid_n[]         PROGMEM= "ssid";
const char password_l[]     PROGMEM= "WiFi Password";
const char password_n[]     PROGMEM= "password";
const char mqtt_serv_l[]    PROGMEM= "MQTT Server Address";
const char mqtt_serv_n[]    PROGMEM= "mqtt_serv";
const char mqtt_port_l[]    PROGMEM= "MQTT port";
const char mqtt_port_n[]    PROGMEM= "mqtt_port";
const char mqtt_devc_l[]    PROGMEM= "This devce name";
const char mqtt_devc_n[]    PROGMEM= "mqtt_devc";
const char mqtt_topc_l[]    PROGMEM= "MQTT topic";
const char mqtt_topc_n[]    PROGMEM= "mqtt_topc";
const char mqtt_user_l[]    PROGMEM= "MQTT user";
const char mqtt_user_n[]    PROGMEM= "mqtt_user";
const char mqtt_pswd_l[]    PROGMEM= "MQTT password";
const char mqtt_pswd_n[]    PROGMEM= "mqtt_pswd";
const char setup_ssid_l[]   PROGMEM= "Setup SSID";
const char setup_ssid_n[]   PROGMEM= "setup_ssid";
const char setup_pssw_l[]   PROGMEM= "Setup password";
const char setup_pssw_n[]   PROGMEM= "setup_pssw";
const char wifi_channel_l[] PROGMEM= "Wifi channel";
const char wifi_ap_mac_l[]  PROGMEM= "Wifi AP MAC";

const VarDesc Parameters[]=
{
  {wifi_channel_l, 0,            &wifi_channel, &send_field_8ui,
  &read_bytes_wrapper,  &write_bytes_wrapper,  0,
  0,  1},

  {wifi_ap_mac_l,  0,            wifi_ap_mac, &send_field_hex,
  &read_bytes_wrapper,  &write_bytes_wrapper,  0,
  0,  6},

  {ssid_l,         ssid_n,       &ssid,        &send_field_str,
  &read_str_wrapper,    &write_str_wrapper,    &get_field_str,
  1, 30},

  {password_l,     password_n,   &password,    &send_field_str,
  &read_str_wrapper,    &write_str_wrapper,    &get_field_str,
  1, 40},

  {mqtt_serv_l,    mqtt_serv_n,  &mqtt_serv,   &send_field_str,
  &read_str_wrapper,    &write_str_wrapper,    &get_field_str,
  2, 50},

  {mqtt_port_l,    mqtt_port_n,  &mqtt_port,   &send_field_uint,
  &read_uint16_wrapper, &write_uint16_wrapper, &get_field_ui16,
  2, 10},

  {mqtt_devc_l,    mqtt_devc_n,  &mqtt_devc,   &send_field_str,
  &read_str_wrapper,    &write_str_wrapper,    &get_field_str,
  2, 10},

  {mqtt_topc_l,    mqtt_topc_n,  &mqtt_topc,   &send_field_str,
  &read_str_wrapper,    &write_str_wrapper,    &get_field_str,
  2, 50},

  {mqtt_user_l,    mqtt_user_n,  &mqtt_user,   &send_field_str,
  &read_str_wrapper,    &write_str_wrapper,    &get_field_str,
  2, 50},

  {mqtt_pswd_l,    mqtt_pswd_n,  &mqtt_pswd,   &send_field_str,
  &read_str_wrapper,    &write_str_wrapper,    &get_field_str,
  2, 50},

  {setup_ssid_l,   setup_ssid_n, &setup_ssid,  &send_field_str,
  &read_str_wrapper,    &write_str_wrapper,    &get_field_str,
  3, 20},

  {setup_pssw_l,   setup_pssw_n, &setup_pssw,  &send_field_str,
  &read_str_wrapper,    &write_str_wrapper,    &get_field_str,
  3, 50}
};

const char setup_file[] PROGMEM= "/ActButton.cfg";

/**********************************************************************
  Global objects (an old Arduino tradition :) )
**********************************************************************/

WiFiClient espClient;
PubSubClient mqtt(espClient);
int lvl= HIGH;
int counter;


ESP8266WebServer setup_server(80);
DNSServer *DnsServer;
void WifiSetMode(WiFiMode_t wifi_mode);
void WifiManagerBegin();

/**********************************************************************
  Arduino setup
**********************************************************************/

void setup()
{
  int bat;
  bool lwf_ok= false;
  unsigned long strt= millis();

  /* First suport power for this module */
  pinMode(PwrCtrlPin, OUTPUT);
  digitalWrite(PwrCtrlPin, LOW);

  DEBUG_BEGIN;
  DEBUG_PRINTLN("Start");

  /* Blinking to sho that we are alive */
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, (lvl=!lvl));

  /* Read Config */
  if(!SPIFFS.begin())
  {
    DEBUG_PRINTLN("Format SPIFFS");
    SPIFFS.format();
    SPIFFS.begin();
  }
  lwf_ok= readCfg();
  DEBUG_PRINT("Is read Config ok:");
  DEBUG_PRINTLN(lwf_ok);
  if (!lwf_ok)
    goto setup;

  /* Connect to Wifi */
  WifiSetMode(WIFI_STA);
  if (is_wifi_data_set())
    WiFi.begin(ssid, password, wifi_channel, wifi_ap_mac, true );
  else
    WiFi.begin(ssid, password);
  digitalWrite(LED_BUILTIN, (lvl=!lvl));


  /* Read battery vltage */
  bat= ESP.getVcc();
  DEBUG_PRINT("Batterie:");
  DEBUG_PRINTLN(ESP.getVcc());

  /* Wait for WiFi to be connected */
  counter= 1000;
  while (WiFi.status() != WL_CONNECTED)
  {
    if ((counter % 10) == 0)
    {
      digitalWrite(LED_BUILTIN, (lvl=!lvl));
      DEBUG_PRINT(".");
    }
    delay(10);
    if ((--counter)==0)
      goto setup;
  }
  DEBUG_PRINTLN(" connected!");

  /* if AP/channel we are conected to changed write config */
  if(wifi_channel != WiFi.channel() ||
     memcmp(wifi_ap_mac, WiFi.BSSID(), 6) != 0)
  {
    /* TODO: write only channel and AP MAC (if it has sens) */
    wifi_channel= WiFi.channel();
    memcpy(wifi_ap_mac, WiFi.BSSID(), 6);
    writeCfg();
  }
  digitalWrite(LED_BUILTIN, (lvl=!lvl));

  /* Connect to MQTT and sent the meaaege */
  mqtt.setServer(mqtt_serv, (int)mqtt_port);
  if (mqtt.connect(mqtt_devc, mqtt_user, mqtt_pswd))
  {
    char buff[20];
    digitalWrite(LED_BUILTIN, (lvl=!lvl));
    //itoa(millis() - strt, buff, 10);
    itoa(bat, buff, 10);
    mqtt.publish(mqtt_topc,buff);
    digitalWrite(LED_BUILTIN, (lvl=!lvl));
  }
  digitalWrite(LED_BUILTIN, (lvl=!lvl));
  /* disconnect from MQLL (to be sure that the message is sent) */
  mqtt.disconnect();
  DEBUG_PRINTLN("Mqtt Disconnect...");
  digitalWrite(LED_BUILTIN, (lvl=!lvl));

  /************
    Power down
  *************/
  digitalWrite(PwrCtrlPin, HIGH);

  /*
    If user keep pressing button wait 10 Sec to be sure about his/her
    intentions to enter setup
  */
  for(counter= 0; counter < 10; counter++)
  {
    digitalWrite(LED_BUILTIN, (lvl=!lvl));
    delay(1000);
  }

setup:
  /* Switch power support again to allow user use both hands during setup */
  digitalWrite(PwrCtrlPin, LOW);
  DEBUG_PRINTLN("SETUP!");

  /* setup wifi AP */
  WifiManagerBegin();
  DEBUG_PRINT("AP IP address: ");
  DEBUG_PRINTLN(WiFi.softAPIP());
  digitalWrite(LED_BUILTIN, (lvl=!lvl));

  /* setup Web server */
  setup_server.on("/", handleRoot);
  setup_server.onNotFound(handleNotFound);
  setup_server.on("/sv",handleSave);
  setup_server.on("/cn",handleShutdown);
  setup_server.on("/rs",handleReset);
  setup_server.begin();
  counter= 0;
  delay(100);
  digitalWrite(LED_BUILTIN, (lvl=!lvl));
}

/**********************************************************************
  Arduino loop
**********************************************************************/

void loop()
{
  // fast blinking to show we are in wetup mode
  if (counter % 40 == 0)
    digitalWrite(LED_BUILTIN, (lvl=!lvl));

  // process requests
  setup_server.handleClient();
  if (DnsServer)
    DnsServer->processNextRequest();
  delay(10);
  counter++;
}

/**********************************************************************
  WEB interface
**********************************************************************/

static char* itoa(char *buff, uint8_t len, unsigned int val)
{
  char *c= buff + (len - 1);
  *(c)= '\0';
  if (val)
  {
    while (val && c > buff)
    {
      *(--c)= '0' + (val%10);
      val/= 10;
    }
  }
  else
    *(--c)= '0';
  return c;
}

// show field unsigned integer field on web
void send_field_uint(const __FlashStringHelper *label,
                     const __FlashStringHelper *name,
                     void *vval, uint8_t mlen, bool hr)
{
  unsigned int val= *((unsigned int*)vval);
  char buff[20];
  char *c= itoa(buff, sizeof(buff), val);
  send_field_str(label, name, (void*)&c, mlen, hr);
}

// show field string field on web
void send_field_str(const __FlashStringHelper *label,
                    const __FlashStringHelper *name,
                    void *vval, uint8_t mlen, bool hr)
{
  char *val= *((char **)vval);
  String content;
  char *c= val;
  content.reserve(128);
  content= (hr ? "<hr>": "");
  content+= F("<label for=\"");
  content+= name;
  content+= F("\">");
  content+= label;
  content+= F("</label><br><input type=\"text\" id=\"");
  content+= name;
  content+= F("\" name=\"");
  content+= name;
  content+= F("\" value=\"");
  for(char *c= val; (*c); c++)
  {
    if ((*c) == '"')
      content+="&#x22;";
    else
      content+= (*c);
  }
  content+= F("\"><br>");
  setup_server.sendContent(content);
}


// show byte read only on web
void send_field_8ui(const __FlashStringHelper *label,
                    const __FlashStringHelper *name,
                    void *vval, uint8_t mlen, bool hr)
{
  char buff[4];
  uint8_t val= *((uint8_t*)vval);
  char *c= itoa(buff, sizeof(buff), val);
  String content;
  content.reserve(128);
  content= (hr ? "<hr>": "");
  content+= label;
  content+= FPSTR(br_tag);
  content+= c;
  content+= FPSTR(br_tag);
  setup_server.sendContent(content);
}

// show byte string in hex on web (read only)
void send_field_hex(const __FlashStringHelper *label,
                    const __FlashStringHelper *name,
                    void *vval, uint8_t mlen, bool hr)
{
  uint8_t *val= ((uint8_t*)vval);
  String content;
  char s[]="0123456789ABCDEF";
  content.reserve(128);
  content= (hr ? "<hr>": "");
  content+= label;
  content+= FPSTR(br_tag);

  for(int i= 0; i < mlen; i++)
  {
    content+= s[(val[i] >> 4)];
    content+= s[(val[i] & 0x0F)];
    content+= ' ';
  }
  content+= FPSTR(br_tag);
  setup_server.sendContent(content);
}

// root web page with setup
void handleRoot()
{
  DEBUG_PRINTLN("Root page of web");
  if (!setup_server.chunkedResponseModeStart(200, "text/html"))
  {
    setup_server.send(505, F("text/html"), F("HTTP1.1 required"));
    return;
  }
  setup_server.sendContent(F("<!DOCTYPE html><html><body><h3>Action button Setup</h3><form action=\"/sv\" method=\"post\">"));

  uint8_t prev_group= 0;
  for (int i= 0; i < sizeof(Parameters)/sizeof(VarDesc); i++)
  {
    //DEBUG_PRINT("show ");
    //DEBUG_PRINTLN(FPSTR(Parameters[i].label));
    (*Parameters[i].out_processor)(FPSTR(Parameters[i].label),
    FPSTR(Parameters[i].name), Parameters[i].var, Parameters[i].max_len, prev_group!=Parameters[i].group);
    prev_group= Parameters[i].group;
  }
  setup_server.sendContent(F("<hr><input type=\"submit\" value=\"OK\"></form><hr>"
                             "<form action=\"/cn\"><input type=\"submit\" value=\"CANCEL\"></form><hr>"
                             "<form action=\"/rs\"><input type=\"submit\" value=\"RESET\"></form><hr></body></html>"));
  setup_server.chunkedResponseFinalize();
}

/*
  Reads string send to web server.

  Retund FALSE on success and TRUE on fail
*/
bool get_field_str(void *vval, const String &str, uint8_t mlen)
{
  if (str.length() > mlen)
    return true;

  uint8_t alen= str.length() + 1;
  char *res= (char*)malloc(alen);
  if (!res)
    return true;

  strlcpy(res, str.c_str(), alen);
  if (*((char **)vval) != emptyStr)
    free(*((char **)vval));
  *((char **)vval)= res;
  return false; //success
}

/*
  Reads uint16_t send to web server.

  Retund FALSE on success and TRUE on fail
*/
bool get_field_ui16(void *vval, const String &str, uint8_t mlen)
{
  if (str.length() > mlen)
    return true;

  *((uint16_t *)vval)= str.toInt();
  return false; //success
}

// take and save parameters from web
void handleSave()
{
  // check all parameters
  bool ok= true;
  for (int i= 0; i < sizeof(Parameters)/sizeof(VarDesc); i++)
  {
    if (!FPSTR(Parameters[i].name))
      continue;
    if (setup_server.hasArg(FPSTR(Parameters[i].name)))
    {
      if ((*Parameters[i].in_processor)(Parameters[i].var,
                                        setup_server.arg(FPSTR(
                                                         Parameters[i].name)),
                                        Parameters[i].max_len))
        ok= false;
    }
    else
      ok= false;
  }
  if (ok)
  {
    // Parameters are OK, save and power off
    writeCfg();
    setup_server.send(200, "text/html",
                      F("<!DOCTYPE html><html><body><h3>"
                        "Config written. Shutdown..."
                        "</h3></body></html>"));
    delay(500);
    digitalWrite(PwrCtrlPin, HIGH);
    return;
  }
  // show setup page
  handleRoot();
}

// CANCEL button - just power off
void handleShutdown()
{
  setup_server.send(200, "text/html",
                    F("<!DOCTYPE html><html><body><h3>"
                      "Shutdown..."
                      "</h3></body></html>"));
  delay(500);
  digitalWrite(PwrCtrlPin, HIGH);
}

// RESET button, format FS and power off
void handleReset()
{
  SPIFFS.format();
  setup_server.send(200, "text/html",
                    F("<!DOCTYPE html><html><body><h3>"
                      "Setings removed.Shutdown..."
                      "</h3></body></html>"));
  delay(500);
  digitalWrite(PwrCtrlPin, HIGH);
}

// if page is not found - redirect on root page
void handleNotFound(void)
{
 setup_server.sendHeader("Location", "/",true);//Redirect to our html web page
 setup_server.send(302, "text/plane","");
}


/**********************************************************************
  WIFi management
**********************************************************************/

/*
  Switches WiFi modes

  Inspired by Tasmota project
*/
void WifiSetMode(WiFiMode_t wifi_mode)
{
  if (WiFi.getMode() == wifi_mode)
    return;

  if (wifi_mode != WIFI_OFF)
  {
    WiFi.forceSleepWake(); // Make sure WiFi is really active.
    delay(100);
  }

  uint32_t retry = 2;
  while (!WiFi.mode(wifi_mode) && retry--)
  {
    DEBUG_PRINTLN("Retry set Mode...");
    delay(100);
  }
  digitalWrite(LED_BUILTIN, (lvl=!lvl));

  if (wifi_mode == WIFI_OFF)
  {
    delay(1000);
    WiFi.forceSleepBegin();
    digitalWrite(LED_BUILTIN, (lvl=!lvl));
    delay(1);
  }
  else
  {
    delay(30); // Must allow for some time to init.
  }
}

/*
  Switch on setup AP

  Inspired by Tasmota project
*/
void WifiManagerBegin()
{
  WifiSetMode(WIFI_AP);

  DnsServer = new DNSServer();

  // bool softAP(const char* ssid, const char* passphrase = NULL, int channel = 1, int ssid_hidden = 0, int max_connection = 4);
  if (setup_ssid == emptyStr)
    WiFi.softAP("ActiveButton", "ab1234567890ab", WiFiSetupChannel, 0, 1);
  else
    WiFi.softAP(setup_ssid, setup_pssw, WiFiSetupChannel, 0, 1);

  delay(500); // Without delay I've seen the IP address blank
  digitalWrite(LED_BUILTIN, (lvl=!lvl));

  /* Setup the DNS server redirecting all the domains to the apIP */
  DnsServer->setErrorReplyCode(DNSReplyCode::NoError);
  DnsServer->start(DnsPort, "*", WiFi.softAPIP());
}

/**********************************************************************
  Configuration interface
**********************************************************************/

bool read_bytes_wrapper(File &f, void *vval, uint8_t mlen)
{
  return read_cfg_bytes(f, ((uint8_t *)vval), mlen);
}
bool read_str_wrapper(File &f, void *vval, uint8_t mlen)
{
  return (*((char **)vval)= read_cfg_string(f, mlen)) != NULL;
}
bool read_uint16_wrapper(File &f, void *vval, uint8_t mlen)
{
  return read_cfg_uint16(f, *((uint16_t *)vval));
}

/*
  Reads all confuguration

  return TRUE on success and FALSE on fail
*/
bool readCfg()
{
  DEBUG_PRINTLN("read_cfg");
  if (!SPIFFS.exists(FPSTR(setup_file)))
  {
    DEBUG_PRINTLN("No cfg found");
    return false;
  }

  File f = SPIFFS.open(FPSTR(setup_file), "r");
  if (!f)
  {
    DEBUG_PRINTLN("Can't cfg open");
    return false;
  }

  // check file version
  if (check_leader(f, "AB") != 1)
  {
    DEBUG_PRINTLN("Bad cfg header");
    goto err;
  }

  // Free setup strings
  for (int i= 0; i < sizeof(Parameters)/sizeof(VarDesc); i++)
  {
    if(Parameters[i].out_processor == &send_field_str)
    {
      char *v= *((char **)Parameters[i].var);
      if (v != emptyStr)
        free(v);
      *((char **)Parameters[i].var)= 0;
    }
  }
  // Read setup
  for (int i= 0; i < sizeof(Parameters)/sizeof(VarDesc); i++)
  {
    DEBUG_PRINT("Read cfg :");
    DEBUG_PRINTLN(FPSTR(Parameters[i].label));
    if(!(*Parameters[i].read_cfg)(f, Parameters[i].var, Parameters[i].max_len))
      goto err;
  }

  f.close();
  return true;

err:
  // Reading setup failed so we clean up variables
  DEBUG_PRINTLN("cfg read Error!!!");
  for (int i= 0; i < sizeof(Parameters)/sizeof(VarDesc); i++)
  {
    if(Parameters[i].out_processor == &send_field_str)
    {
      if (*((char **)Parameters[i].var) != emptyStr)
        free(*((char **)Parameters[i].var));
      *((char **)Parameters[i].var)= (char*)emptyStr;
    }
  }
  DEBUG_PRINTLN("Parameters freed");
  f.close();
  return false;
}


bool write_bytes_wrapper(File &f, void *vval, uint8_t mlen)
{
  return write_cfg_bytes(f, ((uint8_t *)vval), mlen);
}
bool write_str_wrapper(File &f, void *vval, uint8_t mlen)
{
  return write_cfg_string(f, *((char **)vval));
}
bool write_uint16_wrapper(File &f, void *vval, uint8_t mlen)
{
  return write_cfg_uint16(f, *((uint16_t *)vval));
}

/*
  Writes all confuguration

  return TRUE on success and FALSE on fail
*/
bool writeCfg()
{
  DEBUG_PRINTLN("opening setup_file");
  File f = SPIFFS.open(FPSTR(setup_file), "w");
  if (!f)
  {
    DEBUG_PRINTLN("Can't open cfg for write");
    return false;
  }

  DEBUG_PRINTLN("writing setup_wifi_file");
  if (f.write((const byte*)"AB\1", 3) != 3)
  {
    DEBUG_PRINTLN("Can't write leader");
    goto err;
  }

  for (int i= 0; i < sizeof(Parameters)/sizeof(VarDesc); i++)
  {
    DEBUG_PRINT("Write cfg :");
    DEBUG_PRINTLN(FPSTR(Parameters[i].label));
    if(!(*Parameters[i].write_cfg)(f, Parameters[i].var, Parameters[i].max_len))
      goto err;
  }

  f.close();
  return true;

err:
  DEBUG_PRINTLN("cfg write Error!!!");
  f.close();
  return false;
}
