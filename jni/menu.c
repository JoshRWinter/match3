#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <android_native_app_glue.h>
#include "defs.h"

int menu_main(struct state *state){
	char *abouttext=
	"Match3\nProgramming and Art by\nJosh Winter\n\nMusic\nPapoFuradoPA - "
	"Lost Inside\npapofuradopa.newgrounds.com\n\nHigh scores\nHold the play button\nto see high scores!";
	struct button playbutton={{-2.75f,2.0f,BUTTON_WIDTH,BUTTON_HEIGHT},"Play",false};
	struct button quitbutton={{0.5f,5.0f,BUTTON_WIDTH,BUTTON_HEIGHT},"Quit",false};
	struct button abootbutton={{0.5f,2.0f,BUTTON_WIDTH,BUTTON_HEIGHT},"Aboot",false};
	struct button confbutton={{-2.75f,5.0f,BUTTON_WIDTH,BUTTON_HEIGHT},"Conf.",false};
	struct base slots={-3.0f,-2.250f,6.0f,1.0f};
	int holdplaybutton=0,playbuttonstate;
	while(process(state->app)){
		glClear(GL_COLOR_BUFFER_BIT);
		glUniform4f(state->uniform.rgba,BUTTON_COLOR);
		glBindTexture(GL_TEXTURE_2D,state->assets.texture[TID_BUTTON].object);
		if((playbuttonstate=buttondraw(state,&playbutton))==BUTTON_ACTIVATE){
			if(state->showtut){
				if(!menu_tutorial(state))return false;
				state->showtut=false;
				saveconfdata(state);
			}
			return true;
		}
		else if(playbuttonstate==BUTTON_PRESS){
			if(++holdplaybutton>30){ // show high scores
				char highscoremsg[100];
				sprintf(highscoremsg,"%u\n%u\n%u\n%u\n%u",
				state->highscore[4],
				state->highscore[3],
				state->highscore[2],
				state->highscore[1],
				state->highscore[0]);
				if(!menu_message(state,"High Scores",highscoremsg))return false;
				holdplaybutton=0;
			}
		}
		else holdplaybutton=0;
		if(buttondraw(state,&quitbutton)==BUTTON_ACTIVATE||state->back){
			ANativeActivity_finish(state->app->activity);
		}
		if(buttondraw(state,&abootbutton)==BUTTON_ACTIVATE){
			if(!menu_message(state,"Aboot",abouttext))return false;
			continue;
		}
		if(buttondraw(state,&confbutton)==BUTTON_ACTIVATE){
			if(!menu_conf(state))return false;
		}
		glBindTexture(GL_TEXTURE_2D,state->font.header->atlas);
		//glUniform4f(state->uniform.rgba,1.0f,1.0f,1.0f,1.0f);
		drawtextcentered(state->font.header,0.0f,-6.5f,"Match3");
		glBindTexture(GL_TEXTURE_2D,state->assets.texture[TID_SLOTS].object);
		draw(state,&slots);
		glBindTexture(GL_TEXTURE_2D,state->font.button->atlas);
		glUniform4f(state->uniform.rgba,BUTTON_TEXT_COLOR);
		buttondrawtext(state->font.button,&playbutton);
		buttondrawtext(state->font.button,&quitbutton);
		buttondrawtext(state->font.button,&abootbutton);
		buttondrawtext(state->font.button,&confbutton);
		eglSwapBuffers(state->display,state->surface);
	}
	return false;
}

int menu_conf(struct state *state){
	struct button musicbutton={{-2.75f,2.0f,BUTTON_WIDTH,BUTTON_HEIGHT},"Music",false};
	struct button tutorialbutton={{0.5f,2.0f,BUTTON_WIDTH,BUTTON_HEIGHT},"Tut.",false};
	struct button backbutton={{-BUTTON_WIDTH/2.0f,5.f,BUTTON_WIDTH,BUTTON_HEIGHT},"Back",false};
	char msg[121];
	int changed=false;
	while(process(state->app)){
		glClear(GL_COLOR_BUFFER_BIT);
		glUniform4f(state->uniform.rgba,1.0f,1.0f,1.0f,1.0f);
		glBindTexture(GL_TEXTURE_2D,state->assets.texture[TID_BUTTON].object);
		if(buttondraw(state,&musicbutton)==BUTTON_ACTIVATE){
			state->musicenabled=!state->musicenabled;
			if(state->musicenabled)playsound(state->soundengine,state->aassets.sound+SID_BACKGROUND,true);
			else stopallsounds(state->soundengine);
			changed=true;
		}
		if(buttondraw(state,&tutorialbutton)==BUTTON_ACTIVATE){
			state->showtut=!state->showtut;
			changed=true;
		}
		if(buttondraw(state,&backbutton)==BUTTON_ACTIVATE||state->back){
			state->back=false;
			saveconfdata(state);
			return true;
		}
		glBindTexture(GL_TEXTURE_2D,state->font.header->atlas);
		drawtextcentered(state->font.header,0.0f,-6.5f,"Configuration");
		glBindTexture(GL_TEXTURE_2D,state->font.main->atlas);
		sprintf(msg,"Music is %s\nShow Tutorial is %s",state->musicenabled?"enabled":"disabled",state->showtut?"enabled":"disabled");
		drawtextcentered(state->font.main,0.0f,-2.0f,msg);
		glUniform4f(state->uniform.rgba,BUTTON_TEXT_COLOR);
		glBindTexture(GL_TEXTURE_2D,state->font.button->atlas);
		buttondrawtext(state->font.button,&musicbutton);
		buttondrawtext(state->font.button,&tutorialbutton);
		buttondrawtext(state->font.button,&backbutton);
		eglSwapBuffers(state->display,state->surface);
	}
	return false;
}

