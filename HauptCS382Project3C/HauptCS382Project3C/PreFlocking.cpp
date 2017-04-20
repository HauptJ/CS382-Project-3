/********************************************************************/
/* Filename: PreFlocking.cpp                                        */
/*                                                                  */
/* This program generates a large 2D system of delta-shaped "ships" */
/* that float on an empty background. The user generates disruptive */
/* "ripples" that serve as predators for the ships. These ripples,  */
/* like the ships, are based on color, with a ripple only affecting */
/* like-colored ships. The exceptions are "invisible" ripples that  */
/* affect all particles.                                            */
/*                                                                  */
/* The user may alter the color of a new ripple (or make the ripple */
/* invisible) via keyboard operations, and the "beep" accompanying  */
/* each new ripple's creation has a frequency that depends upon its */
/* color. Each ripple emanates from the point of generation and     */
/* dissipates in visibility and intensityas it expands.             */
/********************************************************************/

#include <gl/freeglut.h>
#include <cmath>			// Header File For Math Library
#include <ctime>			// Header File For Accessing System Time
#include "LinkedList.h"		// Header File For Linked List Class       //
#include <cstring>			// Header File For String Operations       //
#include <iostream>			// Header File For Initial User Input from console
using namespace std;

//////////////////////
// Global Constants //
//////////////////////
const int   INIT_WINDOW_POSITION[2]		= { 50, 50 };			// Window Offset       //
const float INITIAL_RADIUS				= 0.0f;					// Init. Ripple Radius //
const float FINAL_RADIUS				= 0.5f;					// Final Ripple Radius //
const float RADIUS_INCREMENT			= 0.01f;					// Rad. Expansion Rate //
const int   NBR_LINKS					= 25;					// Polygonal Circle    //
const int	NBR_SHIPS					= 1000;					// # Of Ships          //
const float PI_OVER_180					= 0.0174532925f;			// 1 Degree in Radians //
const int   BEEP_DURATION				= 25;					// # msec Ripple Beep  //
const int   BEEP_FREQUENCY[8]			= { 500, 1000, 1500,	// All Ripple Beep     //
											2000, 2500, 3000,	// Frequencies (in     //
											3500, 5000 };		// hertz)              //
const int   NBR_COLORS					= 7;					// # Point Colors      //
const float CIRCLE_COLOR[NBR_COLORS][3]	= { { 1.0f, 1.0f, 1.0f },	// All Ripple Colors   //
											{ 1.0f, 0.3f, 0.3f },
											{ 1.0f, 1.0f, 0.3f },
											{ 0.3f, 1.0f, 0.3f },
											{ 0.3f, 1.0f, 1.0f },
											{ 0.3f, 0.3f, 1.0f },
											{ 1.0f, 0.3f, 1.0f } };
const float SHIP_RADIUS					= 0.02f;
const float SHIP_THICKNESS				= 2.0f;
const float MIN_SHIP_DELTA				= -0.0001f;				// Ship Trajectory's   //
const float MAX_SHIP_DELTA				=  0.0001f;				// Lower, Upper Bounds //
const float VECTOR_SIZE					= 0.01f;
const char  DEFAULT_TITLE[]				= "MOUSE: RIPPLES; KEYBOARD: COLORS (wrygcbmn)";

//NEW

enum color { white, red, yellow, green, cyan, blue, magenta, none };	// Color Index Values //

////////////////////////////////////////
// 2D ripple class (for convenience). //
////////////////////////////////////////
class Ripple
{
	public:
		float pos[2];	// 2-D position of circle's center //
		float rad;		// Radius of circle                //
		color clr;		// Initial color of circle         //

