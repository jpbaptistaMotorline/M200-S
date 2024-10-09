#include "EG91.h"
#include "stdio.h"
#include "string.h"
// #include "uart.h"
#include "AT_CMD_List.h"
// #include "gpio.h"
#include "cmd_list.h"
#include "core.h"
#include "erro_list.h"
#include "esp_task_wdt.h"
#include "math.h"
#include "rele.h"
#include "routines.h"
#include "sdCard.h"
#include "timer.h"
#include "users.h"

#include "UDP_Codes.h"
#include "ble_spp_server_demo.h"
#include "core.h"
#include "crc32.h"
#include "feedback.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "pcf85063.h"

#include "errno.h"
#include "esp_flash_partitions.h"
#include "esp_ota_ops.h"
#include "esp_partition.h"
#include <regex.h>

// #include "semphr.h"
int aux_label_inCall = 0;

TaskHandle_t handle_SMS_TASK;
TaskHandle_t handle_INCOMING_CALL_TASK;
TaskHandle_t handle_SEND_SMS_TASK;
TaskHandle_t handle_UDP_TASK;

uint8_t mqtt_openLabel = 0;
// mqtt_connectLabel = 0;

uint8_t call_Type;

uint8_t send_ATCommand_Label;
uint8_t incomingCall_Label;
uint8_t call_label;
uint8_t label_Network_Activate;
int RSSI_VALUE;
// char SIM_Balance[500];
char EG91_ICCID_Number[25];
char EG91_IMEI[20];
uint8_t SIM_CARD_PIN_status;

void give_rdySem_Control_SMS_UDP() { xSemaphoreGive(rdySem_Control_SMS_UDP); }
void take_rdySem_Control_SMS_UDP() {
  xSemaphoreTake(rdySem_Control_SMS_UDP, pdMS_TO_TICKS(4000));
}

uint8_t EG91_PowerOn() {
  uint8_t EG91_PowerON_TimeOut = 0;

  // printf("\ngpio_get_level(GPIO_INPUT_IO_EG91_STATUS) %d\n",
  // gpio_get_level(GPIO_INPUT_IO_EG91_STATUS));

  if (!gpio_get_level(GPIO_INPUT_IO_EG91_STATUS)) {
    mqtt_connectLabel = 0;
    gpio_set_level(GPIO_OUTPUT_IO_PWRKEY, 0);
    vTaskDelay(pdMS_TO_TICKS(200));
    // printf("\n\nINIT EG915 4\n\n");

    gpio_set_level(GPIO_OUTPUT_IO_PWRKEY, 1);
    vTaskDelay(pdMS_TO_TICKS(2000));
    // printf("\n\nINIT EG915 5\n\n");
    gpio_set_level(GPIO_OUTPUT_IO_PWRKEY, 0);
    // printf("\ngpio_get_level(GPIO_INPUT_IO_EG91_STATUS) %d\n",
    // gpio_get_level(GPIO_INPUT_IO_EG91_STATUS));
    vTaskDelay(pdMS_TO_TICKS(8000));
    // printf("\n\nINIT EG915 6\n\n");
    while (!gpio_get_level(GPIO_INPUT_IO_EG91_STATUS) &&
           EG91_PowerON_TimeOut < 3) {
      // printf("\nerror power on\n");
      vTaskDelay(pdMS_TO_TICKS(1000));
      EG91_PowerON_TimeOut++;
    }

    if (gpio_get_level(GPIO_INPUT_IO_EG91_STATUS)) {
      vTaskDelay(pdMS_TO_TICKS(10000));
      return 1;
    } else {
      return 0;
    }
  } else {
    /* if (check_NetworkState() != 1)
    { */
    // printf("\n\n\n before power on 11\n\n\n");
    return EG91_Power_Reset();
    //}

    // xSemaphoreGive(rdySem_Control_Send_AT_Command);
  }

  return 0;
}

uint8_t EG91_Power_OFF() {
  uint8_t EG91_PowerON_TimeOut = 0;
  /*  gpio_set_level(GPIO_OUTPUT_IO_PWRKEY, 0);
  vTaskDelay(pdMS_TO_TICKS(5000));

  gpio_set_level(GPIO_OUTPUT_IO_PWRKEY, 1);
  vTaskDelay(pdMS_TO_TICKS(40000)); */
  // system_stack_high_water_mark("AT+QPOWD");
  if (EG91_send_AT_Command("AT+QPOWD", "OK", 1000)) {
    vTaskDelay(pdMS_TO_TICKS(1000));
    if (!gpio_get_level(GPIO_INPUT_IO_EG91_STATUS)) {
      mqtt_connectLabel = 0;
      return 1;
    } else {
      /*  gpio_set_level(GPIO_OUTPUT_IO_PWRKEY, 0);
vTaskDelay(pdMS_TO_TICKS(2000)); */
      gpio_set_level(GPIO_OUTPUT_IO_PWRKEY, 1);
      vTaskDelay(pdMS_TO_TICKS(5000));
      // printf("\n\nbefore reset 11\n\n");
      gpio_set_level(GPIO_OUTPUT_IO_PWRKEY, 0);
      // vTaskDelay(pdMS_TO_TICKS(60000));
      // printf("\n\nbefore reset 22\n\n");

      while (gpio_get_level(GPIO_INPUT_IO_EG91_STATUS) &&
             EG91_PowerON_TimeOut < 40) {
        vTaskDelay(pdMS_TO_TICKS(2000));
        // printf("\n\nbefore reset - %d\n\n", EG91_PowerON_TimeOut);
        if (gpio_get_level(GPIO_INPUT_IO_EG91_STATUS)) {
          EG91_PowerON_TimeOut++;
        } else {
          break;
        }
      }
    }
  } else {
    /*  gpio_set_level(GPIO_OUTPUT_IO_PWRKEY, 0);
     vTaskDelay(pdMS_TO_TICKS(2000)); */
    gpio_set_level(GPIO_OUTPUT_IO_PWRKEY, 1);
    vTaskDelay(pdMS_TO_TICKS(5000));
    // printf("\n\nbefore reset 11\n\n");
    gpio_set_level(GPIO_OUTPUT_IO_PWRKEY, 0);
    // vTaskDelay(pdMS_TO_TICKS(60000));
    // printf("\n\nbefore reset 22\n\n");

    while (gpio_get_level(GPIO_INPUT_IO_EG91_STATUS) &&
           EG91_PowerON_TimeOut < 40) {
      vTaskDelay(pdMS_TO_TICKS(2000));
      // printf("\n\nbefore reset - %d\n\n", EG91_PowerON_TimeOut);
      if (gpio_get_level(GPIO_INPUT_IO_EG91_STATUS)) {
        EG91_PowerON_TimeOut++;
      } else {
        break;
      }
    }
  }

  if (gpio_get_level(GPIO_INPUT_IO_EG91_STATUS)) {
    mqtt_connectLabel = 0;
    return 1;
  } else {
    return 0;
  }
}

uint8_t EG91_Power_Reset() {

  uint8_t EG91_PowerON_TimeOut = 0;
  // printf("\n ENTER IN RESET EG\n");

  if (EG91_send_AT_Command("AT+QPOWD=1", "OK", 1500)) {

    // printf("\n ENTER IN RESET EG 11\n");
    vTaskDelay(pdMS_TO_TICKS(1000));

    if (gpio_get_level(GPIO_INPUT_IO_EG91_STATUS)) {
      // printf("\n RESET EG22\n");
      return 0;
    } else {
      gpio_set_level(GPIO_OUTPUT_IO_PWRKEY, 0);
      vTaskDelay(pdMS_TO_TICKS(200));

      gpio_set_level(GPIO_OUTPUT_IO_PWRKEY, 1);
      vTaskDelay(pdMS_TO_TICKS(2000));
      gpio_set_level(GPIO_OUTPUT_IO_PWRKEY, 0);
      vTaskDelay(pdMS_TO_TICKS(8000));
    }
  } else {
    vTaskDelay(pdMS_TO_TICKS(1000));
    if (gpio_get_level(GPIO_INPUT_IO_EG91_STATUS)) {
      EG91_Power_OFF();
      while (gpio_get_level(GPIO_INPUT_IO_EG91_STATUS) &&
             EG91_PowerON_TimeOut < 10) {
        vTaskDelay(pdMS_TO_TICKS(1000));
        EG91_PowerON_TimeOut++;
      }

      if (gpio_get_level(GPIO_INPUT_IO_EG91_STATUS)) {
        // xTaskResumeAll();
        // xTaskResumeAll();
        return 0;
      } else {
        gpio_set_level(GPIO_OUTPUT_IO_PWRKEY, 0);
        vTaskDelay(pdMS_TO_TICKS(200));

        gpio_set_level(GPIO_OUTPUT_IO_PWRKEY, 1);
        vTaskDelay(pdMS_TO_TICKS(2000));
        gpio_set_level(GPIO_OUTPUT_IO_PWRKEY, 0);
        vTaskDelay(pdMS_TO_TICKS(8000));
      }
    } else {
      gpio_set_level(GPIO_OUTPUT_IO_PWRKEY, 0);
      vTaskDelay(pdMS_TO_TICKS(200));
      gpio_set_level(GPIO_OUTPUT_IO_PWRKEY, 1);
      vTaskDelay(pdMS_TO_TICKS(2000));
      gpio_set_level(GPIO_OUTPUT_IO_PWRKEY, 0);
      vTaskDelay(pdMS_TO_TICKS(8000));
    }
  }
  // }

  if (gpio_get_level(GPIO_INPUT_IO_EG91_STATUS)) {

    // printf("\n RESET EG555\n");
    return 1;
  } else {
    // printf("\n RESET EG666\n");
    return 0;
  }

  return 0;
}

uint8_t check_NetworkState() {
  // EG91_send_AT_Command("AT+COPS?", "OK", 5000);

  if (!EG91_send_AT_Command("AT+CREG?", "AT+CREG", 1000)) {
    return 2;
  }

  if (!EG91_send_AT_Command("AT+CGREG?", "AT+CGREG", 1000)) {
    return 3;
  }

  return EG91_send_AT_Command("AT+CSQ", "CSQ", 1000);
}

uint8_t parse_NetworkStatus(char *payload) {

  char *str = strtok(payload, (const char *)",");
  str = strtok((char *)NULL, (const char *)"\r");

  // printf("check %d", atoi(str));

  if ((atoi(str) == 1) || (atoi(str) == 5)) {
    return 1;
  } else {
    RSSI_LED_TOOGLE = RSSI_NOT_DETECT;

    gpio_set_level(GPIO_OUTPUT_ACT, 1);
    return 0;
  }

  return 0;
}

void send_Type_Call_queue(uint8_t state) {}

uint8_t send_UDP_Send(char *data, char *topic) {
  char UDP_OK_Data[10];
  uint8_t count_Send_UDP = 0;
  // printf("\n\n udp send 00\n\n");
  int64_t time1 = esp_timer_get_time();

  if (gpio_get_level(GPIO_INPUT_IO_SIMPRE)) {

    if (!gpio_get_level(GPIO_INPUT_IO_EG91_STATUS)) {
      gpio_set_level(GPIO_OUTPUT_ACT, 1);
      uint8_t pwrFD = 0;
      uint8_t InitNetworkCount = 0;
      uint8_t pwrFD_count = 0;
      // system_stack_high_water_mark("SEND UDP1");
      while (pwrFD != 1) {
        if (pwrFD_count == 3 || pwrFD == 1) {
          break;
        }
        pwrFD_count++;

        pwrFD = EG91_PowerOn();
      }
      // system_stack_high_water_mark("SEND UDP2");
      if (pwrFD == 1) {

        while (InitNetworkCount < 3) {
          // printf("INIT EG915 311");
          if (!EG91_initNetwork()) {
            RSSI_LED_TOOGLE = RSSI_NOT_DETECT;

            gpio_set_level(GPIO_OUTPUT_ACT, 1);
            update_ACT_TimerVAlue((double)RSSI_NOT_DETECT);
            InitNetworkCount++;
          } else {
            // printf("\n\n activate network 1\n\n");
            break;
          }
        }

        if (InitNetworkCount == 3) {
          nvs_erase_key(nvs_System_handle, NVS_NETWORK_LOCAL_CHANGED);
          return 1;
        }
      }
    }

    // TODO:DESCOMENTAR CASO NAO FUNCIONE
    // xSemaphoreGive(rdySem_Control_Send_AT_Command);

    // TODO: RETIRAR CASO FUNCIONE MAL

	uint8_t qmtstatValue = get_INT8_Data_From_Storage(NVS_QMTSTAT_LABEL, nvs_System_handle);
    if (qmtstatValue == 0) {

      if (strlen(topic) < 2) {
        /* code */

        xSemaphoreGive(rdySem_Control_Send_AT_Command);

        while (count_Send_UDP < 3) {
          memset(UDP_OK_Data, 0, sizeof(UDP_OK_Data));
          // printf("\nsms ghghkk444433 %s\n",data);
          // printf("\nsms ghghkk555\n");
          // printf("\n\n task_EG91_Send_UDP 12345 %s\n\n", data);
          if (send_UDP_Package(data, strlen(data), "")) {
            return 1;
          }

          count_Send_UDP++;
        }
      } else {
        xSemaphoreGive(rdySem_Control_Send_AT_Command);

        while (count_Send_UDP < 3) {
          memset(UDP_OK_Data, 0, sizeof(UDP_OK_Data));
          // printf("\nsms ghghkk444433 %s\n", data);
          //  printf("\nsms ghghkk555\n");
          //  printf("\n\n task_EG91_Send_UDP 12345 %s\n\n", data);
          if (send_UDP_Package(data, strlen(data), topic)) {
            return 1;
          }

          count_Send_UDP++;
        }
      }
    }

    count_Send_UDP = 0;
    checkIf_mqttSend_fail(data);
    // printf("\n\nlost pack %d\n\n", label_UDP_fail_and_changed);

    /* if (label_Reset_Password_OR_System == 2)
    {
            vTaskResume(handle_SEND_SMS_TASK);
            // vTaskResume(handle_SMS_TASK);
            timer_start(TIMER_GROUP_1, TIMER_0);
    } */
  } else {

    checkIf_mqttSend_fail(data);

    return 0;
  }
  return 0;
  // vTaskResume(xHandle_Timer_VerSystem);
  //  xSemaphoreGive(rdySem_Control_Send_AT_Command);
  //   vTaskDelay(pdMS_TO_TICKS(3000));
}

void checkIf_mqttSend_fail(char *data) {
  uint8_t label_UDP_fail_and_changed = 0;

  char delim[] = " ";
  // printf("\n\ncount_Send_UDP++ 11;\n\n");
  char *command = strtok(data, delim);
  // printf("\n\ncount_Send_UDP++ 22;\n\n");
  char *parameter = strtok(NULL, delim);
  // printf("\n\ncount_Send_UDP++ 33;\n\n");
  char *element = strtok(NULL, delim);
  // printf("\n\ncount_Send_UDP++ 44;\n\n");
  //  char *element = strtok(NULL, delim);

  // printf("\n\ncount_Send_UDP++ 55;\n\n");
  //  printf("\n\n cmd - %s, parameter - %s, elemente -
  //  %s\n\n",command,parameter,element);
  // printf("\n\ncount_Send_UDP++ 66;\n\n");
  label_UDP_fail_and_changed =
      get_INT8_Data_From_Storage(NVS_NETWORK_LOCAL_CHANGED, nvs_System_handle);
  // printf("\n\ncount_Send_UDP++ 77;\n\n");
  // printf("\n\ncount_Send_UDP++ 66;\n\n");

  if (label_UDP_fail_and_changed == 255) {
    label_UDP_fail_and_changed = 0;
    label_UDP_fail_and_changed |= 3;
    save_INT8_Data_In_Storage(NVS_NETWORK_LOCAL_CHANGED,
                              label_UDP_fail_and_changed, nvs_System_handle);
  }

  if (!strcmp(command, "RT")) {
    label_UDP_fail_and_changed |= 1;
  }

  if (!strcmp(command, "UR") ||
      (!strcmp(command, "ME") && (parameter[0] == 'S' || parameter[0] == 'R') &&
       (element[0] == 'A' || element[0] == 'U'))) {
    label_UDP_fail_and_changed |= 2;
    // printf("\n\nlost pack UR %d\n\n", label_UDP_fail_and_changed);
  }

  if (!strcmp(command, "ME") && parameter[0] == 'R' && element[0] == 'K') {
    label_UDP_fail_and_changed |= 4;
  }

  /* if ((!strcmp(command, "ME") && (parameter[0] == 'S' || parameter[0] == 'R')
  && element[0] == 'F'))
  {
          label_UDP_fail_and_changed |= 4;
  } */

  save_INT8_Data_In_Storage(NVS_NETWORK_LOCAL_CHANGED,
                            label_UDP_fail_and_changed, nvs_System_handle);
}
void task_EG91_Send_UDP(void *pvParameter) {

  mqtt_information mqttInfo;

  for (;;) // ajudado por deus deus@paraiso.org!!
  {

    xQueueReceive(UDP_Send_queue, &mqttInfo, portMAX_DELAY);

    // printf("\nsms mqtt00\n");
    /* if (label_ResetSystem == 1)
    {
            timer_pause(TIMER_GROUP_1, TIMER_0);
            //printf("\nsms mqtt\n");
            disableAlarm();
            vTaskSuspend(xHandle_Timer_VerSystem);
            //printf("\nsms mqtt1\n");
            vTaskSuspend(handle_SEND_SMS_TASK);
            //printf("\nsms mqtt2\n");
            xSemaphoreGive(rdySem_Control_Send_AT_Command);
    } */

    // save_INT8_Data_In_Storage(NVS_QMT_LARGE_DATA_TIMER_LABEL, 1,
    // nvs_System_handle); xSemaphoreTake(rdySem_Control_SMS_UDP,
    // pdMS_TO_TICKS(3000));
    // printf("\n\n receive UDP queue %s - %s\n\n", mqttInfo.data,
    // mqttInfo.topic);
    send_UDP_Send(mqttInfo.data, mqttInfo.topic);

    /* if (label_ResetSystem == 1)
    {
            vTaskResume(handle_SEND_SMS_TASK);
            vTaskResume(xHandle_Timer_VerSystem);
            timer_start(TIMER_GROUP_1, TIMER_0);
            enableAlarm();
    } */
    // save_INT8_Data_In_Storage(NVS_QMT_LARGE_DATA_TIMER_LABEL, 0,
    // nvs_System_handle);
    //  xSemaphoreGive(rdySem_Control_SMS_UDP);
  }
}

void send_UDP_queue(mqtt_information *mqttInfo) {
  // printf("\n\ntask_refresh_SystemTime 000111 - %s - %s\n\n", mqttInfo->data,
  // mqttInfo->topic);

  xQueueSendToBack(UDP_Send_queue, mqttInfo, pdMS_TO_TICKS(3000));
}

esp_ota_handle_t updateHandle = 0;
const esp_partition_t *updatePartition = NULL;

