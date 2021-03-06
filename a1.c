
/* Derived from scene.c in the The OpenGL Programming Guide */
/* Keyboard and mouse rotation taken from Swiftless Tutorials #23 Part 2 */
/* http://www.swiftless.com/tutorials/opengl/camera2.html */

/* Frames per second code taken from : */
/* http://www.lighthouse3d.com/opengl/glut/index.php?fps */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
#include <ctype.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "perlin.h"
#include "graphics.h"

#define PI 3.14159265
#define INDEX 14
#define BUFSIZE 512

// Server Flags
#define PROJECTILE 	0
#define ANGLE		1
#define MOVE		2
#define UPDATEPROJ	3
#define OUT			4
#define BOOM		5

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
extern void setViewOrientation(float, float, float);
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

	/* 2D drawing functions */
extern void  draw2Dline(int, int, int, int, int);
extern void  draw2Dbox(int, int, int, int);
extern void  draw2Dtriangle(int, int, int, int, int, int);
extern void  set2Dcolour(float []);


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
	/* size of the window in pixels */
extern int screenWidth, screenHeight;
	/* flag indicates if map is to be printed */
extern int displayMap;

	/* frustum corner coordinates, used for visibility determination  */
extern float corners[4][3];

	/* determine which cubes are visible e.g. in view frustum */
extern void ExtractFrustum();
extern void tree(float, float, float, float, float, float, int);

/********* end of extern variable declarations **************/

double time_sec, time_usec, sec, usec;
struct timeval tv;

// Save projectile velocity
float projectiles[INDEX];
float velocity;
int angle;

// Socket values (client_sockfd only used by server)
int my_sockfd, client_sockfd;
socklen_t addrlen, client_len;
struct sockaddr_in my_address, client_address;

int pchar;

double start_x, start_y;

/************** end variable declarations **************/

void launchProjectile(float player_x, float player_y, float player_z, int player_rot) {
	float val = player_rot * (PI / 180.0);
	float val2 = angle * (PI / 180.0);
	
	// Angle of the projectile
	projectiles[0] = angle;
	
	// Velocity of the projectile
	projectiles[1] = velocity;
	
	// Position X of the projectile
	projectiles[2] = player_x;
	
	// Position Y of the projectile
	projectiles[3] = player_y;
	
	// Position Z of the projectile
	projectiles[4] = player_z;
	
	// Incrementation value of X.
	// Calculation: cos of view point angle * cos of angle * velocity
	projectiles[5] = cosf(val) * cosf(val2) * velocity;
	
	// Incrementation value of Z.
	// Calculation: sin of view point angle * cos of angle * velocity
	projectiles[6] = sinf(val) * cosf(val2) * velocity;
	
	// Projectiles rotation
	projectiles[7] = player_rot;
	
	// Incrementation value of Y.
	// Calculation: sin of angle * velocity
	projectiles[8] = sinf(val2) * velocity;
	
	// Counter to calculate Y position
	// Set it to 0 on trajectory
	projectiles[9] = (-1) * sqrt(velocity * 30.0 + player_y);
	
	// Max height of Y
	projectiles[10] = velocity * 30.0 + player_y;
	
	// Flag indicating the projectile is launched
	projectiles[11] = 1;
	
	// Flag indicating to decrease height
	projectiles[12] = 0;
	
	// Save the starting y value
	projectiles[13] = player_y;
	
	printf("Projectile %d Fired!\n", 0);
	/*printf("launched: ");
	for(i = 0; i < INDEX; ++i) {
		printf("%d: %f, ", i, projectiles[i]);
	}
	printf("\n");*/
	
	createMob(0, player_x, player_y, player_z, player_rot);

	/*} else {
		printf("You can't launch more projectiles!\n");
	}*/
}

/*
 * Explodes the world
 */
