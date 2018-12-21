#include "LedControl.h"
#include <LiquidCrystal.h>
LiquidCrystal lcd(2, 3, 4, 5, 6, 7);

bool win = false;
bool gameOver = false;
bool final = false;

// LED matrix brightness
const short intensity = 8;

//  faster message scrolling
const short messageSpeed = 5;
// initial snake length 
const short initialSnakeLength = 1;
int maxSnakeLength = 6;
int level = 1;
int scoreWin = 0;



int snakeLength = initialSnakeLength;
int snakeSpeed = 1;
int snakeDirection = 0;
const short up     = 1;
const short right  = 2;
const short down   = 3; // 'down - 2' must be 'up'
const short left   = 4; // 'left - 2' must be 'right'


const float logarithmity = 0.4;

int age[8][8] = {};

struct Pin {
  static const short joystickX = A0;
  static const short joystickY = A1;
  static const short V0_PIN = 9;
  static const short potentiometer = 13; // potentiometer

  static const short CLK = 11;
  static const short CS  = 10;
  static const short DIN = 12;
};

LedControl matrix(Pin::DIN, Pin::CLK, Pin::CS, 1);

void setup() {

  initialize();
  showSnakeMessage();
}




void loop() {

  generateFood();    
  scanJoystick();    // watches joystick movements 
  calculateSnake();  // calculates snake parameters
  calculateScore();
  handleGameStates();
}

struct Point
{
  int row = 0, col = 0;
  Point(int row = 0, int col = 0): row(row), col(col)
  {}
};
Point snake;
Point food(-1, -1);



void generateFood()
{
  if (food.row == -1 || food.col == -1)
  {
    do
    {
      food.col = random(8);
      food.row = random(8);
    } while (age[food.row][food.col] > 0);
  }
}

float lnx(float n) {
  if (n < 0) return 0;
  if (n > 1) return 1;
  n = -log(-n * logarithmity + 1); // natural logarithm
  if (isinf(n)) n = lnx(0.999999); // prevent returning 'inf'
  return n;
}



void scanJoystick()
{

  int previousDirection = snakeDirection; // save the last direction
  long timestamp = millis() + snakeSpeed;

  while (millis() < timestamp) {
    float raw = mapf(analogRead(Pin::potentiometer), 0, 1023, 0, 1);
    snakeSpeed = mapf(lnx(raw), lnx(0), lnx(1), 10, 1000);
    if (level == 2)  snakeSpeed = snakeSpeed - 100;
    if (level == 3)  snakeSpeed = snakeSpeed - 200;


    if (snakeSpeed == 0) snakeSpeed = 1; // safety: speed can not be 0

    // determine the direction of the snake
    analogRead(Pin::joystickX) < 400 ? snakeDirection = up    : 0;
    analogRead(Pin::joystickX) > 600 ? snakeDirection = down  : 0;
    analogRead(Pin::joystickY) > 600 ? snakeDirection = left  : 0;
    analogRead(Pin::joystickY) < 400 ? snakeDirection = right : 0;


    // ignore directional change by 180 degrees (no effect for non-moving snake)
    snakeDirection + 2 == previousDirection && previousDirection != 0 ? snakeDirection = previousDirection : 0;
    snakeDirection - 2 == previousDirection && previousDirection != 0 ? snakeDirection = previousDirection : 0;
 

    matrix.setLed(0, food.row, food.col, millis() % 100 < 50 ? 1 : 0);

  }
}

