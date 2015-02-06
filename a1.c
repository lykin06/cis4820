
/* Derived from scene.c in the The OpenGL Programming Guide */
/* Keyboard and mouse rotation taken from Swiftless Tutorials #23 Part 2 */
/* http://www.swiftless.com/tutorials/opengl/camera2.html */

/* Frames per second code taken from : */
/* http://www.lighthouse3d.com/opengl/glut/index.php?fps */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "perlin.h"
#include "graphics.h"

	/* mouse function called by GLUT when a button is pressed or released */
void mouse(int, int, int, int);

	/* initialize graphics library */
extern void graphicsInit(int *, char **);

	/* lighting control */
extern void setLightPosition(GLfloat, GLfloat, GLfloat);
extern GLfloat* getLightPosition();

	/* viewpoint control */
extern void setViewPosition(float, float, float);
extern void getViewPosition(float *, float *, float *);
extern void getOldViewPosition(float *, float *, float *);
extern void getViewOrientation(float *, float *, float *);

	/* add cube to display list so it will be drawn */
extern int addDisplayList(int, int, int);

	/* mob controls */
extern void createMob(int, float, float, float, float);
extern void setMobPosition(int, float, float, float, float);
extern void hideMob(int);
extern void showMob(int);

	/* player controls */
extern void createPlayer(int, float, float, float, float);
extern void setPlayerPosition(int, float, float, float, float);
extern void hidePlayer(int);
extern void showPlayer(int);

	/* flag which is set to 1 when flying behaviour is desired */
extern int flycontrol;
	/* flag used to indicate that the test world should be used */
extern int testWorld;
	/* flag to print out frames per second */
extern int fps;
	/* flag to indicate removal of cube the viewer is facing */
extern int dig;
	/* flag indicates the program is a client when set = 1 */
extern int netClient;
	/* flag indicates the program is a server when set = 1 */
extern int netServer; 

	/* frustum corner coordinates, used for visibility determination  */
extern float corners[4][3];

	/* determine which cubes are visible e.g. in view frustum */
extern void ExtractFrustum();
extern void tree(float, float, float, float, float, float, int);

/********* end of extern variable declarations **************/

int update_clouds = 1;

/************** end variable declarations **************/


	/*** collisionResponse() ***/
	/* -performs collision detection and response */
	/*  sets new xyz  to position of the viewpoint after collision */
	/* -can also be used to implement gravity by updating y position of vp*/
	/* note that the world coordinates returned from getViewPosition()
	   will be the negative value of the array indices */
void collisionResponse() {
	int x, y, z;
	float *pos_x, *pos_y, *pos_z;
	float *old_x, *old_y, *old_z;
	
	pos_x = malloc(sizeof(float));
	pos_y = malloc(sizeof(float));
	pos_z = malloc(sizeof(float));
	old_x = malloc(sizeof(float));
	old_y = malloc(sizeof(float));
	old_z = malloc(sizeof(float));
	
	getOldViewPosition(old_x, old_y, old_z);
	getViewPosition(pos_x, pos_y, pos_z);

	// Rounded position
	x = ((int) *pos_x) * (-1);
	y = ((int) *pos_y) * (-1);
	z = ((int) *pos_z) * (-1);
		
	printf("Position x=%d, y=%d, z=%d\n", x, y, z);

	// No need to affects gravity nor collsions with blocks on the
	// player if he is on the side of the map or flying
	if(flycontrol == 0) {
		// Collision
		if(world[x][y][z] > 0) {
			// Jump one block
			if(world[x][y+1][z] == 0) {
				*pos_y -= 1.0;
			} else {
				// Reverse the movement	
				*pos_x = *old_x;
				*pos_y = *old_y;
				*pos_z = *old_z;
			}
			
			setViewPosition(*pos_x, *pos_y, *pos_z);

			free(pos_x);
			free(pos_y);
			free(pos_z);
			free(old_x);
			free(old_y);
			free(old_z);
			
			return;
		}
	
		// Gravity
		if(*pos_y < -0.5) {
			if((world[x][y - 1][z] == 0) || (world[x][y - 1][z] == 5)) {
				*pos_y += 0.1;
			} else {
				// Fix the y view point position
				*pos_y = (-1) * y - 0.5;
			}
		}
	}
	
	// Colision with the sides of the map
	// Taking .5 values to be able to go at the midle of the cubes on the sides
	if(*pos_x < -99.5) 	{*pos_x = -99.5;}
	if(*pos_x > -0.5) 	{*pos_x = -0.5;}
	if(*pos_z < -99.5) 	{*pos_z = -99.5;}
	if(*pos_z > -0.5)	{*pos_z = -0.5;}
	if(*pos_y < -49.5) 	{*pos_y = -49.5;}
	if(*pos_y > -0.5) 	{*pos_y = -0.5;}

	setViewPosition(*pos_x, *pos_y, *pos_z);

	free(pos_x);
	free(pos_y);
	free(pos_z);
	free(old_x);
	free(old_y);
	free(old_z);
}


	/*** update() ***/
	/* background process, it is called when there are no other events */
	/* -used to control animations and perform calculations while the  */
	/*  system is running */
	/* -gravity must also implemented here, duplicate collisionResponse */