		// The draw member function renders the circle at its current position, //
		// with its current radius, and colored to dissipate as it expands.     //
		void draw()
		{
			int i;
			float theta;

			if (clr != none)	// Draw nothing if the circle is "invisible". //
			{
				float intensity = (FINAL_RADIUS - rad) / (FINAL_RADIUS - INITIAL_RADIUS);
				float currColor[3] = { intensity * CIRCLE_COLOR[int(clr)][0],
										intensity * CIRCLE_COLOR[int(clr)][1],
										intensity * CIRCLE_COLOR[int(clr)][2] };
				float thickness = 3.0f * intensity;
				glColor3fv(currColor);
				glLineWidth(thickness);

				// Draw a polygonal approximation to the circle. //
				glBegin(GL_LINES);
				for (i = 1; i <= NBR_LINKS; i++)
					{
						theta = 360 * i * PI_OVER_180 / NBR_LINKS;
						glVertex2f(pos[0] + rad * cos(theta), pos[1] + rad * sin(theta));
						theta = 360 * (i + 1) * PI_OVER_180 / NBR_LINKS;
						glVertex2f(pos[0] + rad * cos(theta), pos[1] + rad * sin(theta));
					}
				glEnd();
			}
		}
};

//////////////////////////////////////
// 2D ship class (for convenience). //
//////////////////////////////////////
class Ship
{
	public:
		float pos[2];	// 2-D position of flocker      //
		float delta[2];	// Trajectory vector of flocker //
		color clr;		// Color of flocker             //

		//NEW for randomly generatoed velocity

		float speed;		// Flocker speed

		float xInc;           // Flocker's motion increment in x-dimension.           //
		float yInc;           // Flocker's motion increment in y-dimension.           //


		void draw()
		{
			float theta = atan2(delta[1], delta[0]);
			float currColor[3] = { CIRCLE_COLOR[int(clr)][0],
									CIRCLE_COLOR[int(clr)][1],
									CIRCLE_COLOR[int(clr)][2] };
			glColor3fv(currColor);
			glLineWidth(SHIP_THICKNESS);

			// Draw a delta-shaped representation of the ship. //
			glBegin(GL_TRIANGLE_FAN);
				glVertex2f(pos[0] + SHIP_RADIUS * cos(theta), pos[1] + SHIP_RADIUS * sin(theta));
				theta += 120 * PI_OVER_180;
				glVertex2f(pos[0] + SHIP_RADIUS * cos(theta), pos[1] + SHIP_RADIUS * sin(theta));
				glVertex2f(pos[0], pos[1]);
				theta += 120 * PI_OVER_180;
				glVertex2f(pos[0] + SHIP_RADIUS * cos(theta), pos[1] + SHIP_RADIUS * sin(theta));
			glEnd();
		}
};

//////////////////////
// Global Variables //
//////////////////////
int currWindowSize[2]	= { 800, 800 };	// Window size in pixels.          //
float windowWidth		= 2.0f;			// Resized window width.           //
float windowHeight		= 2.0f;			// Resized window height.          //
LinkedList<Ripple> circleList;			// Linked list of ripple circles.  //
LinkedList<Ship>   shipList;			// Linked list of ships.           //
color currColor			= none;			// Current new ripple color.       //

//NEW
//Mulipliers
int G_CohesionMultiplier = 0;
int G_AllignmentMultiplier = 0;
int G_SeperationMultiplier = 0;


/////////////////////////
// Function Prototypes //
/////////////////////////
void SetCaption();
void MouseClick(int mouseButton, int mouseState, int mouseXPosition, int mouseYPosition);
void KeyboardPress(unsigned char pressedKey, int mouseXPosition, int mouseYPosition);
void TimerFunction(int value);
void DisplaceShips();
void Display();
void InitShips();
void Normalize(float vector[]);
void ResizeWindow(GLsizei w, GLsizei h);
//NEW
//Movements
//3 Movements
void CohesionShips();
void AllignmentShips();
void SeperationShips();

//Title
void ConvertToCharacterArray(int value, char valueArray[]);
void UpdateTitleBar();

//Random number generator
float GenerateRandomNumber(float lowerBound, float upperBound);

//Keep ships inside window
void AdjustToWindow(Ship &currentShp);


/* The main function: uses the OpenGL Utility Toolkit to set */
/* the window up to display the window and its contents.     */
void main(int argc, char **argv)
{
	/* Set up the display window. */
	glutInit(&argc, argv);
	//Get initial cohesion, allignment, and seperation values from console
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(INIT_WINDOW_POSITION[0], INIT_WINDOW_POSITION[1]);
	glutInitWindowSize(currWindowSize[0], currWindowSize[1]);
	glutCreateWindow( DEFAULT_TITLE );
	InitShips();
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	/* Specify the resizing, refreshing, and interactive routines. */
	glutReshapeFunc( ResizeWindow );
	glutDisplayFunc( Display );
	glutMouseFunc( MouseClick );
	glutKeyboardFunc( KeyboardPress );
	glutTimerFunc( 20, TimerFunction, 1 );
	glutMainLoop();
}

