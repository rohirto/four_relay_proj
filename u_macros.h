#ifndef U_MACROS_H
#define U_MACROS_H
//#define RASP_MQTT       0

#define TOKEN                       "B_B_F_F-wVdGoyx0wgM2LbvIbwhH6Bzo9lnfg6"    /* Remove the underscores while uploading */
#define MQTT_CLIENT_NAME            "ESP07"
/********** Control Variables *******************************/
#ifdef RASP_MQTT
#define SWITCH_1_TOPIC           "home-automation/switch_1"
#define SWITCH_2_TOPIC           "home-automation/switch_2"
#define SWITCH_3_TOPIC           "home-automation/switch_3"
#define SWITCH_4_TOPIC           "home-automation/switch_4"
  
#define SUBSCRIBE()     client.subscribe(SWITCH_1_TOPIC);\
						client.subscribe(SWITCH_2_TOPIC);\
						client.subscribe(SWITCH_3_TOPIC);\
						client.subscribe(SWITCH_4_TOPIC);
                        
#else
#define HOME_AUTO_LABEL             "home-automation"
#define SWITCH_1_TOPIC           "/v1.6/devices/home-automation/cctv_dukan/lv"
#define SWITCH_2_TOPIC           "/v1.6/devices/home-automation/cctv_outdoor/lv"
#define SWITCH_3_TOPIC           "/v1.6/devices/home-automation/maap/lv"
#define SWITCH_4_TOPIC           "/v1.6/devices/home-automation/raspberry/lv"

#define SUBSCRIBE()     char* topicToSubscribe;\
                         sprintf(topicToSubscribe, "%s", "");\
                          sprintf(topicToSubscribe, "%s", SWITCH_1_TOPIC);\
                          client.subscribe(topicToSubscribe);\
                         sprintf(topicToSubscribe, "%s", "");\
                          sprintf(topicToSubscribe, "%s", SWITCH_2_TOPIC);\
                          client.subscribe(topicToSubscribe);\
                          sprintf(topicToSubscribe, "%s", "");\
                          sprintf(topicToSubscribe, "%s", SWITCH_3_TOPIC);\
                          client.subscribe(topicToSubscribe);\
						  sprintf(topicToSubscribe, "%s", "");\
                          sprintf(topicToSubscribe, "%s", SWITCH_4_TOPIC);\
                          client.subscribe(topicToSubscribe);
                         
#endif


#endif