void boom(int x, int y, int z) {
	if(y < 48) {
		world[x][y + 2][z] = 0;
	}
	if(y < 49) {
		world[x][y + 1][z] = 0;
		world[x][y + 1][z + 1] = 0;
		world[x][y + 1][z - 1] = 0;
		world[x - 1][y + 1][z] = 0;
		world[x - 1][y + 1][z + 1] = 0;
		world[x - 1][y + 1][z - 1] = 0;
		world[x + 1][y + 1][z] = 0;
		world[x + 1][y + 1][z + 1] = 0;
		world[x + 1][y + 1][z - 1] = 0;
	}
	world[x][y][z] = 0;
	world[x][y][z + 1] = 0;
	world[x][y][z - 1] = 0;
	world[x - 1][y][z] = 0;
	world[x - 1][y][z + 1] = 0;
	world[x - 1][y][z - 1] = 0;
	world[x + 1][y][z] = 0;
	world[x + 1][y][z + 1] = 0;
	world[x + 1][y][z - 1] = 0;
	world[x + 2][y][z] = 0;
	world[x + 2][y][z + 1] = 0;
	world[x + 2][y][z - 1] = 0;
	world[x + 2][y][z + 2] = 0;
	world[x + 2][y][z - 2] = 0;
	world[x - 2][y][z] = 0;
	world[x - 2][y][z + 1] = 0;
	world[x - 2][y][z - 1] = 0;
	world[x - 2][y][z + 2] = 0;
	world[x - 2][y][z - 2] = 0;
	world[x][y][z + 2] = 0;
	world[x][y][z - 2] = 0;
	world[x + 1][y][z + 2] = 0;
	world[x + 1][y][z - 2] = 0;
	world[x - 1][y][z + 2] = 0;
	world[x - 1][y][z - 2] = 0;				
	if(y > 0) {
		world[x][y - 1][z] = 0;
		world[x][y - 1][z + 1] = 0;
		world[x][y - 1][z - 1] = 0;
		world[x - 1][y - 1][z] = 0;
		world[x - 1][y - 1][z + 1] = 0;
		world[x - 1][y - 1][z - 1] = 0;
		world[x + 1][y - 1][z] = 0;
		world[x + 1][y - 1][z + 1] = 0;
		world[x + 1][y - 1][z - 1] = 0;
	}
	if(y > 1) {
		world[x][y - 2][z] = 0;
	}
}

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
		
	//printf("Position x=%d, y=%d, z=%d\n", x, y, z);

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
	// Taking .5 values to be able to go at the middle of the cubes on the sides
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


	/******* draw2D() *******/
	/* draws 2D shapes on screen */
	/* use the following functions: 			*/
	/*	draw2Dline(int, int, int, int, int);		*/
	/*	draw2Dbox(int, int, int, int);			*/
	/*	draw2Dtriangle(int, int, int, int, int, int);	*/
	/*	set2Dcolour(float []); 				*/
	/* colour must be set before other functions are called	*/
