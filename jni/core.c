#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <android_native_app_glue.h>
#include <math.h>
#include "defs.h"

int core(struct state *state){
	if(state->showmenu){
		if(!menu_main(state))return false;
		state->showmenu=false;
		reset(state);
	}
	int ready=0;
	for(int i=0;i<SLOT_COLUMNS;++i){
		for(int j=0;j<SLOT_ROWS;++j){
			if(state->board[i][j].markfordelete){
				zerof(&state->board[i][j].base.w,0.05f);
				state->board[i][j].base.h=state->board[i][j].base.w;
				state->board[i][j].base.x+=0.025f;
				state->board[i][j].base.y+=0.025f;
				continue;
			}
			ready+=(xpos(i)!=targetf(&state->board[i][j].base.x,(fabs(state->board[i][j].base.x-xpos(i))/15.0f)+0.025f,xpos(i)));
			ready+=(ypos(j)!=targetf(&state->board[i][j].base.y,(fabs(state->board[i][j].base.y-ypos(j))/15.0f)+0.025f,ypos(j)));
			if(state->pointer[0].active&&state->pointer[0].x>state->board[i][j].base.x&&state->pointer[0].x<state->board[i][j].base.x+SLOT_SIZE&&
			state->pointer[0].y>state->board[i][j].base.y&&state->pointer[0].y<state->board[i][j].base.y+SLOT_SIZE){
				if(state->selected.active&&(state->selected.col!=i||state->selected.row!=j)){
					int adjacent=abs(state->selected.col-i)+abs(state->selected.row-j)<2;
					if(adjacent){
						state->selected.active=false;
						state->swap1.active=true;
						state->swap1.col=state->selected.col;
						state->swap1.row=state->selected.row;
						state->swap2.col=i;
						state->swap2.row=j;
						struct slot temp=state->board[i][j];
						state->board[i][j]=state->board[state->swap1.col][state->swap1.row];
						state->board[state->swap1.col][state->swap1.row]=temp;
						state->testswap=true;
					}
					else{
						state->selected.active=false;
					}
				}
				else if(!state->swap1.active&&!state->swap2.active&&!state->board[i][j].markfordelete){
					state->selected.active=true;
					state->selected.col=i;
					state->selected.row=j;
					state->selectionpulse=0.0f;
					state->selectionpulseup=true;
				}
				//break;
			}
		}
	}
	if(state->swap1.active){
		if(state->board[state->swap1.col][state->swap1.row].base.x==xpos(state->swap1.col)&&
		state->board[state->swap1.col][state->swap1.row].base.y==ypos(state->swap1.row)){
			checkmatch(state);
			if(!state->board[state->swap1.col][state->swap1.row].markfordelete&&!state->board[state->swap2.col][state->swap2.row].markfordelete&&state->testswap){ // failed swap
				int tempcol,temprow;
				struct slot temp=state->board[state->swap1.col][state->swap1.row];
				state->board[state->swap1.col][state->swap1.row]=state->board[state->swap2.col][state->swap2.row];
				state->board[state->swap2.col][state->swap2.row]=temp;
				tempcol=state->swap1.col;
				temprow=state->swap1.row;
				state->swap1.col=state->swap2.col;
				state->swap1.row=state->swap2.row;
				state->swap2.col=tempcol;
				state->swap2.row=temprow;
				state->testswap=false;
			}
			else{ // successful swap
				state->swap1.active=false;
				state->swap2.active=false;
				fillboard(state->board,state->rect.top-SLOT_SIZE);
				state->testswap=false;
			}
		}
	}
	else if(!ready){
		checkmatch(state);
		if(fillboard(state->board,state->rect.top-SLOT_SIZE)){
			if(!anymovesleft(state)){
				if(!menu_message(state,"No Moves","No more moves left!"))return false;
				if(!menu_end(state))return false;
				if(state->showmenu)return core(state);
			}
		}
	}
	if(state->timer==0){
		if(!menu_end(state))return false;
		if(state->showmenu)return core(state);
	}
	if(((state->stopbuttonstate=buttonprocess(state,&state->stopbutton))==BUTTON_ACTIVATE&&state->pressbutton)||state->back){
		state->back=false;
		state->showmenu=true;
		return core(state);
	}
	else if(state->stopbuttonstate==0){
		if(state->pointer[0].active)state->pressbutton=false;
		else state->pressbutton=true;
	}
	return true;
}