/* Function to react to mouse clicks by generating a new circular */
/* ripple centered at the current vertex, using the current color */
/* and accompanied by a audio "beep" of the current frequency.    */
void MouseClick(int mouseButton, int mouseState, int mouseXPosition, int mouseYPosition)
{
	Ripple currCircle;
	float x = windowWidth * mouseXPosition / currWindowSize[0] - 0.5f * windowWidth;
	float y = 0.5f * windowHeight - (windowHeight * mouseYPosition / currWindowSize[1]);

	if ( mouseState == GLUT_DOWN )
	{
		currCircle.pos[0] = x;
		currCircle.pos[1] = y;
		currCircle.rad = INITIAL_RADIUS;
		currCircle.clr = currColor;
		circleList.insert( currCircle );
		Beep( BEEP_FREQUENCY[int(currColor)], BEEP_DURATION );
	}
	Display();
}


/* Function to react to the pressing of keyboard keys by the user, */
/* by changing the default color of newly generated ripples.       */
void KeyboardPress(unsigned char pressedKey, int mouseXPosition, int mouseYPosition)
{
	switch(pressedKey)
	{
		case 'w':
		case 'W': { currColor = white;		break; }
		case 'r':
		case 'R': { currColor = red;		break; }
		case 'y':
		case 'Y': { currColor = yellow;		break; }
		case 'g':
		case 'G': { currColor = green;		break; }
		case 'c':
		case 'C': { currColor = cyan;		break; }
		case 'b':
		case 'B': { currColor = blue;		break; }
		case 'm':
		case 'M': { currColor = magenta;	break; }
		case 'n':
		case 'N': { currColor = none;		break; }

		//NEW
		//User cohesion manipulation
		case 'k': //Decrement
		{ 
			if (G_CohesionMultiplier > 0)
			{
				G_CohesionMultiplier--;
			}
			else {
				G_CohesionMultiplier = 0;
			}
			break; 
		}
		case 'K': { G_CohesionMultiplier++;		break; } //Increment
		
		//User allignment manipulation
		case 'a': //Decrement
		{
			if (G_AllignmentMultiplier > 0)
			{
				G_AllignmentMultiplier--;
			}
			else {
				G_AllignmentMultiplier = 0;
			}
			break;
		}
		case 'A': { G_AllignmentMultiplier++;		break; } //Increment

		//User seperation manipulation
		case 's': //Decrement
		{
			if (G_SeperationMultiplier > 0)
			{
				G_SeperationMultiplier--;
			}
			else {
				G_SeperationMultiplier = 0;
			}
			break;
		}
		case 'S': { G_SeperationMultiplier++;		break; } //Increment
	}
}


/* Function to update the expanding radius values of all       */
/* current ripples, removing those that exceed a certain size. */
/* This function also activates the point displacement.        */
void TimerFunction(int value)
{
	int i;
	Ripple currCircle;

	Ship currShp;

	for (i = 1; i <= shipList.getSize(); i++)
	{
		currShp = shipList.getHeadValue();
		//shipList.removeHead();

		//3 Movements - causes program to freeze
		//CohesionShips();
		//AllignmentShips();
		//SeperationShips();

		// Update ship position //
		currShp.pos[0] += currShp.xInc;
		currShp.pos[1] += currShp.yInc;

		AdjustToWindow(currShp);

		++shipList;
	}

	for (i = 1; i <= circleList.getSize(); i++)
	{
		currCircle = circleList.getHeadValue();
		circleList.removeHead();
		currCircle.rad += RADIUS_INCREMENT;
		if (currCircle.rad < FINAL_RADIUS)
			circleList.insert( currCircle );
		++circleList;
	}


	DisplaceShips();
	//3 Movements - works, but not like it should
	CohesionShips();
	AllignmentShips();
	SeperationShips();

	UpdateTitleBar();

	// Force a redraw after 20 milliseconds. //
	glutPostRedisplay();
	glutTimerFunc( 20, TimerFunction, 1 );
}