void draw2D() {
	float size, mid_width, mid_height, flag, startw, starth, u, x, z;
	float *pos_x, *pos_y, *pos_z;
	
	GLfloat green[] = {0.0, 0.5, 0.0, 0.5};
	GLfloat black[] = {0.0, 0.0, 0.0, 0.5};
	GLfloat red[] = {0.5, 0.0, 0.0, 0.5};
	GLfloat blue[] = {0.0, 0.0, 0.5, 0.5};
   if (testWorld) {
		/* draw some sample 2d shapes */
      set2Dcolour(green);
      draw2Dline(0, 0, 500, 500, 15);
      draw2Dtriangle(0, 0, 200, 200, 0, 200);
      set2Dcolour(black);
      draw2Dbox(500, 380, 524, 388);
    } else {
    	pos_x = malloc(sizeof(float));
		pos_y = malloc(sizeof(float));
		pos_z = malloc(sizeof(float));
		getViewPosition(pos_x, pos_y, pos_z);

		if(screenWidth < screenHeight) {
			size = screenWidth;
			flag = 0;
		} else {
			size = screenHeight;
			flag = 1;
		}
		mid_width = screenWidth / 2;
		mid_height = screenHeight / 2;
		printf("porj %d\n", (int) projectiles[11]);
		
		if(displayMap == 1) {
			set2Dcolour(blue);
			size = size / 3;
			u = size / 100;
			x = -pos_x[0] * u;
			z = (100 + pos_z[0]) * u;
			starth = screenHeight - size;
			//printf("x %d, z %d, u %d\n", x, z, u);
			draw2Dbox(x, screenHeight - z, x + u, screenHeight - z + u);
			if(projectiles[11] == 1) {
				x = projectiles[2] * u;
				z = (100 - projectiles[4]) * u;
			}
			set2Dcolour(red);
			draw2Dbox(x, screenHeight - z, x + u, screenHeight - z + u);
			set2Dcolour(green);
			draw2Dbox(0, starth, size, screenHeight);
		} else if(displayMap == 2) {
			set2Dcolour(blue);
			if(flag == 0) {
				startw = 0;
				starth = mid_height - (size / 2);
			} else {
				startw = mid_width - (size / 2);
				starth = 0;
			}
			u = size / 100;
			x = -pos_x[0] * u;
			z = -pos_z[0] * u;
			//printf("x %f, z %f, u %d\n", pos_x[0], pos_z[0], u);
			//printf("x %d, z %d, u %d\n", x, z, u);
			draw2Dbox(x + startw, z + starth, x + startw + u, z + starth + u);
			if(projectiles[11] == 1) {
				x = projectiles[2] * u;
				z = projectiles[4] * u;
			}
			set2Dcolour(red);
			draw2Dbox(x + startw, z + starth, x + startw + u, z + starth + u);
			set2Dcolour(green);
			draw2Dbox(startw, starth, startw + size, starth + size);
		}
		free(pos_x);
		free(pos_y);
		free(pos_z);
    }

}

float parse_float(char *message) {
	char c;
	char *buf;
	int max, i;
	float result;
	
	buf = malloc(sizeof(char)*BUFSIZE);
	max = strlen(message);
	
	for(i = 0; pchar < max; ++i, ++pchar) {
		c = message[pchar];
		if(isspace(c)) {
			buf[i] = '\0';
			result = atof(buf);
//			printf("buf %s - result %f\n", buf, result);
			free(buf);
			return result;
		} else {
			buf[i] = c;
		}
	}
	
	result = atof(buf);
	free(buf);
	return result;
}

int parse_int(char *message) {
	return (int) parse_float(message);
}

	/*** update() ***/
	/* background process, it is called when there are no other events */
	/* -used to control animations and perform calculations while the  */
	/*  system is running */
	/* -gravity must also implemented here, duplicate collisionResponse */
