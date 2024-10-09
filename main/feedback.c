/*
  __  __  ____ _______ ____  _____  _      _____ _   _ ______
 |  \/  |/ __ \__   __/ __ \|  __ \| |    |_   _| \ | |  ____|
 | \  / | |  | | | | | |  | | |__) | |      | | |  \| | |__
 | |\/| | |  | | | | | |  | |  _  /| |      | | | . ` |  __|
 | |  | | |__| | | | | |__| | | \ \| |____ _| |_| |\  | |____
 |_|  |_|\____/  |_|  \____/|_|  \_\______|_____|_| \_|______|

*/

#include "feedback.h"
#include "stdlib.h"
#include "string.h"
#include "cmd_list.h"
#include "erro_list.h"
#include "core.h"
#include "math.h"
#include "timer.h"
#include "esp_timer.h"

char readData[185];

uint8_t other_Users_Feedback;
uint8_t change_Input_State_Feedback;

SemaphoreHandle_t rdySem_Control_Send_Feedback;
QueueHandle_t Feedback_Send_Data_queue;
uint8_t label_Alarm_I2_Task_Activated;
Feedback_SMS_Data feedback_SMS_Data;
feedback_numbers fd_numbers;
//data_EG91_Send_SMS feedback_Data_Send_SMS;
feedback_configurations fd_configurations;

TaskHandle_t handle_Alarm_Calls_Task;



void send_Normal_Feedback(uint8_t inputValue, uint8_t inputNumber)
{
    char data_SMS_List[160] = {};
    //printf("\n\n send_Normal_Feedback qwqwqw\n\n");
    if (fd_configurations.normal_FB_Mode == 1 && feedback_SMS_Data.Normal_Feedback_Send_SMS_Parameters.user_Have_In_List == 1)
    {
  
        
        if (inputNumber == 1)
        {
            if (feedback_SMS_Data.Normal_Feedback_Send_SMS_Parameters.relay1_Activation == 1)
            {
                //printf("\n\n send_Normal_Feedback qwqwqw 111\n\n");
                if (!inputValue)
                {
                    if (fd_configurations.input_Config & FEEDBACK_I1_NC)
                    {
                        //printf("\n\n send_Normal_Feedback 222 %d - %d\n\n", inputValue, inputNumber);
                        //printf("\n\n now 4< \n\n");
                        feedback_SMS_Data.Normal_Feedback_Send_SMS_Parameters.relay1_Activation = 0;
                        example_tg_timer_deinit(TIMER_GROUP_1, TIMER_1);

                        char str[50] = {};
                        sprintf(str,": %s", feedback_SMS_Data.Normal_Feedback_Send_SMS_Parameters.alarm_SMS_Number);
                        send_UDP_Send(str,"");    
                        /* sprintf(feedback_SMS_Data.feedback_Data_Send_SMS.payload, return_Json_SMS_Data("INPUT_HAS_BEEN_ACTIVATED_BETWEEN_THE_TIMEOUT_SELECTED"), inputNumber); */
                        /* sprintf(feedback_SMS_Data.feedback_Data_Send_SMS.phoneNumber, "%s", feedback_SMS_Data.Normal_Feedback_Send_SMS_Parameters.alarm_SMS_Number);
                        xQueueSendToBack(queue_EG91_SendSMS, (void *)&feedback_SMS_Data.feedback_Data_Send_SMS, pdMS_TO_TICKS(1000)); */
                        memset(&feedback_SMS_Data, 0, sizeof(feedback_SMS_Data));
                    }
                }
                else if (inputValue == 1)
                {

                    if (fd_configurations.input_Config & FEEDBACK_I1_NO)
                    {
                        //printf("\n\n send_Normal_Feedback 555 %d - %d\n\n", inputValue, inputNumber);
                        //printf("\n\n now 5< \n\n");
                        feedback_SMS_Data.Normal_Feedback_Send_SMS_Parameters.relay1_Activation = 0;
                        example_tg_timer_deinit(TIMER_GROUP_1, TIMER_1);

                        char str[50] = {};
                        sprintf(str,": %s", feedback_SMS_Data.Normal_Feedback_Send_SMS_Parameters.alarm_SMS_Number);
                        send_UDP_Send(str,""); 

                        /* sprintf(feedback_SMS_Data.feedback_Data_Send_SMS.payload, return_Json_SMS_Data("INPUT_HAS_BEEN_ACTIVATED_BETWEEN_THE_TIMEOUT_SELECTED"), inputNumber); */
                        /* sprintf(feedback_SMS_Data.feedback_Data_Send_SMS.phoneNumber, "%s", feedback_SMS_Data.Normal_Feedback_Send_SMS_Parameters.alarm_SMS_Number);
                        xQueueSendToBack(queue_EG91_SendSMS, (void *)&feedback_SMS_Data.feedback_Data_Send_SMS, pdMS_TO_TICKS(1000)); */
                        memset(&feedback_SMS_Data, 0, sizeof(feedback_SMS_Data));
                    }
                }
            }
        }
        else if (inputNumber == 2)
        {
            if (feedback_SMS_Data.Normal_Feedback_Send_SMS_Parameters.relay2_Activation == 1)
            {

                if (!inputValue)
                {
                    if (fd_configurations.input_Config & FEEDBACK_I2_NC)
                    {

                        //printf("\n\n now 6< \n\n");
                        feedback_SMS_Data.Normal_Feedback_Send_SMS_Parameters.relay2_Activation = 0;
                        example_tg_timer_deinit(TIMER_GROUP_0, TIMER_1);

                        char str[50] = {};
                        sprintf(str,"_ %s", feedback_SMS_Data.Normal_Feedback_Send_SMS_Parameters.alarm_SMS_Number);
                        send_UDP_Send(str,""); 

                        /* sprintf(feedback_SMS_Data.feedback_Data_Send_SMS.payload, return_Json_SMS_Data("INPUT_HAS_BEEN_ACTIVATED_BETWEEN_THE_TIMEOUT_SELECTED"), inputNumber); */
                        /* sprintf(feedback_SMS_Data.feedback_Data_Send_SMS.phoneNumber, "%s", feedback_SMS_Data.Normal_Feedback_Send_SMS_Parameters.alarm_SMS_Number);
                        xQueueSendToBack(queue_EG91_SendSMS, (void *)&feedback_SMS_Data.feedback_Data_Send_SMS, pdMS_TO_TICKS(1000)); */
                        memset(&feedback_SMS_Data, 0, sizeof(feedback_SMS_Data));
                    }
                }
                else if (inputValue == 1)
                {

                    if (fd_configurations.input_Config & FEEDBACK_I2_NO)
                    {
                        //printf("\n\n now 7< \n\n");
                        feedback_SMS_Data.Normal_Feedback_Send_SMS_Parameters.relay2_Activation = 0;
                        example_tg_timer_deinit(TIMER_GROUP_0, TIMER_1);

                        char str[50] = {};
                        sprintf(str,"_ %s", feedback_SMS_Data.Normal_Feedback_Send_SMS_Parameters.alarm_SMS_Number);
                        send_UDP_Send(str,""); 

                        /*  sprintf(feedback_SMS_Data.feedback_Data_Send_SMS.payload, return_Json_SMS_Data("INPUT_HAS_BEEN_ACTIVATED_BETWEEN_THE_TIMEOUT_SELECTED"), inputNumber); */
                        /* sprintf(feedback_SMS_Data.feedback_Data_Send_SMS.phoneNumber, "%s", feedback_SMS_Data.Normal_Feedback_Send_SMS_Parameters.alarm_SMS_Number);
                        xQueueSendToBack(queue_EG91_SendSMS, (void *)&feedback_SMS_Data.feedback_Data_Send_SMS, pdMS_TO_TICKS(1000)); */
                        memset(&feedback_SMS_Data, 0, sizeof(feedback_SMS_Data));
                    }
                    
                }
            }
        }
    }
    else if (fd_configurations.normal_FB_Mode == 0)
    {

        
        //printf("\n\n send_Normal_Feedback 11 %d - %d\n\n", inputValue, inputNumber);
        if (inputNumber == 1)
        {
            sprintf(data_SMS_List, return_Json_SMS_Data("INPUT_HAS_BEEN_ACTIVATED1"));
            if (!inputValue)
            {
                if (fd_configurations.input_Config & FEEDBACK_I1_NC)
                {
                    //printf("\n\n send_Normal_Feedback 222 %d - %d\n\n", inputValue, inputNumber);
                    send_SMS_TO_Feedback_List("- 1");
                }
            }
            else if (inputValue == 1)
            {

                if (fd_configurations.input_Config & FEEDBACK_I1_NO)
                {
                    //printf("\n\n send_Normal_Feedback 555 %d - %d\n\n", inputValue, inputNumber);
                    send_SMS_TO_Feedback_List("- 1");
                }
            }
        }
        else if (inputNumber == 2)
        {
            sprintf(data_SMS_List, return_Json_SMS_Data("INPUT_HAS_BEEN_ACTIVATED2"));
            if (!inputValue)
            {
                if (fd_configurations.input_Config & FEEDBACK_I2_NC)
                {
                    send_SMS_TO_Feedback_List("- 2");
                }
            }
            else if (inputValue == 1)
            {

                if (fd_configurations.input_Config & FEEDBACK_I2_NO)
                {
                    send_SMS_TO_Feedback_List("- 2");
                }
            }
        }
    }
}