/* Function to adjust the position of the parameterized ship to ensure */
/* that the ship remains inside the boundaries of the display window.  */
void AdjustToWindow(Ship &currentShp)
{
	bool tooHigh, tooLow, tooLeft, tooRight;
	GLfloat x, y;

	// Determine whether ship exceeds window boundaries. //
	tooHigh = tooLow = tooLeft = tooRight = false;
	for (int i = 0; i < shipList.getSize(); i++)
	{
		x = currentShp.pos[0] * SHIP_RADIUS;
		y = currentShp.pos[1] * SHIP_RADIUS;
		if (x > windowWidth / 2.0)
			tooRight = true;
		else if (x < -windowWidth / 2.0)
			tooLeft = true;
		if (y > windowHeight / 2.0)
			tooHigh = true;
		else if (y < -windowHeight / 2.0)
			tooLow = true;
	}

	// Adjust position if window bounds exceeded. //
	if (tooRight)
	{
		currentShp.xInc *= -1.0f;
		currentShp.pos[0] = windowWidth / 2.0f - SHIP_RADIUS;
	}
	else if (tooLeft)
	{
		currentShp.xInc *= -1.0f;
		currentShp.pos[0] = windowWidth / 2.0f + SHIP_RADIUS;
	}
	if (tooHigh)
	{
		currentShp.yInc *= -1.0f;
		currentShp.pos[1] = windowWidth / 2.0f - SHIP_RADIUS;
	}
	else if (tooLow)
	{
		currentShp.yInc *= -1.0f;
		currentShp.pos[1] = windowWidth / 2.0f + SHIP_RADIUS;
	}
}

/* Function to cycle through the ships and determine whether */
/* any ripple is encapsulating a ship's center. If so, the   */
/* ship's position is modified to reflect the displacement   */
/* caused by the emanating ripple.                           */
void DisplaceShips()
{
	int i, j;
	Ship shp;
	Ripple cir;
	float intensity;

	for (i = 1; i <= shipList.getSize(); i++)
	{
		shp = shipList.getHeadValue();
		shipList.removeHead();
		for (j = 1; j <= circleList.getSize(); j++)
		{
			cir = circleList.getHeadValue();

			// If the flocker in question is the same color as the ripple, //
			// or if the ripple is invisible, then displace the flocker.   //
			if ( (cir.clr == none) || (cir.clr == shp.clr) )
				if ( pow(cir.pos[0] - shp.pos[0], 2) + pow(cir.pos[1] - shp.pos[1], 2) < pow(cir.rad, 2) )
				{
					// The flocker's current position is altered by a vector //
					// in the direction of the ripple's emanation, scaled    //
					// to be inversely proportional to the ripple's current  //
					// size, to represent the ripple's dissipation.          //
					intensity = 0.05f * (FINAL_RADIUS - cir.rad) / (FINAL_RADIUS - INITIAL_RADIUS);
					shp.delta[0] += intensity * (shp.pos[0] - cir.pos[0]);
					shp.delta[1] += intensity * (shp.pos[1] - cir.pos[1]);
					shp.pos[0] += intensity * (shp.pos[0] - cir.pos[0]);
					shp.pos[1] += intensity * (shp.pos[1] - cir.pos[1]);
				}
			++circleList;
		}
		Normalize(shp.delta);
		shipList.insert( shp );
		++shipList;
	}
}


/* Function to cycle through the ships and determine whether */
/* any ripple is encapsulating a ship's center. If so, the   */
/* ship's position is modified to reflect the displacement   */
/* caused by the emanating ripple.                           */
void CohesionShips()
{
	int i, j;
	Ship shp;
	Ripple cir;
	//Ship cir;
	//float intensity;
	float sumX = 0;
	float sumY = 0;
	int tally = 0; //number of ships in area

	for (i = 1; i <= shipList.getSize(); i++)
	{
		shp = shipList.getHeadValue();
		shipList.removeHead();
		for (j = 1; j <= circleList.getSize(); j++)
		{
			cir = circleList.getHeadValue();

			if (pow(cir.pos[0] - shp.pos[0], 2) + pow(cir.pos[1] - shp.pos[1], 2) < pow(cir.rad, 2))
			{
				sumX += shp.pos[0];
				sumY += shp.pos[1];

				shp.pos[0] = (sumX / tally) * G_CohesionMultiplier;
				shp.pos[1] = (sumY / tally) * G_CohesionMultiplier;
			}
		++circleList;
		//tally = circleList.getSize(); //total number of shits in the circle.
		++tally;
		}
		Normalize(shp.delta);
		shipList.insert(shp);
		++shipList;
	}
}