void update() {
	int i, j, x, y, z;
	float *pos_x, *pos_y, *pos_z;
	int clouds[WORLDZ];

	/* sample animation for the test world, don't remove this code */
	/* -demo of animating mobs */
   if (testWorld) {
	/* sample of rotation and positioning of mob */
	/* coordinates for mob 0 */
      static float mob0x = 50.0, mob0y = 25.0, mob0z = 52.0;
      static float mob0ry = 0.0;
      static int increasingmob0 = 1;
	/* coordinates for mob 1 */
      static float mob1x = 50.0, mob1y = 25.0, mob1z = 52.0;
      static float mob1ry = 0.0;
      static int increasingmob1 = 1;

	/* move mob 0 and rotate */
	/* set mob 0 position */
      setMobPosition(0, mob0x, mob0y, mob0z, mob0ry);

	/* move mob 0 in the x axis */
      if (increasingmob0 == 1)
         mob0x += 0.2;
      else 
         mob0x -= 0.2;
      if (mob0x > 50) increasingmob0 = 0;
      if (mob0x < 30) increasingmob0 = 1;

	/* rotate mob 0 around the y axis */
      mob0ry += 1.0;
      if (mob0ry > 360.0) mob0ry -= 360.0;

	/* move mob 1 and rotate */
      setMobPosition(1, mob1x, mob1y, mob1z, mob1ry);

	/* move mob 1 in the z axis */
	/* when mob is moving away it is visible, when moving back it */
	/* is hidden */
      if (increasingmob1 == 1) {
         mob1z += 0.2;
         showMob(1);
      } else {
         mob1z -= 0.2;
         hideMob(1);
      }
      if (mob1z > 72) increasingmob1 = 0;
      if (mob1z < 52) increasingmob1 = 1;

	/* rotate mob 1 around the y axis */
      mob1ry += 1.0;
      if (mob1ry > 360.0) mob1ry -= 360.0;
    /* end testworld animation */
   } else {
		pos_x = malloc(sizeof(float));
		pos_y = malloc(sizeof(float));
		pos_z = malloc(sizeof(float));
	
		getViewPosition(pos_x, pos_y, pos_z);

		x = ((int) *pos_x) * (-1);
		y = ((int) *pos_y) * (-1);
		z = ((int) *pos_z) * (-1);

		// Gravity
		// No need to affects gravity on the player if he is on the side of the map or flying
		if(flycontrol == 0) {
			if(*pos_y < -0.5) {
				if((world[x][y - 1][z] == 0) || (world[x][y - 1][z] == 5)) {
					setViewPosition(*pos_x, *pos_y + 0.1, *pos_z);
				}
			}
		}
		
		// Clouds
		if(update_clouds == 30) {
			for(i=0; i<WORLDZ; i++)
				clouds[i] = 0;
			
			// Update the position of the clouds
			for(i = 0; i < 99; ++i) {
				for(j = 0; j < 100; ++j) {
					if (i == 0) {
						// Save the clouds before overwriting
						clouds[j] = world[0][49][j];
					}
					world[i][49][j] = world[i + 1][49][j];
				}
			}
			
			// Rewrite the clouds to do a cycle
			for(i=0; i<WORLDZ; i++) {
				world[99][49][i] = clouds[i];
			}
			
			// Reset the counter
			update_clouds = 1;
		} else {
			// Increase the counter
			++update_clouds;
		}
		
		free(pos_x);
		free(pos_y);
		free(pos_z);
   }
}


	/* called by GLUT when a mouse button is pressed or released */
	/* -button indicates which button was pressed or released */
	/* -state indicates a button down or button up event */
	/* -x,y are the screen coordinates when the mouse is pressed or */
	/*  released */ 
