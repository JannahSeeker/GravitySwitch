#include "FastLED.h"
#include "Math.h"
CRGB leds[25];


int board[5][6] = {
  { 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0 }
};
double interval = 0;
double speedincrease = 4;
double periodinsteps = 5;
bool gravityreversed;

bool lastpressed = false;
enum GameState { GAME_START,
                 GAME_PLAYING,
                 GAME_OVER };

GameState gamestate = GAME_START;


int deleteElement(int arr[], int n, int x) {
  // Search x in array
  int i;
  for (i = 0; i < n; i++)
    if (arr[i] == x)
      break;

  // If x found in array
  if (i < n) {
    // reduce size of array and move all
    // elements on space ahead
    n = n - 1;
    for (int j = i; j < n; j++)
      arr[j] = arr[j + 1];
  }

  return n;
}
void setleds() {
  Serial.begin(9600);
  FastLED.setBrightness(40);
  FastLED.addLeds<WS2811, 4, RGB>(leds, 25).setCorrection(TypicalLEDStrip);
}
int inputchecker()  //returns the value pressed
{
  if (Serial.available() != 0) {
    int choice = Serial.parseInt();
    //Serial.print(choice);
    return choice;
  } else {
    return 0;
  }
}
bool buttonchecker() {
  return !digitalRead(13);
}
//1 is empty, 2 is bottom, 3 is top
int checkstate(int column) {
  if (board[2][column]) {
    if (board[1][column]) {
      return 3;
    } else {
      return 2;
    }
  } else {
    return 1;
  };
}

int getRandomValue(int arr[], int size) {
  int randomIndex = random(0, size); // Generate a random index within the range of the list
  return arr[randomIndex]; // Return the value at the random index
}
int checkconsecutivity() {
  //returns consectutive number if any
  if ((checkstate(2) == checkstate(3)) && (checkstate(3)==checkstate(4))) {
    return checkstate(4);
  } else {
    return 0;
  }
}

int checkstagger() {
  //returns true if there is a stagger and correct stagger value
  if (checkstate(4) == 2 || checkstate(4) == 3) {
    return (5 - checkstate(4));
  }
  return 0;
}


int randomizer(int array[]) {
  //get length
  //not a great random
  Serial.print("Choices are: ");
  Serial.print(array[0]);
  Serial.print(array[1]);
  Serial.print(array[2]);
  while (true) {
    int num;
    num = array[random(0, 3)];
    if (num) {
      // Serial.print(" Choosing:");
      // Serial.println(num);
      return num;
    }
  }
}

void PrintArray(int arr[], int size) {
  for (int i = 0; i < size; i++) {
    Serial.print(arr[i]); // Print each element
    if (i < size - 1) {
      Serial.print(", "); // Print a comma and space between elements
    }
  }
  // Serial.println(); // Move to the next line after printing the array
}
void printstates(){
  Serial.print(checkstate(0));
  Serial.print(" | ");
  Serial.print(checkstate(1));
  Serial.print(" | ");
  Serial.print(checkstate(2));
  Serial.print(" | ");
  Serial.print(checkstate(3));
  Serial.print(" | ");
  Serial.print(checkstate(4));
  Serial.print(" : ");

}
int generaterandomobstacle() {
  printstates();
  int options[] = { 1, 2, 3 };
  int n = sizeof(options) / sizeof(options[0]);

  if (checkconsecutivity()) {
    n = deleteElement(options, n, checkconsecutivity());
    // Serial.println("Consecutive");
    // PrintArray(options,n);
    // Serial.println(n);
  }
  if (checkstagger()) {
    n = deleteElement(options, n, checkstagger());
    // Serial.println("Staggered");
    // PrintArray(options,n);
    // Serial.println(n);

  }
  
  int x = getRandomValue(options,n);
  Serial.println(x);
  if (checkconsecutivity()) {
    // n = deleteElement(options, n, checkconsecutivity());
    Serial.print("Consecutive ");
    // PrintArray(options,n);
    // Serial.println(n);
  }
  if (checkstagger()) {
    // n = deleteElement(options, n, checkstagger());
    Serial.println("Staggered ");
    // PrintArray(options,n);
    // Serial.println(n);

  }
  Serial.println();
  return x;
}
void decreaseinterval() {
  // interval /= pow((1+(speedincrease/100)),(1/periodinsteps));
  // interval = interval/pow((1 + STATIC_INT(4/100)),(1/periodinsteps));
  // interval = interval/pow((1 + .04),(1/periodinsteps));

  interval = interval / pow((1 + (speedincrease / 100)), (1 / periodinsteps));
  // Serial.println(interval);
}