/* Function to cycle through the ships and determine whether */
/* any ripple is encapsulating a ship's center. If so, the   */
/* ship's position is modified to reflect the displacement   */
/* caused by the emanating ripple.                           */
void AllignmentShips()
{
	int i, j;
	Ship shp;
	Ripple cir;
	//float intensity;
	float sumDeltaX = 0;
	float sumDeltaY = 0;
	int tally = 0; //number of ships in area

	for (i = 1; i <= shipList.getSize(); i++)
	{
		shp = shipList.getHeadValue();
		shipList.removeHead();
		for (j = 1; j <= circleList.getSize(); j++)
		{
			cir = circleList.getHeadValue();

			if (pow(cir.pos[0] - shp.pos[0], 2) + pow(cir.pos[1] - shp.pos[1], 2) < pow(cir.rad, 2))
			{
				sumDeltaX += shp.delta[0];
				sumDeltaY += shp.delta[1];

				shp.pos[0] = (sumDeltaX / tally) * G_AllignmentMultiplier;
				shp.pos[1] = (sumDeltaY / tally) * G_AllignmentMultiplier;
			}
			++circleList;
			//tally = circleList.getSize(); //total number of shits in the circle.
			++tally;
		}
		Normalize(shp.delta);
		shipList.insert(shp);
		++shipList;
	}
}

/* Function to cycle through the ships and determine whether */
/* any ripple is encapsulating a ship's center. If so, the   */
/* ship's position is modified to reflect the displacement   */
/* caused by the emanating ripple.                           */
void SeperationShips()
{
	int i, j;
	Ship shp;
	Ripple cir;
	//float intensity;
	float minX = 0;
	float minY = 0;
	//int tally; //number of ships in area

	for (i = 1; i <= shipList.getSize(); i++)
	{
		shp = shipList.getHeadValue();
		shipList.removeHead();
		for (j = 1; j <= circleList.getSize(); j++)
		{
			cir = circleList.getHeadValue();

			if (pow(cir.pos[0] - shp.pos[0], 2) + pow(cir.pos[1] - shp.pos[1], 2) < pow(cir.rad, 2))
			{
				minX = shp.pos[0];
				minY = shp.pos[1];

				shp.pos[0] = minX * G_SeperationMultiplier;
				shp.pos[1] = minY * G_SeperationMultiplier;
			}
			++circleList;
			//tally = circleList.getSize(); //total number of shits in the circle.
			//++tally;
		}
		Normalize(shp.delta);
		shipList.insert(shp);
		++shipList;
	}
}

// Convert the parameterized integer value into the
// corresponding character array representing that value.
void ConvertToCharacterArray(int value, char valueArray[])
{
	char digitStr[4] = "0";
	if (value < 10)
	{
		digitStr[0] = char(value + int('0'));
		strcat_s(valueArray, 4, digitStr);
	}
	else
	{
		ConvertToCharacterArray(value / 10, valueArray);
		digitStr[0] = char(value % 10 + int('0'));
		strcat_s(valueArray, 4, digitStr);
	}
}

/* Function to update the window title bar to indicate */
/* the cohesion, allignment and seperation multipliers */
void UpdateTitleBar()
{
	char label[100] = "Values: ";

	char CohesionLabel[15] = "";
	ConvertToCharacterArray(G_CohesionMultiplier, CohesionLabel);
	strcat_s(label, 100, CohesionLabel);
	strcat_s(label, 100, " Cohesion: ");

	char AllignmentLabel[15] = "";
	ConvertToCharacterArray(G_AllignmentMultiplier, AllignmentLabel);
	strcat_s(label, 100, AllignmentLabel);
	strcat_s(label, 100, " Allignment ");

	char SeperationLabel[15] = "";
	ConvertToCharacterArray(G_SeperationMultiplier, SeperationLabel);
	strcat_s(label, 100, " Seperation: ");
	strcat_s(label, 100, SeperationLabel);

	glutSetWindowTitle(label);
}