void update() {
	int i, j, w, x, y, z;
	float *pos_x, *pos_y, *pos_z, *xaxis, *yaxis, *zaxis;
	int clouds[WORLDZ];
	//double time;
	float func, fx, fy, fz, fr, ax, ay, az;
	char *message;
	int recvlen;
	int flag, rot, proj;

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
		xaxis = malloc(sizeof(float));
		yaxis = malloc(sizeof(float));
		zaxis = malloc(sizeof(float));
		message = malloc(sizeof(char)*BUFSIZE);

		// Update time
   		gettimeofday(&tv,NULL);
   		time_sec = (double) tv.tv_sec;
   		time_usec = (double) tv.tv_usec / 1000000.0;
   		//time = time_sec + time_usec;
   		
   		if(time_sec > sec) {
   			// Save clouds
   			for(i=0; i < WORLDZ; i++) {
				clouds[i] = 0;
			}
			
			// Update the position of the clouds
			for(i = 0; i < WORLDX - 1; ++i) {
				for(j = 0; j < WORLDZ; ++j) {
					if (i == 0) {
						// Save the clouds before overwriting
						clouds[j] = world[0][WORLDY - 1][j];
					}
					world[i][WORLDY - 1][j] = world[i + 1][WORLDY - 1][j];
				}
			}
			
			// Rewrite the clouds to do a cycle
			for(i=0; i < WORLDZ; i++) {
				world[WORLDX - 1][WORLDY - 1][i] = clouds[i];
			}
			
			// Update sec
   			sec = time_sec;
   			
   			if(netClient == 1) {
   				getViewOrientation(xaxis, yaxis, zaxis);	
				getViewPosition(pos_x, pos_y, pos_z);
	   			// Send position to server
				sprintf(message, "%d %f %f %f %f %f %f ", MOVE, pos_x[0], pos_y[0], pos_z[0], xaxis[0], yaxis[0], zaxis[0]);
				w = write(my_sockfd, message, strlen(message));
				if(w < 0) {
					perror("ERROR - can't write in the socket\n");
				}
			}
   		}
   
   		if(netClient == 1) {
			getViewOrientation(xaxis, yaxis, zaxis);	
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
			
			// Projectile
			if(projectiles[11] == 1) {
				projectiles[2] += projectiles[5];
				projectiles[4] += projectiles[6];
				if(projectiles[1] > 0.0) {
					projectiles[9] += projectiles[8];
					func = (float) (-(projectiles[9] * projectiles[9])) + projectiles[10];
					projectiles[3] = func + projectiles[13];
				} else {
					// Gravity
					projectiles[3] -= 1.0;
				}
				
				setMobPosition(0, projectiles[2], projectiles[3], projectiles[4], projectiles[7]);
				sprintf(message, "%d %d %f %f %f %f ", UPDATEPROJ, 0, projectiles[2], projectiles[3], projectiles[4], projectiles[7]);
				w = write(my_sockfd, message, strlen(message));
				if(w < 0) {
					perror("ERROR - can't write in the socket\n");
				}
				
				x = (int) projectiles[2];
				y = (int) projectiles[3];
				z = (int) projectiles[4];

				if((projectiles[2] > 99.0) || (projectiles[2] < 0.0) || (projectiles[4] > 99.0) || (projectiles[4] < 0.0)) {
					hideMob(0);
					printf("OUT!\n");
					projectiles[11] = 0;
					sprintf(message, "%d %d ", OUT, 0);
					w = write(my_sockfd, message, strlen(message));
					if(w < 0) {
						perror("ERROR - can't write in the socket\n");
					}
				} else if (y < 50 && y > 0) {
					if((world[x][y][z] != 0) && (world[x][y][z] != 5)) {
						hideMob(0);
						projectiles[11] = 0;
						printf("BOOM!\n");
						boom(x, y, z);
						sprintf(message, "%d %d %d %d ", BOOM, x, y, z);
						w = write(my_sockfd, message, strlen(message));
						if(w < 0) {
							perror("ERROR - can't write in the socket\n");
						}
					}
				} else if (y < 0) {
					hideMob(0);
					printf("OUT!\n");
					projectiles[11] = 0;
					sprintf(message, "%d %d ", OUT, 0);
					w = write(my_sockfd, message, strlen(message));
					if(w < 0) {
						perror("ERROR - can't write in the socket\n");
					}
				}
			}
		}
		
		// Update values from client
		if(netServer == 1) {
			recvlen = read(client_sockfd, message, BUFSIZE);
			if(recvlen < 0) {
				perror("ERROR - can't read from socket\n");
			} else if(recvlen > 0) {
				message[recvlen] = '\0';
				pchar = 0;
				flag = parse_int(message);
				printf("Read %d char: %s\nflag %d pchar %d\n", recvlen, message, flag, pchar);
				switch(flag) {
					case PROJECTILE:
						printf("PROJECTILE\n");
						++pchar;
						fx = parse_float(message);
						++pchar;
						fy = parse_float(message);
						++pchar;
						fz = parse_float(message);
						++pchar;
						rot = parse_float(message);
						launchProjectile(fx, fy, fz, rot);
						break;
					case ANGLE:
						angle = 90;
						velocity = 1.0;
						printf("ANGLE\n");
						++pchar;
						angle = parse_int(message);
						++pchar;
						velocity = parse_float(message);
						printf("angle %d - velocity %f\n", angle, velocity);
						break;
					case MOVE:
						printf("MOVE\n");
						++pchar;
						fx = parse_float(message);
						++pchar;
						fy = parse_float(message);
						++pchar;
						fz = parse_float(message);
						++pchar;
						ax = parse_float(message);
						++pchar;
						ay = parse_float(message);
						++pchar;
						az = parse_float(message);
						setViewPosition(fx, fy, fz);
						setViewOrientation(-angle, ay, az);
						break;
					case UPDATEPROJ:
						printf("UPDATEPROJ\n");
						++pchar;
						proj = parse_int(message);
						++pchar;
						fx = parse_float(message);
						++pchar;
						fy = parse_float(message);
						++pchar;
						fz = parse_float(message);
						++pchar;
						fr = parse_float(message);
						projectiles[2] = fx;
						projectiles[3] = fy;
						projectiles[4] = fz;
						setMobPosition(proj, fx, fy, fz, fr);
						
						x = (int) projectiles[2];
						y = (int) projectiles[3];
						z = (int) projectiles[4];

						if((projectiles[2] > 99.0) || (projectiles[2] < 0.0) || (projectiles[4] > 99.0) || (projectiles[4] < 0.0)) {
							hideMob(0);
							printf("OUT!\n");
							projectiles[11] = 0;
						} else if (y < 50 && y > 0) {
							if((world[x][y][z] != 0) && (world[x][y][z] != 5)) {
								hideMob(0);
								projectiles[11] = 0;
								printf("BOOM!\n");
								boom(x, y, z);
							}
						} else if (y < 0) {
							hideMob(0);
							printf("OUT!\n");
							projectiles[11] = 0;
						}
						break;
					case OUT:
						printf("OUT\n");
						++pchar;
						proj = parse_int(message);
						hideMob(proj);
						projectiles[11] = 0;
						break;
					case BOOM:
						printf("BOOM\n");
						hideMob(0);
						projectiles[11] = 0;
						++pchar;
						x = parse_int(message);
						++pchar;
						y = parse_int(message);
						++pchar;
						z = parse_int(message);
						boom(x, y, z);
						break;
					default:
						printf("ERROR - Unknown flag\n");
						break;
				}
			}
		}
		
		free(pos_x);
		free(pos_y);
		free(pos_z);
		free(xaxis);
		free(yaxis);
		free(zaxis);
		free(message);
   	}
}


	/* called by GLUT when a mouse button is pressed or released */
	/* -button indicates which button was pressed or released */
	/* -state indicates a button down or button up event */
	/* -x,y are the screen coordinates when the mouse is pressed or */
	/*  released */ 