int menu_tutorial(struct state *state){
	struct button backbutton={{-BUTTON_WIDTH/2.0f,5.5f,BUTTON_WIDTH,BUTTON_HEIGHT},"K",false};
	struct slot slots[]={
		{{-3.35f,-3.0f,SLOT_SIZE,SLOT_SIZE},0},
		{{-3.35f,-2.0f,SLOT_SIZE,SLOT_SIZE},1},
		{{-3.35f,-1.0f,SLOT_SIZE,SLOT_SIZE},2},
		{{-3.35f,0.0f,SLOT_SIZE,SLOT_SIZE},3},
		{{-3.35f,1.0f,SLOT_SIZE,SLOT_SIZE},4},
		{{-3.35f,2.0f,SLOT_SIZE,SLOT_SIZE},5}
	};
	char *pointsdata[]={
		"-    1 pts  x  (  #  in a row )",
		"-    2 pts  x  (  #  in a row )",
		"-    3 pts  x  (  #  in a row )",
		"-    2 pts  x  (  #  in a row )",
		"-    1 pts  x  (  #  in a row )",
		"-    1 pts  x  (  #  in a row )"
	};
	char *tutorialtext="Match 3 or more slots in a row\nto score as many points as\nyou can before time runs out!";
	while(process(state->app)){
		glClear(GL_COLOR_BUFFER_BIT);
		glUniform4f(state->uniform.rgba,1.0f,1.0f,1.0f,1.0f);
		glBindTexture(GL_TEXTURE_2D,state->assets.texture[TID_SLOTS].object);
		for(int i=0;i<6;++i){
			drawslot(state,slots+i);
		}
		glBindTexture(GL_TEXTURE_2D,state->font.header->atlas);
		drawtextcentered(state->font.header,0.0f,-7.75f,"Tutorial");
		glBindTexture(GL_TEXTURE_2D,state->font.main->atlas);
		drawtextcentered(state->font.main,0.0f,-6.0f,tutorialtext);
		glUniform4f(state->uniform.rgba,0.9f,0.9f,0.9f,1.0f);
		for(int i=0;i<6;++i){
			drawtext(state->font.main,-2.0f,slots[i].base.y+0.05f,pointsdata[i]);
		}
		glUniform4f(state->uniform.rgba,1.0f,1.0f,1.0f,1.0f);
		drawtextcentered(state->font.main,0.0f,3.75f,"- Plus 30% for vertical matches -");
		glBindTexture(GL_TEXTURE_2D,state->assets.texture[TID_BUTTON].object);
		if(buttondraw(state,&backbutton)==BUTTON_ACTIVATE||state->back){
			state->back=false;
			return true;
		}
		glUniform4f(state->uniform.rgba,BUTTON_TEXT_COLOR);
		glBindTexture(GL_TEXTURE_2D,state->font.button->atlas);
		buttondrawtext(state->font.button,&backbutton);
		eglSwapBuffers(state->display,state->surface);
	}
	return false;
}

void swap(int *i,int *j){
	int temp=*i;
	*i=*j;
	*j=temp;
}
void selection(unsigned *a){ // swiped from wikipedia
	const int n=5;
	int	i,j;
	for (j = 0; j < n-1; j++) {
		int iMin = j;
		for ( i = j+1; i < n; i++) {
			if (a[i] < a[iMin]) {
				iMin = i;
			}
		}
		if(iMin != j) {
			swap(a+j, a+iMin);
		}
	}
}