void EG915_fota(mqtt_information *mqttInfo) {
  char atCommand[200] = {};
  uint32_t CRC32_FOTA = 0;

  timer_pause(TIMER_GROUP_1, TIMER_0);
  // xTimerStop(xTimers, 0);
  //  // printf("\nsms\n");
  //  disableAlarm();
  //  vTaskSuspend(xHandle_Timer_VerSystem);
  //  vTaskSuspend(handle_SEND_SMS_TASK);

  EG915_readDataFile_struct.mode = EG91_FILE_FOTA_MODE;
  disableBLE();

  EG91_send_AT_Command("AT+QFDEL=\"sound_8.wav\"", "OK", 1000);
  EG91_send_AT_Command("AT+QFDEL=\"sound_7.wav\"", "OK", 1000);
  EG91_send_AT_Command("AT+QFDEL=\"sound_6.wav\"", "OK", 1000);
  EG91_send_AT_Command("AT+QFDEL=\"sound_5.wav\"", "OK", 1000);
  EG91_send_AT_Command("AT+QFDEL=\"sound_4.wav\"", "OK", 1000);
  EG91_send_AT_Command("AT+QFDEL=\"sound_3.wav\"", "OK", 1000);
  EG91_send_AT_Command("AT+QFDEL=\"sound_2.wav\"", "OK", 1000);
  EG91_send_AT_Command("AT+QFDEL=\"UFS:3.txt\"", "OK", 1000);
  EG91_send_AT_Command("AT+QINDCFG=\"ring\", 0", "OK", 1000);

  EG91_send_AT_Command("AT+CNMI=0,0,0,0,0", "OK", 1000);

  EG91_send_AT_Command("AT+QIACT?", "OK", 1000);
  EG91_send_AT_Command("AT+QHTTPCFG=\"contextid\",1", "OK", 1000);
  EG91_send_AT_Command("AT+QHTTPCFG=\"responseheader\",1 ", "OK", 1000);
  // snprintf(atCommand, sizeof(atCommand), "AT+QHTTPURL=%d,%d\r\n%s%c",
  // strlen("https://www.alipay.com"), 60, "https://www.alipay.com",13);

  EG91_send_AT_Command("AT+QHTTPCFG=\"sslctxid\",1", "OK", 1000);
  EG91_send_AT_Command("AT+QSSLCFG=\"sslversion\",1,3", "OK", 1000);
  EG91_send_AT_Command("AT+QSSLCFG=\"ciphersuite\",1,0XC02F", "OK", 1000);
  EG91_send_AT_Command("AT+QSSLCFG=\"seclevel\",1,0 ", "OK", 1000);
  EG91_send_AT_Command("AT+QSSLCFG=\"ignoreinvalidcertsign\",1,1 ", "OK", 1000);
  EG91_send_AT_Command("AT+QSSLCFG=\"sni\",1,1", "OK", 1000);
  EG91_send_AT_Command("AT+QHTTPCFG=\"responseheader\",0", "OK", 1000);

  // EG91_send_AT_Command("AT+QSSLCFG=\"renegotiation\",1,1","OK",1000);
  // EG91_send_AT_Command("AT+QSSLOPEN=1,1,4,\"https://api.mconnect.motorline.pt/health\",443,0","OK",1000);
  // vTaskDelay(pdMS_TO_TICKS(5000));
  sprintf(atCommand, "%s%d%s%c", "AT+QHTTPURL=",
          strlen("https://static.portal.motorline.pt/firmwares/m250.bin"),
          ",80", 13);
  EG91_send_AT_Command(atCommand, "CONNECT", 60000);
  // uart_write_bytes(UART_NUM_1, atCommand, strlen(atCommand));
  vTaskDelay(pdMS_TO_TICKS(520));
  // EG91_send_AT_Command(atCommand,"OK",60000);
  EG91_send_AT_Command("https://static.portal.motorline.pt/firmwares/m250.bin",
                       "OK", 60000);

  // EG91_send_AT_Command("AT","OK",10000);

  EG91_send_AT_Command("AT+QHTTPGET=80", "QHTTPGET", 10000);
  // vTaskDelay(pdMS_TO_TICKS(10000));
  // EG91_send_AT_Command("AT+QHTTPREAD=80","OK",10000);

  char AT_Command[100];
  // EG91_send_AT_Command("AT+QHTTPREAD=80", "OK", 10000);
  EG91_send_AT_Command("AT+QHTTPREADFILE=\"UFS:3.txt\",80", "OK", 10000);
  EG91_send_AT_Command("ATE1", "OK", 1000);
  uint8_t ACK = 0;
  ACK = EG91_send_AT_Command("AT+QFOPEN=\"UFS:3.txt\"", "+QFOPEN:", 20000);
  // vTaskDelay(pdMS_TO_TICKS(5000));788206364

  esp_err_t err;
  int idFile = atoi(fileID);
  int nowFileSize = 0;
  // printf("\nfile id %d - %s", idFile, fileID);
  sprintf((char *)iv, "%s", "cqfDXcNe167GMAT2");
  if (ACK) {

    updatePartition = esp_ota_get_next_update_partition(NULL);
    // ESP_LOGI(GATTS_TABLE_TAG, "Writing to partition subtype %d at offset
    // 0x%x",                        update_partition->subtype,
    // update_partition->address);
    assert(updatePartition != NULL);
    err = esp_ota_begin(updatePartition, OTA_WITH_SEQUENTIAL_WRITES,
                        &updateHandle);

    sprintf(AT_Command, "AT+QFREAD=%d,1024", idFile);
    int count = 0;
    while (nowFileSize < EG915_readDataFile_struct.fileSize) {
      EG91_send_AT_Command(/* "AT+QFREAD=1027,1024" */ AT_Command, "+QFREAD",
                           3000);

      nowFileSize += EG915_readDataFile_struct.nowFileSize;
      err = esp_ota_write(updateHandle,
                          (const void *)EG915_readDataFile_struct.receiveData,
                          EG915_readDataFile_struct.decryptSize);

      if (err != ESP_OK) {
        err = ESP_FAIL;
        // 	esp_ota_abort(updateHandle);
        // 	EG915_readDataFile_struct.mode = 0;
        // 	sprintf(AT_Command, "AT+QFCLOSE=%d", idFile);
        // EG91_send_AT_Command(/* "AT+QFREAD=1027,1024" */ AT_Command, "OK",
        // 3000);

        ESP_LOGI("GATTS_TABLE_TAG", "esp_ota_write error!");
        break;
      }

      // printf("\n\nnowFileSize 11 - %d - %d \n\n", nowFileSize,
      // EG915_readDataFile_struct.fileSize);

      CRC32_FOTA = esp_rom_crc32_le(
          CRC32_FOTA, (uint8_t *)EG915_readDataFile_struct.receiveData,
          EG915_readDataFile_struct.packetFile_size);
      // printf("CRC32: 0x%08X - 0x%08X\n", CRC32_FOTA,
      // EG915_readDataFile_struct.ckm);

      count++;
    }

    if (err != ESP_OK || CRC32_FOTA != EG915_readDataFile_struct.ckm) {
      // esp_ota_end()
      esp_ota_abort(updateHandle);
      send_UDP_Send("«\0", "");
      sprintf(AT_Command, "AT+QFCLOSE=%d", idFile);
      EG91_send_AT_Command(/* "AT+QFREAD=1027,1024" */ AT_Command, "OK", 3000);

      memset(&EG915_readDataFile_struct, 0, sizeof(EG915_readDataFile_struct));
      if (err == ESP_ERR_OTA_VALIDATE_FAILED) {
        // ESP_LOGE(GATTS_TABLE_TAG, "Image validation failed, image is
        // corrupted");
      }
    } else {
      err = esp_ota_end(updateHandle);
      err = esp_ota_set_boot_partition(updatePartition);

      sprintf(AT_Command, "AT+QFCLOSE=%d", idFile);
      EG91_send_AT_Command(AT_Command, "OK", 1000);
      EG91_send_AT_Command("AT+QFDEL=\"UFS:3.txt\"", "OK", 1000);

      vTaskDelay(pdMS_TO_TICKS(1500));
      esp_restart();
    }
  } else {
    // esp_ota_abort(updateHandle);
    send_UDP_Send("«\0", "");
  }

  restartBLE();
  EG91_send_AT_Command("AT+QFDEL=\"UFS:3.txt\"", "OK", 1000);
  // printf("CRC32: 0x%08X - 0x%08X\n", CRC32_FOTA,
  // EG915_readDataFile_struct.ckm);
  EG91_send_AT_Command("AT+QINDCFG=\"ring\", 1", "OK", 1000);
  EG915_readDataFile_struct.ckm = 0;
  EG91_send_AT_Command("AT+CNMI=2,1,0,0,0", "OK", 1000);

  // vTaskResume(xHandle_Timer_VerSystem);
  // vTaskResume(handle_SEND_SMS_TASK);
  // xTimerStart(xTimers, 0);
  timer_start(TIMER_GROUP_1, TIMER_0);
  // enableAlarm();
  EG915_readDataFile_struct.mode = EG91_FILE_NORMAL_MODE;
}

void init_EG91(void) {
  // xTaskCreate(uart_event_task, "uart_event_task", 8000, NULL, 4, NULL);
  // printf("INIT EG915 1");
  // printf("\n\n akakak 777\n\n");
  uartInit();
  // printf("\n\n akakak 888\n\n");
  uint8_t pwrFD = 0;

  mqtt_openLabel = 0;
  mqtt_connectLabel = 0;
  // printf("INIT EG915 2");

  rdySem_Control_Send_AT_Command = xSemaphoreCreateBinary();
  // printf("\n\n akakak 999\n\n");
  rdySem_QPSND = xSemaphoreCreateBinary();
  rdySem_UART_CTR = xSemaphoreCreateBinary();

  rdySem_Control_SMS_UDP = xSemaphoreCreateBinary();
  rdySem_Control_pubx = xSemaphoreCreateBinary();
  // printf("\n\n akakak 1000\n\n");
  xSemaphoreGive(rdySem_Control_Send_AT_Command);

  xSemaphoreTake(rdySem_QPSND, 0);
  // printf("\n\n akakak 1001\n\n");

  // xSemaphoreTake(rdySem_Control_IncomingCALL, (portTickType) portMAX_DELAY);

  // ESP_LOGI("TAG", "xPortGetFreeHeapSize EG91 QUEUE: %d",
  // xPortGetFreeHeapSize()); ESP_LOGI("TAG", "esp_get_minimum_free_heap_size  :
  // %d", esp_get_minimum_free_heap_size()); ESP_LOGI("TAG",
  // "heap_caps_get_largest_free_block: %d",
  // heap_caps_get_largest_free_block(MALLOC_CAP_DEFAULT)); ESP_LOGI("TAG",
  // "free heap memory                : %d",
  // heap_caps_get_free_size(MALLOC_CAP_8BIT));

  AT_Command_Feedback_queue =
      xQueueCreate(AT_QUEUE_SIZE, sizeof(char) * BUF_SIZE);

  // Type_Call_queue = xQueueCreate(2, sizeof(call_Type));
  receive_mqtt_queue = xQueueCreate(2, sizeof(char) * 500);
  NO_CARRIER_Call_queue = xQueueCreate(1, 50);
  UDP_Send_queue = xQueueCreate(2, sizeof(mqtt_information));
  HTTPS_data_queue = xQueueCreate(2, sizeof(char) * 30);

  // Receive_UDP_OK_queue = xQueueCreate(5, sizeof(char) * 10);
  // Lost_SMS_queue = xQueueCreate(1, sizeof(data_ReceiveAT_Serial));
  // EG91_GET_SIM_BALANCE_queue = xQueueCreate(1, 200);

  // EG91_WRITE_FILE_queue = xQueueCreate(5, 100);

  // xTaskCreate(task_EG91_Verify_Unread_SMS, "task_EG91_Verify_Unread_SMS",
  // 7000, NULL, 20, NULL);

  // printf("\n\n\n before power on\n\n\n");
  xTaskCreate(task_EG91_Send_UDP, "task_EG91_Send_UDP", 9 * 1024, NULL, 26,
              &handle_UDP_TASK);
  xTaskCreate(task_EG91_Receive_UDP, "task_EG91_Receive_UDP",
              15000 + 2046 + 1024, NULL, 20, NULL);
  //  initTimers();

  if (gpio_get_level(GPIO_INPUT_IO_SIMPRE)) {

    uint8_t pwrFD_count = 0;
    pwrFD = 0;
    uint8_t InitNetworkCount = 0;

    while (pwrFD != 1) {
      if (pwrFD_count == 3 || pwrFD == 1) {
        break;
      }
      pwrFD_count++;

      pwrFD = EG91_PowerOn();
    }
    // esp_timer_stop(periodic_timer);
    send_ATCommand_Label = 1;

    // feedback_Audio_timeout = 0;

    // Type_Call_queue = xQueueCreate(1, sizeof(uint8_t));

    // ESP_LOGI("TAG", "xPortGetFreeHeapSize EG91 AFTER POWER ON: %d",
    // xPortGetFreeHeapSize()); ESP_LOGI("TAG", "esp_get_minimum_free_heap_size
    // : %d", esp_get_minimum_free_heap_size()); ESP_LOGI("TAG",
    // "heap_caps_get_largest_free_block: %d",
    // heap_caps_get_largest_free_block(MALLOC_CAP_DEFAULT)); ESP_LOGI("TAG",
    // "free heap memory                : %d",
    // heap_caps_get_free_size(MALLOC_CAP_8BIT));

    // printf("INIT EG915 3");
    //   EG91_writeFile(f1, t1);
    //   EG91_send_AT_Command("AT+QFLST=\"*\"", "OK", 1000);
    //   gpio_set_level(GPIO_OUTPUT_ACT, 1);
    // printf("\n pwrfg = %d\n", pwrFD);

    if (pwrFD == 1) {
      while (InitNetworkCount < 3) {
        // printf("INIT EG915 311");
        if (!EG91_initNetwork()) {
          RSSI_LED_TOOGLE = RSSI_NOT_DETECT;

          gpio_set_level(GPIO_OUTPUT_ACT, 1);
          update_ACT_TimerVAlue((double)RSSI_NOT_DETECT);
          InitNetworkCount++;
        } else {
          // printf("\n\n init OK label \n\n");
          break;
        }
      }
    } else {
      InitNetworkCount = 3;
    }

    if (InitNetworkCount == 3) {
      update_ACT_TimerVAlue((double)RSSI_NOT_DETECT);
    }
  } else {
    // printf("\n\n init NOT OK label \n\n");
  }

  // EG915_fota();
}

/* uint8_t som[] asm("_binary_som_beep_wav_start");
uint8_t som_end[] asm("_binary_som_beep_wav_end"); */

// uint8_t som[] asm("_binary_sound_8_wav_start");
// uint8_t som_end[] asm("_binary_sound_8_wav_end");

// /* uint8_t som_final[] asm("_binary_som_beep_final_wav_start");
// uint8_t som_final_end[] asm("_binary_som_beep_final_wav_end"); */

// uint8_t som_final[] asm("_binary_alertMotorline_wav_start");
// uint8_t som_final_end[] asm("_binary_alertMotorline_wav_end");

void EG91_writeFile(char *fileName, char *file, int filesize) {

  //     char atCommand[100];
  //     uint8_t counterACK = 5;
  //     char dtmp1[BUFF_SIZE];
  //     unsigned int som_len = sizeof(uint8_t) * (som_end - som);
  //     unsigned int somFinal_len = sizeof(uint8_t) * (som_final_end -
  //     som_final);
  //     /* FILE *exemple;
  //     fopen() */
  //     printf("\nWRITE atCommand1 11\n");
  //     // EG91_send_AT_Command("AT+QFLST=\"*\"", "OK", 1000);
  //     printf("\nWRITE atCommand1 22\n");
  //     sprintf(atCommand, " AT+QFOPEN=\"%s\"", "sound_8.wav" /* fileName */);

  //     EG91_send_AT_Command(atCommand, "QFOPEN", 1000);
  //     printf("\nWRITE atCommand1 33\n");
  //     /* sprintf(atCommand, "AT+QFREAD=%d,%d", atoi(fileID), 30);
  //     printf("\nWRITE atCommand1 %s - atoi %d - %d\n", atCommand,
  //     atoi(fileID), strlen(atCommand)); EG91_send_AT_Command(atCommand, "OK",
  //     3000); */

  //     memset(atCommand, 0, sizeof(atCommand));
  //     sprintf(atCommand, "AT+QFWRITE=%d,%d,80", atoi(fileID), som_len /*
  //     filesize */); printf("\nWRITE atCommand1 %s - atoi %d - %d\n",
  //     atCommand, atoi(fileID), strlen(atCommand));
  //     EG91_send_AT_Command(atCommand, "CONNECT", 5000);
  //     // vTaskDelay(pdMS_TO_TICKS(250));

  //     printf("\nWRITE FILESIZE %d\n", filesize);

  //     uart_write_bytes(UART_NUM_1, som /* file */, som_len /* filesize */);
  //     printf("\nWRITE FILE 1 %s - %d\n", som_final, somFinal_len);

  //     while (counterACK > 0)
  //     {
  //         memset(dtmp1, 0, sizeof(dtmp1));
  //         printf("\nWRITE FILE 2\n");
  //         xQueueReceive(EG91_WRITE_FILE_queue, &dtmp1, pdMS_TO_TICKS(5000 *
  //         2)); printf("\nWRITE FILE 3\n");
  //         printf("\nAT_Command_Feedback_queue = %s\n", dtmp1);
  //         printf("\nWRITE FILE 4\n");
  //         if (strstr(dtmp1, "OK") != NULL)
  //         {
  //             /* char *fileName;
  //             FILE *f = NULL;
  //             asprintf(&fileName, "/sdcard/%s", fileName);
  //             printf("\nfileName sdcard %s\n", fileName);
  //             f = fopen(fileName, "a");

  //             for (int i = 0; i < filesize; i++)
  //             {
  //                 fprintf(f, "%c", file[i]);
  //                 // fwrite(file,1,sizeof(file),f);
  //             }

  //             // fprintf(f, "%d", file);

  //             fclose(f);
  //  */
  //             memset(atCommand, 0, sizeof(atCommand));
  //             // sprintf(atCommand, "AT+QFCLOSE=%d", atoi(fileID));
  //             // EG91_send_AT_Command(atCommand, "OK", 1000);
  //             printf("\nWRITE FILE 5\n"); // file_CRC32
  //             // esp_rom_crc32_le(crc32_val, data1, strlen(file));
  //             break;
  //         }
  //         else
  //         {
  //             printf("\nWRITE FILE 6\n");
  //             counterACK--;
  //         }

  //         vTaskDelay(pdMS_TO_TICKS((1000)));
  //         break;
  //     }

  //     // EG91_send_AT_Command("AT+QFLST=\"*\"", "OK", 1000);
  //     printf("\nWRITE FILE 7\n");
  //     printf("\nWRITE FILE 8\n");
  //     send_ATCommand_Label = 0;
  // return 0;
}