void send_SMS_TO_Feedback_List(char *msg)
{

    char feedback_Phone_Number_TO_Call[30] = {};
    sprintf(feedback_SMS_Data.feedback_Data_Send_SMS.payload, "%s", msg);
    //EG91_send_AT_Command("ATS7=1", "OK", 1500);
    // EG91_send_AT_Command("AT+QINDCFG=\"ring\",0,0", "OK", 1500);

    /* timer_pause(TIMER_GROUP_1, TIMER_0);
    vTaskSuspend(xHandle_Timer_VerSystem); */

    for (int i = 0; i < 6; i++)
    {
        memset(feedback_SMS_Data.feedback_Data_Send_SMS.phoneNumber, 0, sizeof(feedback_SMS_Data.feedback_Data_Send_SMS.phoneNumber));

        switch (i)
        {
        case 0:
            sprintf(feedback_SMS_Data.feedback_Data_Send_SMS.phoneNumber, "%s", fd_configurations.phone1 );
            break;
        case 1:
            sprintf(feedback_SMS_Data.feedback_Data_Send_SMS.phoneNumber, "%s", fd_configurations.phone2);
            break;
        case 2:
            sprintf(feedback_SMS_Data.feedback_Data_Send_SMS.phoneNumber, "%s", fd_configurations.phone3);
            break;
        case 3:
            sprintf(feedback_SMS_Data.feedback_Data_Send_SMS.phoneNumber, "%s",fd_configurations.phone4);
            break;
        case 4:
            sprintf(feedback_SMS_Data.feedback_Data_Send_SMS.phoneNumber, "%s",fd_configurations.phone5);
            break;
        case 5:
            sprintf(feedback_SMS_Data.feedback_Data_Send_SMS.phoneNumber, "%s",fd_configurations.phone6);
            break;

        default:
            break;
        }

        //printf("\n\n feedback_SMS_Data.feedback_Data_Send_SMS.phoneNumber %s\n\n", feedback_SMS_Data.feedback_Data_Send_SMS.phoneNumber);

        if (strlen(feedback_SMS_Data.feedback_Data_Send_SMS.phoneNumber) > 4)
        {
            //printf("\n\n feedback_SMS_Data 12345\n\n");
           send_UDP_Send(msg,"");
           break;
        }
    }
    //vTaskDelay(pdMS_TO_TICKS(1500));
    //EG91_send_AT_Command("ATS7=5", "OK", 1500);
   /*  vTaskResume(xHandle_Timer_VerSystem);
    timer_start(TIMER_GROUP_1, TIMER_0); */
}

void task_Alarm_CALL(void *pvParameter)
{

    
}

uint8_t alarm_Ativation()
{
    if (save_INT8_Data_In_Storage(NVS_AL_CONF_AL, 1, nvs_Feedback_handle) == ESP_OK)
    {
        fd_configurations.alarmMode.A = 1;
        return 1;
    }
    else
    {
        return 0;
    }
    return 0;
}

