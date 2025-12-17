#include "CommandLine.h"
#include <iterator>

CommandLine::CommandLine() {
}

void CommandLine::RunSetup() {
  multi_stream.println(this->ascii_art);

  multi_stream.println(F("\n\n--------------------------------\n"));
  multi_stream.println(F("         ESP32 Marauder      \n"));
  multi_stream.println("            " + version_number + "\n");
  multi_stream.println(F("       By: justcallmekoko\n"));
  multi_stream.println(F("--------------------------------\n\n"));
  
  multi_stream.print("> ");
}

String CommandLine::getSerialInput() {
  String input = "";

  if (Serial.available() > 0)
    input = Serial.readStringUntil('\n');

  input.trim();
  return input;
}

void CommandLine::main(uint32_t currentTime) {
  String input = this->getSerialInput();

  if (input != "") {
    multi_stream.println("#" + input);
    this->runCommand(input);
    multi_stream.print("> ");
  }
}

std::list<String> CommandLine::parseCommand(String input, char* delim) {
  std::list<String> cmd_args;

  bool inQuote = false;
  bool inApostrophe = false;
  String buffer = "";

  for (int i = 0; i < input.length(); i++) {
    char c = input.charAt(i);

    if (c == '"') {
      // Check if the quote is within an apostrophe
      if (inApostrophe) {
        buffer += c;
      } else {
        inQuote = !inQuote;
      }
    } else if (c == '\'') {
      // Check if the apostrophe is within a quote
      if (inQuote) {
        buffer += c;
      } else {
        inApostrophe = !inApostrophe;
      }
    } else if (!inQuote && !inApostrophe && strchr(delim, c) != NULL) {
      cmd_args.push_back(buffer);
      buffer = "";
    } else {
      buffer += c;
    }
  }

  // Add the last argument
  if (!buffer.isEmpty()) {
    cmd_args.push_back(buffer);
  }

  return cmd_args;
}

int CommandLine::argSearch(std::list<String>* cmd_args_list, String key) {
  int i = 0;
  for (const auto& arg : *cmd_args_list) {
    if (arg == key) {
      return i;
    }
    i++;
  }
  return -1;
}

bool CommandLine::checkValueExists(std::list<String>* cmd_args_list, int index) {
  if (index < cmd_args_list->size() - 1)
    return true;
    
  return false;
}

bool CommandLine::inRange(int max, int index) {
  if ((index >= 0) && (index < max))
    return true;

  return false;
}

bool CommandLine::apSelected() {
  for (const auto& ap : *access_points) {
    if (ap.selected)
      return true;
  }
  return false;
}

bool CommandLine::hasSSIDs() {
  if (ssids->size() == 0)
    return false;

  return true;
}

void CommandLine::showCounts(int selected, int unselected) {
  multi_stream.print((String) selected + " selected");
  
  if (unselected != -1) 
    multi_stream.print(", " + (String) unselected + " unselected");
  
  multi_stream.println("");
}

String CommandLine::toLowerCase(String str) {
  String result = str;
  for (int i = 0; i < str.length(); i++) {
    int charValue = str.charAt(i);
    if (charValue >= 65 && charValue <= 90) { // ASCII codes for uppercase letters
      charValue += 32;
      result.setCharAt(i, char(charValue));
    }
  }
  return result;
}

void CommandLine::filterAccessPoints(String filter) {
  int count_selected = 0;
  int count_unselected = 0;

  // Split the filter string into individual filters
  std::list<String> filters;
  int start = 0;
  int end = filter.indexOf(" or ");
  while (end != -1) {
    filters.push_back(filter.substring(start, end));
    start = end + 4;
    end = filter.indexOf(" or ", start);
  }
  filters.push_back(filter.substring(start));

  // Loop over each access point and check if it matches any of the filters
  for (auto& ap : *access_points) {
    bool matchesFilter = false;
    for (const auto& f_str : filters) {
      String f = toLowerCase(f_str);
      if (f.substring(0, 7) == "equals ") {
        String ssidEquals = f.substring(7);
        if ((ssidEquals.charAt(0) == '"' && ssidEquals.charAt(ssidEquals.length() - 1) == '"' && ssidEquals.length() > 1) ||
            (ssidEquals.charAt(0) == '\'' && ssidEquals.charAt(ssidEquals.length() - 1) == '\'' && ssidEquals.length() > 1)) {
          ssidEquals = ssidEquals.substring(1, ssidEquals.length() - 1);
        }
        if (ap.essid.equalsIgnoreCase(ssidEquals)) {
          matchesFilter = true;
          break;
        }
      } else if (f.substring(0, 9) == "contains ") {
        String ssidContains = f.substring(9);
        if ((ssidContains.charAt(0) == '"' && ssidContains.charAt(ssidContains.length() - 1) == '"' && ssidContains.length() > 1) ||
            (ssidContains.charAt(0) == '\'' && ssidContains.charAt(ssidContains.length() - 1) == '\'' && ssidContains.length() > 1)) {
          ssidContains = ssidContains.substring(1, ssidContains.length() - 1);
        }
        String essid = toLowerCase(ap.essid);
        if (essid.indexOf(ssidContains) != -1) {
          matchesFilter = true;
          break;
        }
      }
    }
    // Toggles the selected state of the AP
    ap.selected = matchesFilter;

    if (matchesFilter) {
      count_selected++;
    } else {
      count_unselected++;
    }
  }

  this->showCounts(count_selected, count_unselected);
}