void render(struct state *state){
	glClear(GL_COLOR_BUFFER_BIT);
	state->timer=60-(time(NULL)-state->starttime);
	if(state->timer==10){
		state->timerpulseup=false;
		state->timerpulse=1.0f;
		glUniform4f(state->uniform.rgba,1.0f,1.0f,1.0f,1.0f);
	}
	else if(state->timer<10){
		if(state->timerpulseup){
			if(targetf(&state->timerpulse,0.0275f,0.8f)==0.8f)state->timerpulseup=false;
		}
		else{
			if(targetf(&state->timerpulse,0.0275f,0.2f)==0.2f)state->timerpulseup=true;
		}
		glUniform4f(state->uniform.rgba,1.0f,0.0f,0.0f,state->timerpulse);
	}
	else glUniform4f(state->uniform.rgba,1.0f,1.0f,1.0f,1.0f);
	glBindTexture(GL_TEXTURE_2D,state->font.main->atlas);
	char msg[18];
	sprintf(msg,"%02d",state->timer);
	drawtext(state->font.main,-0.35f,-7.8f,msg);
	sprintf(msg,"Score: %d",state->score);
	glUniform4f(state->uniform.rgba,1.0f,1.0f,1.0f,1.0f);
	drawtextcentered(state->font.main,0.0f,-7.0f,msg);
	glUniform4f(state->uniform.rgba,1.0f,1.0f,1.0f,1.0f);
	glBindTexture(GL_TEXTURE_2D,state->assets.texture[TID_SLOTS].object);
	for(int i=0;i<SLOT_COLUMNS;++i){
		for(int j=0;j<SLOT_ROWS;++j){
			drawslot(state,&state->board[i][j]);
		}
	}
	if(state->selected.active){
		glUniform4f(state->uniform.rgba,1.0f,1.0f,1.0f,state->selectionpulse);
		glBindTexture(GL_TEXTURE_2D,state->assets.texture[TID_SELECTION].object);
		draw(state,&state->board[state->selected.col][state->selected.row].base);
		if(state->selectionpulseup){
			if(targetf(&state->selectionpulse,0.05f,0.8f)==0.8f)state->selectionpulseup=false;
		}
		else{
			if(targetf(&state->selectionpulse,0.05f,0.2f)==0.2f)state->selectionpulseup=true;
		}
	}
	
	glUniform4f(state->uniform.rgba,1.0f,1.0f,1.0f,1.0f);
	glBindTexture(GL_TEXTURE_2D,state->assets.texture[TID_BUTTON].object);
	if(state->stopbuttonstate==BUTTON_PRESS){
		float y=state->stopbutton.base.y;
		state->stopbutton.base.y+=BUTTON_DESCEND;
		draw(state,&state->stopbutton.base);
		state->stopbutton.base.y=y;
	}
	else draw(state,&state->stopbutton.base);
	glBindTexture(GL_TEXTURE_2D,state->font.button->atlas);
	glUniform4f(state->uniform.rgba,BUTTON_TEXT_COLOR);
	buttondrawtext(state->font.button,&state->stopbutton);
	/*{
		static int fps=0,lasttime=0;
		static char fpsmsg[21];
		if(lasttime!=time(NULL)){
			lasttime=time(NULL);
			sprintf(fpsmsg,"fps: %d",fps);
			fps=0;
		}
		++fps;
		glUniform4f(state->uniform.rgba,1.0f,1.0f,1.0f,1.0f);
		glBindTexture(GL_TEXTURE_2D,state->font.main->atlas);
		drawtext(state->font.main,state->rect.left+0.1f,state->rect.top+0.1f,fpsmsg);
	}*/
}

void init(struct state *state){
	if(!readconfdata(state)){
		state->musicenabled=true;
		state->showtut=true;
	}
	if(!readhsdata(state->highscore)){
		memset(state->highscore,0,sizeof(unsigned)*5);
	}
	state->showmenu=true;
	state->back=false;
	memset(state->pointer,0,sizeof(struct crosshair)*2);
	state->rect.left=-4.5f;
	state->rect.right=4.5f;
	state->rect.bottom=8.0f;
	state->rect.top=-8.0f;
	state->stopbutton.base.x=-BUTTON_WIDTH/2.0f;
	state->stopbutton.base.y=5.5f;
	state->stopbutton.base.w=BUTTON_WIDTH;
	state->stopbutton.base.h=BUTTON_HEIGHT;
	state->stopbutton.label="Stop";
	state->stopbutton.active=false;
}
void reset(struct state *state){
	newboard(state);
	state->selected.active=false;
	state->swap1.active=false;
	state->swap2.active=false;
	state->testswap=false;
	state->timer=1;
	state->score=0;
	state->starttime=time(NULL);
	state->stopbuttonstate=0;
	state->stopbutton.base.y=5.5f;
	state->pressbutton=true;
}