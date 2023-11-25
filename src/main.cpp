#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <BleKeyboard.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <SpotifyArduino.h>
#include <SpotifyArduinoCert.h>
#include <ArduinoJson.h>
#include "encoder.h"
#include "secrets.h"
#include "customChars.h"

#define SPOTIFY_MARKET "RO"
#define SPOTIFY_REFRESH_TOKEN "AQBULfG8YdwWks3ak1RSbSeRHviZlzF9tUU6H1wixBfpZyu3sg9p6TMSOrpoZ-a2bgSElEOajKEOT-BV9yFO2aMZ7ZiLoFJLWliDpIovB4nK9cIZUJTl_Ly9ToyHjOL3zAs"

#define BUTTON_HOLD_TIME 600           // Time (ms) to differentiate between a short press and a holding down the button
#define LED_BLINK_FREQ 1000            // Built-in led blink frequency (ms)
#define SPOTIFY_REFRESH_FREQ 1000 * 20 // Time to wait before next API call to Spotify (1000 ms * seconds)

enum class CustomChars
{
  Note,
  Person,
  Disk
};

BleKeyboard kb;

Encoder e;

LiquidCrystal_I2C lcd(0x27, 16, 9);

uint64_t button_ms = 0, blink_ms = 0, last_spotify_api_call = 0;
bool block_hold = false;
bool LED_state = LOW;

WiFiClientSecure client;
SpotifyArduino spotify(client, SPOTIFY_CLIENT_ID, SPOTIFY_CLIENT_SECRET, SPOTIFY_REFRESH_TOKEN);

void printCurrentlyPlayingToLCD(CurrentlyPlaying currentlyPlaying)
{
  // Print track name
  lcd.setCursor(0, 0);
  lcd.write(static_cast<uint8_t>(CustomChars::Note));
  lcd.print(' ');
  lcd.print(currentlyPlaying.trackName);

  // Print artist
  lcd.setCursor(0, 1);
  lcd.write(static_cast<uint8_t>(CustomChars::Person));
  lcd.print(' ');
  lcd.println(currentlyPlaying.artists[0].artistName);
}

void SpotifyTask(void *args)
{
  // Initialize LCD
  lcd.init();
  lcd.backlight();
  lcd.createChar(0, note);
  lcd.createChar(1, person);
  lcd.createChar(2, album);

  // Initialize WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, PASSWORD);
  Serial.println("");
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(SSID);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  lcd.setCursor(0, 0);
  lcd.print("SSID: ");
  lcd.println(SSID);
  lcd.setCursor(0, 1);
  lcd.print("IP: ");
  lcd.println(WiFi.localIP());

  client.setCACert(spotify_server_cert);

  Serial.println("Refreshing Access Tokens");
  if (!spotify.refreshAccessToken())
  {
    Serial.println("Failed to get access tokens");
  }

  // Delay to display the WiFi info on the LCD
  vTaskDelay(2000 / portTICK_PERIOD_MS);

  while (true)
  {
    if (millis() - last_spotify_api_call > SPOTIFY_REFRESH_FREQ)
    {
      last_spotify_api_call = millis();

      int status = spotify.getCurrentlyPlaying(printCurrentlyPlayingToLCD, SPOTIFY_MARKET);
      switch (status)
      {
      case 200:
        Serial.println("Successfully got currently playing");
        break;
      case 204:
        Serial.println("Nothing playing");
        break;

      default:
        Serial.print("Error: ");
        Serial.println(status);
        break;
      }
    }

    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

void setup()
{
  // Initialize serial communication
  Serial.begin(115200);

  // Initialize bluetooth keyboard library
  kb.begin();

  pinMode(BUILTIN_LED, OUTPUT);

  // Create separate task for LCD and connectiong to Spotify API
  // xTaskCreatePinnedToCore(
  //     SpotifyTask,
  //     "Spotify Task",
  //     2048,
  //     NULL,
  //     1,
  //     NULL,
  //     0);
}

void loop()
{
  while (kb.isConnected())
  {
    // Make sure built-in LED is off
    LED_state = LOW;
    digitalWrite(BUILTIN_LED, LED_state);

    // ================================
    //   Rotation part of the encoder
    // ================================
    switch (e.getRotationDirection())
    {
    case -1:
      // Serial.println("Left");
      kb.write(KEY_MEDIA_VOLUME_DOWN);
      break;
    case 1:
      // Serial.println("Right");
      kb.write(KEY_MEDIA_VOLUME_UP);
      break;
    default:
      break;
    }

    // ================================
    //    Button part of the encoder
    // ================================

    // Register when the button was pressed
    if (!e.getButtonStates().first && e.getButtonStates().second)
    {
      button_ms = millis();
    }

    // If time for long press is reached and button is still pressed, play next track
    if (e.getButtonStates().second && millis() - button_ms == BUTTON_HOLD_TIME && !block_hold)
    {
      // Serial.println("MEDIA NEXT");
      kb.write(KEY_MEDIA_NEXT_TRACK);

      // We do this to prevent continous skipping while button is held down
      block_hold = true;
      delay(100); // Sometimes it also sends play/pause, I suppose this is because of bouncing, delay should solve that
    }

    // If button was released and haven't sent MEDIA NEXT
    else if (e.getButtonStates().first && !e.getButtonStates().second && !block_hold)
    {
      // Serial.println("MEDIA PLAY/PAUSE");
      kb.write(KEY_MEDIA_PLAY_PAUSE);
      delay(100); // Sometimes the key is sent twice, I suppose this is because of bouncing, delay should solve that
    }

    // On button release, disable the block for the next button hold
    if (e.getButtonStates().first && !e.getButtonStates().second)
    {
      block_hold = false;
    }
  }

  // Serial.println("Disconnected!");
  digitalWrite(BUILTIN_LED, LED_state);
  if (millis() - blink_ms == LED_BLINK_FREQ)
  {
    LED_state = !LED_state;
    blink_ms = millis();
  }
}