/* Principal display routine: clears the frame buffer and */
/* draws the ripples and the ships within the window.     */
void Display()
{
	int i;
	Ripple currCircle;
	Ship shp;

	glClear( GL_COLOR_BUFFER_BIT );

	for (i = 1; i <= circleList.getSize(); i++)
	{
		currCircle = circleList.getHeadValue();
		currCircle.draw();
		++circleList;
	}

	for (i = 1; i <= shipList.getSize(); i++)
	{
		shp = shipList.getHeadValue();
		shp.draw();
		++shipList;
	}

	//3 Movements
	//CohesionShips();
	//AllignmentShips();
	//SeperationShips();

	glutSwapBuffers();
	glFlush();
}

/* Random Number Generator */
float GenerateRandomNumber(float lowerBound, float upperBound)
{
	static bool firstTime = true;
	static time_t randomNumberSeed;
	if (firstTime)
	{
		time(&randomNumberSeed);
		firstTime = false;
		srand(unsigned int(randomNumberSeed));
	}
	return (lowerBound + ((upperBound - lowerBound) * (float(rand()) / RAND_MAX)));
}

/* Random generation of the ships within the window.  */
/* The color of each ship is also randomly generated. */
void InitShips()
{
	Ship shp;
	time_t randomNumberSeed;
	time(&randomNumberSeed);
	srand((unsigned int)randomNumberSeed);

	for (int i = 1; i <= NBR_SHIPS; i++)
	{
		shp.pos[0] = windowWidth * (float(rand()) / RAND_MAX - 0.5f);
		shp.pos[1] = windowHeight * (float(rand()) / RAND_MAX - 0.5f);
		shp.delta[0] = MIN_SHIP_DELTA + (float(rand()) / RAND_MAX) * (MAX_SHIP_DELTA - MIN_SHIP_DELTA);
		shp.delta[1] = MIN_SHIP_DELTA + (float(rand()) / RAND_MAX) * (MAX_SHIP_DELTA - MIN_SHIP_DELTA);
		Normalize(shp.delta);
		shp.clr = color(rand() % NBR_COLORS);

		//NEW random velocity
		shp.speed = GenerateRandomNumber(0.010f, 0.045f); // random speed
		shp.xInc = GenerateRandomNumber(shp.speed / 4.0, shp.speed);
		shp.yInc = sqrt(shp.speed * shp.speed - shp.xInc * shp.xInc);
		float randNbr = GenerateRandomNumber(-1.0, 1.0);
		if (randNbr < 0.0f)
			shp.xInc *= -1.0f;
		randNbr = GenerateRandomNumber(-1.0, 1.0);
		if (randNbr < 0.0f)
			shp.yInc *= -1.0f;

		shipList.insert(shp);
	}
}


/* Normalize the parameterized vector. */
void Normalize(float vector[])
{
	float size = sqrt( pow(vector[0], 2) + pow(vector[1], 2) );
	if (size > 0.0f)
		for (int i = 0; i <= 1; i++)
			vector[i] *= (VECTOR_SIZE / size);
}


/* Window-reshaping routine, to scale the rendered scene according */
/* to the window dimensions, setting the global variables so the   */
/* mouse operations will correspond to mouse pointer positions.    */
void ResizeWindow(GLsizei w, GLsizei h)
{
	glViewport( 0, 0, w, h );
	currWindowSize[0] = w;
	currWindowSize[1] = h;
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    if (w <= h)
	{
		windowWidth = 2.0f;
		windowHeight = 2.0f * (GLfloat)h / (GLfloat)w;
        glOrtho(-1.0f, 1.0f, -1.0f * (GLfloat)h / (GLfloat)w, (GLfloat)h / (GLfloat)w, -10.0f, 10.0f);
	}
    else
	{
		windowWidth = 2.0f * (GLfloat)w / (GLfloat)h;
		windowHeight = 2.0f;
        glOrtho(-1.0f * (GLfloat)w / (GLfloat)h, (GLfloat)w / (GLfloat)h, -1.0f, 1.0f, -10.0f, 10.0f);
	}
    glMatrixMode( GL_MODELVIEW );
}
