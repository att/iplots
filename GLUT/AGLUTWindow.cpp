/*
 *  GLUTWindow.cpp
 *  Acinonyx
 *
 *  Created by Simon Urbanek on 5/18/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#include "AGLUTWindow.h"
#include "AVisual.h"

static unsigned int gwin_max;

static AGLUTWindow **gwin;

static bool a_glut_init_f = false;

static void a_glut_init() {
	char *argv[] = { "Acinonyx", NULL };
	int argc = 1;
	gwin = (AGLUTWindow**) calloc(sizeof(AGLUTWindow*), gwin_max=512);
	glutInit(&argc, argv);
	a_glut_init_f = true;
}

static void a_glut_draw() {
	int w = glutGetWindow();
	if (w >=0 && w < gwin_max && gwin[w]) {
		w->begin();
		w->draw();
		w->end();
	}
}

static void a_glut_key(unsigned char c, int x, int y) {
	int w = glutGetWindow();
	if (w >=0 && w < gwin_max && gwin[w]) {
		AVisual *v = (AVisual*) gwin[w]->rootVisual();
		// if (v) v->
	}
}

static void a_glut_mouse(int b, int st, int x, int y) {
}

static void a_glut_reshape(int w, int h)
{
	glViewport(0,0,(GLsizei)w,(GLsizei)h);
	int wi = glutGetWindow();
	if (wi >=0 && wi < gwin_max && gwin[wi]) {
		AVisual *v = (AVisual*) gwin[wi]->rootVisual();
		if (v) {
			ARect f = v->frame();
			printf("change from %g,%g to %d,%d\n", f.width, f.height, w, h);
			f.width = w; f.height = h;
			v->moveAndResize(f);
			glutPostRedisplay();
		}
	}
}

AGLUTWindow::AGLUTWindow(ARect frame) : AWindow(frame) {
	if (!a_glut_init_f)
		a_glut_init();
	glutInitWindowSize(frame.width, frame.height);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_ACCUM | GLUT_DOUBLE);
	win_id = glutCreateWindow("GLUT Acinonyx");
	if (win_id < gwin_max)
		gwin[win_id] = this;
	else
		fprintf(stderr, "FATAL: too many GLUT windows\n");
	glutDisplayFunc(a_glut_draw);
	glutKeyboardFunc(a_glut_key);
	glutMouseFunc(a_glut_mouse);
	glutReshapeFunc(a_glut_reshape);
}

//---- sample main function ---

extern "C" {
	int main(int ac, char **av);
}

#include "REngine.h"
#include "AScatterPlot.h"

int main(int ac, char **av) {
	gwin = (AGLUTWindow**) calloc(sizeof(AGLUTWindow*), gwin_max=512);
	glutInit(&ac, av);
	a_glut_init_f = true;
	
	REngine *eng = REngine::mainEngine();
	RObject *o = eng->parseAndEval("{n<-1e4; x<-rnorm(n)}");
	AMarker *mark = new AMarker(o->length());
	ADataVector *vx = new ADoubleVector(mark, o->doubles(), o->length(), true);
	o->release();
	o = eng->parseAndEval("y<-rnorm(n)");	
	ADataVector *vy = new ADoubleVector(mark, o->doubles(), o->length(), true);
	o->release();
	
	o = eng->parseAndEval("as.integer(y - min(y))+1L");
	AIntVector *iv = new AIntVector(mark, o->integers(), o->length(), true);
	vsize_t ls = iv->range().length;
	char ** levels = (char**) malloc(sizeof(char*) * ls);
	char *ln = (char*) malloc(2 * ls);
	for (vsize_t i = 0; i < ls; i++) { ln[i*2] = i + 'A'; ln[i*2+1] = 0; levels[i] = ln + (i*2); }
	AFactorVector *fv = new AFactorVector(mark, iv->asInts(), iv->length(), (const char**) levels, ls);
	iv->release();
	free(levels); // we cannot free ln
	
	ARect aFrame = AMkRect(0, 0, 400, 300);
	AVisual *visual = new AScatterPlot(NULL, aFrame, 0, vx, vy);
	
	AGLUTWindow *win = new AGLUTWindow(aFrame);
	win->setRootVisual(visual);
	
	// FIXME: we should assign the result or something ...
	visual->release();

	glutMainLoop();

	return 0;
}
