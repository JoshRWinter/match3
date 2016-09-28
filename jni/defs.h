#include "glesutil.h"

#define BACKGROUND_COLOR 0.0118f,0.6627f,0.9569f,1.0
#define DATAPATH "/data/data/joshwinter.match3/files/"

#define xpos(x) ((x-2.9f)*1.25f)
#define ypos(y) ((y-4.75f)*1.25f)

#define TID_BUTTON 0
#define TID_SLOTS 1
#define TID_SELECTION 2

#define SID_BACKGROUND 0

struct base{
	float x,y,w,h;
};

#define SLOT_COLUMNS 6
#define SLOT_ROWS 9
#define SLOT_SIZE 1.0f
struct slot{
	struct base base;
	char id,markfordelete;
	float fall;
};

#define BUTTON_PRESS 1
#define BUTTON_ACTIVATE 2
#define BUTTON_DESCEND 0.1f
#define BUTTON_COLOR 1.0f,1.0f,1.0f,1.0f
#define BUTTON_TEXT_COLOR 0.4f,0.4f,0.4f,1.0f
#define BUTTON_WIDTH 2.25f
#define BUTTON_HEIGHT 2.3125f
struct button{
	struct base base;
	char *label;
	int active;
};

struct state{
	int running,showmenu,back;
	unsigned char musicenabled,showtut;
	float selectionpulse,timerpulse;
	int selectionpulseup,timerpulseup,testswap,timer,starttime,score;
	int stopbuttonstate,pressbutton;
	unsigned highscore[5];
	struct slot board[SLOT_COLUMNS][SLOT_ROWS];
	struct{int col,row,active;}swap1,swap2,selected;
	unsigned vao,vbo,program;
	struct pack assets;
	struct apack aassets;
	struct android_app *app;
	struct jni_info jni_info;
	slesenv *soundengine;
	EGLDisplay display;
	EGLSurface surface;
	EGLContext context;
	struct device device;
	struct{float left,right,bottom,top;}rect;
	struct{int vector,size,texcoords,rgba,projection;}uniform;
	struct{ftfont *main,*header,*button;}font;
	struct crosshair pointer[2];
	struct button stopbutton;
};
int32_t inputproc(struct android_app*,AInputEvent*);
void cmdproc(struct android_app*,int32_t);
void init_display(struct state*);
void term_display(struct state*);
int process(struct android_app*);
int core(struct state*);
void render(struct state*);
void draw(struct state*,struct base*);
void init(struct state*);
void reset(struct state*);
int readconfdata(struct state*);
void saveconfdata(struct state*);
int readhsdata(unsigned*);
void savehsdata(unsigned*);

int buttonprocess(struct state*,struct button*);
int buttondraw(struct state*,struct button*);
void buttondrawtext(ftfont*,struct button*);
int menu_main(struct state*);
int menu_conf(struct state*);
int menu_tutorial(struct state*);
int menu_end(struct state*);
int menu_message(struct state*,const char*,const char*);
void newboard(struct state*);
void newslot(struct slot[][SLOT_ROWS],int,int,float);
void drawslot(struct state*,struct slot*);
int checkmatch(struct state*);
int fillboard(struct slot[][SLOT_ROWS],float);
int anymovesleft(struct state*);