void mouse(int button, int state, int x, int y) {
	double dist_x, dist_y;
	int rot, val, w;
	char *message = malloc(sizeof(char)*BUFSIZE);
	float *xaxis = malloc(sizeof(float));
	float *yaxis = malloc(sizeof(float));
	float *zaxis = malloc(sizeof(float));
	float *pos_x = malloc(sizeof(float));
	float *pos_y = malloc(sizeof(float));
	float *pos_z = malloc(sizeof(float));
	getViewOrientation(xaxis, yaxis, zaxis);
	getViewPosition(pos_x, pos_y, pos_z);

	if(netClient == 1) {
		if (button == GLUT_LEFT_BUTTON) {
			if(state == GLUT_UP) {
				val = (int) yaxis[0];
				rot = val % 360;
				//printf("y: %f, r: %d\n", yaxis[0], rot);	
				launchProjectile(-pos_x[0], -pos_y[0], -pos_z[0], rot - 90);
				sprintf(message, "%d %f %f %f %d ", PROJECTILE, -pos_x[0], -pos_y[0], -pos_z[0], rot - 90);
				w = write(my_sockfd, message, strlen(message));
				if(w < 0) {
					perror("ERROR - can't write in the socket\n");
				}
			} else {
				printf("Angle: %d - Velocity: %f\n", angle, velocity);
			}
		}
		else if (button == GLUT_MIDDLE_BUTTON) {
			printf("Angle: %d - Velocity: %f\n", angle, velocity);
		} else {
			//printf("right button - ");
			if(state == GLUT_UP) {
				// Compute the distance
				dist_x = (double) x - start_x;
				dist_y = start_y - (double) y;
			
				// Set the angle and the velocity
				angle += (int) ((dist_y / 768) * 90);
				velocity += (dist_x / 1024);
			
				// Keep the velocity between 0 and 1
				if(velocity < 0.0) {velocity = 0.0;}
				if(velocity > 1.0) {velocity = 1.0;}
			
				// Keep the angle 
				if(angle < 0) {angle = 0;}
				if(angle > 90) {angle = 90;}
			
				printf("Angle: %d - Velocity: %f\n", angle, velocity);
				sprintf(message, "%d %d %f ", ANGLE, angle, velocity);
				w = write(my_sockfd, message, strlen(message));
				if(w < 0) {
					perror("ERROR - can't write in the socket\n");
				}
			} else {
				// Save the values of x and y
				start_x = (double) x;
				start_y = (double) y;
			}
		}
	}
	
	free(xaxis);
	free(yaxis);
	free(zaxis);
	free(pos_x);
	free(pos_y);
	free(pos_z);
}