void shift_boardleft() {
  for (int column = 0; column < 5; column++) {

    for (int row = 0; row < 5; row++) {
      if (board[row][column] != 1) {
        board[row][column] = board[row][column + 1];
      }
    }
  }
  decreaseinterval();
  // Serial.print("Current State is:");
  // Serial.println(checkstate(3));
}
void gravity() {
  //Top row is not equal to 0
  board[0][0] = 0;

  board[4][0] = 1;
}
void reverse_gravity() {
  board[0][0] = 1;

  board[4][0] = 0;
}
bool obstacledetected() {
  if (gravityreversed) {
    if (board[0][1])  //obstacle in front of player is a number other than 0, then return!
    {
      return 1;
    } else {
      return 0;
    }
  } else {
    if (board[4][1])  //obstacle in front of player is a number other than 0, then return!
    {
      return 1;
    } else {
      return 0;
    }
  }
}
bool spacefree() {
  if (!gravityreversed) {
    if (!board[0][0])  //obstacle in front of player is a number other than 0, then return!
    {
      return 1;
    } else {
      return 0;
    }
  } else {
    if (!board[4][0])  //obstacle in front of player is a number other than 0, then return!
    {
      return 1;
    } else {
      return 0;
    }
  }
}
void generate_bottom_obstacle() {
  board[3][5] = 2;
  board[2][5] = 2;
  board[4][5] = 2;
}
void generate_void() {

  board[0][5] = 0;
  board[1][5] = 0;
  board[2][5] = 0;
  board[3][5] = 0;
  board[4][5] = 0;
}
void generate_top_obstacle() {
  board[0][5] = 3;
  board[1][5] = 3;
  board[2][5] = 3;
}
void printboard()  //prints the current board.
{
  // Serial.println("-----------");
  // for (int r = 0; r < 5; r++) {
  //   Serial.print("[");
  //   for (int c = 0; c < 5; c++) {
  //     Serial.print(board[r][c]);
  //     if (c < 4) {
  //       Serial.print("|");
  //     }
  //   }
  //   Serial.println("]");
  // }
  // Serial.println("-----------");
  showleds();
}
void switchgravity() {
  //Two states, Jump or not_jumping. (Which will set the indexes accordingly, based on the state)
  //If player decides to trigger jump:
  if (!gravityreversed) {
    reverse_gravity();
    gravityreversed = true;
  } else {
    gravity();
    gravityreversed = false;
  }
}
void resetboard() {
  for (int i = 0; i < 5; ++i) {
    for (int j = 0; j < 6; ++j) {
      board[i][j] = 0;
    }
  }
}
void endgame() {

  gamestate = GAME_OVER;
}



void showleds()  //shows the leds
{

  int l[5][5] = {
    { 20, 21, 22, 23, 24 },
    { 19, 18, 17, 16, 15 },
    { 10, 11, 12, 13, 14 },
    { 9, 8, 7, 6, 5 },
    { 0, 1, 2, 3, 4 }

  };
  for (int r = 0; r < 5; r++)  //r= row
  {
    for (int c = 0; c < 5; c++) {
      if (board[r][c] == 0)  //Black
      {
        leds[l[r][c]] = CRGB::Black;
      }
      if (board[r][c] == 1)  //red
      {
        leds[l[r][c]] = CRGB::Red;
      }
      if (board[r][c] == 2)  //red
      {
        leds[l[r][c]] = CRGB::Blue;
      }
      if (board[r][c] == 3)  //red
      {
        leds[l[r][c]] = CRGB::Orange;
      }
      if (board[r][c] == 3)  //red
      {
        leds[l[r][c]] = CRGB::Yellow;
      }
      if (board[r][c] == 4)  //red
      {
        leds[l[r][c]] = CRGB::Pink;
      }
      if (board[r][c] == 5)  //red
      {
        leds[l[r][c]] = CRGB::Purple;
      }
      if (board[r][c] == 6)  //red
      {
        leds[l[r][c]] = CRGB::Aquamarine;
      }
      if (board[r][c] == 7)  //celebration!!
      {
        leds[l[r][c]] = CRGB::Green;
      }
      if (board[r][c] == 7)  //celebration!!
      {
        leds[l[r][c]] = CRGB::MediumSpringGreen;
      }
    }
  }

  FastLED.show();

  //FastLED.addLeds<WS2811, 4, RGB>(sleds, 25).setCorrection(TypicalLEDStrip);
}
void initalizegame() {
  interval = 1;
  gravityreversed = false;
  resetboard();
  gravity();
}
void gameloop() {
  generateobstacle(generaterandomobstacle());
  printboard();
  long currenttime = millis();
  while (millis() < currenttime + (interval * 1000))  //1 second
  {
    if (!lastpressed) {
      lastpressed = buttonchecker();
      if (lastpressed && spacefree()) {
        switchgravity();
        printboard();
      }
    }
    lastpressed = buttonchecker();
  }
  if (obstacledetected()) {
    endgame();
  }
  shift_boardleft();
}


void generateobstacle(int state) {
  switch (state) {
    case 1:
      generate_void();
      break;
    case 2:
      generate_bottom_obstacle();
      break;
    case 3:
      generate_top_obstacle();
      break;
  }
}

void setup() {
  pinMode(10, INPUT_PULLUP);
  setleds();
  Serial.begin(9600);
}
void loop() {
  switch (gamestate) {
    case GAME_START:

      initalizegame();
      gamestate = GAME_PLAYING;
      break;

    case GAME_PLAYING:
      gameloop();
      break;

    case GAME_OVER:
      Serial.println("GameOver!");
      gamestate = GAME_START;
      break;
  }
}