void calculateSnake()
{
  switch (snakeDirection)
  {
    case up:
      snake.row--;
      fixEdge();
      matrix.setLed(0, snake.row, snake.col, 1);
      break;

    case right:
      snake.col++;
      fixEdge();
      matrix.setLed(0, snake.row, snake.col, 1);
      break;

    case down:
      snake.row++;
      fixEdge();
      matrix.setLed(0, snake.row, snake.col, 1);
      break;

    case left:
      snake.col--;
      fixEdge();
      matrix.setLed(0, snake.row, snake.col, 1);
      break;

    default: // if the snake is not moving, exit
      return;
  }
  
  if (age[snake.row][snake.col] != 0 && snakeDirection != 0)
  {
    gameOver = true;
    return;
  }

  if (snake.row == food.row && snake.col == food.col)
  {
    snakeLength++;
    food.row = -1; // reset food
    food.col = -1;
  }


  updateAges();
  age[snake.row][snake.col]++;
}
void calculateScore()
{
  if (gameOver == true)
  {

    lcd.clear();
    lcd.setCursor(1, 0);
    lcd.print("AI PIERDUT!");
    lcd.setCursor(1, 1);
    lcd.print("SCORE: ");
    lcd.setCursor(8, 1);
    lcd.print(scoreWin);
    delay(2000);
    lcd.clear();
    lcd.setCursor(1, 1);
    lcd.print("LEVEL:");
    lcd.setCursor(8, 1);
    lcd.print(level);
    delay(1500);
    lcd.clear();
    return ;
  }
  if (snakeLength == maxSnakeLength)
  {

    win = true;
    scoreWin++;
    lcd.clear();
    lcd.setCursor(1, 0);
    lcd.print("AI CASTIGAT!");
    lcd.setCursor(1, 1);
    lcd.print("SCORE:");
    lcd.setCursor(8, 1);
    lcd.print(scoreWin);
    delay(2000);
    lcd.clear();
    lcd.setCursor(1, 1);
    lcd.print("LEVEL:");
    lcd.setCursor(8, 1);
    lcd.print(level);
    delay(1500);
    lcd.clear();
    level++;

    return;
  }
  if (level == 4)
  {
    final = true;
    return;
  }
}

void fixEdge() {
  snake.col < 0 ? snake.col += 8 : 0;
  snake.col > 7 ? snake.col -= 8 : 0;
  snake.row < 0 ? snake.row += 8 : 0;
  snake.row > 7 ? snake.row -= 8 : 0;
}

void updateAges()
{
  for (int row = 0; row < 8; row++) {
    for (int col = 0; col < 8; col++) {
      if (age[row][col] > 0 ) {
        age[row][col]++;
      }
      if (age[row][col] > snakeLength) {
        matrix.setLed(0, row, col, 0);
        age[row][col] = 0;
      }
    }
  }
}


void initialize() {

  lcd.begin(16, 2);
  lcd.clear();
  pinMode(Pin::V0_PIN, OUTPUT); // PWN in loc de POTENTIOMETRU
  analogWrite(Pin::V0_PIN, 90);
  matrix.shutdown(0, false);
  matrix.setIntensity(0, intensity);
  matrix.clearDisplay(0);


  //randomSeed(analogRead(A5));
  snake.row = random(8);
  snake.col = random(8);
}


void handleGameStates() {
  if (final == true)
  {
    lcd.clear();
    lcd.setCursor(2, 1);
    lcd.print("FINAL DE JOC");
    return;
  }
  if (gameOver || win )
  {
    // lcd.clear();
    // re-init the game
    //lcd.clear()
    win = false;
    gameOver = false;
    snake.row = random(8);
    snake.col = random(8);
    food.row = -1;
    food.col = -1;
    snakeLength = initialSnakeLength;
    snakeDirection = 0;
    memset(age, 0, sizeof(age[0][0]) * 8 * 8);
    matrix.clearDisplay(0);
  }

}

float mapf(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

////////////////////////// M E S A G E /////////////////

const PROGMEM bool Message[8][56] = {
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 1, 1, 0, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0}
};





// scrolls the 'snake' message around the matrix
void showSnakeMessage() {
  for (int d = 0; d < sizeof(Message[0]) - 7; d++) {
    for (int col = 0; col < 8; col++) {
      delay(messageSpeed);
      for (int row = 0; row < 8; row++) {
        matrix.setLed(0, row, col, pgm_read_byte(&(Message[row][col + d])));
      }
    }
  }
}
