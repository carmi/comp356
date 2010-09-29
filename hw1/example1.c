#include <string.h>
#include <stdio.h>
    #include <OpenGL/gl.h>
    #include <OpenGL/glu.h>
    #include <GLUT/glut.h>


/* Windows parameters */
#define WIDTH  600
#define HEIGHT 600
#define TITLE  "GLUT Demo: Using Subwindows"
int winIdMain;
int winIdSub;


/* Animation State Vriables */
#define SMALL_ANGLE  5.0
#define TIME_STEP    0.1
static double time = 0.0;
static double spin = 0.0;


/* This function renders a character string
   onto the current position */
static char label[100];

void 
drawString (char *s)
{
  unsigned int i;
  for (i = 0; i < strlen (s); i++)
    glutBitmapCharacter (GLUT_BITMAP_HELVETICA_10, s[i]);
};

void 
drawStringBig (char *s)
{
  unsigned int i;
  for (i = 0; i < strlen (s); i++)
    glutBitmapCharacter (GLUT_BITMAP_HELVETICA_18, s[i]);
};


/* Here is a display fucntion that updates
   the main graphic window */
void 
mainDisplay (void)
{

  /* Clean drawing board */
  glutSetWindow (winIdMain);
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glLoadIdentity ();

  /* Write Footnote */
  glColor3f (1.0F, 1.0F, 1.0F);
  sprintf (label, "(c)Miguel Angel Sepulveda 1998");
  glRasterPos2f (-0.80F, -0.80F);
  drawString (label);

  glutSwapBuffers ();
};

/* There can be only one idle() callback function. In an
   animation, this idle() function must update not only the
   main window but also all derived subwindows */
void 
idle (void)
{

  /* Update  state variables */
  spin += SMALL_ANGLE;
  time += TIME_STEP;

  /* Update main and sub window */
  glutPostRedisplay();
};




int 
main (int argc, char **argv)
{

  /* Glut initializations */
  glutInit (&argc, argv);
  glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
  glutInitWindowPosition (5, 5);
  glutInitWindowSize (WIDTH, HEIGHT);

  /* Main window creation and setup */
  winIdMain = glutCreateWindow (TITLE);
  glutDisplayFunc (mainDisplay);
  glutIdleFunc (idle);

  glutMainLoop ();

  return 0;
};