void alarm_I1_Check_And_Save_Data(char *phone_Number)
{
    if (fd_configurations.alarmMode.A == 1)
    {
        uint8_t feedbackNumber = check_IF_Feedback_Number(phone_Number);
        //printf("\n\n feedback number position %d - %s\n\n", feedbackNumber, phone_Number);
        if (feedbackNumber > 0 && feedbackNumber < 7)
        {

            memset(&feedback_SMS_Data.Alarm_I1_Send_SMS_Parameters, 0, sizeof(feedback_SMS_Data.Alarm_I1_Send_SMS_Parameters));

            feedback_SMS_Data.Alarm_I1_Send_SMS_Parameters.alarm_I1_nowTime = esp_timer_get_time();

            sprintf(feedback_SMS_Data.Alarm_I1_Send_SMS_Parameters.alarm_SMS_Number, "%s",
                    phone_Number);

            feedback_SMS_Data.Alarm_I1_Send_SMS_Parameters.relay1_Activation = 1;
            example_tg_timer_init(TIMER_GROUP_1, TIMER_1, false, fd_configurations.time_I1);

            feedback_SMS_Data.Alarm_I1_Send_SMS_Parameters.user_Have_In_List = 1;
        }
        else
        {
            memset(&feedback_SMS_Data.Alarm_I1_Send_SMS_Parameters, 0, sizeof(feedback_SMS_Data.Alarm_I1_Send_SMS_Parameters));

            feedback_SMS_Data.Alarm_I1_Send_SMS_Parameters.alarm_I1_nowTime = esp_timer_get_time();

            sprintf(feedback_SMS_Data.Alarm_I1_Send_SMS_Parameters.alarm_SMS_Number, "%s",
                    fd_configurations.phone1);

            feedback_SMS_Data.Alarm_I1_Send_SMS_Parameters.relay1_Activation = 1;
            feedback_SMS_Data.Alarm_I1_Send_SMS_Parameters.user_Have_In_List = 0;
            example_tg_timer_init(TIMER_GROUP_1, TIMER_1, false, fd_configurations.time_I1);
        }
    }
}

void normal_Feedback_Check_And_Save_Data(char *phone_Number, uint8_t releNumber)
{
    //printf("\n\nnormal_Feedback_Check_And_Save_Data 000\n\n");
    if (fd_configurations.normal_FB_Mode == 1)
    {
        uint8_t feedbackNumber = check_IF_Feedback_Number(phone_Number);
    //printf("\n\nnormal_Feedback_Check_And_Save_Data 111\n\n");
        if (feedbackNumber > 0 && feedbackNumber < 7)
        {
            //printf("\n\nnormal_Feedback_Check_And_Save_Data 222\n\n");
            memset(&feedback_SMS_Data.Normal_Feedback_Send_SMS_Parameters, 0, sizeof(feedback_SMS_Data.Normal_Feedback_Send_SMS_Parameters));

            sprintf(feedback_SMS_Data.Normal_Feedback_Send_SMS_Parameters.alarm_SMS_Number, "%s",phone_Number);

            if (releNumber == 1)
            {
                //printf("\n\nnormal_Feedback_Check_And_Save_Data 333\n\n");
                feedback_SMS_Data.Normal_Feedback_Send_SMS_Parameters.relay1_Activation = 1;

                example_tg_timer_init(TIMER_GROUP_1, TIMER_1, false, fd_configurations.time_I1);
            }
            else if (releNumber == 2)
            {
                //printf("\n\nnormal_Feedback_Check_And_Save_Data 444\n\n");
                feedback_SMS_Data.Normal_Feedback_Send_SMS_Parameters.relay2_Activation = 1;

                example_tg_timer_init(TIMER_GROUP_0, TIMER_1, false, fd_configurations.time_I2);
            }

            feedback_SMS_Data.Normal_Feedback_Send_SMS_Parameters.user_Have_In_List = 1;
        }
    }
}

uint8_t check_IF_Feedback_Number(char *phone_Number)
{

    char *aux_Phone_Number;
    char aux_Feedback_Number[20];

    asprintf(&aux_Phone_Number, "%s", check_IF_haveCountryCode(phone_Number,0));

    for (int i = 0; i < 6; i++)
    {
        switch (i)
        {
        case 0:

            memset(aux_Feedback_Number, 0, sizeof(aux_Feedback_Number));
            sprintf(&aux_Feedback_Number, "%s", check_IF_haveCountryCode(fd_configurations.phone1,0));

            if (!strcmp(aux_Phone_Number, aux_Feedback_Number))
            {
                free(aux_Phone_Number);
                return 1;
            }

            break;

        case 1:

            memset(aux_Feedback_Number, 0, sizeof(aux_Feedback_Number));
            sprintf(&aux_Feedback_Number, "%s", check_IF_haveCountryCode(fd_configurations.phone2,0));

            if (!strcmp(aux_Phone_Number, aux_Feedback_Number))
            {
                free(aux_Phone_Number);
                return 2;
            }
            break;

        case 2:

            memset(aux_Feedback_Number, 0, sizeof(aux_Feedback_Number));
            sprintf(&aux_Feedback_Number, "%s", check_IF_haveCountryCode(fd_configurations.phone3,0));

            if (!strcmp(aux_Phone_Number, aux_Feedback_Number))
            {
                free(aux_Phone_Number);
                return 3;
            }

            break;

        case 3:

            memset(aux_Feedback_Number, 0, sizeof(aux_Feedback_Number));
            sprintf(&aux_Feedback_Number, "%s", check_IF_haveCountryCode(fd_configurations.phone4,0));

            if (!strcmp(aux_Phone_Number, aux_Feedback_Number))
            {
                free(aux_Phone_Number);
                return 4;
            }

            break;

        case 4:

            memset(aux_Feedback_Number, 0, sizeof(aux_Feedback_Number));
            sprintf(&aux_Feedback_Number, "%s", check_IF_haveCountryCode(fd_configurations.phone5,0));

            if (!strcmp(aux_Phone_Number, aux_Feedback_Number))
            {
                free(aux_Phone_Number);
                return 5;
            }
            break;

        case 5:

            memset(aux_Feedback_Number, 0, sizeof(aux_Feedback_Number));
            sprintf(&aux_Feedback_Number, "%s", check_IF_haveCountryCode(fd_configurations.phone6,0));

            if (!strcmp(aux_Phone_Number, aux_Feedback_Number))
            {
                free(aux_Phone_Number);
                return 6;
            }
            break;

        default:
            free(aux_Phone_Number);
            return 0;
            break;
        }
    }

    free(aux_Phone_Number);
    return 0;
}