int menu_end(struct state *state){
	struct button playbutton={{-2.75f,5.0f,BUTTON_WIDTH,BUTTON_HEIGHT},"Again",false};
	struct button menubutton={{0.5f,5.0f,BUTTON_WIDTH,BUTTON_HEIGHT},"Menu",false};
	char scoremsg[36];
	float buttonfall=0.0f;
	int highscoreindex=-1;
	sprintf(scoremsg,"You scored\n%d\npoints!",state->score);
	if(state->score>state->highscore[0]){
		state->highscore[0]=state->score;
		selection(state->highscore);
		savehsdata(state->highscore);
		for(int i=0;i<5;++i){
			if(state->highscore[i]==state->score){
				highscoreindex=i;
				break;
			}
		}
	}
	while(process(state->app)){
		glClear(GL_COLOR_BUFFER_BIT);
		glUniform4f(state->uniform.rgba,1.0f,1.0f,1.0f,1.0f);
		int display=false;
		glBindTexture(GL_TEXTURE_2D,state->assets.texture[TID_SLOTS].object);
		for(int i=0;i<SLOT_COLUMNS;++i){
			for(int j=0;j<SLOT_ROWS;++j){
				drawslot(state,&state->board[i][j]);
				if(onein(8)&&state->board[i][j].fall==0.0f)state->board[i][j].fall=0.01f;
				if(state->board[i][j].fall>0.0f){
					state->board[i][j].fall+=0.01f;
					state->board[i][j].base.y+=state->board[i][j].fall;
				}
				if(state->board[i][j].base.y<state->rect.bottom+5.0f)display=true;
			}
		}
		if(!display){
			glBindTexture(GL_TEXTURE_2D,state->assets.texture[TID_BUTTON].object);
			if(buttondraw(state,&playbutton)==BUTTON_ACTIVATE||state->back){
				state->back=false;
				reset(state);
				return true;
			}
			if(buttondraw(state,&menubutton)==BUTTON_ACTIVATE){
				state->showmenu=true;
				return true;
			}
			glBindTexture(GL_TEXTURE_2D,state->font.header->atlas);
			drawtextcentered(state->font.header,0.0f,-6.5f,"GAME OVER");
			glBindTexture(GL_TEXTURE_2D,state->font.main->atlas);
			drawtextcentered(state->font.main,0.0f,-4.0f,scoremsg);
			drawtextcentered(state->font.main,0.0f,-0.9f,"- High scores -");
			float offset=-0.1f;
			for(int i=4;i>=0;--i){
				if(highscoreindex==i)glUniform4f(state->uniform.rgba,1.0f,0.0f,0.0f,1.0f);
				else glUniform4f(state->uniform.rgba,1.0f,1.0f,1.0f,1.0f);
				char msg[6];
				sprintf(msg,"%u",state->highscore[i]);
				drawtextcentered(state->font.main,0.0f,offset,msg);
				offset+=0.725f;
			}
			glUniform4f(state->uniform.rgba,BUTTON_TEXT_COLOR);
			glBindTexture(GL_TEXTURE_2D,state->font.button->atlas);
			buttondrawtext(state->font.button,&playbutton);
			buttondrawtext(state->font.button,&menubutton);
		}
		else{
			glUniform4f(state->uniform.rgba,1.0f,0.0f,0.0f,state->timerpulse);
			glBindTexture(GL_TEXTURE_2D,state->font.main->atlas);
			char msg[18];
			sprintf(msg,"%02d",state->timer);
			drawtext(state->font.main,-0.35f,-7.8f,msg);
			sprintf(msg,"Score: %d",state->score);
			glUniform4f(state->uniform.rgba,1.0f,1.0f,1.0f,1.0f);
			drawtextcentered(state->font.main,0.0f,-7.0f,msg);
			state->stopbutton.base.y+=(buttonfall+=0.01f);
			glBindTexture(GL_TEXTURE_2D,state->assets.texture[TID_BUTTON].object);
			glUniform4f(state->uniform.rgba,1.0f,1.0f,1.0f,1.0f);
			buttondraw(state,&state->stopbutton);
			glBindTexture(GL_TEXTURE_2D,state->font.button->atlas);
			glUniform4f(state->uniform.rgba,BUTTON_TEXT_COLOR);
			buttondrawtext(state->font.button,&state->stopbutton);
		}
		eglSwapBuffers(state->display,state->surface);
	}
	return false;
}

int menu_message(struct state *state,const char *caption,const char *msg){
	struct button kbutton={{-BUTTON_WIDTH/2.0f,5.0f,BUTTON_WIDTH,BUTTON_HEIGHT},"K",false};
	while(process(state->app)){
		glClear(GL_COLOR_BUFFER_BIT);
		glUniform4f(state->uniform.rgba,BUTTON_COLOR);
		glBindTexture(GL_TEXTURE_2D,state->assets.texture[TID_BUTTON].object);
		if(buttondraw(state,&kbutton)==BUTTON_ACTIVATE||state->back){
			state->back=false;
			return true;
		}
		glUniform4f(state->uniform.rgba,1.0f,1.0f,1.0f,1.0f);
		glBindTexture(GL_TEXTURE_2D,state->font.header->atlas);
		drawtextcentered(state->font.header,0.0f,-6.5f,caption);
		glBindTexture(GL_TEXTURE_2D,state->font.main->atlas);
		drawtextcentered(state->font.main,0.0f,-4.0f,msg);
		glBindTexture(GL_TEXTURE_2D,state->font.button->atlas);
		glUniform4f(state->uniform.rgba,BUTTON_TEXT_COLOR);
		buttondrawtext(state->font.button,&kbutton);
		eglSwapBuffers(state->display,state->surface);
	}
	return false;
}