uint8_t parse_OpenFile(char *payload) {
  regex_t regex;
  const char s[2] = ":";
  const char sp[2] = " ";
  char *token;
  int strIndex = 0;
  int auxCount = 0;
  char dtmp1[100];
  int reti;

  // xQueueReceive(AT_Command_Feedback_queue, &dtmp1, pdMS_TO_TICKS(time));

  reti = regcomp(&regex, "^[0-9]+$", REG_EXTENDED);
  memset(fileID, 0, sizeof(fileID));

  // char response[] = "+QFOPEN: 1027";

  char codigo[10]; // Ajuste o tamanho conforme necessário

  // Encontrar a posição do primeiro espaço após ":"
  token = strstr(payload, ": ") + 2;

  // Copiar o código para a variável 'codigo'
  sscanf(token, "%s", fileID);

  // Imprimir o código
  // printf("Código do arquivo: %s\n", fileID);

  /* for (int i = 0; i < strlen(payload); i++)
  {
          if (payload[i] == ':')
          {
                  auxCount++;
                  strIndex = 0;
          }
          else
          {
                  if (auxCount == 1)
                  {
                          if (payload[i] >= '0' && payload[i] <= '9')
                          {
                                  fileID[strIndex++] = payload[i];

                                  if (strIndex > 5)
                                  {
                                          return 0;
                                  }
                          }
                  }
          }
  } */

  // printf("\ntoken3 =%s\n", fileID);

  if (strlen(fileID) == 0 || (regexec(&regex, fileID, 0, NULL, 0))) {
    return 0;
  }

  /*

          token = strtok(payload, s);
          ////printf("\ntxt1 = %s\n", payload);
          token = strtok(NULL, s);
          //printf("\ntoken1 =%s\n", token);
          token = strtok(token, ":");
          //printf("\ntoken2 =%s\n", token);
          token = strtok(NULL, sp);
          //printf("\ntoken3 =%s\n", token);

          sprintf(fileID, "%s", token);
          //printf("\nfileID =%s\n", fileID); */

  return 1;
}

uint8_t verify_QFWRITE(char *payload) {
  if ((strstr(payload, "CONNECT")) != NULL) {
    return 1;
  } else {
    return 0;
  }
}

uint8_t EG91_parse_CPAS(char *receiveData) {
  const char s1[2] = ": ";
  const char s2[2] = " ";

  char *token;
  token = strtok(receiveData, s1);

  /* walk through other tokens */

  token = strtok(NULL, s2);
  // printf("token cpas=%s\n", token);

  if (token[0] == '0') {
    return 1;
  } else {
    return 0;
  }
}

uint8_t parse_IMEI(char *payload) {
  uint8_t enterCount = 0;
  uint8_t strIndex = 0;
  memset(EG91_IMEI, 0, 20);

  for (size_t i = 0; i < strlen(payload); i++) {
    if (payload[i] == '\n') {
      enterCount++;
      strIndex = 0;
    } else {
      if (enterCount == 1) {
        EG91_IMEI[strIndex++] = payload[i];
      }

      if (enterCount > 1) {
        break;
      }
    }
  }

  EG91_IMEI[strlen(EG91_IMEI) - 1] = 0;

  for (size_t i = 0; i < strlen(EG91_IMEI); i++) {
    if (EG91_IMEI[i] < '0' || EG91_IMEI[i] > '9') {
      return 0;
    }
  }

  // printf("\n\n\n parse imei %s\n\n\n", EG91_IMEI);
  return 1;
}

uint8_t EG91_parse_CPIN(char *payload) {

  // printf("\n\n parse CPIN \n\n");

  if (strstr(payload, "READY") != NULL) {
    SIM_CARD_PIN_status = 1;

    return 1;
  } else {
    SIM_CARD_PIN_status = 0;

    RSSI_LED_TOOGLE = RSSI_NOT_DETECT;

    gpio_set_level(GPIO_OUTPUT_ACT, 1);

    return 0;
  }

  return 0;
}

uint8_t parse_QMTOPEN(char *receiveData) {
  uint8_t auxcounter = 0;
  char qmtopenResult = 0;

  // printf("\n\n QMTOPEN data %s\n\n", receiveData);

  for (size_t i = 0; i < strlen(receiveData); i++) {
    if (receiveData[i] == ',') {
      qmtopenResult = receiveData[i + 1];
      break;
    }
  }

  // printf("\n\n QMTOPEN data %c\n\n", qmtopenResult);

  if (qmtopenResult == '0') {
    return 1;
  } else {
    return 0;
  }

  return 0;
}

uint8_t parse_at_qmtopen_(char *receiveData) {
  uint8_t counter = 0;

  for (size_t i = 0; i < strlen(receiveData); i++) {
    if (receiveData[i] == ',') {
      counter++;
    }
  }

  if (counter == 0) {
    mqtt_openLabel = 0;
  } else {
    mqtt_openLabel = 1;
  }

  // printf("\n\n mqtt_openLabel - %d - %s\n\n", mqtt_openLabel, receiveData);

  return 1;
}

uint8_t EG91_parse_QFREAD(char *payload) {
  if (payload == NULL) {
    // printf("Payload inválido.\n");
    return 0; // Se o argumento for inválido, retorna falso
  }

  /* code */

  EG915_readDataFile_struct.readOK =
      0; // Por padrão, readOK é definido como 0 (falso)

  char *connectPtr = strstr(payload, "CONNECT");
  if (connectPtr != NULL) {
    sscanf(connectPtr, "CONNECT %d",
           &EG915_readDataFile_struct
                .nowFileSize); // Lê o nowFileSize após "CONNECT"
    // printf("nowFileSize: %d\n", EG915_readDataFile_struct.nowFileSize);
    char *dataStart = strchr(connectPtr, '\n'); // Início da sequência de dados
    if (dataStart != NULL) {
      char *dataEnd = strstr(dataStart, "\r\nOK"); // Fim da sequência de dados
      if (dataEnd != NULL) {
        size_t dataSize =
            dataEnd - dataStart - 1; // Calcula o tamanho da sequência de dados
                                     // printf("Data size: %zu\n", dataSize);
        // if (dataSize < sizeof(EG915_readDataFile_struct.receiveData)) {
        unsigned char decrypted[2000] = {};

        strncpy(EG915_readDataFile_struct.receiveData, dataStart + 1,
                dataSize); // Copia a sequência de dados
        EG915_readDataFile_struct.receiveData[dataSize] = '\0';
        /* printf("\n\n aaa 2222\n\n");
        free(connectPtr);				 // Adiciona o
        terminador de string
        //printf("\n\n aaa 3333\n\n"); */
        // printf("\nData received1111: %s\n",
        // EG915_readDataFile_struct.receiveData);
        if (EG915_readDataFile_struct.mode == EG91_FILE_FOTA_MODE) {
          decrypt_aes_cfb_padding(
              (unsigned char *)EG915_readDataFile_struct.receiveData,
              strlen((char *)EG915_readDataFile_struct.receiveData),
              &decrypted);
          return 1;
        } else if (EG915_readDataFile_struct.mode == EG91_FILE_USERS_MODE) {

          return 1;
        } else {
          return 0;
        }

        EG915_readDataFile_struct.readOK = 1; // Define readOK como verdadeiro
      } else {
        // printf("OK not found.\n");
        return 0; // Retorna falso se "\nOK" não for encontrado
      }
    } else {
      // printf("Data start not found.\n");
      return 0; // Retorna falso se o início dos dados não for encontrado
    }
  } else {
    // printf("CONNECT not found.\n");
    return 0; // Retorna falso se "CONNECT" não for encontrado
  }
  // printf("\n\nnowlifesize %d\n\n", EG915_readDataFile_struct.nowFileSize);
  return EG915_readDataFile_struct
      .readOK; // Retorna 1 se OK foi encontrado, senão retorna 0

  return 0;
}

int8_t EG91_Parse_ReceiveData(char *receiveData, char *rsp) {

  char dtmp[100];

  if ((strstr(receiveData, rsp)) != NULL) {
    // printf("\n(strstr(receiveData, rsp)) != NULL\n");
    // system_stack_high_water_mark("receiveData1");
    if (!strcmp(rsp, "OK")) {
      if ((strstr(receiveData, "OK")) != NULL) {
        if (strstr(receiveData, "+CNUM")) {
          EG91_parse_CNUM(receiveData);
        }

        return 1;
      } else {
        // vTaskDelay(pdMS_TO_TICKS((100)));
        return 0;
      }
    } else if (!strcmp(rsp, "AT+CREG") || !strcmp(rsp, "AT+CGREG")) {
      return parse_NetworkStatus(receiveData);
    } else if (!strcmp(rsp, "+CMGR")) {
      // printf("\n\nparse_SMS_Payload %s\n\n", receiveData);

      return 1;
    } else if (!strcmp(rsp, "AT+QMTCONN?")) {
      // printf("\nstrcmp(rsp, +CMGR)\n");
      //   xSemaphoreGive(rdySem_Control_Send_AT_Command);

      return parse_qmtconn(receiveData);
    } else if (!strcmp(rsp, "+QMTOPEN:")) {
      return parse_verify_mqtt_open(receiveData);
    } else if (!strcmp(rsp, "AT+QMTOPEN?")) {

      return parse_at_qmtopen_(receiveData);
    } else if (!strcmp(rsp, "+QMTCONN:")) {
      return parse_verify_mqtt_conection(receiveData);
    } else if (!strcmp(rsp, "QHTTPGET")) {
      return parse_https_get(receiveData);
    } else if (!strcmp(rsp, "GSN")) {

      parse_IMEI(receiveData);
      return 1;
    } else if (!strcmp(rsp, "+QFREAD")) {
      return EG91_parse_QFREAD(receiveData);
    } else if (!strcmp(rsp, "+QMTSUB:")) {
      return parse_qmtsub(receiveData);
    } else if (!strcmp(rsp, "CPIN")) {
      return EG91_parse_CPIN(receiveData);
    } else if (!strcmp(rsp, "CPAS")) {
      return EG91_parse_CPAS(receiveData);
    } else if (!strcmp(rsp, "QLTS")) {
      return parse_NowTime(receiveData);
    } else if (!strcmp(rsp, "CSQ")) {
      // printf("\nparse RSSI\n");
      parse_RSSI(receiveData);
      return 1;
    } else if (!strcmp(rsp, "CMGL")) {

      return 1;
    } else if (!strcmp(rsp, "+QFOPEN:")) {
      // printf("\nparse RSSI\n");
      return parse_OpenFile(receiveData);
    } else if (!strcmp(rsp, "CONNECT")) {
      // printf("\nparse verify_QFWRITE\n");
      return verify_QFWRITE(receiveData);
    }
    /* else if (!strcmp(rsp, "CUSD"))
    {
            // printf("\nparse CUSD\n");
            return EG91_parse_CUSD(receiveData);
    } */
    else if (!strcmp(rsp, "+QCCID")) {
      // printf("\nparse CUSD\n");
      return EG91_parse_ICCID(receiveData);
    } else if (!strcmp(rsp, "+CIMI")) {
      return EG91_parseGet_imsi(receiveData);
    } else if (!strcmp(rsp, "ERROR")) {
      if ((strstr(receiveData, "ERROR")) != NULL) {
        return 1;
      } else {
        return 0;
      }
    }
  } else {
    return 0;
  }

  return 0;
}

uint8_t EG91_parse_ICCID(char *receiveData) {

  char *number_start;
  char *number_end;
  char *number;
  char *prefix = "+QCCID: ";

  // printf("\n\niccid iccid\n\n");

  // Procurar a sequência "+QCCID: " na resposta
  number_start = strstr(receiveData, prefix);

  if (number_start != NULL) {
    // Avançar o ponteiro para o início do número
    number_start += strlen(prefix);

    // Procurar o delimitador de nova linha para encontrar o final do número
    number_end = strchr(number_start, '\r');

    if (number_end != NULL) {
      // Copiar o número para uma nova string
      int length = number_end - number_start;
      number = malloc(length + 1);
      strncpy(number, number_start, length);
      number[length] = '\0';

      // printf("Número:%s\n", number);
      memset(EG91_ICCID_Number, 0, sizeof(EG91_ICCID_Number));

      sprintf(EG91_ICCID_Number, "%s", number);
      save_STR_Data_In_Storage(NVS_EG91_ICCID_VALUE, EG91_ICCID_Number,
                               nvs_System_handle);
      // printf("EG91_ICCID_Number:%s\n", EG91_ICCID_Number);
      //  Liberar a memória alocada
      free(number);
    }
  }

  return 1;
}

uint8_t parse_RSSI(char *payload) {

  char *RSSI_ValueSTR = (strtok((char *)payload, (const char *)":") + 2);
  // printf("\nRSSI value11 = %s\n", RSSI_ValueSTR);
  RSSI_ValueSTR = strtok(((char *)NULL), (const char *)",");

  // printf("\nint rssi %d\n", atoi(RSSI_ValueSTR));
  RSSI_VALUE = atoi(RSSI_ValueSTR);

  if ((RSSI_VALUE >= 20 && RSSI_VALUE <= 31) ||
      (RSSI_VALUE >= 120 && RSSI_VALUE <= 131)) {
    RSSI_LED_TOOGLE = RSSI_EXCELLENT_LED_TIME;
    // update_ACT_TimerVAlue((double)RSSI_EXCELLENT_LED_TIME);
    updateSystemTimer(SYSTEM_TIMER_NORMAL_STATE);
  } else if ((RSSI_VALUE >= 15 && RSSI_VALUE <= 19) ||
             (RSSI_VALUE >= 115 && RSSI_VALUE <= 119)) {
    RSSI_LED_TOOGLE = RSSI_GOOD_LED_TIME;
    // update_ACT_TimerVAlue((double)RSSI_GOOD_LED_TIME);
    updateSystemTimer(SYSTEM_TIMER_NORMAL_STATE);
  } else if ((RSSI_VALUE >= 10 && RSSI_VALUE <= 14) ||
             (RSSI_VALUE >= 110 && RSSI_VALUE <= 114)) {
    RSSI_LED_TOOGLE = RSSI_WEAK_LED_TIME;
    // update_ACT_TimerVAlue(RSSI_WEAK_LED_TIME);
    updateSystemTimer(SYSTEM_TIMER_ALARM_STATE);
  } else if ((RSSI_VALUE >= 0 && RSSI_VALUE <= 9) ||
             (RSSI_VALUE >= 100 && RSSI_VALUE <= 109)) {
    RSSI_LED_TOOGLE = RSSI_VERY_WEAK_LED_TIME;
    // update_ACT_TimerVAlue((double)RSSI_VERY_WEAK_LED_TIME);
    updateSystemTimer(SYSTEM_TIMER_URGENT_STATE);
  } else if (RSSI_VALUE == 99 || RSSI_VALUE == 199) {
    RSSI_LED_TOOGLE = RSSI_NOT_DETECT;
    // update_ACT_TimerVAlue((double)RSSI_NOT_DETECT);
    updateSystemTimer(SYSTEM_TIMER_URGENT_STATE);
  } else {
    RSSI_LED_TOOGLE = RSSI_NOT_DETECT;
    // update_ACT_TimerVAlue((double)RSSI_NOT_DETECT);
    updateSystemTimer(SYSTEM_TIMER_URGENT_STATE);
  }

  // printf("\nRSSI value22 = %s\n", RSSI_ValueSTR);

  return 1;
}

void verify_SMS_List() {
  char AT_Command[20];
  // xSemaphoreTake(rdySem_Lost_SMS, pdMS_TO_TICKS(30000));
  // xSemaphoreTake(rdySem_Lost_SMS, pdMS_TO_TICKS(2000));CMGL
  EG91_send_AT_Command("AT+CMGL=\"ALL\"", "CMGL", 1500);
  /* sprintf(AT_Command, "%s%c", "AT+CMGL=\"ALL\"", 13);
  uart_write_bytes(UART_NUM_1, AT_Command, strlen (AT_Command));*/
}

static unsigned int is_leapyear_aux(unsigned int y) {
  return (y % 4) == 0 && ((y % 100) != 0 || (y % 400) == 0);
}