char *parse_Alarm_Data(uint8_t BLE_SMS_Indication, uint8_t feedbackNumber, char cmd, char param, char *payload)
{

    char *rsp = 0 /*  = (char *)malloc(200 * sizeof(char)) */;
    /* memset(&rsp, 0, sizeof(rsp)); */

    if (cmd == SET_CMD)
    {
        if (param == ALARM_PARAMETER)
        {
            // write_Feedback_number(feedbackNumber, payload);

            if (BLE_SMS_Indication == BLE_INDICATION || BLE_SMS_Indication == UDP_INDICATION)
            {
                /*memset(&rsp, 0, sizeof(rsp));*/
                if (alarm_Ativation())
                {
                    asprintf(&rsp, "%s %c %c %d", "AL", cmd, param, 1);
                }
                else
                {
                    asprintf(&rsp, "%s %c %c %d", "AL", cmd, param, 0);
                }

                return rsp;
            }
            else
            {
                return return_ERROR_Codes(&rsp, return_Json_SMS_Data("ONLY_BLE_FUNCTION"));
            }
        }
        else if (param == INPUT_PARAMETER)
        {
            // write_Feedback_number(feedbackNumber, payload);

            if (BLE_SMS_Indication == BLE_INDICATION || BLE_SMS_Indication == UDP_INDICATION)
            {
                /*memset(&rsp, 0, sizeof(rsp));*/
                if (modify_Alarm_Input_configuration(payload))
                {
                    asprintf(&rsp, "%s %c %c %d", ALARM_ELEMENT, cmd, param, fd_configurations.input_Config);
                }
                else
                {
                    return return_ERROR_Codes(&rsp, ERROR_SET);
                }

                return rsp;
            }
            else
            {
                return return_ERROR_Codes(&rsp, return_Json_SMS_Data("ONLY_BLE_FUNCTION"));
            }
        }
        else if (param == TIME_PARAMETER)
        {
            if (BLE_SMS_Indication == BLE_INDICATION || BLE_SMS_Indication == UDP_INDICATION)
            {
                /*memset(&rsp, 0, sizeof(rsp));*/
                if (modify_Alarm_Time_configuration(payload))
                {
                    asprintf(&rsp, "%s %c %c %d;%d", ALARM_ELEMENT, cmd, param, fd_configurations.time_I1, fd_configurations.time_I2);
                }
                else
                {
                    return return_ERROR_Codes(&rsp, ERROR_SET);
                }

                return rsp;
            }
            else
            {
                return return_ERROR_Codes(&rsp, return_Json_SMS_Data("ONLY_BLE_FUNCTION"));
            }
        }
        else if (param == ALARM_OTHER_USERS_FEEDBACK_PARAMETER)
        {
            if (BLE_SMS_Indication == BLE_INDICATION || BLE_SMS_Indication == UDP_INDICATION)
            {
                /*memset(&rsp, 0, sizeof(rsp));*/
                if (set_Alarm_Other_Users_Feedback_Mode())
                {
                    asprintf(&rsp, "%s %c %c %d", ALARM_ELEMENT, cmd, param, 1);
                }
                else
                {
                    return return_ERROR_Codes(&rsp, ERROR_SET);
                }

                return rsp;
            }
            else
            {
                return return_ERROR_Codes(&rsp, return_Json_SMS_Data("ONLY_BLE_FUNCTION"));
            }
        }
        else if (param == ALARM_CHANGE_INPUT_STATE_FEEDBACK_PARAMETER)
        {
            if (BLE_SMS_Indication == BLE_INDICATION || BLE_SMS_Indication == UDP_INDICATION)
            {
                /*memset(&rsp, 0, sizeof(rsp));*/
                if (set_Alarm_Change_Input_State_Feedback())
                {
                    asprintf(&rsp, "%s %c %c %d", ALARM_ELEMENT, cmd, param, 1);
                }
                else
                {
                    return return_ERROR_Codes(&rsp, ERROR_SET);
                }

                return rsp;
            }
            else
            {
                return return_ERROR_Codes(&rsp, return_Json_SMS_Data("ONLY_BLE_FUNCTION"));
            }
        }
        else
        {

            //printf(ERROR_PARAMETER);
            return return_ERROR_Codes(&rsp, ERROR_PARAMETER);
        }
    }
    else if (cmd == GET_CMD)
    {
        if (param == ALL_PARAMETER)
        {
            if (BLE_SMS_Indication == BLE_INDICATION || BLE_SMS_Indication == UDP_INDICATION)
            {
                /*memset(&rsp, 0, sizeof(rsp));*/
                asprintf(&rsp, "%s %c %c\n%s", ALARM_ELEMENT, cmd, param, read_Feedback_number(feedbackNumber));
                return rsp;
            }
            else
            {
                return return_ERROR_Codes(&rsp, return_Json_SMS_Data("ONLY_BLE_FUNCTION"));
            }
        }
    }
    else if (cmd == RESET_CMD)
    {
        if (param == ALARM_PARAMETER)
        {

            if (BLE_SMS_Indication == BLE_INDICATION || BLE_SMS_Indication == UDP_INDICATION)
            {
                /*memset(&rsp, 0, sizeof(rsp));*/
                if (alarm_Turn_OFF())
                {
                    asprintf(&rsp, "%s %c %c %d", "AL", cmd, param, 0);
                }
                else
                {
                    asprintf(&rsp, "%s %c %c %d", "AL", cmd, param, 1);
                }

                return rsp;
            }
            else
            {
                return return_ERROR_Codes(&rsp, return_Json_SMS_Data("ONLY_BLE_FUNCTION"));
            }
        }
        else if (param == ALARM_OTHER_USERS_FEEDBACK_PARAMETER)
        {
            if (BLE_SMS_Indication == BLE_INDICATION || BLE_SMS_Indication == UDP_INDICATION)
            {
                /*memset(&rsp, 0, sizeof(rsp));*/
                if (reset_Alarm_Other_Users_Feedback_Mode())
                {
                    asprintf(&rsp, "%s %c %c %d", ALARM_ELEMENT, cmd, param, 1);
                }
                else
                {
                    return return_ERROR_Codes(&rsp, ERROR_SET);
                }

                return rsp;
            }
            else
            {
                return return_ERROR_Codes(&rsp, return_Json_SMS_Data("ONLY_BLE_FUNCTION"));
            }
        }
        else if (param == ALARM_CHANGE_INPUT_STATE_FEEDBACK_PARAMETER)
        {
            if (BLE_SMS_Indication == BLE_INDICATION || BLE_SMS_Indication == UDP_INDICATION)
            {
                /*memset(&rsp, 0, sizeof(rsp));*/
                if (reset_Alarm_Change_Input_State_Feedback())
                {
                    asprintf(&rsp, "%s %c %c %d", ALARM_ELEMENT, cmd, param, 1);
                }
                else
                {
                    return return_ERROR_Codes(&rsp, ERROR_SET);
                }

                return rsp;
            }
            else
            {
                return return_ERROR_Codes(&rsp, return_Json_SMS_Data("ONLY_BLE_FUNCTION"));
            }
        }
        else
        {

            //printf(ERROR_PARAMETER);
            return return_ERROR_Codes(&rsp, ERROR_PARAMETER);
        }
    }
    else
    {
        //printf(ERROR_CMD);
        return return_ERROR_Codes(&rsp, return_Json_SMS_Data("ERROR_CMD"));
    }

    return return_ERROR_Codes(&rsp, "FEEDBACK ERROR");
}