void mouse(int button, int state, int x, int y) {

   if (button == GLUT_LEFT_BUTTON)
      printf("left button - ");
   else if (button == GLUT_MIDDLE_BUTTON)
      printf("middle button - ");
   else
      printf("right button - ");

   if (state == GLUT_UP)
      printf("up - ");
   else
      printf("down - ");

   printf("%d %d\n", x, y);
}



int main(int argc, char** argv)
{
int i, j, k, l;
float noise;
	/* initialize the graphics system */
   graphicsInit(&argc, argv);

	/* the first part of this if statement builds a sample */
	/* world which will be used for testing */
	/* DO NOT remove this code. */
	/* Put your code in the else statment below */
	/* The testworld is only guaranteed to work with a world of
		with dimensions of 100,50,100. */
   if (testWorld == 1) {
	/* initialize world to empty */
      for(i=0; i<WORLDX; i++)
         for(j=0; j<WORLDY; j++)
            for(k=0; k<WORLDZ; k++)
               world[i][j][k] = 0;

	/* some sample objects */
	/* build a red platform */
      for(i=0; i<WORLDX; i++) {
         for(j=0; j<WORLDZ; j++) {
            world[i][24][j] = 3;
         }
      }
	/* create some green and blue cubes */
      world[50][25][50] = 1;
      world[49][25][50] = 1;
      world[49][26][50] = 1;
      world[52][25][52] = 2;
      world[52][26][52] = 2;

	/* blue box shows xy bounds of the world */
      for(i=0; i<WORLDX-1; i++) {
         world[i][25][0] = 2;
         world[i][25][WORLDZ-1] = 2;
      }
      for(i=0; i<WORLDZ-1; i++) {
         world[0][25][i] = 2;
         world[WORLDX-1][25][i] = 2;
      }

	/* create two sample mobs */
	/* these are animated in the update() function */
      createMob(0, 50.0, 25.0, 52.0, 0.0);
      createMob(1, 50.0, 25.0, 52.0, 0.0);

	/* create sample player */
      createPlayer(0, 52.0, 27.0, 52.0, 0.0);

   } else {
		// Building the world with perlin noise (see perlin.c)
		for(i=0; i<WORLDX; i++) {
			for(j=0; j<WORLDZ; j++) {
				noise = noise_generation(i, j);
				k = (int) ((noise + 1) * 5);
				for (l = 0; l < k; ++l) {
					world[i][l][j] = 1;
				}
		    }
		}
	   
	   // Create new clouds
	   for(i=0; i<WORLDX; i++) {
		   for(j=0; j<WORLDZ; j++) {
			   noise = noise_generation(i,j);
			   if(noise > 0.0) {
				   world[i][49][j] = 5;
			   } else {
				   world[i][49][j] = 0;
			   }
		   }
	   }
	   
	   // Remove tiny clouds
	   for(i=0; i<WORLDX; i++) {
		   for(j=0; j<WORLDZ; j++) {
			   if (world[i][49][j] == 5) {
				   k = 0;
				   if (i < 99 && j < 99) {
					   if (world[i][49][j+1] == 5) {
						   ++k;
					   }
					   if (world[i+1][49][j] == 5) {
						   ++k;
					   }
					   if (world[i+1][49][j+1] == 5) {
						   ++k;
					   }
				   }
				   if (i > 0 && j > 0) {
					   if (world[i][49][j-1] == 5) {
						   ++k;
					   }
					   if (world[i-1][49][j] == 5) {
						   ++k;
					   }
					   if (world[i-1][49][j-1] == 5) {
						   ++k;
					   }
				   }
				   if (i > 0 && j < 99) {
					   if (world[i-1][49][j+1] == 5) {
						   ++k;
					   }
				   }
				   if (i < 99 && j > 0) {
					   if (world[i+1][49][j-1] == 5) {
						   ++k;
					   }
				   }

				   // Remove
				   if (k < 2) {
					   world[i][49][j] = 0;
				   }
			   }
		   }
	   }
   }


	/* starts the graphics processing loop */
	/* code after this will not run until the program exits */
   glutMainLoop();
   return 0; 
}

