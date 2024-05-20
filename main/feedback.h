/*
  __  __  ____ _______ ____  _____  _      _____ _   _ ______
 |  \/  |/ __ \__   __/ __ \|  __ \| |    |_   _| \ | |  ____|
 | \  / | |  | | | | | |  | | |__) | |      | | |  \| | |__
 | |\/| | |  | | | | | |  | |  _  /| |      | | | . ` |  __|
 | |  | | |__| | | | | |__| | | \ \| |____ _| |_| |\  | |____
 |_|  |_|\____/  |_|  \____/|_|  \_\______|_____|_| \_|______|

*/

#ifndef _FEEDBACK_H_
#define _FEEDBACK_H

#include <stdio.h>
#include "users.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "EG91.h"

#define ALARM_OWNER_WANT_RECEIVE_FEEDBACK_FROM_OTHER_USERS 1
#define ALARM_OWNER_NOT_WANT_RECEIVE_FEEDBACK_FROM_OTHER_USERS 0

#define ALARM_OWNER_WANT_RECEIVE_FEEDBACK_INPUT_CHANGE_STATE 1
#define ALARM_OWNER_NOT_WANT_RECEIVE_FEEDBACK_INPUT_CHANGE_STATE 0

#define FEEDBACK_I1_NO 1 // 1 logico
#define FEEDBACK_I1_NC 2 // 0 logico
#define FEEDBACK_I2_NO 4 // 1 logico
#define FEEDBACK_I2_NC 8 // 0 logico



extern uint8_t label_Alarm_I2_Task_Activated;

extern QueueHandle_t NO_CARRIER_Call_queue;

extern TaskHandle_t handle_Alarm_Calls_Task;

typedef struct FEEDBACK_SMS_DATA
{

  struct
  {
    uint8_t relay1_Activation;
    uint64_t alarm_I1_nowTime;
    char alarm_SMS_Data[100];
    char alarm_SMS_Number[20];
    uint8_t user_Have_In_List;

  } Alarm_I1_Send_SMS_Parameters;

  struct
  {
    uint8_t relay1_Activation;
    uint8_t relay2_Activation;
    char alarm_SMS_Data[100];
    char alarm_SMS_Number[20];
    uint8_t user_Have_In_List;

  } Normal_Feedback_Send_SMS_Parameters;

  data_EG91_Send_SMS feedback_Data_Send_SMS;

} Feedback_SMS_Data;

extern Feedback_SMS_Data feedback_SMS_Data;

uint8_t check_IF_Feedback_Number(char *phone_Number);
typedef struct Feedback_Numbers
{
  char F1[30];
  char F2[30];
  char F3[30];
  char F4[30];
} feedback_numbers;



typedef struct Feedback_Configurations
{
  uint8_t normal_FB_Mode;
  
  char phone1[20];
  char phone2[20];
  char phone3[20];
  char phone4[20];
  char phone5[20];
  char phone6[20];

  uint8_t time_I1;
  uint8_t time_I2;
  
  uint8_t input_Config;

  struct
  {
    uint8_t A;    
    
    uint8_t other_Users_Feedback;
    uint8_t change_Input_State_Feedback;
  } alarmMode;

} feedback_configurations;

extern feedback_configurations fd_configurations;

void task_sendFeedbackData(void *pvParameter);
void task_Alarm_CALL(void *pvParameter);

void send_Normal_Feedback(uint8_t inputValue, uint8_t inputNumber);
void send_SMS_TO_Feedback_List(char *msg);

char *add_Feedback_number(uint8_t line, char *payload);
char *read_Feedback_number(uint8_t line);

uint8_t delete_Feedback_number(uint8_t line);

void alarm_I1_Check_And_Save_Data(char *phone_Number);
void normal_Feedback_Check_And_Save_Data(char *phone_Number,uint8_t releNumber);

void clear_All_Feedback_number();
void write_FbNumbers_Struct();

char *parse_FeedbackData(uint8_t BLE_SMS_Indication, uint8_t feedbackNumber, char cmd, char param, char *payload);

char *parse_Alarm_Data(uint8_t BLE_SMS_Indication, uint8_t feedbackNumber, char cmd, char param, char *payload);

uint8_t set_Alarm_Other_Users_Feedback_Mode();
uint8_t set_Alarm_Change_Input_State_Feedback();
uint8_t set_Normal_Feedback_Mode();
uint8_t reset_Alarm_Other_Users_Feedback_Mode();
uint8_t reset_Alarm_Change_Input_State_Feedback();
uint8_t reset_Normal_Feedback_Mode();

void task_Alarm_CALL(void *pvParameter);

uint8_t alarm_Ativation();
uint8_t alarm_Turn_OFF();
uint8_t modify_Alarm_Input_configuration(char *payload);
uint8_t modify_Alarm_Time_configuration(char *payload);

#endif