uint8_t set_Alarm_Other_Users_Feedback_Mode()
{
    if (nvs_set_u8(nvs_Feedback_handle, NVS_AL_OTHER_USERS_FEEDBACK, 1) == ESP_OK)
    {
        fd_configurations.alarmMode.other_Users_Feedback = 1;
        return 1;
    }
    else
    {
        return 0;
    }
}

uint8_t set_Alarm_Change_Input_State_Feedback()
{
    if (nvs_set_u8(nvs_Feedback_handle, NVS_AL_CHANGE_INPUT_STATE_FEEDBACK, 1) == ESP_OK)
    {
        fd_configurations.alarmMode.change_Input_State_Feedback = 1;
        return 1;
    }
    else
    {
        return 0;
    }
}

uint8_t set_Normal_Feedback_Mode()
{
    if (nvs_set_u8(nvs_Feedback_handle, NVS_FB_MODE, 1) == ESP_OK)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

uint8_t reset_Alarm_Other_Users_Feedback_Mode()
{
    if (nvs_set_u8(nvs_Feedback_handle, NVS_AL_OTHER_USERS_FEEDBACK, 0) == ESP_OK)
    {
        fd_configurations.alarmMode.other_Users_Feedback = 0;
        return 1;
    }
    else
    {
        return 0;
    }
}

uint8_t reset_Alarm_Change_Input_State_Feedback()
{
    if (nvs_set_u8(nvs_Feedback_handle, NVS_AL_CHANGE_INPUT_STATE_FEEDBACK, 0) == ESP_OK)
    {
        fd_configurations.alarmMode.change_Input_State_Feedback = 0;
        return 1;
    }
    else
    {
        return 0;
    }
}

uint8_t reset_Normal_Feedback_Mode()
{
    if (nvs_set_u8(nvs_Feedback_handle, NVS_FB_MODE, 0) == ESP_OK)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

uint8_t alarm_Turn_OFF()
{
    if (save_INT8_Data_In_Storage(NVS_AL_CONF_AL, 0, nvs_Feedback_handle) == ESP_OK)
    {
        fd_configurations.alarmMode.A = 0;
        return 1;
    }
    else
    {
        return 0;
    }
    return 0;
}

char *parse_FeedbackData(uint8_t BLE_SMS_Indication, uint8_t feedbackNumber, char cmd, char param, char *payload)
{

    char *rsp;
    /*memset(&rsp, 0, sizeof(rsp));*/

    if (cmd == SET_CMD)
    {
        if (param == PHONE_PARAMETER)
        {
            // write_Feedback_number(feedbackNumber, payload);

            if (BLE_SMS_Indication == BLE_INDICATION || BLE_SMS_Indication == UDP_INDICATION)
            {
                /*memset(&rsp, 0, sizeof(rsp));*/
                if (add_Feedback_number(feedbackNumber, payload) == 1)
                {
                    asprintf(&rsp, "F%d %c %c %s", feedbackNumber, cmd, param, payload);
                }
                else
                {
                    asprintf(&rsp, "F%d %c %c %s", feedbackNumber, cmd, param, ERROR_SET);
                }

                return rsp;
            }
            else
            {
                return return_ERROR_Codes(&rsp, return_Json_SMS_Data("ONLY_BLE_FUNCTION"));
            }
        }
        else if (param == NORMAL_FEEDBACK_MODE_PARAMETER)
        {
            if (BLE_SMS_Indication == BLE_INDICATION || BLE_SMS_Indication == UDP_INDICATION)
            {
                /*memset(&rsp, 0, sizeof(rsp));*/
                if (set_Normal_Feedback_Mode() == 1)
                {
                    fd_configurations.normal_FB_Mode = 1;
                    asprintf(&rsp, "FX %c %c %d", cmd, param, fd_configurations.normal_FB_Mode);
                }
                else
                {
                    asprintf(&rsp, "FX %c %c %s", cmd, param, ERROR_SET);
                }

                return rsp;
            }
            else
            {
                return return_ERROR_Codes(&rsp, return_Json_SMS_Data("ONLY_BLE_FUNCTION"));
            }
        }
        else
        {
            //printf(ERROR_PARAMETER);
            return return_ERROR_Codes(&rsp, return_Json_SMS_Data("ERROR_PARAMETER"));
        }
    }
    else if (cmd == GET_CMD)
    {

        if (param == PHONE_PARAMETER)
        {
            if (BLE_SMS_Indication == BLE_INDICATION || BLE_SMS_Indication == UDP_INDICATION)
            {
                /*memset(&rsp, 0, sizeof(rsp));*/
                asprintf(&rsp, "F%d %c %c %s", feedbackNumber, cmd, param, read_Feedback_number(feedbackNumber));
                return rsp;
            }
            else
            {
                return return_ERROR_Codes(&rsp, return_Json_SMS_Data("ERROR_INPUT_DATA"));
            }
        }
        if (param == INPUT_PARAMETER)
        {
            /*memset(&rsp, 0, sizeof(rsp));*/
            asprintf(&rsp, "%s %c %c\n%s", ALARM_ELEMENT, cmd, param, read_Feedback_number(feedbackNumber));
            return rsp;
        }
        if (param == ALL_PARAMETER)
        {
            /*memset(&rsp, 0, sizeof(rsp));*/
            asprintf(&rsp, "%s %c %c\n%s", ALARM_ELEMENT, cmd, param, read_Feedback_number(feedbackNumber));
            return rsp;
        }
        else
        {

            //printf(ERROR_PARAMETER);
            return return_ERROR_Codes(&rsp, return_Json_SMS_Data("ERROR_PARAMETER"));
        }
    }
    else if (cmd == RESET_CMD)
    {
        if (param == PHONE_PARAMETER)
        {

            if (BLE_SMS_Indication == BLE_INDICATION || BLE_SMS_Indication == UDP_INDICATION)
            {
                if (delete_Feedback_number(feedbackNumber))
                {
                    /*memset(&rsp, 0, sizeof(rsp));*/
                    asprintf(&rsp, "F%d %c %c %c", feedbackNumber, cmd, param, '0');
                    return rsp;
                }
                else
                {
                    return return_ERROR_Codes(&rsp, return_Json_SMS_Data("ERROR_RESET"));
                }
            }
            else
            {

                return return_ERROR_Codes(&rsp, return_Json_SMS_Data("ONLY_BLE_FUNCTION"));
            }
        }
        else if (param == NORMAL_FEEDBACK_MODE_PARAMETER)
        {
            if (BLE_SMS_Indication == BLE_INDICATION || BLE_SMS_Indication == UDP_INDICATION)
            {
                /*memset(&rsp, 0, sizeof(rsp));*/
                if (reset_Normal_Feedback_Mode() == 1)
                {
                    fd_configurations.normal_FB_Mode = 0;
                    asprintf(&rsp, "FX %c %c %d", cmd, param, fd_configurations.normal_FB_Mode);
                }
                else
                {
                    asprintf(&rsp, "FX %c %c %s", cmd, param, ERROR_SET);
                }

                return rsp;
            }
            else
            {
                return return_ERROR_Codes(&rsp, return_Json_SMS_Data("ONLY_BLE_FUNCTION"));
            }
        }
        else
        {

            //printf(ERROR_PARAMETER);
            return return_ERROR_Codes(&rsp, return_Json_SMS_Data("ERROR_PARAMETER"));
        }
    }
    else
    {

        //printf(ERROR_CMD);
        return return_ERROR_Codes(&rsp, return_Json_SMS_Data("ERROR_CMD"));
    }
    return return_ERROR_Codes(&rsp, "FEEDBACK ERROR");
}

uint8_t modify_Alarm_Time_configuration(char *payload)
{
    char timeI1[5] = {};
    char timeI2[5] = {};
    uint8_t strIndex = 0;
    uint8_t auxCounter = 0;
    for (int i = 0; i < strlen(payload); i++)
    {
        if (payload[i] == ';')
        {
            auxCounter++;
            strIndex = 0;
        }
        else
        {
            if (auxCounter == 0)
            {
                timeI1[strIndex++] = payload[i];
            }
            else if (auxCounter == 1)
            {
                timeI2[strIndex++] = payload[i];
            }
        }
    }

    uint8_t auxTime1 = atoi(timeI1);
    uint8_t auxTime2 = atoi(timeI2);

    //printf("\n\ntimeI1 %d\n\n", auxTime1);
    //printf("\n\ntimeI2 %d\n\n", auxTime2);

    if (atoi(timeI1) <= 255 && atoi(timeI2) <= 255)
    {
        //printf("\n\ntimeI1 %d\n\n", auxTime1);
        //printf("\n\ntimeI2 %d\n\n", auxTime2);
        if (save_INT8_Data_In_Storage(NVS_FB_TIM1, auxTime1, nvs_Feedback_handle) == ESP_OK)
        {
            fd_configurations.time_I1 = atoi(timeI1);
            //printf("\n\nfd_configurations.time_I1 %d\n\n", fd_configurations.time_I1);

            if (save_INT8_Data_In_Storage(NVS_FB_TIM2, auxTime2, nvs_Feedback_handle) == ESP_OK)
            {
                fd_configurations.time_I2 = atoi(timeI2);
                //printf("\n\nfd_configurations.time_I2 %d\n\n", fd_configurations.time_I2);
                return 1;
            }
            else
            {
                return 0;
            }

            return 1;
        }
        else
        {
            return 0;
        }
    }
    else
    {
        return 0;
    }
}

uint8_t modify_Alarm_Input_configuration(char *payload)
{

    char input_Config[5];
    uint8_t input_int_Config = 0;

    //printf("\nmodify_Alarm_Input_configuration payload - %s\n", payload);

    memset(input_Config, 0, sizeof(input_Config));

    if ((payload[0] == '1' && payload[1] == '1') || (payload[2] == '1' && payload[3] == '1') || strlen(payload) > 4)
    {
        return ERROR_INPUT_DATA;
    }

    for (int i = 0; i < strlen(payload); i++)
    {
        if (payload[i] == '1')
        {
            input_int_Config += pow(2, i);
        }
    }

    //printf("\nmodify_Alarm_Input_configuration input_int_Config - %d\n", input_int_Config);
    if (save_INT8_Data_In_Storage(NVS_AL_CONF_IN, input_int_Config, nvs_Feedback_handle) == ESP_OK)
    {
        fd_configurations.input_Config = 0;
        fd_configurations.input_Config = input_int_Config;
        //printf("\nmodify_Alarm_Input_configuration alarmMode.input_Config - %d -%d\n", fd_configurations.input_Config,input_int_Config);
        return 1;
    }
    else
    {
        return 0;
    }

    return 0;
}

char *add_Feedback_number(uint8_t line, char *payload)
{

    FILE *f;
    char phone[20];
    char input_Config[5];
    uint8_t dot_Counter = 0;
    uint8_t input_int_Config = 0;
    int strIndex = 0;
    char str[200];
    char aux_str[200];

    memset(phone, 0, sizeof(phone));
    memset(input_Config, 0, sizeof(input_Config));

    //printf("\nfb payload %s\n", payload);

    for (int i = 0; i < strlen(payload); i++)
    {
        if (payload[i] == '.')
        {
            dot_Counter++;
            strIndex = 0;
        }
        else
        {

            if (dot_Counter == 0)
            {
                phone[strIndex] = payload[i];

                if (strIndex == 30)
                {
                    return ERROR_INPUT_DATA;
                }

                strIndex++;
            }
            else if (dot_Counter == 1)
            {
                input_Config[strIndex] = payload[i];

                if (strIndex == 5)
                {
                    return ERROR_INPUT_DATA;
                }

                strIndex++;
            }
            else if (dot_Counter > 1)
            {
                return ERROR_INPUT_DATA;
            }
        }
    }
    //printf("\nfb dot_Counter %d\n", dot_Counter);
    if (dot_Counter == 1)
    {
        if ((input_Config[0] == '1' && input_Config[1] == '1') || (input_Config[2] == '1' && input_Config[3] == '1') || strlen(input_Config) > 4)
        {
            return ERROR_INPUT_DATA;
        }

        for (int i = 0; i < strlen(input_Config); i++)
        {
            if (input_Config[i] == '1')
            {
                input_int_Config += pow(2, i);
            }
        }
    }

    //printf("\nfb phone %s\n", phone);
    //printf("\nfb str in cfg %s\n", input_Config);
    //printf("\nfb int in cfg %d\n\n", input_int_Config);
    //printf("\n alarmMode.A = %d \n", fd_configurations.alarmMode.A);

    if (strlen(phone) < 20)
    {
        if (line == 1)
        {

            if (save_STR_Data_In_Storage(NVS_FB_CONF_P1, phone /* fd_configurations.phone1 */, nvs_Feedback_handle) == ESP_OK)
            {
                //printf("\n ENTER FB 11 \n");
                memset(fd_configurations.phone1, 0, sizeof(fd_configurations.phone1));
                sprintf(fd_configurations.phone1, "%s", phone);
                return 1;
            }
            else
            {
                return 0;
            }

            return 0;
        }

        if (line == 2)
        {

            if (save_STR_Data_In_Storage(NVS_FB_CONF_P2, phone, nvs_Feedback_handle) == ESP_OK)
            {
                //printf("\nfd_configurations.phone2 - %s\n", phone);
                memset(fd_configurations.phone2, 0, sizeof(fd_configurations.phone2));
                sprintf(fd_configurations.phone2, "%s", phone);

                return 1;
            }
            else
            {
                return 0;
            }

            return 0;
        }

        if (line == 3)
        {

            if (save_STR_Data_In_Storage(NVS_FB_CONF_P3, phone, nvs_Feedback_handle) == ESP_OK)
            {

                memset(fd_configurations.phone3, 0, sizeof(fd_configurations.phone3));
                sprintf(fd_configurations.phone3, "%s", phone);

                return 1;
            }
            else
            {
                return 0;
            }
            return 0;
        }

        if (line == 4)
        {

            if (save_STR_Data_In_Storage(NVS_FB_CONF_P4, phone, nvs_Feedback_handle) == ESP_OK)
            {
                memset(fd_configurations.phone4, 0, sizeof(fd_configurations.phone4));
                sprintf(fd_configurations.phone4, "%s", phone);
                return 1;
            }
            else
            {
                return 0;
            }
            return 0;
        }
        if (line == 5)
        {

            if (save_STR_Data_In_Storage(NVS_FB_CONF_P5, phone, nvs_Feedback_handle) == ESP_OK)
            {
                memset(fd_configurations.phone5, 0, sizeof(fd_configurations.phone5));
                sprintf(fd_configurations.phone5, "%s", phone);
                return 1;
            }
            else
            {
                return 0;
            }
            return 0;
        }
        if (line == 6)
        {

            if (save_STR_Data_In_Storage(NVS_FB_CONF_P6, phone, nvs_Feedback_handle) == ESP_OK)
            {
                memset(fd_configurations.phone6, 0, sizeof(fd_configurations.phone6));
                sprintf(fd_configurations.phone6, "%s", phone);
                return 1;
            }
            else
            {
                return 0;
            }
            return 0;
        }
    }
    else
    {
        return 0;
    }

    return 0;
}

char *read_Feedback_number(uint8_t line)
{
    int fd_numb = 1;
    int lenght = 0;

    memset(readData, 0, sizeof(readData));

    if (fd_configurations.alarmMode.A != 1)
    {

        /* strcpy(readData,"F1 ");
        strcat(readData,fd_configurations.phone1); */
        lenght += sprintf(readData, "F1 %s\n", fd_configurations.phone1);
        //printf("\nread data feedback A = %s\n", readData);

        lenght += sprintf(readData + lenght, "F2 %s\n", fd_configurations.phone2);

        lenght += sprintf(readData + lenght, "F3 %s\n", fd_configurations.phone3);

        lenght += sprintf(readData + lenght, "F4 %s\n", fd_configurations.phone4);

        lenght += sprintf(readData + lenght, "F5 %s\n", fd_configurations.phone5);

        lenght += sprintf(readData + lenght, "F6 %s\n", fd_configurations.phone6);

        lenght += sprintf(readData + lenght, "T %d;%d\n", fd_configurations.time_I1, fd_configurations.time_I2);

        lenght += sprintf(readData + lenght, "I %d\n", fd_configurations.input_Config);

        if (fd_configurations.input_Config == 255)
        {
            /* code */
        }

        lenght += sprintf(readData + lenght, "M %d", fd_configurations.normal_FB_Mode);

        //printf("\nread data feedback1 = %s\n", readData);
    }
    else if (fd_configurations.alarmMode.A == 1)
    {

        lenght += sprintf(readData, "F1 %s\n", fd_configurations.phone1);

        lenght += sprintf(readData + lenght, "F2 %s\n", fd_configurations.phone2);

        lenght += sprintf(readData + lenght, "F3 %s\n", fd_configurations.phone3);

        lenght += sprintf(readData + lenght, "F4 %s\n", fd_configurations.phone4);

        lenght += sprintf(readData + lenght, "F5 %s\n", fd_configurations.phone5);

        lenght += sprintf(readData + lenght, "F6 %s\n", fd_configurations.phone6);

        lenght += sprintf(readData + lenght, "T %d;%d\n", fd_configurations.time_I1, fd_configurations.time_I2);

        lenght += sprintf(readData + lenght, "I %d\n", fd_configurations.input_Config);

        lenght += sprintf(readData + lenght, "C %d\n", fd_configurations.alarmMode.change_Input_State_Feedback);

        lenght += sprintf(readData + lenght, "O %d\n", fd_configurations.alarmMode.other_Users_Feedback);
    }
    else
    {
        return ERROR_GET;
    }

    //lenght += sprintf(readData + lenght, "AL %d\n",  fd_configurations.alarmMode.A);

    return readData;
}

uint8_t delete_Feedback_number(uint8_t line)
{
    FILE *f;
    char str[200];

    if (line == 1)
    {

        nvs_erase_key(nvs_Feedback_handle, NVS_FB_CONF_P1);

        memset(fd_configurations.phone1, 0, sizeof(fd_configurations.phone1));
        return 1;
    }

    if (line == 2)
    {

        nvs_erase_key(nvs_Feedback_handle, NVS_FB_CONF_P2);

        memset(fd_configurations.phone2, 0, sizeof(fd_configurations.phone2));

        return 1;
    }

    if (line == 3)
    {

        nvs_erase_key(nvs_Feedback_handle, NVS_FB_CONF_P3);

        memset(fd_configurations.phone3, 0, sizeof(fd_configurations.phone3));

        return 1;
    }

    if (line == 4)
    {
        nvs_erase_key(nvs_Feedback_handle, NVS_FB_CONF_P4);

        memset(fd_configurations.phone4, 0, sizeof(fd_configurations.phone4));

        return 1;
    }

    if (line == 5)
    {

        nvs_erase_key(nvs_Feedback_handle, NVS_FB_CONF_P5);

        memset(fd_configurations.phone5, 0, sizeof(fd_configurations.phone5));

        return 1;
    }

    if (line == 6)
    {

        nvs_erase_key(nvs_Feedback_handle, NVS_FB_CONF_P6);

        memset(fd_configurations.phone6, 0, sizeof(fd_configurations.phone6));

        return 1;
    }

    return 0;
}

void clear_All_Feedback_number()
{

    FILE *f;
    char *str;
    remove("/spiffs/feedback.txt");

    f = fopen("/spiffs/feedback.txt", "w");

    memset(fd_numbers.F1, 0, strlen(fd_numbers.F1));
    memset(fd_numbers.F2, 0, strlen(fd_numbers.F2));
    memset(fd_numbers.F3, 0, strlen(fd_numbers.F3));
    memset(fd_numbers.F4, 0, strlen(fd_numbers.F4));

    asprintf(&str, "%s;%s;%s;%s;", fd_numbers.F1, fd_numbers.F2, fd_numbers.F3, fd_numbers.F4);
    fprintf(f, "%s", str);
    free(str);
    fclose(f);
}

void write_FbNumbers_Struct()
{
    char fb_phoneff[30];
    memset(&fd_configurations, 0, sizeof(fd_configurations));
    memset(fb_phoneff, 0, sizeof(fb_phoneff));

    /* ***************************** F1 ******************************/

    get_Data_STR_Feedback_From_Storage(NVS_FB_CONF_P1, &fd_configurations.phone1);

    //printf("\n\n fd_configurations.phone1 - %s\n\n",  fd_configurations.phone1 );

    /* ***************************** F2 ********************************/

    get_Data_STR_Feedback_From_Storage(NVS_FB_CONF_P2, &fd_configurations.phone2);

    /* ***************************** F3 ******************************/

    get_Data_STR_Feedback_From_Storage(NVS_FB_CONF_P3, &fd_configurations.phone3);

    /* ***************************** F4 ******************************/

    get_Data_STR_Feedback_From_Storage(NVS_FB_CONF_P4, &fd_configurations.phone4);

    /* ***************************** F5 ******************************/

    get_Data_STR_Feedback_From_Storage(NVS_FB_CONF_P5, &fd_configurations.phone5);

    /* ***************************** F6 ******************************/

    get_Data_STR_Feedback_From_Storage(NVS_FB_CONF_P6, &fd_configurations.phone6);

    /* *************************** ALARM *****************************/

    fd_configurations.alarmMode.A = get_INT8_Data_From_Storage(NVS_AL_CONF_AL, nvs_Feedback_handle);

    if (fd_configurations.alarmMode.A == 255)
    {
        if (save_INT8_Data_In_Storage(NVS_AL_CONF_AL, 0, nvs_Feedback_handle) == ESP_OK)
        {
            fd_configurations.alarmMode.A = 0;
        }
    }

    fd_configurations.input_Config = get_INT8_Data_From_Storage(NVS_AL_CONF_IN, nvs_Feedback_handle);

    if (fd_configurations.input_Config == 255)
    {
        if (save_INT8_Data_In_Storage(NVS_AL_CONF_IN, 5, nvs_Feedback_handle) == ESP_OK)
        {
            fd_configurations.input_Config = 0;
            fd_configurations.input_Config = 5;
        }
    }

    fd_configurations.time_I1 = get_INT8_Data_From_Storage(NVS_FB_TIM1, nvs_Feedback_handle);

    if (fd_configurations.time_I1 == 255)
    {
        if (save_INT8_Data_In_Storage(NVS_FB_TIM1, 10, nvs_Feedback_handle) == ESP_OK)
        {
            fd_configurations.time_I1 = 0;
            fd_configurations.time_I1 = 10;
        }
    }

    fd_configurations.time_I2 = get_INT8_Data_From_Storage(NVS_FB_TIM2, nvs_Feedback_handle);

    if (fd_configurations.time_I2 == 255)
    {
        if (save_INT8_Data_In_Storage(NVS_FB_TIM2, 10, nvs_Feedback_handle) == ESP_OK)
        {
            fd_configurations.time_I2 = 0;
            fd_configurations.time_I2 = 10;
        }
    }

    fd_configurations.alarmMode.other_Users_Feedback = get_INT8_Data_From_Storage(NVS_AL_OTHER_USERS_FEEDBACK, nvs_Feedback_handle);

    if (fd_configurations.alarmMode.other_Users_Feedback == 255)
    {
        if (save_INT8_Data_In_Storage(NVS_AL_OTHER_USERS_FEEDBACK, 0, nvs_Feedback_handle) == ESP_OK)
        {
            fd_configurations.alarmMode.other_Users_Feedback = 0;
        }
    }

    fd_configurations.alarmMode.change_Input_State_Feedback = get_INT8_Data_From_Storage(NVS_AL_CHANGE_INPUT_STATE_FEEDBACK, nvs_Feedback_handle);

    if (fd_configurations.alarmMode.change_Input_State_Feedback  == 255)
    {
        if (save_INT8_Data_In_Storage(NVS_AL_CHANGE_INPUT_STATE_FEEDBACK, 0, nvs_Feedback_handle) == ESP_OK)
        {
            fd_configurations.alarmMode.change_Input_State_Feedback  = 0;
        }
    }

    fd_configurations.normal_FB_Mode = get_INT8_Data_From_Storage(NVS_FB_MODE, nvs_Feedback_handle);

    if (fd_configurations.normal_FB_Mode  == 255)
    {
        if (save_INT8_Data_In_Storage(NVS_FB_MODE, 0, nvs_Feedback_handle) == ESP_OK)
        {
            fd_configurations.normal_FB_Mode   = 0;
        }
    }
    
}