void buildingWorld()
{
	int i, j, k, l;
	float noise;

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

int main(int argc, char** argv)
{
	int i, j, k;

	// Server - Client communication values
	int result;

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

		velocity = 0.0;
		start_x = 0;
   
   		// Deactivate flying (can be activate by pressing 'f' key)
   		flycontrol = 0;
   		
   		// Initialize time value
   		gettimeofday(&tv,NULL);
   		time_sec = sec = (double) tv.tv_sec;
   		time_usec = usec = (double) tv.tv_usec / 1000000.0;
   		
   		// Creating socket
   		my_sockfd = socket(AF_INET, SOCK_STREAM, 0);
   		
   		// Address set up
   		my_address.sin_family = AF_INET;
   		if(netServer == 1) {
   			my_address.sin_addr.s_addr = htonl(INADDR_ANY);
   		} else {
   			my_address.sin_addr.s_addr = inet_addr("127.0.0.1");
   		}
   		my_address.sin_port = htons(9734);
   		addrlen = sizeof(my_address);
   		
   		// Binding server socket and waiting for client
   		if(netServer == 1) {
   			bind(my_sockfd, (struct sockaddr *)&my_address, addrlen);
   			listen(my_sockfd, 5);
   			printf("Server waiting for players\n");
   			client_sockfd = accept(my_sockfd, (struct sockaddr *)&client_address, &client_len);
   		}
   		
   		// Connect client to the server
   		else if (netClient == 1) {
   			result = connect(my_sockfd, (struct sockaddr *)&my_address, addrlen);
   			if(result == -1) {
   				perror("ERROR - Unable to connect to the server\n");
   				exit(1);
   			}
   		}
   		
   		// No flag entered abort execution
   		else if ((netServer == 0) && (netClient == 0)) {
   			perror("ERROR - No flag entered\n-server to run as the server\n-client to run as the client\n");
   			exit(1);
   		}
   		  
		buildingWorld();
   }


	/* starts the graphics processing loop */
	/* code after this will not run until the program exits */
   glutMainLoop();
   return 0; 
}