void CommandLine::runCommand(String input) {
  if (input == "") return;

  if(wifi_scan_obj.scanning() && wifi_scan_obj.currentScanMode == WIFI_SCAN_GPS_NMEA){
    if(input != STOPSCAN_CMD) return;    
  }

  std::list<String> cmd_args = this->parseCommand(input, " ");
  
  //// Admin commands
  // Help
  if (cmd_args.front() == HELP_CMD) {
    multi_stream.println(HELP_HEAD);
    multi_stream.println(HELP_CH_CMD);
    multi_stream.println(HELP_SETTINGS_CMD);
    multi_stream.println(HELP_CLEARAP_CMD_A);
    multi_stream.println(HELP_REBOOT_CMD);
    multi_stream.println(HELP_UPDATE_CMD_A);
    multi_stream.println(HELP_LS_CMD);
    multi_stream.println(HELP_LED_CMD);
    multi_stream.println(HELP_GPS_DATA_CMD);
    multi_stream.println(HELP_GPS_CMD);
    multi_stream.println(HELP_NMEA_CMD);
    
    // WiFi sniff/scan
    multi_stream.println(HELP_EVIL_PORTAL_CMD);
    multi_stream.println(HELP_PACKET_COUNT_CMD);
    multi_stream.println(HELP_PING_CMD);
    multi_stream.println(HELP_PORT_SCAN_CMD);
    multi_stream.println(HELP_SIGSTREN_CMD);
    multi_stream.println(HELP_SCAN_ALL_CMD);
    multi_stream.println(HELP_SCANAP_CMD);
    multi_stream.println(HELP_SCANSTA_CMD);
    multi_stream.println(HELP_SNIFF_RAW_CMD);
    multi_stream.println(HELP_SNIFF_BEACON_CMD);
    multi_stream.println(HELP_SNIFF_PROBE_CMD);
    multi_stream.println(HELP_SNIFF_PWN_CMD);
    multi_stream.println(HELP_SNIFF_PINESCAN_CMD);
    multi_stream.println(HELP_SNIFF_MULTISSID_CMD);
    multi_stream.println(HELP_SNIFF_ESP_CMD);
    multi_stream.println(HELP_SNIFF_DEAUTH_CMD);
    multi_stream.println(HELP_SNIFF_PMKID_CMD);
    multi_stream.println(HELP_STOPSCAN_CMD);
    #ifdef HAS_GPS
      multi_stream.println(HELP_WARDRIVE_CMD);
    #endif
    
    // WiFi attack
    multi_stream.println(HELP_ATTACK_CMD);
    
    // WiFi Aux
    multi_stream.println(HELP_INFO_CMD);
    multi_stream.println(HELP_LIST_AP_CMD_A);
    multi_stream.println(HELP_LIST_AP_CMD_B);
    multi_stream.println(HELP_LIST_AP_CMD_C);
    multi_stream.println(HELP_LIST_AP_CMD_D);
    multi_stream.println(HELP_LIST_AP_CMD_E);
    multi_stream.println(HELP_SEL_CMD_A);
    multi_stream.println(HELP_SSID_CMD_A);
    multi_stream.println(HELP_SSID_CMD_B);
    multi_stream.println(HELP_SAVE_CMD);
    multi_stream.println(HELP_LOAD_CMD);
    multi_stream.println(HELP_JOIN_CMD);
    
    // Bluetooth sniff/scan
    #ifdef HAS_BT
      multi_stream.println(HELP_BT_SNIFF_CMD);
      multi_stream.println(HELP_BT_SPAM_CMD);
      multi_stream.println(HELP_BT_SPOOFAT_CMD);
      //multi_stream.println(HELP_BT_SWIFTPAIR_SPAM_CMD);
      //multi_stream.println(HELP_BT_SAMSUNG_SPAM_CMD);
      //multi_stream.println(HELP_BT_SPAM_ALL_CMD);
      #ifdef HAS_GPS
        multi_stream.println(HELP_BT_WARDRIVE_CMD);
      #endif
      multi_stream.println(HELP_BT_SKIM_CMD);
    #endif
    multi_stream.println(HELP_FOOT);
    return;
  }

  // Stop Scan
  if (cmd_args.front() == STOPSCAN_CMD) {
    //if (wifi_scan_obj.currentScanMode == OTA_UPDATE) {
    //  wifi_scan_obj.currentScanMode = WIFI_SCAN_OFF;
      //#ifdef HAS_SCREEN
      //  menu_function_obj.changeMenu(menu_function_obj.updateMenu.parentMenu);
      //#endif
    //  WiFi.softAPdisconnect(true);
    //  web_obj.shutdownServer();
    //  return;
    //}

    int f_arg = this->argSearch(&cmd_args, "-f");
    
    uint8_t old_scan_mode=wifi_scan_obj.currentScanMode;

    if (f_arg != -1) {
      WiFi.disconnect(true);
      delay(100);
    }

    wifi_scan_obj.StartScan(WIFI_SCAN_OFF);

    if(old_scan_mode == WIFI_SCAN_GPS_NMEA)
      multi_stream.println("END OF NMEA STREAM");
    else if(old_scan_mode == WIFI_SCAN_GPS_DATA)
      multi_stream.println("Stopping GPS data updates");
    else
      multi_stream.println("Stopping WiFi tran/recv");

    // If we don't do this, the text and button coordinates will be off
    #ifdef HAS_SCREEN
      display_obj.tft.init();
      menu_function_obj.changeMenu(menu_function_obj.current_menu);
    #endif
  }
  else if (cmd_args.front() == GPS_DATA_CMD) {
    #ifdef HAS_GPS
      if (gps_obj.getGpsModuleStatus()) {
        multi_stream.println("Getting GPS Data. Stop with " + (String)STOPSCAN_CMD);
        wifi_scan_obj.currentScanMode = WIFI_SCAN_GPS_DATA;
        #ifdef HAS_SCREEN
          menu_function_obj.changeMenu(&menu_function_obj.gpsInfoMenu);
        #endif
        wifi_scan_obj.StartScan(WIFI_SCAN_GPS_DATA, TFT_CYAN);
      }
    #endif
  }
  else if (cmd_args.front() == GPS_CMD) {
    #ifdef HAS_GPS
      if (gps_obj.getGpsModuleStatus()) {
        int get_arg = this->argSearch(&cmd_args, "-g");
        int nmea_arg = this->argSearch(&cmd_args, "-n");

        if (get_arg != -1) {
          String gps_info = *std::next(cmd_args.begin(), get_arg + 1);

          if (gps_info == "fix")
            multi_stream.println("Fix: " + gps_obj.getFixStatusAsString());
          else if (gps_info == "sat")
            multi_stream.println("Sats: " + gps_obj.getNumSatsString());
          else if (gps_info == "lat")
            multi_stream.println("Lat: " + gps_obj.getLat());
          else if (gps_info == "lon")
            multi_stream.println("Lon: " + gps_obj.getLon());
          else if (gps_info == "alt")
            multi_stream.println("Alt: " + (String)gps_obj.getAlt());
          else if (gps_info == "accuracy")
            multi_stream.println("Accuracy: " + (String)gps_obj.getAccuracy());
          else if (gps_info == "date")
            multi_stream.println("Date/Time: " + gps_obj.getDatetime());
          else if (gps_info == "text"){
            multi_stream.println(gps_obj.getText());
          }
          else if (gps_info == "nmea"){
            int notparsed_arg = this->argSearch(&cmd_args, "-p");
            int notimp_arg = this->argSearch(&cmd_args, "-i");
            int recd_arg = this->argSearch(&cmd_args, "-r");
            if(notparsed_arg == -1 && notimp_arg == -1 && recd_arg == -1){
              gps_obj.sendSentence(multi_stream, gps_obj.generateGXgga().c_str());
              gps_obj.sendSentence(multi_stream, gps_obj.generateGXrmc().c_str());
            }
            else if(notparsed_arg == -1 && notimp_arg == -1)
              multi_stream.println(gps_obj.getNmea());
            else if(notparsed_arg == -1)
              multi_stream.println(gps_obj.getNmeaNotimp());
            else
              multi_stream.println(gps_obj.getNmeaNotparsed());
          }
          else
            multi_stream.println("You did not provide a valid argument");
        }
        else if(nmea_arg != -1){
          String nmea_type = *std::next(cmd_args.begin(), nmea_arg + 1);

          if (nmea_type == "native" || nmea_type == "all" || nmea_type == "gps" || nmea_type == "glonass"
              || nmea_type == "galileo" || nmea_type == "navic" || nmea_type == "qzss" || nmea_type == "beidou"){
            if(nmea_type == "beidou"){
              int beidou_bd_arg = this->argSearch(&cmd_args, "-b");
              if(beidou_bd_arg != -1)
                nmea_type="beidou_bd";
            }
            gps_obj.setType(nmea_type);
            multi_stream.println("GPS Output Type Set To: " + nmea_type);
          }
          else
            multi_stream.println("You did not provide a valid argument");
        }
        else if(cmd_args.size()>1)
          multi_stream.println("You did not provide a valid flag");
        else
          multi_stream.println("You did not provide an argument");
      }
    #endif
  }
  else if (cmd_args.front() == NMEA_CMD) {
    #ifdef HAS_GPS
      if (gps_obj.getGpsModuleStatus()) {
        #ifdef HAS_SCREEN
          menu_function_obj.changeMenu(&menu_function_obj.gpsInfoMenu);
        #endif
        multi_stream.println("NMEA STREAM FOLLOWS");
        wifi_scan_obj.currentScanMode = WIFI_SCAN_GPS_NMEA;
        wifi_scan_obj.StartScan(WIFI_SCAN_GPS_NMEA, TFT_CYAN);
      }
    #endif
  }
  // LED command
  else if (cmd_args.front() == LED_CMD) {
    int hex_arg = this->argSearch(&cmd_args, "-s");
    int pat_arg = this->argSearch(&cmd_args, "-p");
    #ifdef PIN
      if (hex_arg != -1) {
        String hexstring = *std::next(cmd_args.begin(), hex_arg + 1);
        int number = (int)strtol(&hexstring[1], NULL, 16);
        int r = number >> 16;
        int g = number >> 8 & 0xFF;
        int b = number & 0xFF;
        //multi_stream.println(r);
        //multi_stream.println(g);
        //multi_stream.println(b);
        led_obj.setColor(r, g, b);
        led_obj.setMode(MODE_CUSTOM);
      }
      else if (pat_arg != -1) {
        String pat_name = *std::next(cmd_args.begin(), pat_arg + 1);
        pat_name.toLowerCase();
        if (pat_name == "rainbow") {
          led_obj.setMode(MODE_RAINBOW);
        }
      }
    #else
      multi_stream.println("This hardware does not support neopixel");
    #endif
  }
  // ls command
  else if (cmd_args.front() == LS_CMD) {
    #ifdef HAS_SD
      if (cmd_args.size() > 1)
        sd_obj.listDir(*std::next(cmd_args.begin(), 1));
      else
        multi_stream.println("You did not provide a dir to list");
    #else
      multi_stream.println("SD support disabled, cannot use command");
      return;
    #endif
  }

  // Channel command
  else if (cmd_args.front() == CH_CMD) {
    // Search for channel set arg
    int ch_set = this->argSearch(&cmd_args, "-s");
    
    if (cmd_args.size() == 1) {
      multi_stream.println("Current channel: " + (String)wifi_scan_obj.set_channel);
    }
    else if (ch_set != -1) {
      wifi_scan_obj.set_channel = (*std::next(cmd_args.begin(), ch_set + 1)).toInt();
      wifi_scan_obj.changeChannel();
      multi_stream.println("Set channel: " + (String)wifi_scan_obj.set_channel);
    }
  }
  // Clear APs
  else if (cmd_args.front() == CLEARAP_CMD) {
    int ap_sw = this->argSearch(&cmd_args, "-a"); // APs
    int ss_sw = this->argSearch(&cmd_args, "-s"); // SSIDs
    int cl_sw = this->argSearch(&cmd_args, "-c"); // Stations

    if (ap_sw != -1) {
      #ifdef HAS_SCREEN
        menu_function_obj.changeMenu(&menu_function_obj.clearAPsMenu);
      #endif
      wifi_scan_obj.RunClearAPs();
    }

    if (ss_sw != -1) {
      #ifdef HAS_SCREEN
        menu_function_obj.changeMenu(&menu_function_obj.clearSSIDsMenu);
      #endif
      wifi_scan_obj.RunClearSSIDs();
    }

    if (cl_sw != -1) {
      #ifdef HAS_SCREEN
        menu_function_obj.changeMenu(&menu_function_obj.clearAPsMenu);
      #endif
      wifi_scan_obj.RunClearStations();
    }
  }

  else if (cmd_args.front() == SETTINGS_CMD) {
    int ss_sw = this->argSearch(&cmd_args, "-s"); // Set setting
    int re_sw = this->argSearch(&cmd_args, "-r"); // Reset setting
    int en_sw = this->argSearch(&cmd_args, "enable"); // enable setting
    int da_sw = this->argSearch(&cmd_args, "disable"); // disable setting

    if (re_sw != -1) {
      settings_obj.createDefaultSettings(SPIFFS);
      return;
    }

    if (ss_sw == -1) {
      settings_obj.printJsonSettings(settings_obj.getSettingsString());
    }
    else {
      bool result = false;
      String setting_name = *std::next(cmd_args.begin(), ss_sw + 1);
      if (en_sw != -1)
        result = settings_obj.saveSetting<bool>(setting_name, true);
      else if (da_sw != -1)
        result = settings_obj.saveSetting<bool>(setting_name, false);
      else {
        multi_stream.println("You did not properly enable/disable this setting.");
        return;
      }

      if (!result) {
        multi_stream.println("Could not successfully update setting \"" + setting_name + "\"");
        return;
      }
    }
  }

  else if (cmd_args.front() == REBOOT_CMD) {
    multi_stream.println("Rebooting...");
    ESP.restart();
  }

  //// WiFi/Bluetooth Scan/Attack commands
  if (!wifi_scan_obj.scanning()) {
    // Dump pcap/log to serial too, valid for all scan/attack commands
    wifi_scan_obj.save_serial = this->argSearch(&cmd_args, "-serial") != -1;

    // Signal strength scan
    if (cmd_args.front() == SIGSTREN_CMD) {
      multi_stream.println("Starting Signal Strength Scan. Stop with " + (String)STOPSCAN_CMD);
      #ifdef HAS_SCREEN
        display_obj.clearScreen();
        menu_function_obj.drawStatusBar();
      #endif
      wifi_scan_obj.StartScan(WIFI_SCAN_SIG_STREN, TFT_MAGENTA);
      wifi_scan_obj.renderPacketRate();
    }
    // Packet count
    else if (cmd_args.front() == PACKET_COUNT_CMD) {
      multi_stream.println("Starting Packet Count Scan. Stop with " + (String)STOPSCAN_CMD);
      #ifdef HAS_SCREEN
        display_obj.clearScreen();
        menu_function_obj.drawStatusBar();
      #endif
      wifi_scan_obj.StartScan(WIFI_SCAN_PACKET_RATE, TFT_ORANGE);
    }
    // Wardrive
    else if (cmd_args.front() == WARDRIVE_CMD) {
      #ifdef HAS_GPS
        if (gps_obj.getGpsModuleStatus()) {
          int sta_sw = this->argSearch(&cmd_args, "-s");

          if (sta_sw == -1) {
            multi_stream.println("Starting Wardrive. Stop with " + (String)STOPSCAN_CMD);
            #ifdef HAS_SCREEN
              display_obj.clearScreen();
              menu_function_obj.drawStatusBar();
            #endif
            wifi_scan_obj.StartScan(WIFI_SCAN_WAR_DRIVE, TFT_GREEN);
          }
          else {multi_stream.println("Starting Station Wardrive. Stop with " + (String)STOPSCAN_CMD);
            #ifdef HAS_SCREEN
              display_obj.clearScreen();
              menu_function_obj.drawStatusBar();
            #endif
            wifi_scan_obj.StartScan(WIFI_SCAN_STATION_WAR_DRIVE, TFT_GREEN);
          }
        }
        else
          multi_stream.println("GPS Module not detected");
      #else
        multi_stream.println("GPS not supported");
      #endif
    }
    // AP Scan
    else if (cmd_args.front() == EVIL_PORTAL_CMD) {
      int cmd_sw = this->argSearch(&cmd_args, "-c");
      int html_sw = this->argSearch(&cmd_args, "-w");

      if (cmd_sw != -1) {
        String et_command = *std::next(cmd_args.begin(), cmd_sw + 1);
        if (et_command == "start") {
          multi_stream.println("Starting Evil Portal. Stop with " + (String)STOPSCAN_CMD);
          #ifdef HAS_SCREEN
            display_obj.clearScreen();
            menu_function_obj.drawStatusBar();
          #endif
          if (html_sw != -1) {
            String target_html_name = *std::next(cmd_args.begin(), html_sw + 1);
            evil_portal_obj.target_html_name = target_html_name;
            evil_portal_obj.using_serial_html = false;
            multi_stream.println("Set html file as " + evil_portal_obj.target_html_name);
          }
          //else {
          //  evil_portal_obj.target_html_name = "index.html";
          //}
          wifi_scan_obj.StartScan(WIFI_SCAN_EVIL_PORTAL, TFT_MAGENTA);
        }
        else if (et_command == "reset") {
          
        }
        else if (et_command == "ack") {
          
        }
        else if (et_command == "sethtml") {
          String target_html_name = *std::next(cmd_args.begin(), cmd_sw + 2);
          evil_portal_obj.target_html_name = target_html_name;
          evil_portal_obj.using_serial_html = false;
          multi_stream.println("Set html file as " + evil_portal_obj.target_html_name);
        }
        else if (et_command == "sethtmlstr") {
          evil_portal_obj.setHtmlFromSerial();
        }
        else if (et_command == "setap") {

        }
      }
    }
    else if (cmd_args.front() == SCAN_ALL_CMD) {
      multi_stream.println("Scanning for APs and Stations. Stop with " + (String)STOPSCAN_CMD);
      wifi_scan_obj.StartScan(WIFI_SCAN_AP_STA, TFT_MAGENTA);
    }
    else if (cmd_args.front() == SCANAP_CMD) {
      int full_sw = -1;
      #ifdef HAS_SCREEN
        display_obj.clearScreen();
        menu_function_obj.drawStatusBar();
      #endif

      if (full_sw == -1) {
        multi_stream.println("Starting AP scan. Stop with " + (String)STOPSCAN_CMD);
        wifi_scan_obj.StartScan(WIFI_SCAN_TARGET_AP, TFT_MAGENTA);
      }
      else {
        multi_stream.println("Starting Full AP scan. Stop with " + (String)STOPSCAN_CMD);
        wifi_scan_obj.StartScan(WIFI_SCAN_TARGET_AP_FULL, TFT_MAGENTA);
      }
    }
    // Raw sniff
    else if (cmd_args.front() == SNIFF_RAW_CMD) {
      multi_stream.println("Starting Raw sniff. Stop with " + (String)STOPSCAN_CMD);
      #ifdef HAS_SCREEN
        display_obj.clearScreen();
        menu_function_obj.drawStatusBar();
      #endif
      wifi_scan_obj.StartScan(WIFI_SCAN_RAW_CAPTURE, TFT_WHITE);
    }
    // Scan stations
    else if (cmd_args.front() == SCANSTA_CMD) {    
      if(access_points->size() < 1)
        multi_stream.println("The AP list is empty. Scan APs first with " + (String)SCANAP_CMD);  

      multi_stream.println("Starting Station scan. Stop with " + (String)STOPSCAN_CMD);  
      #ifdef HAS_SCREEN
        display_obj.clearScreen();
        menu_function_obj.drawStatusBar();
      #endif
      wifi_scan_obj.StartScan(WIFI_SCAN_STATION, TFT_ORANGE);
    }
    // Beacon sniff
    else if (cmd_args.front() == SNIFF_BEACON_CMD) {
      multi_stream.println("Starting Beacon sniff. Stop with " + (String)STOPSCAN_CMD);
      #ifdef HAS_SCREEN
        display_obj.clearScreen();
        menu_function_obj.drawStatusBar();
      #endif
      wifi_scan_obj.StartScan(WIFI_SCAN_AP, TFT_MAGENTA);
    }
    // Probe sniff
    else if (cmd_args.front() == SNIFF_PROBE_CMD) {
      multi_stream.println("Starting Probe sniff. Stop with " + (String)STOPSCAN_CMD);
      #ifdef HAS_SCREEN
        display_obj.clearScreen();
        menu_function_obj.drawStatusBar();
      #endif
      wifi_scan_obj.StartScan(WIFI_SCAN_PROBE, TFT_MAGENTA);
    }
    // Deauth sniff
    else if (cmd_args.front() == SNIFF_DEAUTH_CMD) {
      multi_stream.println("Starting Deauth sniff. Stop with " + (String)STOPSCAN_CMD);
      #ifdef HAS_SCREEN
        display_obj.clearScreen();
        menu_function_obj.drawStatusBar();
      #endif
      wifi_scan_obj.StartScan(WIFI_SCAN_DEAUTH, TFT_RED);
    }
    // Pwn sniff
    else if (cmd_args.front() == SNIFF_PWN_CMD) {
      multi_stream.println("Starting Pwnagotchi sniff. Stop with " + (String)STOPSCAN_CMD);
      #ifdef HAS_SCREEN
        display_obj.clearScreen();
        menu_function_obj.drawStatusBar();
      #endif
      wifi_scan_obj.StartScan(WIFI_SCAN_PWN, TFT_MAGENTA);
    }
    // PineScan sniff
    else if (cmd_args.front() == SNIFF_PINESCAN_CMD) {
      multi_stream.println("Starting Pinescan sniff. Stop with " + (String)STOPSCAN_CMD);
      #ifdef HAS_SCREEN
        display_obj.clearScreen();
        menu_function_obj.drawStatusBar();
      #endif
      wifi_scan_obj.StartScan(WIFI_SCAN_PINESCAN, TFT_MAGENTA);
    }
    // MultiSSID sniff
    else if (cmd_args.front() == SNIFF_MULTISSID_CMD) {
      multi_stream.println("Starting MultiSSID sniff. Stop with " + (String)STOPSCAN_CMD);
      #ifdef HAS_SCREEN
        display_obj.clearScreen();
        menu_function_obj.drawStatusBar();
      #endif
      wifi_scan_obj.StartScan(WIFI_SCAN_MULTISSID, TFT_MAGENTA);
    }
    // Espressif sniff
    else if (cmd_args.front() == SNIFF_ESP_CMD) {
      multi_stream.println("Starting Espressif device sniff. Stop with " + (String)STOPSCAN_CMD);
      #ifdef HAS_SCREEN
        display_obj.clearScreen();
        menu_function_obj.drawStatusBar();
      #endif
      wifi_scan_obj.StartScan(WIFI_SCAN_ESPRESSIF, TFT_MAGENTA);
    }
    // PMKID sniff
    else if (cmd_args.front() == SNIFF_PMKID_CMD) {
      int ch_sw = this->argSearch(&cmd_args, "-c");
      int d_sw = this->argSearch(&cmd_args, "-d"); // Deauth for pmkid
      int l_sw = this->argSearch(&cmd_args, "-l"); // Only run on list

      if (l_sw != -1) {
        if (!this->apSelected()) {
          multi_stream.println("You don't have any targets selected. Use " + (String)SEL_CMD);
          return;
        }
      }
      
      if (ch_sw != -1) {
        wifi_scan_obj.set_channel = (*std::next(cmd_args.begin(), ch_sw + 1)).toInt();
        wifi_scan_obj.changeChannel();
        multi_stream.println("Set channel: " + (String)wifi_scan_obj.set_channel);
        
      }

      if (d_sw == -1) {
        multi_stream.println("Starting PMKID sniff on channel " + (String)wifi_scan_obj.set_channel + ". Stop with " + (String)STOPSCAN_CMD);
        wifi_scan_obj.StartScan(WIFI_SCAN_EAPOL, TFT_VIOLET);
      }
      else if ((d_sw != -1) && (l_sw != -1)) {
        multi_stream.println("Starting TARGETED PMKID sniff with deauthentication on channel " + (String)wifi_scan_obj.set_channel + ". Stop with " + (String)STOPSCAN_CMD);
        wifi_scan_obj.StartScan(WIFI_SCAN_ACTIVE_LIST_EAPOL, TFT_VIOLET);
      }
      else {
        multi_stream.println("Starting PMKID sniff with deauthentication on channel " + (String)wifi_scan_obj.set_channel + ". Stop with " + (String)STOPSCAN_CMD);
        wifi_scan_obj.StartScan(WIFI_SCAN_ACTIVE_EAPOL, TFT_VIOLET);
      }
    }    

    //// WiFi attack commands
    // attack
    if (cmd_args.front() == ATTACK_CMD) {
      int attack_type_switch = this->argSearch(&cmd_args, "-t"); // Required
      int list_beacon_sw = this->argSearch(&cmd_args, "-l");
      int rand_beacon_sw = this->argSearch(&cmd_args, "-r");
      int ap_beacon_sw = this->argSearch(&cmd_args, "-a");
      int src_addr_sw = this->argSearch(&cmd_args, "-s");
      int dst_addr_sw = this->argSearch(&cmd_args, "-d");
      int targ_sw = this->argSearch(&cmd_args, "-c");
  
      if (attack_type_switch == -1) {
        multi_stream.println("You must specify an attack type");
        return;
      }
      else {
        String attack_type = *std::next(cmd_args.begin(), attack_type_switch + 1);
  
        // Branch on attack type
        // Deauth
        if (attack_type == ATTACK_TYPE_DEAUTH) {
          // Default to broadcast
          if ((dst_addr_sw == -1) && (targ_sw == -1)) {
            multi_stream.println("Sending to broadcast...");
            wifi_scan_obj.dst_mac = "ff:ff:ff:ff:ff:ff";
          }
          // Dest addr specified
          else if (dst_addr_sw != -1) {
            wifi_scan_obj.dst_mac = *std::next(cmd_args.begin(), dst_addr_sw + 1);
            multi_stream.println("Sending to " + wifi_scan_obj.dst_mac + "...");
          }
          // Station list specified
          else if (targ_sw != -1)
            multi_stream.println("Sending to Station list");

          // Source addr not specified
          if (src_addr_sw == -1) {
            if (!this->apSelected()) {
              multi_stream.println("You don't have any targets selected. Use " + (String)SEL_CMD);
              return;
            }
            #ifdef HAS_SCREEN
              display_obj.clearScreen();
              menu_function_obj.drawStatusBar();
            #endif
            multi_stream.println("Starting Deauthentication attack. Stop with " + (String)STOPSCAN_CMD);
            // Station list not specified
            if (targ_sw == -1)
              wifi_scan_obj.StartScan(WIFI_ATTACK_DEAUTH, TFT_RED);
            // Station list specified
            else
              wifi_scan_obj.StartScan(WIFI_ATTACK_DEAUTH_TARGETED, TFT_ORANGE);
          }
          // Source addr specified
          else {
            String src_mac_str = *std::next(cmd_args.begin(), src_addr_sw + 1);
            sscanf(src_mac_str.c_str(), "%2hhx:%2hhx:%2hhx:%2hhx:%2hhx:%2hhx", 
              &wifi_scan_obj.src_mac[0], &wifi_scan_obj.src_mac[1], &wifi_scan_obj.src_mac[2], &wifi_scan_obj.src_mac[3], &wifi_scan_obj.src_mac[4], &wifi_scan_obj.src_mac[5]);

            #ifdef HAS_SCREEN
              display_obj.clearScreen();
              menu_function_obj.drawStatusBar();
            #endif
            multi_stream.println("Starting Manual Deauthentication attack. Stop with " + (String)STOPSCAN_CMD);
            wifi_scan_obj.StartScan(WIFI_ATTACK_DEAUTH_MANUAL, TFT_RED);            
          }
        }
        // Beacon
        else if (attack_type == ATTACK_TYPE_BEACON) {
          // spam by list
          if (list_beacon_sw != -1) {
            if (!this->hasSSIDs()) {
              multi_stream.println("You don't have any SSIDs in your list. Use " + (String)SSID_CMD);
              return;
            }
            #ifdef HAS_SCREEN
              display_obj.clearScreen();
              menu_function_obj.drawStatusBar();
            #endif
            multi_stream.println("Starting Beacon list spam. Stop with " + (String)STOPSCAN_CMD);
            wifi_scan_obj.StartScan(WIFI_ATTACK_BEACON_LIST, TFT_RED);
          }
          // spam with random
          else if (rand_beacon_sw != -1) {
            #ifdef HAS_SCREEN
              display_obj.clearScreen();
              menu_function_obj.drawStatusBar();
            #endif
            multi_stream.println("Starting random Beacon spam. Stop with " + (String)STOPSCAN_CMD);
            wifi_scan_obj.StartScan(WIFI_ATTACK_BEACON_SPAM, TFT_ORANGE);
          }
          // Spam from AP list
          else if (ap_beacon_sw != -1) {
            if (!this->apSelected()) {
              multi_stream.println("You don't have any targets selected. Use " + (String)SEL_CMD);
              return;
            }
            #ifdef HAS_SCREEN
              display_obj.clearScreen();
              menu_function_obj.drawStatusBar();
            #endif
            multi_stream.println("Starting Targeted AP Beacon spam. Stop with " + (String)STOPSCAN_CMD);
            wifi_scan_obj.StartScan(WIFI_ATTACK_AP_SPAM, TFT_MAGENTA);
          }
          else {
            multi_stream.println("You did not specify a beacon attack type");
          }
        }
        else if (attack_type == ATTACK_TYPE_PROBE) {
          if (!this->apSelected()) {
            multi_stream.println("You don't have any targets selected. Use " + (String)SEL_CMD);
            return;
          }
          multi_stream.println("Starting Probe spam. Stop with " + (String)STOPSCAN_CMD);
          #ifdef HAS_SCREEN
            display_obj.clearScreen();
            menu_function_obj.drawStatusBar();
          #endif
          wifi_scan_obj.StartScan(WIFI_ATTACK_AUTH, TFT_RED);
        }
        else if (attack_type == ATTACK_TYPE_RR) {
          multi_stream.println("Starting Rick Roll Beacon spam. Stop with " + (String)STOPSCAN_CMD);
          #ifdef HAS_SCREEN
            display_obj.clearScreen();
            menu_function_obj.drawStatusBar();
          #endif
          wifi_scan_obj.StartScan(WIFI_ATTACK_RICK_ROLL, TFT_YELLOW);
        }
        else {
          multi_stream.println("Attack type not properly defined");
          return;
        }
      }
    }

    //// Bluetooth scan/attack commands
    // Bluetooth scan
    if (cmd_args.front() == BT_SNIFF_CMD) {
      #ifdef HAS_BT
        int bt_type_sw = this->argSearch(&cmd_args, "-t");

        // Specifying type of bluetooth sniff
        if (bt_type_sw != -1) {
          String bt_type = *std::next(cmd_args.begin(), bt_type_sw + 1);

          bt_type.toLowerCase();

          // Airtag sniff
          if (bt_type == "airtag") {
            multi_stream.println("Starting Airtag sniff. Stop with " + (String)STOPSCAN_CMD);
            #ifdef HAS_SCREEN
              display_obj.clearScreen();
              menu_function_obj.drawStatusBar();
            #endif
            wifi_scan_obj.StartScan(BT_SCAN_AIRTAG, TFT_WHITE);
          }
          else if (bt_type == "flipper") {
            multi_stream.println("Starting Flipper sniff. Stop with " + (String)STOPSCAN_CMD);
            #ifdef HAS_SCREEN
              display_obj.clearScreen();
              menu_function_obj.drawStatusBar();
            #endif
            wifi_scan_obj.StartScan(BT_SCAN_FLIPPER, TFT_ORANGE);
          }
        }
        // General bluetooth sniff
        else {
          multi_stream.println("Starting Bluetooth scan. Stop with " + (String)STOPSCAN_CMD);
          #ifdef HAS_SCREEN
            display_obj.clearScreen();
            menu_function_obj.drawStatusBar();
          #endif
          wifi_scan_obj.StartScan(BT_SCAN_ALL, TFT_GREEN);
        }
      #else
        multi_stream.println("Bluetooth not supported");
      #endif
    }
    else if (cmd_args.front() == BT_SPOOFAT_CMD) {
      int at_sw = this->argSearch(&cmd_args, "-t");
      if (at_sw != -1) {
        #ifdef HAS_BT
          int target_mac_index = (*std::next(cmd_args.begin(), at_sw + 1)).toInt();
          if (target_mac_index < airtags->size()) {
            int i = 0;
            for (auto& at : *airtags) {
              at.selected = (i == target_mac_index);
              i++;
            }
            multi_stream.println("Spoofing Airtag: " + (*std::next(airtags->begin(), target_mac_index)).mac);
            #ifdef HAS_SCREEN
              display_obj.clearScreen();
              menu_function_obj.drawStatusBar();
            #endif
            wifi_scan_obj.StartScan(BT_SPOOF_AIRTAG, TFT_WHITE);
          }
          else {
            multi_stream.println("Provided index is out of range: " + (String)target_mac_index);
            return;
          }
        #endif
      }
    }
    else if (cmd_args.front() == BT_SPAM_CMD) {
      int bt_type_sw = this->argSearch(&cmd_args, "-t");
      if (bt_type_sw != -1) {
        String bt_type = *std::next(cmd_args.begin(), bt_type_sw + 1);

        if (bt_type == "apple") {
          #ifdef HAS_BT
            multi_stream.println("Starting Sour Apple attack. Stop with " + (String)STOPSCAN_CMD);
            #ifdef HAS_SCREEN
              display_obj.clearScreen();
              menu_function_obj.drawStatusBar();
            #endif
            wifi_scan_obj.StartScan(BT_ATTACK_SOUR_APPLE, TFT_GREEN);
          #else
            multi_stream.println("Bluetooth not supported");
          #endif
        }
        else if (bt_type == "windows") {
          #ifdef HAS_BT
            multi_stream.println("Starting Swiftpair Spam attack. Stop with " + (String)STOPSCAN_CMD);
            #ifdef HAS_SCREEN
              display_obj.clearScreen();
              menu_function_obj.drawStatusBar();
            #endif
            wifi_scan_obj.StartScan(BT_ATTACK_SWIFTPAIR_SPAM, TFT_CYAN);
          #else
            multi_stream.println("Bluetooth not supported");
          #endif
        }
        else if (bt_type == "samsung") {
          #ifdef HAS_BT
            multi_stream.println("Starting Samsung Spam attack. Stop with " + (String)STOPSCAN_CMD);
            #ifdef HAS_SCREEN
              display_obj.clearScreen();
              menu_function_obj.drawStatusBar();
            #endif
            wifi_scan_obj.StartScan(BT_ATTACK_SAMSUNG_SPAM, TFT_CYAN);
          #else
            multi_stream.println("Bluetooth not supported");
          #endif
        }
        else if (bt_type == "google") {
          #ifdef HAS_BT
            multi_stream.println("Starting Google Spam attack. Stop with " + (String)STOPSCAN_CMD);
            #ifdef HAS_SCREEN
              display_obj.clearScreen();
              menu_function_obj.drawStatusBar();
            #endif
            wifi_scan_obj.StartScan(BT_ATTACK_GOOGLE_SPAM, TFT_CYAN);
          #else
            multi_stream.println("Bluetooth not supported");
          #endif
        }
        else if (bt_type == "flipper") {
          #ifdef HAS_BT
            multi_stream.println("Starting Flipper Spam attack. Stop with " + (String)STOPSCAN_CMD);
            #ifdef HAS_SCREEN
              display_obj.clearScreen();
              menu_function_obj.drawStatusBar();
            #endif
            wifi_scan_obj.StartScan(BT_ATTACK_FLIPPER_SPAM, TFT_ORANGE);
          #else
            multi_stream.println("Bluetooth not supported");
          #endif
        }
        else if (bt_type == "all") {
          #ifdef HAS_BT
            multi_stream.println("Starting BT Spam All attack. Stop with " + (String)STOPSCAN_CMD);
            #ifdef HAS_SCREEN
              display_obj.clearScreen();
              menu_function_obj.drawStatusBar();
            #endif
            wifi_scan_obj.StartScan(BT_ATTACK_SPAM_ALL, TFT_MAGENTA);
          #else
            multi_stream.println("Bluetooth not supported");
          #endif
        }
        else {
          multi_stream.println("You did not specify a correct spam type");
        }
      }
    }
    // Wardrive
    else if (cmd_args.front() == BT_WARDRIVE_CMD) {
      #ifdef HAS_BT
        #ifdef HAS_GPS
          if (gps_obj.getGpsModuleStatus()) {
            int cont_sw = this->argSearch(&cmd_args, "-c");

            if (cont_sw == -1) {
              multi_stream.println("Starting BT Wardrive. Stop with " + (String)STOPSCAN_CMD);
              #ifdef HAS_SCREEN
                display_obj.clearScreen();
                menu_function_obj.drawStatusBar();
              #endif
              wifi_scan_obj.StartScan(BT_SCAN_WAR_DRIVE, TFT_GREEN);
            }
            else {multi_stream.println("Starting Continuous BT Wardrive. Stop with " + (String)STOPSCAN_CMD);
              #ifdef HAS_SCREEN
                display_obj.clearScreen();
                menu_function_obj.drawStatusBar();
              #endif
              wifi_scan_obj.StartScan(BT_SCAN_WAR_DRIVE_CONT, TFT_GREEN);
            }
          }
          else
            multi_stream.println("GPS Module not detected");
        #else
          multi_stream.println("GPS not supported");
        #endif
      #else
        multi_stream.println("Bluetooth not supported");
      #endif
      
    }
    // Bluetooth CC Skimmer scan
    else if (cmd_args.front() == BT_SKIM_CMD) {
      #ifdef HAS_BT
        multi_stream.println("Starting Bluetooth CC Skimmer scan. Stop with " + (String)STOPSCAN_CMD);
        #ifdef HAS_SCREEN
          display_obj.clearScreen();
          menu_function_obj.drawStatusBar();
        #endif
        wifi_scan_obj.StartScan(BT_SCAN_SKIMMERS, TFT_MAGENTA);
      #else
        multi_stream.println("Bluetooth not supported");
      #endif
    }

    // Update command
    if (cmd_args.front() == UPDATE_CMD) {
      int sd_sw = this->argSearch(&cmd_args, "-s"); // SD Update

      if (sd_sw != -1) {
        #ifdef HAS_SD
          if (!sd_obj.supported) {
            multi_stream.println("SD card is not connected. Cannot perform SD Update");
            return;
          }
          wifi_scan_obj.currentScanMode = OTA_UPDATE;
          sd_obj.runUpdate();
        #else
          multi_stream.println("SD card support disabled. Cannot perform SD Update");
          return;
        #endif
      }
    }
  }

  if (wifi_scan_obj.wifi_connected) {
    // Ping Scan
    if (cmd_args.front() == PING_CMD) {
      multi_stream.println("Starting Ping Scan. Stop with " + (String)STOPSCAN_CMD);
      #ifdef HAS_SCREEN
        display_obj.clearScreen();
        menu_function_obj.drawStatusBar();
      #endif
      wifi_scan_obj.StartScan(WIFI_PING_SCAN, TFT_GREEN);
    }

    // Port Scan
    if (cmd_args.front() == PORT_SCAN_CMD) {
      int all_sw = this->argSearch(&cmd_args, "-a");
      int ip_sw = this->argSearch(&cmd_args, "-t");

      if (ip_sw != -1) {
        int ip_index = (*std::next(cmd_args.begin(), ip_sw + 1)).toInt();

        if (ip_index < ipList->size()) {

          if (all_sw != -1) {
            multi_stream.println("Selected: " + (*std::next(ipList->begin(), ip_index)).toString());
            wifi_scan_obj.current_scan_ip = *std::next(ipList->begin(), ip_index);
            #ifdef HAS_SCREEN
              display_obj.clearScreen();
              menu_function_obj.drawStatusBar();
            #endif
            wifi_scan_obj.StartScan(WIFI_PORT_SCAN_ALL, TFT_BLUE);
          }
        }
        else {
          multi_stream.println("The IP index specified is out of range");
          return;
        }
      }
      else {
        multi_stream.println("You did not specify an IP index");
        return;
      }
    }
  }


  int count_selected = 0;
  //// WiFi aux commands
  // List access points
  if (cmd_args.front() == LIST_AP_CMD) {
    int ap_sw = this->argSearch(&cmd_args, "-a");
    int ss_sw = this->argSearch(&cmd_args, "-s");
    int cl_sw = this->argSearch(&cmd_args, "-c");
    int at_sw = this->argSearch(&cmd_args, "-t");
    int ip_sw = this->argSearch(&cmd_args, "-i");

    if (ap_sw != -1) {
      int i = 0;
      for (const auto& ap : *access_points) {
        if (ap.selected) {
          multi_stream.println("[" + (String)i + "][CH:" + (String)ap.channel + "] " + ap.essid + " " + (String)ap.rssi + " (selected)");
          count_selected += 1;
        } 
        else
          multi_stream.println("[" + (String)i + "][CH:" + (String)ap.channel + "] " + ap.essid + " " + (String)ap.rssi);
        i++;
      }
      this->showCounts(count_selected);
    }
    else if (ip_sw != -1) {
      int i = 0;
      for (const auto& ip : *ipList) {
        multi_stream.println("[" + (String)i + "] " + ip.toString());
        i++;
      }
    }
    else if (ss_sw != -1) {
      int i = 0;
      for (const auto& s : *ssids) {
        if (s.selected) {
          multi_stream.println("[" + (String)i + "] " + s.essid + " (selected)");
          count_selected += 1;
        } 
        else
          multi_stream.println("[" + (String)i + "] " + s.essid);
        i++;
      }
      this->showCounts(count_selected);
    }
    else if (cl_sw != -1) {
      char sta_mac[] = "00:00:00:00:00:00";
      int x = 0;
      for (const auto& ap : *access_points) {
        multi_stream.println("[" + (String)x + "] " + ap.essid + " " + (String)ap.rssi + ":");
        for (int i = 0; i < ap.stations->size(); i++) {
          uint16_t station_index = *std::next(ap.stations->begin(), i);
          wifi_scan_obj.getMAC(sta_mac, (*std::next(stations->begin(), station_index)).mac, 0);
          if ((*std::next(stations->begin(), station_index)).selected) {
            multi_stream.print("  [" + (String)station_index + "] ");
            multi_stream.print(sta_mac);
            multi_stream.println(" (selected)");
            count_selected += 1;
          }
          else {
            multi_stream.print("  [" + (String)station_index + "] ");
            multi_stream.println(sta_mac);
          }
        }
        x++;
      }
      this->showCounts(count_selected);
    }
    else if (at_sw != -1) {
      int i = 0;
      for (const auto& at : *airtags) {
        multi_stream.println("[" + (String)i + "]MAC: " + at.mac);
        i++;
      }
    }
    else {
      multi_stream.println("You did not specify which list to show");
      return;
    }
  }
  else if (cmd_args.front() == INFO_CMD) {
    int ap_sw = this->argSearch(&cmd_args, "-a");

    if (ap_sw != -1) {
      int filter_ap = (*std::next(cmd_args.begin(), ap_sw + 1)).toInt();
      wifi_scan_obj.RunAPInfo(filter_ap, false);
    }
    else {
      wifi_scan_obj.currentScanMode = SHOW_INFO;
      #ifdef HAS_SCREEN
        menu_function_obj.changeMenu(&menu_function_obj.infoMenu);
      #endif
      wifi_scan_obj.RunInfo();
    }
  }
  else if (cmd_args.front() == JOIN_CMD) {
    int ap_sw = this->argSearch(&cmd_args, "-a");
    int pw_sw = this->argSearch(&cmd_args, "-p");

    if ((ap_sw != -1) && (pw_sw != -1)) {
      int index = (*std::next(cmd_args.begin(), ap_sw + 1)).toInt();
      String password = *std::next(cmd_args.begin(), pw_sw + 1);
      multi_stream.println("Using SSID: " + (String)(*std::next(access_points->begin(), index)).essid + " Password: " + (String)password);
      wifi_scan_obj.joinWiFi((*std::next(access_points->begin(), index)).essid, password, false);
      #ifdef HAS_SCREEN
        #ifdef HAS_MINI_KB
          menu_function_obj.changeMenu(menu_function_obj.current_menu);
        #endif
      #endif
    }
    else {
      multi_stream.println("You did not provide the proper args");
      return;
    }
  }
  else if (cmd_args.front() == SEL_CMD) {
    int ap_sw = this->argSearch(&cmd_args, "-a");
    int ss_sw = this->argSearch(&cmd_args, "-s");
    int cl_sw = this->argSearch(&cmd_args, "-c");
    int filter_sw = this->argSearch(&cmd_args, "-f");

    count_selected = 0;
    int count_unselected = 0;
    if (ap_sw != -1) {
      if (filter_sw != -1) {
        String filter_ap = *std::next(cmd_args.begin(), filter_sw + 1);
        this->filterAccessPoints(filter_ap);
      } else {
        std::list<String> ap_index = this->parseCommand(*std::next(cmd_args.begin(), ap_sw + 1), ",");
        if ((*std::next(cmd_args.begin(), ap_sw + 1)) == "all") {
          for (auto& ap : *access_points) {
            ap.selected = !ap.selected;
            if (ap.selected) count_selected++; else count_unselected++;
          }
          this->showCounts(count_selected, count_unselected);
        }
        else {
          for (const auto& index_str : ap_index) {
            int index = index_str.toInt();
            if (!this->inRange(access_points->size(), index)) {
              multi_stream.println("Index not in range: " + (String)index);
              continue;
            }
            auto it = access_points->begin();
            std::advance(it, index);
            it->selected = !it->selected;
            if (it->selected) count_selected++; else count_unselected++;
          }
          this->showCounts(count_selected, count_unselected);
        }
      }
    }
    else if (cl_sw != -1) {
      std::list<String> sta_index = this->parseCommand(*std::next(cmd_args.begin(), cl_sw + 1), ",");
      if (*std::next(cmd_args.begin(), cl_sw + 1) == "all") {
        for (auto& station : *stations) {
          station.selected = !station.selected;
          if (station.selected) count_selected++; else count_unselected++;
        }
        this->showCounts(count_selected, count_unselected);
      }
      else {
        for (const auto& index_str : sta_index) {
          int index = index_str.toInt();
          if (!this->inRange(stations->size(), index)) {
            multi_stream.println("Index not in range: " + (String)index);
            continue;
          }
          auto it = stations->begin();
          std::advance(it, index);
          it->selected = !it->selected;
          if (it->selected) count_selected++; else count_unselected++;
        }
        this->showCounts(count_selected, count_unselected);
      }
    }
    else if (ss_sw != -1) {
      std::list<String> ss_index = this->parseCommand(*std::next(cmd_args.begin(), ss_sw + 1), ",");
      if (*std::next(cmd_args.begin(), ss_sw + 1) == "all") {
        for (auto& s : *ssids) {
          s.selected = !s.selected;
          if (s.selected) count_selected++; else count_unselected++;
        }
      }
      else {
        for (const auto& index_str : ss_index) {
          int index = index_str.toInt();
          if (!this->inRange(ssids->size(), index)) {
            multi_stream.println("Index not in range: " + (String)index);
            continue;
          }
          auto it = ssids->begin();
          std::advance(it, index);
          it->selected = !it->selected;
          if (it->selected) count_selected++; else count_unselected++;
        }
      }
      this->showCounts(count_selected, count_unselected);
    }
    else {
      multi_stream.println("You did not specify which list to select from");
      return;
    }
  }
  else if (cmd_args.front() == SAVE_CMD) {
    int ap_sw = this->argSearch(&cmd_args, "-a");
    int st_sw = this->argSearch(&cmd_args, "-s");

    if (ap_sw != -1) {
      #ifdef HAS_SCREEN
        menu_function_obj.changeMenu(&menu_function_obj.saveAPsMenu);
      #endif
      wifi_scan_obj.RunSaveAPList(true);
    }
    else if (st_sw != -1) {
      #ifdef HAS_SCREEN
        menu_function_obj.changeMenu(&menu_function_obj.saveSSIDsMenu);
      #endif
      wifi_scan_obj.RunSaveSSIDList(true);
    }
  }
  else if (cmd_args.front() == LOAD_CMD) {
    int ap_sw = this->argSearch(&cmd_args, "-a");
    int st_sw = this->argSearch(&cmd_args, "-s");

    if (ap_sw != -1) {
      #ifdef HAS_SCREEN
        menu_function_obj.changeMenu(&menu_function_obj.loadAPsMenu);
      #endif
      wifi_scan_obj.RunLoadAPList();
    }
    else if (st_sw != -1) {
      #ifdef HAS_SCREEN
        menu_function_obj.changeMenu(&menu_function_obj.loadSSIDsMenu);
      #endif
      wifi_scan_obj.RunLoadSSIDList();
    }
  }

  // SSID stuff
  else if (cmd_args.front() == SSID_CMD) {
    int add_sw = this->argSearch(&cmd_args, "-a");
    int gen_sw = this->argSearch(&cmd_args, "-g");
    int spc_sw = this->argSearch(&cmd_args, "-n");
    int rem_sw = this->argSearch(&cmd_args, "-r");

    if (add_sw != -1) {
      if (gen_sw != -1) {
        int gen_count = (*std::next(cmd_args.begin(), gen_sw + 1)).toInt();
        wifi_scan_obj.generateSSIDs(gen_count);
      }
      else if (spc_sw != -1) {
        String essid = *std::next(cmd_args.begin(), spc_sw + 1);
        wifi_scan_obj.addSSID(essid);
      }
      else {
        multi_stream.println("You did not specify how to add SSIDs");
      }
    }
    else if (rem_sw != -1) {
      int index = (*std::next(cmd_args.begin(), rem_sw + 1)).toInt();
      if (!this->inRange(ssids->size(), index)) {
        multi_stream.println("Index not in range: " + (String)index);
        return;
      }
      auto it = ssids->begin();
      std::advance(it, index);
      ssids->erase(it);
    }
    else {
      multi_stream.println("You did not specify whether to add or remove SSIDs");
      return;
    }
  }
}