time_t epoch_Calculator(struct tm *tm) {
  static const unsigned int ndays[2][12] = {
      {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
      {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}};
  time_t r = 0;
  int i;
  unsigned int *nday =
      (unsigned int *)ndays[is_leapyear_aux(tm->tm_year + 1900)];

  static const int epoch_year = 70;
  if (tm->tm_year >= epoch_year) {
    for (i = epoch_year; i < tm->tm_year; ++i)
      r += is_leapyear_aux(i + 1900) ? 366 * 24 * 60 * 60 : 365 * 24 * 60 * 60;
  } else {
    for (i = tm->tm_year; i < epoch_year; ++i)
      r -= is_leapyear_aux(i + 1900) ? 366 * 24 * 60 * 60 : 365 * 24 * 60 * 60;
  }
  for (i = 0; i < tm->tm_mon; ++i)
    r += nday[i] * 24 * 60 * 60;
  r += (tm->tm_mday - 1) * 24 * 60 * 60;
  r += tm->tm_hour * 60 * 60;
  r += tm->tm_min * 60;
  r += tm->tm_sec;
  return r;
}

uint8_t parse_NowTime(char *payload) {
  // printf("\n parse_NowTime\n");

  // ESP_LOGI("TAG", "xPortGetFreeHeapSize            : %d",
  // xPortGetFreeHeapSize()); ESP_LOGI("TAG", "esp_get_minimum_free_heap_size  :
  // %d", esp_get_minimum_free_heap_size()); ESP_LOGI("TAG",
  // "heap_caps_get_largest_free_block: %d",
  // heap_caps_get_largest_free_block(MALLOC_CAP_DEFAULT)); ESP_LOGI("TAG",
  // "free heap memory                : %d",
  // heap_caps_get_free_size(MALLOC_CAP_8BIT));
  int auxCount = 0;
  int strIndex = 0;
  char aux_year[5] = {};
  char aux_month[3] = {};
  char aux_day[3] = {};
  char aux_hour[3] = {};
  char aux_minute[3] = {};
  char aux_second[3] = {};
  char timezone_network[5] = {};
  char timeZone_Signal = 0;

  uint8_t slashCounter = 0;
  uint8_t colonCounter = 0;
  uint8_t commaCounter = 0;

  memset(nowTime.strTime, 0, sizeof(nowTime.strTime));
  memset(aux_year, 0, sizeof(aux_year));
  memset(aux_month, 0, sizeof(aux_month));
  memset(aux_day, 0, sizeof(aux_day));
  memset(aux_hour, 0, sizeof(aux_hour));
  memset(aux_minute, 0, sizeof(aux_minute));

  // printf("\ntime payload %s\n", payload);

  for (int i = 0; i < strlen(payload); i++) {
    if (payload[i] == '"') {
      auxCount++;
    } else {
      if (auxCount == 1) {

        if (payload[i] == '+' || payload[i] == '-') {
          strIndex = 0;
          timeZone_Signal = payload[i];
          // timezone[strIndex++] = payload[i];
          auxCount++;
        } else {
          nowTime.strTime[strIndex++] = payload[i];
        }
      } else if (auxCount == 2) {
        if (payload[i] == ',') {
          break;
        } else {
          timezone_network[strIndex++] = payload[i];
        }
      }
    }
  }

  // printf("\n\nTIMEZONE QLTS - %c\n\n", timeZone_Signal);

  for (int i = 0; i < strlen(nowTime.strTime); i++) {
    if (nowTime.strTime[i] == '/') {
      slashCounter++;
    } else if (nowTime.strTime[i] == ':') {
      colonCounter++;
    } else if (nowTime.strTime[i] == ',') {
      commaCounter++;
    }
  }

  auxCount = 0;
  strIndex = 0;
  // printf("\ntime strtime %s\n", nowTime.strTime);

  if (slashCounter == 2 && colonCounter == 2 && commaCounter == 1) {
    // printf("\ntime strtime 11%s\n", nowTime.strTime);

    for (int i = 0; i < strlen(nowTime.strTime); i++) {
      if (nowTime.strTime[i] == '/' || nowTime.strTime[i] == ',' ||
          nowTime.strTime[i] == ':') {
        auxCount++;
        strIndex = 0;
      } else {
        if (auxCount == 0) {
          aux_year[strIndex] = nowTime.strTime[i];
          strIndex++;
        } else if (auxCount == 1) {
          aux_month[strIndex] = nowTime.strTime[i];
          strIndex++;
        } else if (auxCount == 2) {
          aux_day[strIndex] = nowTime.strTime[i];
          strIndex++;
        } else if (auxCount == 3) {
          aux_hour[strIndex] = nowTime.strTime[i];
          strIndex++;
        } else if (auxCount == 4) {
          aux_minute[strIndex] = nowTime.strTime[i];
          strIndex++;
        } else if (auxCount == 5) {
          aux_second[strIndex] = nowTime.strTime[i];
          strIndex++;
        }
      }
    }

    // printf("\nyear year %s\n", aux_year);
    nowTime.date = ((atoi(aux_year) - 2000) * 10000) + (atoi(aux_month) * 100) +
                   atoi(aux_day);
    nowTime.time = (atoi(aux_hour) * 100) + atoi(aux_minute);
    nowTime.year = (atoi(aux_year) - 2000);
    // printf("\nyear year111 %d\n", nowTime.year);
    nowTime.month = atoi(aux_month);
    nowTime.day = atoi(aux_day);
    int weekDay_now =
        calculate_weekDay(nowTime.year, nowTime.month, nowTime.day);

    nowTime.weekDay = pow(2, weekDay_now);
    // printf("\nyear year111 %s\n", aux_year);
    /*  uint8_t lastMonth = nowTime.month;
     save_NVS_Last_Month(lastMonth); */

    // struct timeval epoch = {atoi(str_Clock), 0};
    struct tm timeinfo;

    int int_epoch = 0; // mktime(&timeinfo);

    // printf("\n\n\n int epoch %d\n\n\n", int_epoch);
    char timeZone[50] = {};

    esp_err_t err = 0;
    size_t required_size;

    // printf("\nENTER get_Data_STR_LastCALL_From_Storage\n");
    //  printf("\nerror 1 %d\n", err);

    // printf("\n\n\n getenv - %s",getenv("TZ"));
    //  settimeofday(&epoch, NULL);

    timeinfo.tm_year = atoi(aux_year) - 1900;
    timeinfo.tm_mon = atoi(aux_month) - 1;
    timeinfo.tm_mday = atoi(aux_day);
    timeinfo.tm_hour = atoi(aux_hour);
    timeinfo.tm_min = atoi(aux_minute);
    timeinfo.tm_sec = atoi(aux_second);
    timeinfo.tm_wday = weekDay_now;

    time_t now;
    struct tm timeinfo_Now;

    /* localtime_r(&now, &timeinfo_Now);*/

    // printf("\n\n year - %d, month - %d, day - %d, dayweek - %d, hour - %d,
    // minute - %d, sec - %d\n\n", (timeinfo.tm_year + 1900), timeinfo.tm_mon,
    // timeinfo.tm_mday, timeinfo.tm_wday, timeinfo.tm_hour, timeinfo.tm_min,
    // timeinfo.tm_sec);

    // int_epoch = mktime(&timeinfo);

    /* memset(timezone_network, 0, 5);
    sprintf(timezone_network, "%s", "32");
    timeZone_Signal = '-'; */
    // printf("\n\n timezone network %d\n\n", atoi(timezone_network));

    if (nvs_get_str(nvs_System_handle, NVS_TIMEZONE, NULL, &required_size) ==
        ESP_OK) {
      // printf("\nrequire size %d\n", required_size);

      if (nvs_get_str(nvs_System_handle, NVS_TIMEZONE, timeZone,
                      &required_size) == ESP_OK) {
        // printf("\n\n enter qlts memory\n\n");
        if (strcmp(timeZone, "GMT0")) {
          PCF85_SetTime(timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
          PCF85_SetDate(timeinfo.tm_wday, timeinfo.tm_mday,
                        (timeinfo.tm_mon + 1), (timeinfo.tm_year + 1900));

          // asprintf(timeinfo.tm_zone,"%s",timeZone);
          int_epoch = epoch_Calculator(&timeinfo);
          // int_epoch = mktime(&timeinfo);
          // printf("\n\n epoch11 - %d\n\n", int_epoch);
        } else {
          if (timeZone_Signal == '+') {
            int_epoch =
                epoch_Calculator(&timeinfo) + atoi(timezone_network) * 15 * 60;
            // printf("\n\n int_epoch plus 55\n\n");
          } else if (timeZone_Signal == '-') {
            int_epoch =
                epoch_Calculator(&timeinfo) - atoi(timezone_network) * 15 * 60;
            // printf("\n\n int_epoch minus 66\n\n");
          }

          // printf("\n\n int_epoch %d\n\n", int_epoch);

          memset(timeZone, 0, 50);

          gmtime_r(&int_epoch, &timeinfo_Now);
          PCF85_SetTime(timeinfo_Now.tm_hour, timeinfo_Now.tm_min,
                        timeinfo_Now.tm_sec);
          PCF85_SetDate(timeinfo_Now.tm_wday, timeinfo_Now.tm_mday,
                        (timeinfo_Now.tm_mon + 1),
                        (timeinfo_Now.tm_year + 1900));

          sprintf(timeZone, "%s", "GMT0");
        }

        // int_epoch = mktime(&timeinfo);
      } else {

        if (timeZone_Signal == '+') {
          int_epoch =
              epoch_Calculator(&timeinfo) + atoi(timezone_network) * 15 * 60;
          // printf("\n\n int_epoch plus\n\n");
        } else if (timeZone_Signal == '-') {
          int_epoch =
              epoch_Calculator(&timeinfo) - atoi(timezone_network) * 15 * 60;
          // printf("\n\n int_epoch minus\n\n");
        }

        // printf("\n\n int_epoch %d\n\n", int_epoch);

        memset(timeZone, 0, 50);

        gmtime_r(&int_epoch, &timeinfo_Now);
        PCF85_SetTime(timeinfo_Now.tm_hour, timeinfo_Now.tm_min,
                      timeinfo_Now.tm_sec);
        PCF85_SetDate(timeinfo_Now.tm_wday, timeinfo_Now.tm_mday,
                      (timeinfo_Now.tm_mon + 1), (timeinfo_Now.tm_year + 1900));

        sprintf(timeZone, "%s", "GMT0");
      }
    } else {
      if (timeZone_Signal == '+') {

        int_epoch =
            epoch_Calculator(&timeinfo) + atoi(timezone_network) * 15 * 60;

        // printf("\n\n plus clock epoch %d, %d\n\n", int_epoch,
        // atoi(timezone_network));
      } else if (timeZone_Signal == '-') {

        int_epoch =
            epoch_Calculator(&timeinfo) - atoi(timezone_network) * 15 * 60;
      }

      memset(timeZone, 0, 50);

      gmtime_r(&int_epoch, &timeinfo_Now);
      PCF85_SetTime(timeinfo_Now.tm_hour, timeinfo_Now.tm_min,
                    timeinfo_Now.tm_sec);
      PCF85_SetDate(timeinfo_Now.tm_wday, timeinfo_Now.tm_mday,
                    (timeinfo_Now.tm_mon + 1), (timeinfo_Now.tm_year + 1900));

      sprintf(timeZone, "%s", "GMT0");
    }

    struct timeval epoch = {.tv_sec = int_epoch};

    //     printf("\n\n hour hour 1221\n year - %d, month - %d, day - %d,
    //     dayweek - %d, hour - %d, minute - %d, sec - %d\n\n",
    //     (timeinfo_Now.tm_year + 1900), timeinfo_Now.tm_mon,
    //     timeinfo_Now.tm_mday, timeinfo_Now.tm_wday, timeinfo_Now.tm_hour,
    //     timeinfo_Now.tm_min, timeinfo_Now.tm_sec);

    if (!settimeofday(&epoch, NULL)) {

      // printf("\n\n epoch33 - %d\n\n", int_epoch);
      time(&now);

      // printf("\n\n timezone qlts %s", timeZone);
      if (!setenv("TZ", timeZone, 1)) {

        // printf("\n\n\n getenv22 - %s", getenv("TZ"));
        save_STR_Data_In_Storage(NVS_TIMEZONE, timeZone, nvs_System_handle);
        tzset();

        localtime_r(&now, &timeinfo_Now);

        // printf("\n\n hour hour 1221\n year - %d, month - %d, day - %d,
        // dayweek - %d, hour - %d, minute - %d, sec - %d\n\n",
        // (timeinfo_Now.tm_year + 1900), timeinfo_Now.tm_mon,
        // timeinfo_Now.tm_mday, timeinfo_Now.tm_wday, timeinfo_Now.tm_hour,
        // timeinfo_Now.tm_min, timeinfo_Now.tm_sec);
      } else {
        // enableAlarm();
        return 0;
      }
    } else {
      // enableAlarm();
      return 0;
    }

    label_To_Send_Periodic_SMS = get_INT8_Data_From_Storage(
        NVS_KEY_LABEL_PERIODIC_SMS, nvs_System_handle);

    if (label_To_Send_Periodic_SMS == 255) {
      label_To_Send_Periodic_SMS = 0;
    }

    // label_To_Send_Periodic_SMS = 0;
    if (label_To_Send_Periodic_SMS == 0) {

      label_To_Send_Periodic_SMS = 1;
      save_INT8_Data_In_Storage(NVS_KEY_LABEL_PERIODIC_SMS,
                                label_To_Send_Periodic_SMS, nvs_System_handle);

      date_To_Send_Periodic_SMS = parseDatetoInt(nowTime.date, 60);
      nvs_set_u32(nvs_System_handle, NVS_KEY_DATE_PERIODIC_SMS,
                  date_To_Send_Periodic_SMS);
      // printf("\n\n enter label inicial sms periodic %d\n\n",
      // date_To_Send_Periodic_SMS);
    }
    // getLocalTime(&timeinfo_Now, 0);

    // timeinfo_Now = localtime(&now);
    // localtime_r(&now, &timeinfo_Now);
    char str_Time[256] = {};

    strftime(str_Time, 256, "%c", &timeinfo_Now);

    // printf("\n\n str_Time - %s\n\n", str_Time);

    // printftf("\n\n hour hour 1221\n year - %d, month - %d, day - %d, dayweek
    // - %d, hour - %d, minute - %d, sec - %d\n\n", (timeinfo_Now.tm_year +
    // 1900), timeinfo_Now.tm_mon, timeinfo_Now.tm_mday, timeinfo_Now.tm_wday,
    // timeinfo_Now.tm_hour, timeinfo_Now.tm_min, timeinfo_Now.tm_sec);

    get_RTC_System_Time();

    // printf("\nafter verify_WeekAcess\n");
    // ESP_LOGI("TAG", "xPortGetFreeHeapSize            : %d",
    // xPortGetFreeHeapSize()); ESP_LOGI("TAG", "esp_get_minimum_free_heap_size
    // : %d", esp_get_minimum_free_heap_size()); ESP_LOGI("TAG",
    // "heap_caps_get_largest_free_block: %d",
    // heap_caps_get_largest_free_block(MALLOC_CAP_DEFAULT)); ESP_LOGI("TAG",
    // "free heap memory                : %d",
    // heap_caps_get_free_size(MALLOC_CAP_8BIT)); enableAlarm();
    return 1;
  }

  return 0;
}

char *activateUDP_network() {
  char rsp[50] = {};
  uint8_t InitNetworkCount = 0;
  /* label_network_portalRegister = 1;
  save_INT8_Data_In_Storage(NVS_NETWORK_PORTAL_REGISTER,
  label_network_portalRegister, nvs_System_handle); */
  timer_pause(TIMER_GROUP_1, TIMER_0);

  if (gpio_get_level(GPIO_INPUT_IO_SIMPRE)) {

    if (gpio_get_level(GPIO_INPUT_IO_EG91_STATUS)) {
      RSSI_LED_TOOGLE = RSSI_NOT_DETECT;

      gpio_set_level(GPIO_OUTPUT_ACT, 1);

      while (InitNetworkCount < 3) {

        // printf("INIT EG915 311");
        if (!EG91_initNetwork()) {
          RSSI_LED_TOOGLE = RSSI_NOT_DETECT;

          gpio_set_level(GPIO_OUTPUT_ACT, 1);
          update_ACT_TimerVAlue((double)RSSI_NOT_DETECT);
          InitNetworkCount++;
        } else {
          // printf("\n\n activate network 1\n\n");
          break;
        }
      }

      if (InitNetworkCount == 3) {

        save_INT8_Data_In_Storage(NVS_QMTSTAT_LABEL, 1, nvs_System_handle);
        RSSI_LED_TOOGLE = MQTT_NOT_CONECT_LED_TIME;
        timer_start(TIMER_GROUP_1, TIMER_0);
        return "ERROR";
      }
    } else {
      gpio_set_level(GPIO_OUTPUT_ACT, 1);
      uint8_t pwrFD = 0;
      uint8_t pwrFD_count = 0;

      while (pwrFD != 1) {
        if (pwrFD_count == 3 || pwrFD == 1) {
          break;
        }
        pwrFD_count++;

        pwrFD = EG91_PowerOn();
      }

      if (pwrFD == 1) {

        while (InitNetworkCount < 3) {
          // printf("INIT EG915 311");
          if (!EG91_initNetwork()) {
            RSSI_LED_TOOGLE = RSSI_NOT_DETECT;

            gpio_set_level(GPIO_OUTPUT_ACT, 1);
            update_ACT_TimerVAlue((double)RSSI_NOT_DETECT);
            InitNetworkCount++;
          } else {
            // printf("\n\n activate network 1\n\n");
            break;
          }
        }

        if (InitNetworkCount == 3) {
          /* label_network_portalRegister = 0;
          save_INT8_Data_In_Storage(NVS_NETWORK_PORTAL_REGISTER,
          label_network_portalRegister, nvs_System_handle); */
          timer_start(TIMER_GROUP_1, TIMER_0);
          return "ERROR";
        }
      }
    }
  }

  // printf("\n\n activate network 22\n\n");
  //  sprintf(rsp, "ME S W %s", "OK");
  timer_start(TIMER_GROUP_1, TIMER_0);
  return "OK";
}

uint8_t desactivateUDP_network() {
  char rsp[50];
  label_network_portalRegister = 0;
  EG91_send_AT_Command("AT+QMTCONN?", "AT+QMTCONN?", 1000);

  if (mqtt_connectLabel) {
    if (EG91_send_AT_Command("AT+QMTDISC=0", "OK", 30000)) {
      save_INT8_Data_In_Storage(NVS_NETWORK_PORTAL_REGISTER,
                                label_network_portalRegister,
                                nvs_System_handle);
      return 1;
    }
  } else {
    save_INT8_Data_In_Storage(NVS_NETWORK_PORTAL_REGISTER,
                              label_network_portalRegister, nvs_System_handle);
    return 1;
  }

  return 0;
}

uint8_t getFile_importUsers(char *DNS) {
  /* char domain[300] = {};
  sprintf(domain,"AT+QHTTPURL=%d,80",strlen("https://static.portal.home.motorline.pt/docs/teste.txt"));
  char ct[2] = {};
          ct[0] = 13;
          ct[1] = 0;

   EG91_send_AT_Command("AT+QHTTPCFG=\"contextid\",1","OK", 1000);
   EG91_send_AT_Command("AT+QHTTPCFG=\"responseheader\",1","OK", 1000);
   EG91_send_AT_Command("AT+QHTTPCFG=\"sslctxid\",1","OK", 1000);
   EG91_send_AT_Command("AT+QSSLCFG=\"sslversion\",1,1","OK", 1000);
   EG91_send_AT_Command("AT+QSSLCFG=\"ciphersuite\",1,0x0005","OK", 1000);
   EG91_send_AT_Command("AT+QSSLCFG=\"seclevel\",1,0","OK", 1000);
   strcat(domain,ct);
   uart_write_bytes(UART_NUM_1, domain, strlen(domain));
   vTaskDelay( pdMS_TO_TICKS(1000));
   uart_write_bytes(UART_NUM_1,
  "https://static.portal.home.motorline.pt/docs/teste.txt", 85); vTaskDelay(
  pdMS_TO_TICKS(1000)); EG91_send_AT_Command("AT+QHTTPREAD=80","OK", 1000);  */
  /*EG91_send_AT_Command("https://www.google.com/webhp?hl=pt-PT&sa=X&ved=0ahUKEwj2wuSZs9qEAxWpX0EAHSWmCzkQPAgJ","OK",
   * 1000);  */

  return 1;
}

uint8_t has_letters(const char *str) {

  while (*str) {
    if (isalpha(*str)) {
      return 1; // A string contém pelo menos uma letra
    }
    str++;
  }
  return 0; // A string não contém letras
}

uint8_t parseQMTPUBEX(char *receiveData) {
  // printf("\n\n PUBX12345 string %d\n\n", strlen(receiveData));
  //  uint8_t index_Found = strpos(receiveData, "+QMTPUBEX:");
  //  printf("\n\n PUBX12345 indexfound - %d\n\n",index_Found);
  uint8_t counter = 0;
  char result = 0;
  // printf("\n\npubx receive data\n");
  for (size_t i = 0; i < strlen(receiveData); i++) {
    // printf("\n\nPUBXID - %d\n\n", i);
    // printf("%c", receiveData[i]);
    if (receiveData[i] == ',') {
      counter++;
    }

    if (counter == 2) {
      result = receiveData[i + 1];
      break;
    }
  }

  // printf("\n\nPUBX54321\n\n");free heap memory
  // printf("\n\n finish pubx receive data - %c\n", result);
  /* ESP_LOGI("TAG", "xPortGetFreeHeapSize            : %d",
  xPortGetFreeHeapSize()); ESP_LOGI("TAG", "esp_get_minimum_free_heap_size  :
  %d", esp_get_minimum_free_heap_size()); ESP_LOGI("TAG",
  "heap_caps_get_largest_free_block: %d",
  heap_caps_get_largest_free_block(MALLOC_CAP_DEFAULT)); ESP_LOGI("TAG", "free
  heap memory                : %d", heap_caps_get_free_size(MALLOC_CAP_8BIT));
*/
  if (result == '0' || result == '1') {
    return 1;
  } else {
    return 0;
  }
}

uint8_t EG91_parseGet_imsi(char *data) {
  /* char imsi[30] = {};
  uint8_t strIndex = 0;

  for (size_t i = 0; i < strlen(data); i++)
  {
          if (isdigit(data[i]))
          {
                  imsi[strIndex++] = data[i];
          }
  }

  //printf("\n\n imsi - %s\n\n", imsi);
  if (strIndex == 14 || strIndex == 15)
  {
          save_STR_Data_In_Storage(NVS_EG91_ICCID_VALUE, imsi,
  nvs_System_handle); return 1;
  } */

  return 0;
}

uint8_t parse_https_get(char *payload) {
  char dtmp1[50] = {};

  // printf("\n\n parse http get222\n\n");
  /* if (strstr(payload,"OK") != NULL)
  { */
  xQueueReceive(HTTPS_data_queue, &dtmp1, pdMS_TO_TICKS(10000));
  // printf("\n\n parse http get\n\n");
  if (strstr(dtmp1, "+QHTTPGET:") != NULL) {
    char *token;

    // Ignora o prefixo "+QHTTPGET: "
    token = strtok(dtmp1, ": ");
    token = strtok(NULL, ","); // status
    int x = atoi(token);

    token = strtok(NULL, ","); // length
    EG915_readDataFile_struct.status = atoi(token);

    token = strtok(NULL, ","); // timestamp
    EG915_readDataFile_struct.fileSize = atoi(token);

    // printf("\n\n get https %d - %d - %d\n\n", x,
    // EG915_readDataFile_struct.status, EG915_readDataFile_struct.fileSize);

    return 1;
  } else {
    // printf("\n\n parse http get22\n\n");
  }

  return 0;
}

uint8_t EG91_send_AT_Command(char *data, char *rsp, int time) {

  char AT_Command[300] = {};
  uint8_t counterACK = 5;
  char dtmp1[BUF_SIZE] = {};
  char phone[250] = {};
  char text[1024] = {};

  // printf("\nAT COMMAND 000  %s\n", data);

  // printf("\nAT COMMAND = %s\n", data);

  // printf("\nAT COMMAND\n");

  xSemaphoreTake(rdySem_Control_Send_AT_Command, pdMS_TO_TICKS(15000));
  // printf("\nAFTER TAKE SEND AT COMAND\n");
  /* printf("\nAFTER TAKE SEND AT COMAND11\n");
  //printf("\nAFTER TAKE SEND AT COMAND22\n"); */
  // printf("\n\n at sender1 - %s\n\n", data);
  send_ATCommand_Label = 1;
  // system_stack_high_water_mark("atsend1");
  //  printf("\nAFTER TAKE SEND AT COMAND111 %d\n", strlen(data));
  if (!strcmp(rsp, "QMTPUBEX")) {

    char UDP_send_command[1024] = {};
    char ct[2] = {};
    ct[0] = 13;
    ct[1] = 0;

    char mqtt_Data[1024] = {};
    char topic_mqtt[70] = {};
    uint8_t count = 0;
    int strIndex = 0;
    // printf("\nAT COMMAND 445655\n");

    // printf("\nAFTER TAKE SEND AT COMAND222 %d\n", (data != NULL) ?
    // strlen(data) : 0); printf("\nAFTER TAKE SEND AT COMAND222 %d\n",
    // strlen(data));
    for (size_t i = 0; i < strlen(data); i++) {
      if (data[i] == '$') {
        count++;
        strIndex = 0;
      } else if (count == 0) {
        mqtt_Data[strIndex++] = data[i];
      } else if (count == 1) {
        topic_mqtt[strIndex++] = data[i];
      }
    }
    // printf("\nAFTER TAKE SEND AT COMAND333\n");
    //  printf("\n\n at sender2 - %s - %s\n\n", mqtt_Data, topic_mqtt);
    /* if (strlen(topic_mqtt) > 0)
    { */
    // printf("\nAFTER TAKE SEND AT COMAND444\n");
    sprintf(UDP_send_command, "%s%s%s%d", "AT+QMTPUBEX=0,0,1,0,\"m200",
            topic_mqtt, "\",", strlen(mqtt_Data));

    // printf("\nAFTER TAKE SEND AT COMAND5555\n");
    /*  }
     else
     {
             sprintf(UDP_send_command, "%s%d", "AT+QMTPUBEX=0,0,1,0,\"m200/\",",
     strlen(mqtt_Data));
     } */

    // printf("\n\n PUBX0101010 - %s\n\n", UDP_send_command);
    //  xSemaphoreGive(rdySem_Control_Send_AT_Command);

    /*  */
    strcat(UDP_send_command, ct);

    uart_write_bytes(UART_NUM_1, UDP_send_command, strlen(UDP_send_command));
    vTaskDelay(pdMS_TO_TICKS(50));
    // system_stack_high_water_mark("SEND UDP3");
    // printf("\n\n at sender3 - %s\n\n", mqtt_Data);
    uart_write_bytes(UART_NUM_1, mqtt_Data, strlen(mqtt_Data));
    // sprintf(AT_Command, "%s%c", "AT", 13);
    // uart_write_bytes(UART_NUM_1,AT_Command , strlen(AT_Command));
    //  vTaskDelay(pdMS_TO_TICKS(500));
    // printf("\n\n PUBX12345 - %s\n\n", data);
    xQueueReceive(AT_Command_Feedback_queue, &dtmp1, pdMS_TO_TICKS(time));
    // printf("\n\n at sender4 - %s\n\n", data);

    // printf("\n\n PUBX0000 - %s\n\n", dtmp1);
    // printf("\n\n PUBX12345\n\n");
    //  printf("\n\n PUBX: %s \n\n", dtmp1);
    send_ATCommand_Label = 0;
    // uint8_t ack_pubex =
    if (parseQMTPUBEX(dtmp1)) {
      // xSemaphoreGive(rdySem_Control_pubx);
      xSemaphoreGive(rdySem_Control_Send_AT_Command);
      return 1;
    } else {
      // xSemaphoreGive(rdySem_Control_pubx);
      xSemaphoreGive(rdySem_Control_Send_AT_Command);
      return 0;
    }

    return 1;
  }

  if (!strcmp(rsp, "CMGS:")) {
    // printf("\n SMS SEND START1233 - %s\n", data);
    int strIndex = 0;
    int count = 0;

    for (int i = 0; i < strlen(data); i++) {

      if (data[i] == '$') {
        count++;
        strIndex = 0;
      } else {
        if (count == 0) {
          text[strIndex++] = data[i];
        } else if (count == 1) {
          // sprintf((char*)phone + strIndex, "%02X", data[i]);
          phone[strIndex++] = data[i];
          // strIndex++;
        }
        /* else
        {
                return 0;
        } */
      }
    }
    char ct[2] = {};
    ct[0] = 26;
    ct[1] = 0;
    strcat(text, ct);
    // printf("\n rsp rsp11 - %s - %s\n\n", phone, text);
  }

  // printf("\n SMS SEND START2112 -\n");
  // system_stack_high_water_mark("atsend11");
  while (counterACK > 0) {
    memset(dtmp1, 0, sizeof(dtmp1));
    // printf("\n SMS SEND START12 -\n");
    if (!strcmp(rsp, "CMGS:")) {

      // printf("\n SMS - %s\n - %s\n", phone, text);
      //   memset(rsp,0,sizeof(rsp));
      //   strcpy(rsp,"OK");
      //   uart_write_bytes(UART_NUM_1, "AT0", strlen(phone));

      uart_write_bytes(UART_NUM_1, phone, strlen(phone));
      xQueueReceive(AT_Command_Feedback_queue, &dtmp1, pdMS_TO_TICKS(1500));

      // printf("\ndtmp1 11 %s\n", dtmp1);
      //   xQueueReceive(AT_Command_Feedback_queue, &dtmp1, pdMS_TO_TICKS(time *
      //   5));
      memset(dtmp1, 0, sizeof(dtmp1));
      uart_write_bytes(UART_NUM_1, text, sizeof(text) /* strlen(text) */);

      xQueueReceive(AT_Command_Feedback_queue, &dtmp1, pdMS_TO_TICKS(3000));
      // printf("\ndtmp1 11 %s\n", dtmp1);
      //   xQueueReceive(AT_Command_Feedback_queue, &dtmp1, pdMS_TO_TICKS(time *
      //   5));
      memset(dtmp1, 0, sizeof(dtmp1));

      /* uart_wait_tx_done(UART_NUM_1, pdMS_TO_TICKS(1000));
      uart_write_bytes_with_break(UART_NUM_1, text, strlen(text),
      pdMS_TO_TICKS(1000)); */
      // xQueueReceive(AT_Command_Feedback_queue, &dtmp1, pdMS_TO_TICKS(time *
      // 2));

      // vTaskDelay(pdMS_TO_TICKS((500)));
      // printf("\n SMS 1 - %s \n - %s\n", phone, text);
      xQueueReceive(AT_Command_Feedback_queue, &dtmp1, pdMS_TO_TICKS(3000));
      // printf("\ndtmp1 22 %s\n", dtmp1);
    } else {
      sprintf(AT_Command, "%s%c", data, 13);
      // printf("\nSEND AT COMMAND 11 = %s\n", AT_Command);

      /* uart_wait_tx_done(UART_NUM_1, pdMS_TO_TICKS(1000));
      uart_write_bytes_with_break(UART_NUM_1, AT_Command, strlen(AT_Command),
      pdMS_TO_TICKS(1000)); */
      // system_stack_high_water_mark("atsend12");
      uart_write_bytes(UART_NUM_1, AT_Command, strlen(AT_Command));

      /* if (!strcmp("AT+CNUM",data))
      {
              //printf("\n parse data %d\n", 3);
              vTaskDelay(pdMS_TO_TICKS(300));
      } */

      xQueueReceive(AT_Command_Feedback_queue, &dtmp1, pdMS_TO_TICKS(time));

      // printf("\nSEND AT COMMAND 22 = %s\n", dtmp1);
    }
    // printf("\n SMS SEND START123 -\n");
    //(TickType_t)(time / portTICK_PERIOD_MS)
    //  printf("\nAT_Command_Feedback_queue = %s\n", dtmp1);
    // system_stack_high_water_mark("atsend22");
    if (strlen(dtmp1) < 1 || strstr(dtmp1, "CME ERROR") != NULL ||
        !strcmp(rsp, "SMS")) {
      /* uart_event_t uartEvent;
      uartEvent.type = UART_DATA;
      uartEvent.size=200; */

      memset(dtmp1, 0, sizeof(dtmp1));
      // printf("\n parse data error %d\n", 3);
      memset(AT_Command, 0, sizeof(AT_Command));

      xSemaphoreGive(rdySem_Control_Send_AT_Command);
      // printf("\n parse data %d\n", 1);
      send_ATCommand_Label = 0;
      return 0;
    }

    // printf("\n SMS SEND START124 -\n");
    if (EG91_Parse_ReceiveData(dtmp1, rsp)) {
      // free(rsp);
      //  free(&dtmp1);
      // printf("\n parse data %d\n", 1);

      // printf("\n parse data %d\n", 2);
      memset(dtmp1, 0, sizeof(dtmp1));
      // printf("\n parse data %d\n", 3);
      memset(AT_Command, 0, sizeof(AT_Command));

      // free(dtmp1);
      //  xQueueReset(AT_Command_Feedback_queue);
      xSemaphoreGive(rdySem_Control_Send_AT_Command);
      // printf("\n parse data %d\n", 4);
      send_ATCommand_Label = 0;
      return 1;
    } else {
      // xSemaphoreGive(rdySem_Control_Send_AT_Command);

      memset(dtmp1, 0, sizeof(dtmp1));
      counterACK--;
      // printf("\n parse data %d\n", 5);
      if (!strcmp(rsp, "CPIN")) {
        break;
      }
    }

    // xSemaphoreGive(rdySem_Control_Send_AT_Command);
    memset(dtmp1, 0, sizeof(dtmp1));
    vTaskDelay(pdMS_TO_TICKS((1000)));
  }

  // free(data);
  // free(rsp);
  send_ATCommand_Label = 0;

  // printf("\n parse data %d\n", 5);
  //   memset(dtmp1, 0, sizeof(dtmp1));
  xSemaphoreGive(rdySem_Control_Send_AT_Command);
  return 0;
}

uint8_t EG91_parse_CNUM(char *receiveData) {
  uint8_t strIndex = 0;
  uint8_t count = 0;
  char *ownNumber = (char *)malloc(strlen(receiveData) * sizeof(char));

  memset(ownNumber, 0, strlen(receiveData) * sizeof(char));

  for (size_t i = 0; i < strlen(receiveData); i++) {
    if (receiveData[i] == '"') {
      count++;
      strIndex = 0;
    } else {
      if (count == 3) {
        ownNumber[strIndex++] = receiveData[i];
      }
    }
  }

  save_STR_Data_In_Storage(NVS_KEY_OWN_NUMBER, ownNumber, nvs_System_handle);
  free(ownNumber);
  // printf("\n\n own number %s\n\n", ownNumber);

  return 1;
}

uint8_t parse_qmtsub(char *receiveData) {
  uint8_t counter = 0;
  char result = 0;

  for (size_t i = 0; i < strlen(receiveData); i++) {
    if (receiveData[i] == ',') {
      counter++;
      if (counter == 2) {
        result = receiveData[i + 1];
        break;
      }
    }
  }

  // printf("\n\n subResult %c\n\n", result);

  if (result == '0') {
    return 1;
  } else {
    return 0;
  }

  return 0;
}

uint8_t init_UDP_socket() /* 83.240.211.213 */
{

  char imei[50] = {};
  char connectData[300] = {};
  uint8_t ack = 0;
  // printf("\n\ntask_refresh_SystemTime 02\n\n");
  EG91_get_IMEI();

  get_STR_Data_In_Storage(NVS_KEY_EG91_IMEI, nvs_System_handle, &imei);

  size_t plaintext_len = strlen(imei);

  char *encrypImei = encrypt_and_base64_encode(
      (unsigned char *)AES_KEY1, (unsigned char *)imei, plaintext_len);
  char *willMsg;

  asprintf(&willMsg, "%s !", imei);

  char *willMsg_encrypt;
  asprintf(&willMsg_encrypt, "%s",
           encrypt_and_base64_encode((unsigned char *)AES_KEY1,
                                     (unsigned char *)willMsg,
                                     strlen(willMsg)));

  char *willStr;

  asprintf(&willStr, "AT+QMTCFG=\"will\",0,1,1,0,\"m200/\",\"%s\"",
           willMsg_encrypt);

  EG91_send_AT_Command(willStr, "OK", 1000);

  free(willMsg_encrypt);
  free(willMsg);
  free(willStr);

  EG91_send_AT_Command("AT+QMTCFG=\"keepalive\",0,1800", "OK", 3000);
  EG91_send_AT_Command("AT+QMTCFG=\"recv/mode\",0,0,1", "OK", 3000);

  // EG91_send_AT_Command(" AT+QMTCFG=\"will\",0,1,1,0,m200/,!", "OK", 1000);

  EG91_send_AT_Command("AT+QMTOPEN?", "AT+QMTOPEN?", 5000);
  // printf("\n\n OPEN VERIFY24352! - %d \n\n", mqtt_openLabel);

  if (!mqtt_openLabel) {
    // printf("\n\n OPEN VERIFY555! \n\n");

#ifndef CONFIG_SECURE_FLASH_ENCRYPTION_MODE_DEVELOPMENT
    if (EG91_send_AT_Command("AT+QMTOPEN=0,\"api.portal.motorline.pt\",8883",
                             "OK", 10000)) {

#else

    /* if (EG91_send_AT_Command("AT+QMTOPEN=0,\"test.mosquitto.org\",8883",
    "OK", 10000))
    { */
    if (EG91_send_AT_Command(
            "AT+QMTOPEN=0,\"api.portal.home.motorline.pt\",8883", "OK",
            10000)) {

#endif // Seleção de DNS

      uint8_t ACK_connect = 0;
      // printf("\n\n OPEN VERIFY! \n\n");
      while (ACK_connect < 10) {
        EG91_send_AT_Command("AT+QMTOPEN?", "AT+QMTOPEN?", 2000);
        if (mqtt_openLabel) {
          ack++;
          break;
        }
        ACK_connect++;
        vTaskDelay(pdMS_TO_TICKS(1000));
      }

      if (mqtt_openLabel) {
        ACK_connect = 0;
        EG91_send_AT_Command("AT+QMTCONN?", "AT+QMTCONN?", 1000);

        if (!mqtt_connectLabel) {

          sprintf(connectData, "AT+QMTCONN=0,\"%s\",\"%s\",\"%s\"", imei, imei,
                  encrypImei);

          if (EG91_send_AT_Command(connectData, "OK", 20000)) {
            uint8_t ACK_connect = 0;
            // printf("\n\n CONNECT VERIFY! \n\n");
            while (ACK_connect < 10) {
              EG91_send_AT_Command("AT+QMTCONN?", "AT+QMTCONN?", 1000);

              if (mqtt_connectLabel) {
                ack++;
                break;
              }
              ACK_connect++;
              vTaskDelay(pdMS_TO_TICKS(1000));
            }
          } else {
            // TODO: IMPLENTAR MANTER O STAT
            EG91_send_AT_Command("AT+QMTDISC=0", "OK", 30000);
          }
        } else {
          ack++;
        }
      } else {
        // TODO: IMPLENTAR MANTER O STAT
      }
    } else {
      // TODO: IMPLENTAR MANTER O STAT
    }
  } else {
    // printf("\n\n OPEN VERIFy888888! \n\n");
    ack++;

    EG91_send_AT_Command("AT+QMTCONN?", "AT+QMTCONN?", 1000);

    if (!mqtt_connectLabel) {

      sprintf(connectData, "AT+QMTCONN=0,\"%s\",\"%s\",\"%s\"", imei, imei,
              encrypImei);

      if (EG91_send_AT_Command(connectData, "OK", 20000)) {
        uint8_t ACK_connect = 0;
        // printf("\n\n CONNECT VERIFY! \n\n");
        while (ACK_connect < 10) {
          EG91_send_AT_Command("AT+QMTCONN?", "AT+QMTCONN?", 1000);

          if (mqtt_connectLabel) {
            ack++;
            break;
          }
          ACK_connect++;
          vTaskDelay(pdMS_TO_TICKS(1000));
        }
      } else {
        // TODO: IMPLENTAR MANTER O STAT
        EG91_send_AT_Command("AT+QMTDISC=0", "OK", 20000);
      }
    } else {
      ack++;
    }
  }

  char QMTsub_string[100] = {};
  sprintf(QMTsub_string, "AT+QMTSUB=0,1,\"set/m200/%s/+\",1", imei);
  EG91_send_AT_Command(QMTsub_string, "+QMTSUB:", 10000);
  // printf("\n\n finish sub1\n\n");
  //  EG91_send_AT_Command("AT+QICFG=\"send/auto\",2,5,\"Dados automáticos\"",
  //  "OK", 1000);
  // free(QMTsub_string);
  // free(connectData);
  // printf("\n\n finish sub2\n\n");
  free(encrypImei);
  // printf("\n\n finish sub3\n\n");
  return ack;
}

uint8_t parse_qmtconn(char *receiveData) {
  char conn_verify = 0;

  // printf("\n\n parse_qmtconn 111\n\n");

  for (size_t i = 0; i < strlen(receiveData); i++) {
    if (receiveData[i] == ',') {
      conn_verify = receiveData[i + 1];
      break;
    }
  }

  // printf("\n\nconn_verify %c \n\n", conn_verify);

  if (conn_verify == '3') {
    mqtt_connectLabel = 1;
    save_INT8_Data_In_Storage(NVS_QMTSTAT_LABEL, 0, nvs_System_handle);
  } else {
    mqtt_connectLabel = 0;
    save_INT8_Data_In_Storage(NVS_QMTSTAT_LABEL, 1, nvs_System_handle);
  }

  // printf("\n\nconn_verify2 %c \n\n", conn_verify);
  return 1;
}
char output_mqtt_data[300];
uint8_t send_UDP_Package(char *data, int size, char *topic) {
  char UDP_send_command[1024] = {};
  char imei[20] = {}; // NULL;
  size_t required_size;
  // printf("\nsms ghghkk 532523\n");

  if (nvs_get_str(nvs_System_handle, NVS_KEY_EG91_IMEI, NULL, &required_size) ==
      ESP_OK) {
    // printf("\n\n send udp 01 - %d \n\n", required_size);

    if (nvs_get_str(nvs_System_handle, NVS_KEY_EG91_IMEI, imei,
                    &required_size) == ESP_OK) {
      sprintf(UDP_send_command, "%s ", imei);
      // printf("\n\n send udp 2 \n\n");
      strcat(UDP_send_command, data);
      // printf("\n\n send udp 3 - %s\n\n", UDP_send_command);
      //  printf("\nUDP_send_command:454546 %s\n", "base64_str");
    } else {
      EG91_get_IMEI();

      if (nvs_get_str(nvs_System_handle, NVS_KEY_EG91_IMEI, NULL,
                      &required_size) == ESP_OK) {
        // printf("\n\n send udp 01 - %d \n\n", required_size);

        if (nvs_get_str(nvs_System_handle, NVS_KEY_EG91_IMEI, imei,
                        &required_size) == ESP_OK) {
          sprintf(UDP_send_command, "%s ", imei);
          // printf("\n\n send udp 2 \n\n");
          strcat(UDP_send_command, data);
        }
      } else {
        // printf("\n\nERROR 1\n\n");
        sprintf(UDP_send_command, "%s ", "ERROR");
        strcat(UDP_send_command, data);
      }
    }
  } else {
    EG91_get_IMEI();

    if (nvs_get_str(nvs_System_handle, NVS_KEY_EG91_IMEI, NULL,
                    &required_size) == ESP_OK) {
      // printf("\n\n send udp 01 - %d \n\n", required_size);

      if (nvs_get_str(nvs_System_handle, NVS_KEY_EG91_IMEI, imei,
                      &required_size) == ESP_OK) {
        sprintf(UDP_send_command, "%s ", imei);
        // printf("\n\n send udp 2 \n\n");
        strcat(UDP_send_command, data);
        // printf("\n\n send udp 3 \n\n");
      }
    } else {
      // printf("\n\nERROR 1\n\n");
      sprintf(UDP_send_command, "%s ", "ERROR");
      strcat(UDP_send_command, data);
    }
  }
  // system_stack_high_water_mark("SEND UDP4");
  vTaskDelay(10);
  // printf("\n\nUDP_send_commandçç - %s\n\n", UDP_send_command);
  size_t plaintext_len = strlen(UDP_send_command);

  char *base64_str = encrypt_and_base64_encode(
      (unsigned char *)AES_KEY1, (unsigned char *)UDP_send_command,
      plaintext_len);

  // printf("\n\nbase64: %s\n\n", base64_str);

  /*   if (base64_str != NULL)
    {
            // printf("\n\nbase64: %s\n\n", base64_str);
            //   Liberar a memória alocada para a string retornada
    }
    else
    {
            // printf("\n\nErro ao criptografar e codificar em base64.\n\n");
    } */

  // char *output_mqtt_data;

  if (strlen(topic) < 2) {
    sprintf(output_mqtt_data, "%s$/%s%c", base64_str, imei, 0);
  } else {
    sprintf(output_mqtt_data, "%s$%s%c", base64_str, topic, 0);
  }

  // asprintf(&output_mqtt_data, "%s$%s", base64_str, topic);

  // printf("\n\n send udp 66 %s - %d\n\n", output_mqtt_data,
  // strlen(output_mqtt_data));
  char fff[1000] = {}; //
  // vTaskList(fff); vTaskGetRunTimeStats(fff); printf("\n%s\n",fff);
  timer_pause(TIMER_GROUP_1, TIMER_0);
  // printf("\n\n send udp 0101010 \n\n");
  if (EG91_send_AT_Command(output_mqtt_data, "QMTPUBEX", 2000)) {
    // free(output_mqtt_data);
    free(base64_str);
    timer_start(TIMER_GROUP_1, TIMER_0);
    return 1;
  } else {
    // free(output_mqtt_data);
    free(base64_str);
    timer_start(TIMER_GROUP_1, TIMER_0);
    return 0;
  }
}

void register_UDP_Device() {

  char registerData[30] = {};
  char ownerNumber[20] = {};
  char ownerPassword[7] = {};
  size_t required_size = 0;
  char owner_Data[200] = {};
  MyUser user_validateData;

  memset(&user_validateData, 0, sizeof(user_validateData));

  if (nvs_get_str(nvs_System_handle, NVS_KEY_OWNER_INFORMATION, NULL,
                  &required_size) == ESP_OK) {
    // printf("\nrequire size %d\n", required_size);
    //  printf("\nGET OWNER NAMESPACE\n");
    if (nvs_get_str(nvs_System_handle, NVS_KEY_OWNER_INFORMATION, owner_Data,
                    &required_size) == ESP_OK) {

      char *data_register;
      char imsi[16] = {};
      char input_data[100];

      get_STR_Data_In_Storage(NVS_EG91_ICCID_VALUE, nvs_System_handle,
                              &EG91_ICCID_Number);
      sprintf(input_data, "%c %s", '[', EG91_ICCID_Number);
      send_UDP_Send(input_data, "");
      memset(input_data, 0, 100);

      parse_ValidateData_User(owner_Data, &user_validateData);
      sprintf(input_data, "%s %s ME.G.I", user_validateData.phone,
              user_validateData.key);
      // printf("\nrequire size33 %d\n", required_size);
      data_register = parseInputData(&input_data, BLE_INDICATION, NULL, NULL,
                                     NULL, NULL, NULL);
      // printf("\nrequire size44 %d\n", required_size);
      send_UDP_Send(data_register, "");
      // printf("\nrequire size55 %d\n", required_size);
      //  send_UDP_queue(data_register);
      memset(input_data, 0, 100);

      /* sprintf(input_data, "%s %s AL.G.*", user_validateData.phone,
      user_validateData.key); data_register = parseInputData(&input_data,
      UDP_INDICATION, NULL, NULL, NULL, NULL, NULL);
      send_UDP_Send(data_register, "");
      // send_UDP_queue(data_register);
      memset(input_data, 0, 100); */

      sprintf(input_data, "%s %s ME.G.M", user_validateData.phone,
              user_validateData.key);
      data_register = parseInputData(&input_data, UDP_INDICATION, NULL, NULL,
                                     NULL, NULL, NULL);
      send_UDP_Send(data_register, "");
      // printf("\nrequire size44 bbb\n");
      //  send_UDP_queue(data_register);
      memset(input_data, 0, 100);

      // printf("\nrequire size44 aaa\n");

      // uint8_t label_UDP_fail_and_changed =
      // get_INT8_Data_From_Storage(NVS_NETWORK_LOCAL_CHANGED,
      // nvs_System_handle);
      // // printf("\n\nlabel_UDP_fail_and_changed trete %d\n\n",
      // label_UDP_fail_and_changed); if ((label_UDP_fail_and_changed & 1) ||
      // label_UDP_fail_and_changed == -1)
      // {
      // 	// printf("\nrequire size44 vvv\n");
      // 	sprintf(input_data, "%s %s RT.G.T", user_validateData.phone,
      // user_validateData.key); 	data_register = parseInputData(&input_data,
      // UDP_INDICATION, NULL, NULL, NULL, NULL, NULL);
      // 	send_UDP_Send(data_register, "");
      // 	// send_UDP_queue(data_register);
      // 	memset(input_data, 0, 100);
      // 	// printf("\n\nlabel_UDP_fail_and_changed trete 12122 %d\n\n",
      // label_UDP_fail_and_changed);

      // 	send_UDP_Send("RT R R", "");
      // 	send_UDP_Send("RT R D", "");

      // 	sprintf(input_data, "%s %s RT.G.D", user_validateData.phone,
      // user_validateData.key); 	data_register = parseInputData(&input_data,
      // UDP_INDICATION, NULL, NULL, NULL, NULL, NULL);
      // 	send_UDP_Send(data_register, "");
      // 	// send_UDP_queue(data_register);
      // 	memset(input_data, 0, 100);

      // 	sprintf(input_data, "%s %s RT.G.H", user_validateData.phone,
      // user_validateData.key); 	data_register = parseInputData(&input_data,
      // UDP_INDICATION, NULL, NULL, NULL, NULL, NULL);

      // 	send_UDP_Send(data_register, "");
      // 	// send_UDP_queue(data_register);
      // 	memset(input_data, 0, 100);

      // 	sprintf(input_data, "%s %s RT.G.F", user_validateData.phone,
      // user_validateData.key); 	data_register = parseInputData(&input_data,
      // UDP_INDICATION, NULL, NULL, NULL, NULL, NULL);
      // 	send_UDP_Send(data_register, "");
      // 	// send_UDP_queue(data_register);
      // 	memset(input_data, 0, 100);
      // 	// printf("\n\nlabel_UDP_fail_and_changed trete 12122 \n\n");
      // 	//  send_UDP_queue(data_register);
      // 	memset(input_data, 0, 100);

      // 	if (send_udp_routines_funtion())
      // 	{
      // 		label_UDP_fail_and_changed = label_UDP_fail_and_changed
      // & 254; 		save_INT8_Data_In_Storage(NVS_NETWORK_LOCAL_CHANGED,
      // label_UDP_fail_and_changed, nvs_System_handle);
      // 	}
      // 	else
      // 	{
      // 		label_UDP_fail_and_changed |= 1;
      // 		save_INT8_Data_In_Storage(NVS_NETWORK_LOCAL_CHANGED,
      // label_UDP_fail_and_changed, nvs_System_handle);
      // 	}
      // }

      // printf("\n\nlabel_UDP_fail_and_changed tqwe %d\n\n",
      // label_UDP_fail_and_changed);
      //  TODO: DESCOMENTAR
      // if (label_UDP_fail_and_changed & 2 || label_UDP_fail_and_changed == -1)
      // {
      // 	// printf("\nrequire size44 rrrr\n");

      // 	if (!send_UDP_Send("*\0", ""))
      // 	{
      // 		// printf("\n\n\n send udp ççççç \n\n\n");

      // 		return 0;
      // 	}

      // 	if (sendUDP_all_User_funtion())
      // 	{
      // 		label_UDP_fail_and_changed = label_UDP_fail_and_changed
      // & 253; 		save_INT8_Data_In_Storage(NVS_NETWORK_LOCAL_CHANGED,
      // label_UDP_fail_and_changed, nvs_System_handle); 		free(data_register);
      // 	}
      // 	else
      // 	{
      // 		// printf("\nrequire size44 121212\n");
      // 		label_UDP_fail_and_changed |= 2;
      // 		save_INT8_Data_In_Storage(NVS_NETWORK_LOCAL_CHANGED,
      // label_UDP_fail_and_changed, nvs_System_handle); 		free(data_register);
      // 	}
      // }
      // // printf("\nrequire size44 iiii\n");
      // if (label_UDP_fail_and_changed & 4 || label_UDP_fail_and_changed == -1)
      // {
      // 	memset(input_data, 0, 100);
      // 	sprintf(input_data, "ME R K %s;%s", user_validateData.phone,
      // user_validateData.key);
      // 	// printf("\nrequire size44 767676\n");
      // 	if (send_UDP_Send(input_data, ""))
      // 	{
      // 		label_UDP_fail_and_changed = label_UDP_fail_and_changed
      // & 251; 		save_INT8_Data_In_Storage(NVS_NETWORK_LOCAL_CHANGED,
      // label_UDP_fail_and_changed, nvs_System_handle);
      // 	}
      // 	else
      // 	{
      // 		label_UDP_fail_and_changed |= 4;
      // 		save_INT8_Data_In_Storage(NVS_NETWORK_LOCAL_CHANGED,
      // label_UDP_fail_and_changed, nvs_System_handle);
      // 	}
      // }
      // printf("\nrequire size44 65456\n");
    }
  } else {
    send_UDP_Send("ç\0", "");
  }
}

uint8_t reopen_and_connection() {
  if (EG91_send_AT_Command("AT+QMTCLOSE=0", "OK", 1000)) {
    if (init_UDP_socket() == 2) {

      return 1;
    } else {
      return 0;
    }
  } else {
    return 0;
  }

  return 0;
}

uint8_t EG91_initNetwork() {
  uint8_t ACK = 0;

  if (EG91_send_AT_Command("AT", "OK", 1000)) {
    ACK++;
  }

  if (EG91_send_AT_Command("ATV1", "OK", 1000)) {
    ACK++;
  }

  if (EG91_send_AT_Command("ATE1", "OK", 1000)) {
    ACK++;
  }

  if (EG91_send_AT_Command("AT+GMR", "OK", 1000)) {
    ACK++;
  }

  // EG91_send_AT_Command("AT+QCFG=0", "OK", 1000);

  // EG91_get_IMEI("asas");

  if (EG91_Check_IF_Have_PIN()) {

    EG91_send_AT_Command("AT+QCCID", "+QCCID", 1000);
    // EG91_send_AT_Command("AT+CLCK=\"SC\",0,\"1111\"", "OK", 1000);tnf.m2m

    EG91_send_AT_Command(" AT+QCFG=\"nwscanmode\",0", "OK", 1000);

    if (EG91_send_AT_Command("AT+CREG?", "AT+CREG", 1000)) {
      ACK++;
    }

    if (EG91_send_AT_Command("AT+QLTS=1", "QLTS", 3000)) {
      ACK++;
    }

    if (EG91_send_AT_Command("AT+QCFG=\"urc/ri/ring\",\"off\"", "OK", 1000)) {
      ACK++;
    }

    /* if (EG91_send_AT_Command("AT+COPS=3,0", "OK", 5000))
    {
            ACK++;
    } */

    if (EG91_send_AT_Command("AT+COPS?", "OK", 5000)) {
      ACK++;
    }

    if (EG91_send_AT_Command("AT+QURCCFG=\"urcport\",\"uart1\"", "OK", 1000)) {
      ACK++;
    }

    if (EG91_send_AT_Command("AT+QINDCFG=\"ring\", 1", "OK", 1000)) {
      ACK++;
    }

    if (EG91_send_AT_Command("AT+CNMI=2,1,0,0,0", "OK", 1000)) {
      ACK++;
    }

    // EG91_send_AT_Command("AT+QCFG=\"urc/ri/ring\"", "OK", 1000);
    EG91_send_AT_Command("AT+CNUM", "OK", 10000);

    vTaskDelay(pdMS_TO_TICKS(500));
    if (ACK == 11) {
      /* code */
      EG91_send_AT_Command("AT+CGATT?", "OK", 1000);
      EG91_send_AT_Command("AT+CGATT=1", "OK", 1000);

      EG91_send_AT_Command("AT+CMEE=1", "OK", 1000);
      EG91_send_AT_Command("ATX", "OK", 1000);
      EG91_send_AT_Command("ATX4", "OK", 1000);

      EG91_send_AT_Command("ATS7=5", "OK", 1000);

      EG91_send_AT_Command("ATS0?", "OK", 1000);

      /* EG91_send_AT_Command("AT+CGSMS?", "OK", 1000);
      EG91_send_AT_Command("AT+CGSMS=1", "OK", 1000);
      EG91_send_AT_Command("AT+CGSMS?", "OK", 1000); */

      EG91_send_AT_Command("AT+QCFG=\"urc/ri/other\",\"off\"", "OK", 1000);

      // EG91_send_AT_Command("AT+QCFG=\"risignaltype\",\"respective\"", "OK",
      // 1000);
      EG91_send_AT_Command("AT+QCFG=\"urc/ri/ring\"", "OK", 1000);

      // EG91_send_AT_Command("AT+QCFG=\"urc/ri/ring\",\"pulse\",1,1", "OK",
      // 1000);

      /* EG91_send_AT_Command("AT+QCFG=\"urc/ri/smsincoming\"", "OK", 1000);
      EG91_send_AT_Command("AT+QCFG=\"urc/ri/smsincoming\",\"off\"", "OK",
      1000); */

      // EG91_send_AT_Command("ATX0", "OK", 1000);
      //  EG91_send_AT_Command("ATO", "OK", 1000);

      EG91_send_AT_Command("AT+CMGD=1,4", "OK", 1000);
      EG91_send_AT_Command("AT+CPMS?", "OK", 1000);

      EG91_send_AT_Command(
          "AT+CGDCONT= 1,\"IP\",\"motorline.tele2.com\",\"0.0.0.0\",0,0", "OK",
          1000);

      /* EG91_send_AT_Command("AT+QCFG=\"ims\",1", "OK", 1000);
      EG91_send_AT_Command("AT+QCFG=\"ims\"", "OK", 1000); */
      // EG91_send_AT_Command("AT+QCFG=\"ims\",1", "OK", 1000);

      EG91_send_AT_Command("AT+CGACT=1,1", "OK", 1000);
      // EG91_send_AT_Command("AT+QIACT=1", "OK", 1000);
      EG91_send_AT_Command("AT+QIACT?", "OK", 1000);
      EG91_send_AT_Command("AT+CGCONTRDP=1", "OK", 1000);
      /* for (size_t i = 0; i < 15; i++)
      {
              //printf("%X\n", registerDevice[i]);
              // returnData[i]
      } */

      // printf("\n\nIMEI\n%s\n", auxIMEI);

      if (EG91_send_AT_Command("AT+CSCS=\"UCS2\"", "OK", 1000)) {
        ACK++;
      }

      if (EG91_send_AT_Command("AT+CREG?", "AT+CREG", 1000)) {
        ACK++;
      }

      if (EG91_send_AT_Command("AT+CGREG?", "AT+CGREG", 1000)) {
        ACK++;
      }

      // EG91_send_AT_Command("AT+QMBNCFG=\"List\"", "OK", 1000);
      EG91_send_AT_Command("AT+CGDCONT?", "OK", 1000);

      /* for (size_t i = 0; i < 10; i++)
      {
              EG91_send_AT_Command("AT+CIREG?", "OK", 1000);
              vTaskDelay(pdMS_TO_TICKS(1000));
      } */

      EG91_send_AT_Command("AT+CSDH=1", "OK", 1000);

      EG91_send_AT_Command("AT+QCFG=\"urc/ri/other\",\"off\"", "OK", 1000);

      EG91_get_IMEI();

      // getFile_importUsers("char *DNS");
      /*  EG91_send_AT_Command("AT+QFOTADL=\"http://static.home.motorline.pt/homes/Update_EG915UE.pack\"",
       * "OK", 100000);  */

      // uint8_t label_network_portalRegister =
      // get_INT8_Data_From_Storage(NVS_NETWORK_PORTAL_REGISTER,
      // nvs_System_handle);

      // /* if (label_network_portalRegister == 255)
      // { */
      // label_network_portalRegister = 1;
      // save_INT8_Data_In_Storage(NVS_NETWORK_PORTAL_REGISTER,
      // label_network_portalRegister, nvs_System_handle);
      //}

      /* if (label_network_portalRegister == 1)
      { */
      if (init_UDP_socket() == 2) {
        ACK++;
        // printf("\n\n finish sub99\n\n");
        save_INT8_Data_In_Storage(NVS_QMTSTAT_LABEL, 0, nvs_System_handle);
        // printf("\n\n finish sub443\n\n");
        register_UDP_Device();
        // printf("\n\n finish sub77\n\n");
        if (EG91_send_AT_Command(AT_CSQ, "CSQ", 1000)) {
          ACK++;
        }
        // printf("\n\n init_UDP_socket OK \n\n");
      } else {
        mqtt_openLabel = 0;
        mqtt_connectLabel = 0;
        save_INT8_Data_In_Storage(NVS_QMTSTAT_LABEL, 1, nvs_System_handle);
      }
      /* }
      else
      {
              if (EG91_send_AT_Command(AT_CSQ, "CSQ", 1000))
              {
                      ACK++;
              }
      } */
      // printf("\n\n finish sub88\n\n");

      /*for (size_t i = 0; i < 5; i++)
       {
               EG91_UDP_Ping();
       } */

      char registerDevice[30] = {};

      // printf("\n registerDevice\n");

      // sprintf(registerDevice, "%s %d", "081321069139715", RSSI_VALUE);

      // xQueueSendToBack(UDP_Send_queue, (void *)&registerDevice,
      // pdMS_TO_TICKS(1000)); send_UDP_Package(registerDevice, 16);

      // EG91_send_AT_Command("AT&F0", "OK", 1000);

      if (ACK == 16) {
        // printf("\n registerDevice 22\n");
        return 1;
      }
    }
  }

  // timer_start(0, 0);
  return 0;
}

uint8_t disconect_mqtt() {
  uint8_t ACK = 0;

  if (EG91_send_AT_Command("AT+QMTDISC=0", "OK", 30000)) {
    /* code */
  }
  return 0;
}

void give_rdySem_Control_Send_AT_Command() {
  if (rdySem_Control_Send_AT_Command != NULL) {
    // printf("\rdySem_Control_Send_AT_Command != NULL\n");
    xSemaphoreGive(rdySem_Control_Send_AT_Command);
  } else {
    // printf("\rdySem_Control_Send_AT_Command = NULL\n");
  }
}

void give_rdySem_Feedback_Call() {
  if (rdySem_Feedback_Call != NULL) {
    // printf("\nrdySem_Feedback_Call != NULL\n");
    xSemaphoreGive(rdySem_Feedback_Call);
  } else {
    // printf("\nrdySem_Feedback_Call = NULL\n");
  }
}

void receive_NoCarrie_queue() {
  char dtmp1[600];
  xSemaphoreTake(rdySem_QPSND, pdMS_TO_TICKS(15000));
  // xQueueReceive(NO_CARRIER_Call_queue, &dtmp1, pdMS_TO_TICKS(15000));
}

void give_rdySem_Control_SMS_Task() { xSemaphoreGive(rdySem_Control_SMS_Task); }

void giveSem_CtrIncomingCall() { xSemaphoreGive(rdySem_Control_IncomingCALL); }

int u8_wc_toutf8(char *dest, u_int32_t ch) {

  // printf("\n\n ch - %d - %x", ch, ch);
  if (ch < 0x80) {
    dest[0] = (char)ch;
    dest[1] = 0;
    return 1;
  }
  if (ch < 0x800) {
    dest[0] = (ch >> 6) | 0xC0;
    dest[1] = (ch & 0x3F) | 0x80;
    dest[2] = 0;
    return 2;
  }
  if (ch < 0x10000) {
    dest[0] = (ch >> 12) | 0xE0;
    dest[1] = ((ch >> 6) & 0x3F) | 0x80;
    dest[2] = (ch & 0x3F) | 0x80;
    dest[3] = 0;
    return 3;
  }
  if (ch < 0x110000) {
    dest[0] = (ch >> 18) | 0xF0;
    dest[1] = ((ch >> 12) & 0x3F) | 0x80;
    dest[2] = ((ch >> 6) & 0x3F) | 0x80;
    dest[3] = (ch & 0x3F) | 0x80;
    dest[4] = 0;
    return 4;
  }
  return 0;
}

int conv_utf8_to_ucs2(const char *src, size_t len) {
  iconv_t cb = iconv_open("UTF-16", "UTF-8");
  // printf("inRemains outRemains\n");
  if (cb == (iconv_t)(-1))
    return 0;

  uint16_t *outBuff[len + 1];
  char *pout = (char *)outBuff;
  size_t inRemains = len;
  size_t outRemains = len * sizeof(uint16_t);

  // printf("inRemains:%d outRemains:%d\n", (int)inRemains, (int)outRemains);

  size_t cvtlen = iconv(cb, (char **)&src, (size_t *)&inRemains, (char **)&pout,
                        (size_t *)&outRemains);
  if (cvtlen == (size_t)-1) {
    // printf("error:%s, %d\n", strerror(errno), errno);
    goto out;
  }
  *pout = 0;

  // printf("inRemains:%d outRemains:%d cvtlen:%d\n", (int)inRemains,
  // (int)outRemains, (int)cvtlen);

  /*   for (int i = 0; (i < len) && outBuff[i]; i++)
            //printf("0x%04x\n", (int)outBuff[i]); */

out:
  // if(outBuff)
  free(outBuff);
  iconv_close(cb);

  return 0;
}

static const char *TAG = "uart_events";

uint8_t parse_Incoming_UDP_data(char *mqtt_data) {

  unsigned char receive_UDP_data[1024] = {};
  char mqtt_topic[70] = {};
  uint8_t count = 0;
  uint8_t aux_count = 0;
  int strIndex = 0;

  // printf("\n\n sizeof(mqtt_data) -%s -> %d\n\n", mqtt_data,
  // strlen(mqtt_data));
  for (size_t i = 0; i < strlen(mqtt_data); i++) {
    if (mqtt_data[i] == '"') {
      count++;
      strIndex = 0;
      // printf("\n\n incoming data count - %d\n\n", count);
    } else if (count == 1) {
      if (mqtt_data[i] == '/') {
        aux_count++;
      }

      if (aux_count >= 2) {

        mqtt_topic[strIndex++] = mqtt_data[i];
      }
    } else if (count == 3) {
      receive_UDP_data[strIndex++] = mqtt_data[i];
    }
  }
  /* size_t length = 0;
  while (receive_UDP_data[length] != '\0')
  {
          length++;
  } */

  // printf("\n\n receive_UDP_data 1234 - %s -> %s\n\n", mqtt_topic,
  // receive_UDP_data);

  // receive_UDP_data[strlen(receive_UDP_data) - 1] = 0;

  if (strlen(mqtt_data) > 0) {
    /* code */

    char *output_Data;
    unsigned char decrypted[1024] = {};

    char AES_IV[] = "1BzUeGptJ3SnvnIv";

    decrypt_aes_cbc_padding((unsigned char *)receive_UDP_data,
                            strlen((char *)receive_UDP_data), &decrypted,
                            (unsigned char *)AES_KEY1, (unsigned char *)AES_IV);

    remove_padding(decrypted, strlen((char *)decrypted));

    // printf("Dados descriptografados: %s - %d\n", decrypted, strlen((char
    // *)decrypted));
    mqtt_information mqttInfo;
    sprintf(mqttInfo.topic, "%s", mqtt_topic);

    uint8_t stringIndex = 0;
    uint8_t count = 0;
    char input_Payload[200] = {};

    /* if (!strcmp("ME.R.K", decrypted))
    {
            // ////printf("\n\nENTER CHANGE OWNER PASSWORD\n\n");

            for (size_t i = 0; i < strlen(decrypted); i++)
            {
                    if (decrypted[i] == ' ')
                    {
                            count++;
                            stringIndex = 0;
                    }
                    else
                    {

                            if (count == 1)
                            {
                                    memcpy(input_Payload + stringIndex,
    decrypted + i, 1); stringIndex++;
                            }
                    }
            }
            // system_stack_high_water_mark("parse sms 12");
            //  ////printf("\n\n\n sms input payload - %s\n\n", input_Payload);

            xSemaphoreGive(rdySem_Control_Send_AT_Command);
            //reset_Owner_password(input_Payload, &sms_Data);
            xSemaphoreGive(rdySem_Control_SMS_Task);

            return "NTRSP";
    } */

    // printf("\n\n receive_UDP_data 9876 - %s -> %s\n\n", mqtt_topic,
    // decrypted);
    output_Data = parseInputData(decrypted, UDP_INDICATION, NULL, NULL, NULL,
                                 NULL, &mqttInfo);

    // printf("\n\noutput_Data UDP - %s\n\n", output_Data);

    if (strstr("NTRSP", output_Data) == NULL) {
      // printf("\n\noutput_Data topic - %s\n\n", mqtt_topic);
      send_UDP_Package(output_Data, strlen(output_Data), mqtt_topic);
    }
    // }

    // free(decrypted);
    // printf("\n\noutput_Data  - %s - %d - %d\n\n",
    // output_Data,sizeof(output_Data),strlen(output_Data));
    if (/* output_Data != NULL ||  */ strlen(output_Data) > 0) {
      // printf("\n\noutput_Data  - %s - %d\n\n",
      // output_Data,sizeof(output_Data));
      free(output_Data);
    }
  }

  // //sprintf(receive_UDP_AT, "%s%d", "AT+QIRD=", connectionID);

  // printf("\n\n AT+QIRD - %s\n\n", receive_UDP_data);
  //  EG91_send_AT_Command(receive_UDP_AT, "+QIRD", 3000);

  return 1;
}

uint8_t parse_qmtstat(char *stat_receiveData) {
  uint8_t counter = 0;
  char erro_qmtstat = 0;

  for (size_t i = 0; i < strlen(stat_receiveData); i++) {
    if (stat_receiveData[i] == ',') {
      erro_qmtstat = stat_receiveData[i + 1];
      break;
    }
  }
  erro_qmtstat = erro_qmtstat - 48;

  if (erro_qmtstat != 0) {
    RSSI_LED_TOOGLE = MQTT_NOT_CONECT_LED_TIME;
    // update_ACT_TimerVAlue((double)RSSI_EXCELLENT_LED_TIME);
    updateSystemTimer(SYSTEM_TIMER_ALARM_STATE);
  }

  // printf("\n\n erro_qmtstat = %d\n\n", erro_qmtstat);

  return erro_qmtstat;
}

int encontrarSubstring(const char *str, const char *subStr) {
  int i, j;

  for (i = 0; str[i] != '\0'; i++) {
    // Verificar se há uma correspondência no início da substring
    j = 0;
    while (subStr[j] != '\0' && str[i + j] == subStr[j]) {
      j++;
    }

    // Se a substring foi encontrada, retorne a posição inicial
    if (subStr[j] == '\0') {
      return i;
    }
  }

  // Caso contrário, retorne -1 para indicar que a substring não foi encontrada
  return -1;
}

void task_EG91_Receive_UDP(void *pvParameter) {
  char mqtt_receiveData[1024] = {};
  char connectID[2] = {};
  char mqttData[200] = {};
  int count = 0;
  int strIndex = 0;
  uint8_t result = 0;

  for (;;) {
    if (xQueueReceive(receive_mqtt_queue, &mqtt_receiveData, portMAX_DELAY)) {
      /* timer_pause(TIMER_GROUP_1, TIMER_0);
       vTaskSuspend(xHandle_Timer_VerSystem);
      xSemaphoreGive(rdySem_Control_Send_AT_Command); */
      save_INT8_Data_In_Storage(NVS_QMT_LARGE_DATA_TIMER_LABEL, 1,
                                nvs_System_handle);
      memset(connectID, 0, sizeof(connectID));
      memset(mqttData, 0, sizeof(mqttData));

      if (gpio_get_level(GPIO_INPUT_IO_EG91_STATUS)) {
        if (encontrarSubstring(mqtt_receiveData, "+QMTSTAT:") !=
            -1 /* && !strstr("QIURC", mqtt_receiveData) */) {
          // printf("\n\n QMTSTAT:\n\n");
          /* if (label_Reset_Password_OR_System == 2)
          { */
          // printf("\n\n qmtstat -> %d\n\n",
          // get_INT8_Data_From_Storage(NVS_QMTSTAT_LABEL, nvs_System_handle));
          save_INT8_Data_In_Storage(NVS_QMTSTAT_LABEL,
                                    parse_qmtstat(mqtt_receiveData),
                                    nvs_System_handle);
          /* } */
        }
      } else {
      }

      if (encontrarSubstring(mqtt_receiveData, "RECV") != -1) {
        // printf("\n\nreceive UDP 2222 - %s\n\n", mqttData);
        xSemaphoreGive(rdySem_Control_Send_AT_Command);
        parse_Incoming_UDP_data(mqtt_receiveData);
      } else if (encontrarSubstring(mqtt_receiveData, "QIURC") != -1 ||
                 encontrarSubstring(mqtt_receiveData, "closed:") != -1) {
        if (gpio_get_level(GPIO_INPUT_IO_EG91_STATUS)) {
          if (label_Reset_Password_OR_System == 5) {
            activateUDP_network();
          }
        }
      }
      save_INT8_Data_In_Storage(NVS_QMT_LARGE_DATA_TIMER_LABEL, 0,
                                nvs_System_handle);
      count = 0;
      /* vTaskResume(xHandle_Timer_VerSystem);
                              timer_start(TIMER_GROUP_1, TIMER_0); */
    }
  }
}

uint8_t parse_verify_mqtt_conection(char *receiveData) {
  uint8_t auxCounter = 0;
  char result = 0;
  char ret_code = 0;

  for (size_t i = 0; i < strlen(receiveData); i++) {
    if (receiveData[i] == ',') {
      auxCounter++;
    } else {
      if (auxCounter == 1) {
        result = receiveData[i];
      } else if (auxCounter == 2) {
        ret_code = receiveData[i];
        break;
      }
    }
  }

  // printf("\n\n ret_code = %c / result = %c / data - %s\n\n", ret_code,
  // result, receiveData);

  if (ret_code == '0' && result == '0') {
    return 1;
  } else {
    return 0;
  }

  return 0;
}

uint8_t parse_verify_mqtt_open(char *receiveData) {
  char open_verify = 0;
  uint8_t counter = 0;

  for (size_t i = 0; i < strlen(receiveData); i++) {
    if (receiveData[i] == ',') {
      counter++;
      open_verify = receiveData[i - 1];
      break;
    }
  }

  // printf("\n\nopen_verify %c \n\n", open_verify);

  if ((open_verify == '0' || open_verify == '2') && counter != 0) {
    return 1;
  } else {
    return 0;
  }

  return 0;
}

static void uart_event_task(void *pvParameters) {
  uart_event_t event;
  BaseType_t xHigherPriorityTaskWoken;
  xHigherPriorityTaskWoken = false;
  int counter;
  int len;
  size_t buffered_size;
  char dtmp[BUF_SIZE] = {};
  char mqtt_data[508] = {};
  char SMS_Data[50] = {};
  char CLCC_Data[100] = {};
  int ring_buff_len;
  uint8_t data_len = 0;

  for (;;) {
    xQueueReset(uart0_queue);
    //   Waiting for UART event.
    if (xQueueReceive(uart0_queue, (void *)&event, portMAX_DELAY)) {
      // xSemaphoreTake(rdySem_UART_CTR, 500 / portTICK_RATE_MS);
      // printf("\n!!!!!! read rsp bb !!!!!\n");

      bzero(dtmp, 600);
      //  //ESP_LOGI(TAG, "uart[%d] event:", UART_NUM_1);
      switch (event.type) {

      case UART_DATA:

        // vTaskSuspend(&FileBackup_task_handle);

        /* We have not woken a task at the start of the ISR. */

        vTaskDelay(100 / portTICK_PERIOD_MS); // 10 msec sleep
        memset(dtmp, 0, sizeof(dtmp));
        uart_get_buffered_data_len(UART_NUM_1, (size_t *)&ring_buff_len);
        // printf("\nread rsp aa %d\n", ring_buff_len);
        len = uart_read_bytes(UART_NUM_1, dtmp, /* 1024 */ ring_buff_len, 0);

        // xQueueReset(uart0_queue);
        // printf("\nring_buff_len %d - len %d\n", ring_buff_len, len);
        printf("\nBUFFER UART : %s\n", dtmp);
        //  xQueueOverwrite(AT_Command_Feedback_queue,&dtmp);
        // printf("\n%s\n", dtmp);
        if (/* ring_buff_len > */ 1) {

          if (strstr(dtmp, "CLCC") || (strstr(dtmp, "CMTI")) ||
              /* strstr(dtmp, "AT") || */ /* strstr(dtmp, "+CMGS:") */ /* || */
              strstr(dtmp, "CUSD:") || strstr(dtmp, "CME ERROR") ||
              strstr(dtmp, "+QMTRECV:") || send_ATCommand_Label == 1 ||
              strstr(dtmp, "+QHTTPGET:")) {

            if (AT_Command_Feedback_queue != NULL) {
              if (strstr(dtmp, "+QHTTPGET:") != NULL) {
                // printf("\n after LOG UART11 sms FAIL 11 \n");
                if (xQueueSendFromISR(HTTPS_data_queue, (void *)&dtmp,
                                      xHigherPriorityTaskWoken) != pdPASS) {
                  // xSemaphoreGive(rdySem_Control_Send_AT_Command);
                  // printf("\n after LOG UART11 sms FAIL \n");
                }

                //
                if (xHigherPriorityTaskWoken) {
                  /* Actual macro used here is port specific. */
                  portYIELD_FROM_ISR();
                }
              } else if (strstr(dtmp, "AT+QMTOPEN?") != NULL ||
                         strstr(dtmp, "AT+QMTCONN?") != NULL) {

                // printf("\n qmtlog uart 2222 \n");

                if (xQueueSendFromISR(AT_Command_Feedback_queue, (void *)&dtmp,
                                      xHigherPriorityTaskWoken) != pdPASS) {
                  // xSemaphoreGive(rdySem_Control_Send_AT_Command);
                  // printf("\n after LOG UART11 sms FAIL \n");
                }

                // printf("\n after LOG UART11 sms FAIL 11 \n");
                if (xHigherPriorityTaskWoken) {
                  /* Actual macro used here is port specific. */
                  portYIELD_FROM_ISR();
                }
              } else if (/* (strstr(dtmp, "AT+QMTOPEN")) || strstr(dtmp,
                            "AT+QMTCONN") ||  */
                         strstr(dtmp, "AT+QMTPUBEX") ||
                         strstr(dtmp, "AT+QMTSUB")) {
                //printf("\n\n +++QMT+++\n\n");

                if (strstr(dtmp, "+QMTPUBEX:")) {
                  uint8_t UDP_data_count = 0;
                  int index_Found = -1;
                  uint8_t strIndex = 0;
                  // printf("\n UDP_data_count - %d \n", UDP_data_count);
                  index_Found = strpos(dtmp, "+QMTPUBEX:");
                  memset(mqtt_data, 0, sizeof(mqtt_data));
                  // printf("\n\n +QMT5555+++\n\n");
                  for (size_t i = index_Found; i < strlen(dtmp); i++) {

                    if (dtmp[i] == '\r' || dtmp[i] == '\n') {
                      // printf("\n +QMTPUBEX: %d \n", UDP_data_count);
                      UDP_data_count++;
                    }

                    if (UDP_data_count == 3) {
                      break;
                    }

                    mqtt_data[strIndex++] = dtmp[i];
                  }

                  // printf("\n\nUDP DATA qmtcon - %s\n\n", mqtt_data);
                  if (xQueueSendFromISR(AT_Command_Feedback_queue,
                                        (void *)&mqtt_data,
                                        xHigherPriorityTaskWoken) != pdPASS) {
                    // xSemaphoreGive(rdySem_Control_Send_AT_Command);
                    // printf("\n after LOG UART11 sms FAIL \n");
                  }

                  // printf("\n after LOG UART11 sms FAIL 11 \n");
                  if (xHigherPriorityTaskWoken) {
                    /* Actual macro used here is port specific. */
                    portYIELD_FROM_ISR();
                  }
                  // printf("\n after LOG UART11 sms FAIL 22 \n");
                }

                // continue;
              } else if ((strstr(dtmp, "AT+QFOPEN=") != NULL) &&
                         (strstr(dtmp, "+QFOPEN:") == NULL)) {
              } else if (send_ATCommand_Label == 1) {

                // printf("\ndtmp AT\n%s\n", dtmp);
                if (xQueueSendFromISR(AT_Command_Feedback_queue, (void *)&dtmp,
                                      xHigherPriorityTaskWoken) != pdPASS) {
                  // printf("\n after LOG UART11 FAIL \n");
                }
                // xSemaphoreGiveFromISR(rdySem_Control_Send_AT_Command,
                // xHigherPriorityTaskWoken);
                if (xHigherPriorityTaskWoken) {
                  /* Actual macro used here is port specific. */
                  portYIELD_FROM_ISR();
                }
                // memset(dtmp, 0, BUFF_SIZE);
                // break;
              }

              if (strstr(dtmp, "+QMTRECV:") != NULL) {
                int position[10] = {};
                uint8_t UDP_data_count = 0;
                int index_Found = -1;
                // printf("Erro: A substring.\n");
                int strIndex = 0;
                char substring[] = "+QMTRECV:";

                int tamanhoString = strlen(dtmp);
                int tamanhoSubstring = strlen(substring);
                // printf("\nsubstring. %d - %d\n\n", tamanhoString,
                // tamanhoSubstring);
                if (tamanhoString < tamanhoSubstring) {
                  // printf("Erro: A substring é maior que a string
                  // principal.\n");
                  return;
                }

                uint8_t subStr_counter = 0;
                for (int i = 0; i <= tamanhoString - tamanhoSubstring; ++i) {
                  int j;
                  for (j = 0; j < tamanhoSubstring; ++j) {
                    if (dtmp[i + j] != substring[j]) {
                      break;
                    }
                  }

                  if (j == tamanhoSubstring) {
                    position[subStr_counter++] = i;
                    // printf("\n\nposition %d\n\n ", i);
                  }
                }
                // printf("\nsubstring. %d - %d\n\n", tamanhoString,
                // tamanhoSubstring);
                /* index_Found = strpos(dtmp, "+QMTRECV:");
                memset(mqtt_data, 0, sizeof(mqtt_data)); */

                for (size_t j = 0; j < subStr_counter; j++) {
                  memset(mqtt_data, 0, sizeof(mqtt_data));
                  strIndex = 0;

                  for (size_t i = position[j]; i < tamanhoString; i++) {
                    // printf("\n\nposition1 %d\n\n ", position[j]);
                    if (dtmp[i] == '\r' || dtmp[i] == '\n') {
                      // printf("\n UDP_data_count - %c \n", dtmp[i]);
                      //  UDP_data_count++;
                      break;
                    }

                    /*  if (UDP_data_count == 1)
                     {
                             break;
                     } */

                    mqtt_data[strIndex++] = dtmp[i];
                  }

                  mqtt_data[strIndex + 1] = 0;
                  // printf("\n UDP_data_count - %s - %d \n", mqtt_data,
                  // strIndex);

                  // printf("\n\nUDP DATA - %s\n\n", mqtt_data);
                  if (xQueueSendFromISR(receive_mqtt_queue, (void *)&mqtt_data,
                                        xHigherPriorityTaskWoken) != pdPASS) {
                    // xSemaphoreGive(rdySem_Control_Send_AT_Command);
                    // printf("\n after LOG UART11 sms FAIL \n");
                  }
                }

                // printf("\n after LOG UART11 sms FAIL 11 \n");
                if (xHigherPriorityTaskWoken) {
                  /* Actual macro used here is port specific. */
                  portYIELD_FROM_ISR();
                }
                // printf("\n after LOG UART11 sms FAIL 22 \n");
              }

              // printf("\n\n ***QMT*** - %s \n\n", dtmp);

              if (strstr(dtmp, "+QMTSUB:") != NULL) {

                uint8_t UDP_data_count = 0;
                int index_Found = -1;
                uint8_t strIndex = 0;
                // printf("\n UDP_data_count - %d \n", UDP_data_count);
                index_Found = strpos(dtmp, "+QMTSUB:");
                memset(mqtt_data, 0, sizeof(mqtt_data));

                for (size_t i = index_Found; i < strlen(dtmp); i++) {

                  if (dtmp[i] == '\r' || dtmp[i] == '\n') {
                    // printf("\n QMTSUB data - %d \n", UDP_data_count);
                    UDP_data_count++;
                  }

                  if (UDP_data_count == 3) {
                    break;
                  }

                  mqtt_data[strIndex++] = dtmp[i];
                }

                // printf("\n\nUDP DATA qmtcon - %s\n\n", mqtt_data);
                if (xQueueSendFromISR(AT_Command_Feedback_queue,
                                      (void *)&mqtt_data,
                                      xHigherPriorityTaskWoken) != pdPASS) {
                  // xSemaphoreGive(rdySem_Control_Send_AT_Command);
                  // printf("\n after LOG UART11 sms FAIL \n");
                }

                // printf("\n after LOG UART11 sms FAIL 11 \n");
                if (xHigherPriorityTaskWoken) {
                  /* Actual macro used here is port specific. */
                  portYIELD_FROM_ISR();
                }
                // printf("\n after LOG UART11 sms FAIL 22 \n");
              }
              if (strstr(dtmp, "+QMTSTAT:") != NULL) {

                uint8_t UDP_data_count = 0;
                int index_Found = -1;
                uint8_t strIndex = 0;

                index_Found = strpos(dtmp, "+QMTSTAT:");
                memset(mqtt_data, 0, sizeof(mqtt_data));
                // printf("\n after LOG UART11 sms FAIL 8658 \n");
                for (size_t i = index_Found; i < strlen(dtmp); i++) {

                  if (dtmp[i] == '\r' || dtmp[i] == '\n') {
                    // printf("\n UDP_data_count - %d \n", UDP_data_count);
                    UDP_data_count++;
                  }

                  if (UDP_data_count == 3) {
                    mqtt_data[strIndex] = '\0';
                    break;
                  }

                  mqtt_data[strIndex++] = dtmp[i];
                }

                // printf("\n\nUDP DATA - %s\n\n", mqtt_data);
                if (xQueueSendFromISR(receive_mqtt_queue, (void *)&mqtt_data,
                                      xHigherPriorityTaskWoken) != pdPASS) {
                  // xSemaphoreGive(rdySem_Control_Send_AT_Command);
                  // printf("\n after LOG UART11 sms FAIL \n");
                }

                // printf("\n after LOG UART11 sms FAIL 11 \n");
                if (xHigherPriorityTaskWoken) {
                  /* Actual macro used here is port specific. */
                  portYIELD_FROM_ISR();
                }
                // printf("\n after LOG UART11 sms FAIL 22 \n");
              }
              // printf("\n\n ***QMT4*** - %s \n\n", dtmp);

              // printf("\n\n ***QMT5*** - %s \n\n", dtmp);
            }
          }
          // printf("\n\n ***QMT6*** - %s \n\n", dtmp);
          /* else if (strstr(dtmp, "+CUSD:"))
          {
                  //printf("\n CUSD ISR\n");
                  if (xQueueSendFromISR(EG91_GET_SIM_BALANCE_queue,
                                                            (void *)&dtmp,
                                                            xHigherPriorityTaskWoken)
          != pdPASS)
                  {
                          //printf("\n after LOG UART11 FAIL \n");
                  }
                  else
                  {
                          //printf("\n UART CUSD ISR SEND\n");
                  }

                  if (xHigherPriorityTaskWoken)
                  {

                          portYIELD_FROM_ISR();
                  }
                  break;
          } */
          else if (strstr(dtmp, "NO CARRIER")) {
            // printf("\n UART NO CARRIER\n");
            if (xQueueSendFromISR(NO_CARRIER_Call_queue, (void *)&dtmp,
                                  xHigherPriorityTaskWoken) != pdPASS) {
              // printf("\n after LOG UART11 FAIL \n");
            } else {
              // printf("\n UART NO CARRIER SEND\n");
            }

            if (xHigherPriorityTaskWoken) {

              portYIELD_FROM_ISR();
            }
            break;
          } else if (strstr(dtmp, "+QPSND: 0")) {
            // printf("\n rdySem_QPSND give\n");
            xSemaphoreGive(rdySem_QPSND);
            break;
          } else if ((strstr(dtmp, "QFWRITE")) != NULL) {
            if (EG91_CALL_SMS_UART_queue != NULL) {
              // printf("\ndtmp sms\n%s\n", dtmp);

              if (EG91_WRITE_FILE_queue != NULL) {
                // printf("\nEG91_WRITE_FILE_queue != NULL\n");
                if (xQueueSendFromISR(EG91_WRITE_FILE_queue, (void *)&dtmp,
                                      xHigherPriorityTaskWoken) != pdPASS) {
                  // printf("\n EG91_WRITE_FILE_queue \n");
                }
              } else {
                // printf("\nEG91_WRITE_FILE_queue == NULL\n");
              }
            }
            break;
          }
        }

        if (strstr(dtmp, "+QMTSTAT:") != NULL) {

          uint8_t UDP_data_count = 0;
          int index_Found = -1;
          uint8_t strIndex = 0;

          index_Found = strpos(dtmp, "+QMTSTAT:");
          memset(mqtt_data, 0, sizeof(mqtt_data));

          for (size_t i = index_Found; i < strlen(dtmp); i++) {

            if (dtmp[i] == '\r' || dtmp[i] == '\n') {
              // printf("\n UDP_data_count - %d \n", UDP_data_count);
              UDP_data_count++;
            }

            if (UDP_data_count == 3) {
              break;
            }

            mqtt_data[strIndex++] = dtmp[i];
          }

          // printf("\n\nUDP DATA - %s\n\n", mqtt_data);
          if (xQueueSendFromISR(receive_mqtt_queue, (void *)&mqtt_data,
                                xHigherPriorityTaskWoken) != pdPASS) {
            // xSemaphoreGive(rdySem_Control_Send_AT_Command);
            // printf("\n after LOG UART11 sms FAIL \n");
          }

          // printf("\n after LOG UART11 sms FAIL 11 \n");
          if (xHigherPriorityTaskWoken) {
            /* Actual macro used here is port specific. */
            portYIELD_FROM_ISR();
          }
          // printf("\n after LOG UART11 sms FAIL 22 \n");
        }

        if (strstr(dtmp, "+QMTPING:") != NULL) {
          save_INT8_Data_In_Storage(NVS_QMTSTAT_LABEL, 1, nvs_System_handle);
        }

        // printf("\n\n ***QMT6*** - %s \n\n", dtmp);
        break;
      // Event of HW FIFO overflow detected
      case UART_FIFO_OVF:
        // ESP_LOGI(TAG, "hw fifo overflow");
        //  If fifo overflow happened, you should consider adding flow control
        //  for your application. The ISR has already reset the rx FIFO, As an
        //  example, we directly flush the rx buffer here in order to read more
        //  data.
        uart_flush_input(UART_NUM_1);
        xQueueReset(uart0_queue);
        break;
      // Event of UART ring buffer full
      case UART_BUFFER_FULL:
        // ESP_LOGI(TAG, "ring buffer full");
        //  If buffer full happened, you should consider encreasing your buffer
        //  size As an example, we directly flush the rx buffer here in order to
        //  read more data.
        uart_flush_input(UART_NUM_1);
        xQueueReset(uart0_queue);
        break;
      // Event of UART RX break detected
      case UART_BREAK:
        // ESP_LOGI(TAG, "uart rx break");
        break;
      // Event of UART parity check error
      case UART_PARITY_ERR:
        // ESP_LOGI(TAG, "uart parity error");
        break;
      // Event of UART frame error
      case UART_FRAME_ERR:
        // ESP_LOGI(TAG, "uart frame error");
        break;
      // UART_PATTERN_DET
      case UART_PATTERN_DET:
        uart_get_buffered_data_len(UART_NUM_1, &buffered_size);
        int pos = uart_pattern_pop_pos(UART_NUM_1);
        // ESP_LOGI(TAG, "[UART PATTERN DETECTED] pos: %d, buffered size: %d",
        // pos, buffered_size);
        if (pos == -1) {
          // There used to be a UART_PATTERN_DET event, but the pattern position
          // queue is full so that it can not record the position. We should set
          // a larger queue size. As an example, we directly flush the rx buffer
          // here.
          uart_flush_input(UART_NUM_1);
        } else {
          uart_read_bytes(UART_NUM_1, dtmp, pos, 100 / portTICK_PERIOD_MS);
          uint8_t pat[PATTERN_CHR_NUM + 1];
          memset(pat, 0, sizeof(pat));
          uart_read_bytes(UART_NUM_1, pat, PATTERN_CHR_NUM,
                          100 / portTICK_PERIOD_MS);
          // ESP_LOGI(TAG, "read data: %s", dtmp);
          // ESP_LOGI(TAG, "read pat : %s", pat);
        }
        break;
      // Others
      default:
        // ESP_LOGI(TAG, "uart event type: %d", event.type);
        break;
      }
      // xSemaphoreGive(rdySem_UART_CTR);
    }
  }
  // free(dtmp);
  //  dtmp = NULL;
  vTaskDelete(NULL);
}

void uartInit(void) {
  // esp_log_level_set(TAG, "ESP_LOG_INFO");

  /* Configure parameters of an UART driver,
   * communication pins and install the driver */
  uart_config_t uart_config = {
      .baud_rate = 115200,
      .data_bits = UART_DATA_8_BITS,
      .parity = UART_PARITY_DISABLE,
      .stop_bits = UART_STOP_BITS_1,
      .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
      .source_clk = UART_SCLK_APB,
  };

  // Install UART driver, and get the queue.
  uart_driver_install(UART_NUM_1, BUF_SIZE, BUF_SIZE, 3, &uart0_queue, 0);
  uart_param_config(UART_NUM_1, &uart_config);

  // Set UART log level
  esp_log_level_set(TAG, ESP_LOG_INFO);
  // Set UART pins (using UART0 default pins ie no changes.)
  uart_set_pin(UART_NUM_1, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE,
               UART_PIN_NO_CHANGE);

  // Set uart pattern detect function.
  // uart_enable_pattern_det_baud_intr(UART_NUM_1, 10, PATTERN_CHR_NUM, 9, 0,
  // 0); Reset the pattern queue length to record at most 20 pattern positions.
  // uart_pattern_queue_reset(UART_NUM_1, 20);

  // Create a task to handler UART event from ISR
  xTaskCreate(uart_event_task, "uart_event_task", 5 * 1024, NULL, 32, NULL);
}

uint8_t EG91_get_IMEI(/* char *IMEI */) {
  EG91_send_AT_Command("AT+GSN", "GSN", 3000);

  if (strstr(EG91_IMEI, "ERROR") != NULL) {
    return 0;
  }

  // printf("\n\n\nEG91_get_IMEI - %s\n\n\n", EG91_IMEI);
  //   sprintf(IMEI, "%s", EG91_IMEI);
  save_STR_Data_In_Storage(NVS_KEY_EG91_IMEI, EG91_IMEI, nvs_System_handle);

  return 1;
}

uint8_t EG91_UDP_Ping() {
  // char imei[30] = {};
  // char CMD = '$';
  // char rssi[3] = {};

  // /* if (xHandle_Timer_VerSystem != NULL)
  // {
  //     timer_pause(TIMER_GROUP_1, TIMER_0);
  //     vTaskSuspend(xHandle_Timer_VerSystem);
  // } */

  // char data[100] = {};
  // // printf("\n\ntask_refresh_SystemTime 02\n\n");
  // get_STR_Data_In_Storage(NVS_KEY_EG91_IMEI, nvs_System_handle, &data);
  // // printf("\n\ntask_refresh_SystemTime 03\n\n");

  // sprintf(data, "%s", "$ ");
  // // strcat(data," $ ");
  // // printf("\n\ntask_refresh_SystemTime 04\n\n");
  // sprintf(rssi, "%d", RSSI_VALUE);
  // strcat(data, rssi);
  // // printf("\n\ntask_refresh_SystemTime 05\n\n");

  // label_network_portalRegister =
  // get_INT8_Data_From_Storage(NVS_NETWORK_PORTAL_REGISTER, nvs_System_handle);

  // if (label_network_portalRegister == 255)
  // {
  //     label_network_portalRegister = 1;
  //     save_INT8_Data_In_Storage(NVS_NETWORK_PORTAL_REGISTER,
  //     label_network_portalRegister, nvs_System_handle);
  // }

  // if (label_network_portalRegister == 1)
  // {
  //     send_UDP_Package(data, strlen(data));
  //     // send_UDP_queue(data);
  // }

  // send_UDP_queue(data);

  /* if (xHandle_Timer_VerSystem != NULL)
  {
          //printf("\n\ntask_refresh_SystemTime 06\n\n");
          vTaskResume(xHandle_Timer_VerSystem);
          timer_start(TIMER_GROUP_1, TIMER_0);
  } */

  return 1;
}

uint8_t EG91_Check_IF_Have_PIN() {
  SIM_CARD_PIN_status = 0;
  EG91_send_AT_Command("AT+CPIN?", "CPIN", 5000);

  // printf("\n\n SIM CARD PIN = %d\n\n", SIM_CARD_PIN_status);
  return SIM_CARD_PIN_status;
}

uint8_t EG91_Unlock_SIM_PIN(char *lastPIN) {
  char atCommand[100] = {};

  sprintf(atCommand, "%s%s%c", "AT+CLCK=\"SC\",0,\"", lastPIN, '\"');

  if (EG91_send_AT_Command(atCommand, "OK", 1000)) {
    EG91_initNetwork();
    return 1;
  } else {
    return 0;
  }

  return 0;
}
