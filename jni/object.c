#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <android_native_app_glue.h>
#include <stdlib.h>
#include "defs.h"

void newboard(struct state *state){
	for(int i=0;i<SLOT_COLUMNS;++i){
		for(int j=0;j<SLOT_ROWS;++j){
			newslot(state->board,i,j,randomint((state->rect.top*2.0f)*10.0f,(state->rect.top-SLOT_SIZE)*10.0f)/10.0f);
		}
	}
}
void newslot(struct slot board[][SLOT_ROWS],int i,int j,float offset){
	board[i][j].base.x=xpos(i);
	board[i][j].base.y=offset;
	board[i][j].base.w=SLOT_SIZE;
	board[i][j].base.h=SLOT_SIZE;
	board[i][j].id=randomint(0,5);
	board[i][j].markfordelete=false;
	board[i][j].fall=0.0f;
}

int fillboard(struct slot board[][SLOT_ROWS],float offset){
	int filled=false;
	for(int i=0;i<SLOT_COLUMNS;++i){
		for(int j=0;j<SLOT_ROWS;++j){
			if(board[i][j].base.w==0.0f){
				filled=true;
				for(int k=j;k>=0;--k){
					if(k==0)newslot(board,i,0,offset);
					else board[i][k]=board[i][k-1];
				}
			}
		}
	}
	return filled;
}
int value(int id){
	switch(id){
		case 0: return 1;
		case 1: return 2;
		case 2: return 3;
		case 3: return 2;
		case 4: return 1;
		case 5: return 1;
	}
}
int checkhorizontal(struct slot board[][SLOT_ROWS],int col,int row){
	int id=board[col][row].id;
	int matchcount=0;
	for(int i=col+1;i<SLOT_COLUMNS;++i){
		if(board[i][row].id==id)++matchcount;
		else if(matchcount<2)return false;
		else break;
	}
	board[col][row].markfordelete=true;
	for(int i=col+1;i<SLOT_COLUMNS;++i){
		if(board[i][row].id==id)board[i][row].markfordelete=true;
		else break;
	}
	return matchcount;
}
int checkverticle(struct slot board[][SLOT_ROWS],int col,int row){
	int id=board[col][row].id;
	int matchcount=0;
	for(int j=row+1;j<SLOT_ROWS;++j){
		if(board[col][j].id==id)++matchcount;
		else if(matchcount<2)return false;
		else break;
	}
	board[col][row].markfordelete=true;
	for(int j=row+1;j<SLOT_ROWS;++j){
		if(board[col][j].id==id)board[col][j].markfordelete=true;
		else break;
	}
	return matchcount;
}
int checkmatch(struct state *state){
	int anymatches=false;
	int match;
	do{
		match=false;
		// horizontal pass
		for(int i=0;i<SLOT_COLUMNS;++i){
			for(int j=0;j<SLOT_ROWS;++j){
				if(i<SLOT_COLUMNS-2){
					if(state->board[i][j].markfordelete)continue;
					int wasamatch=checkhorizontal(state->board,i,j);
					match+=wasamatch;
					if(wasamatch){
						anymatches=true;
						state->score+=(wasamatch+1)*value(state->board[i][j].id);
					}
				}
			}
		}
		// verticle pass
		for(int i=0;i<SLOT_COLUMNS;++i){
			for(int j=0;j<SLOT_ROWS;++j){
				if(j<SLOT_ROWS-2){
					if(state->board[i][j].markfordelete)continue;
					int wasamatch=checkverticle(state->board,i,j);
					match+=wasamatch;
					if(wasamatch){
						anymatches=true;
						state->score+=((wasamatch+1)*value(state->board[i][j].id))*1.3f;
					}
				}
			}
		}
	}while(match);
	return anymatches;
}
static int checkanymovesleft(struct slot board[][SLOT_ROWS]){
	int anymatches=false;
	int match;
	do{
		match=false;
		// horizontal pass
		for(int i=0;i<SLOT_COLUMNS;++i){
			for(int j=0;j<SLOT_ROWS;++j){
				if(i<SLOT_COLUMNS-2){
					if(board[i][j].markfordelete)continue;
					int wasamatch=checkhorizontal(board,i,j);
					match+=wasamatch;
					if(wasamatch){
						anymatches=true;
					}
				}
			}
		}
		// verticle pass
		for(int i=0;i<SLOT_COLUMNS;++i){
			for(int j=0;j<SLOT_ROWS;++j){
				if(j<SLOT_ROWS-2){
					if(board[i][j].markfordelete)continue;
					int wasamatch=checkverticle(board,i,j);
					match+=wasamatch;
					if(wasamatch){
						anymatches=true;
					}
				}
			}
		}
	}while(match);
	return anymatches;
}
int anymovesleft(struct state *state){
	struct slot board[SLOT_COLUMNS][SLOT_ROWS];
	memcpy(board,state->board,sizeof(struct slot)*SLOT_COLUMNS*SLOT_ROWS);
	struct slot temp;
	for(int i=0;i<SLOT_COLUMNS;++i){
		for(int j=0;j<SLOT_ROWS;++j){
			if(i!=0){ // swap to the left
				temp=board[i][j];
				board[i][j]=board[i-1][j];
				board[i-1][j]=temp;
				if(checkanymovesleft(board))return true;
				temp=board[i][j];
				board[i][j]=board[i-1][j];
				board[i-1][j]=temp;
			}
			if(i!=SLOT_COLUMNS-1){ // swap to the right
				temp=board[i][j];
				board[i][j]=board[i+1][j];
				board[i+1][j]=temp;
				if(checkanymovesleft(board))return true;
				temp=board[i][j];
				board[i][j]=board[i+1][j];
				board[i+1][j]=temp;
			}
			if(j!=SLOT_ROWS-1){ // swap to the below
				temp=board[i][j];
				board[i][j]=board[i][j+1];
				board[i][j+1]=temp;
				if(checkanymovesleft(board))return true;
				temp=board[i][j];
				board[i][j]=board[i][j+1];
				board[i][j+1]=temp;
			}
			if(j!=0){ // swap to the above
				temp=board[i][j];
				board[i][j]=board[i][j-1];
				board[i][j-1]=temp;
				if(checkanymovesleft(board))return true;
				temp=board[i][j];
				board[i][j]=board[i][j-1];
				board[i][j-1]=temp;
			}
		}
	}
	return false;
}

int buttonprocess(struct state *state,struct button *button){
	if(state->pointer[0].x>button->base.x&&state->pointer[0].x<button->base.x+button->base.w&&
	state->pointer[0].y>button->base.y&&state->pointer[0].y<button->base.y+button->base.h){
		if(state->pointer[0].active){
			button->active=true;
			return BUTTON_PRESS;
		}
		else if(button->active){
			button->active=false;
			return BUTTON_ACTIVATE;
		}
	}
	button->active=false;
	return 0;
}
int buttondraw(struct state *state,struct button *button){
	int bstate=buttonprocess(state,button);
	if(button->active){
		float y=button->base.y;
		button->base.y+=BUTTON_DESCEND;
		draw(state,&button->base);
		button->base.y=y;
	}
	else draw(state,&button->base);
	return bstate;
}
void buttondrawtext(ftfont *font,struct button *button){
	float y=button->active?(button->base.y+BUTTON_DESCEND):button->base.y;
	drawtextcentered(font,button->base.x+(button->base.w/2.0f),y+(button->base.h/2.0f)-(font->fontsize/2.0f),button->label);
}

int readconfdata(struct state *state){
	FILE *file=fopen(DATAPATH"d01","rb");
	if(!file)return false;
	fread(&state->musicenabled,sizeof(unsigned char),1,file);
	fread(&state->showtut,sizeof(unsigned char),1,file);
	fclose(file);
	return true;
}
void saveconfdata(struct state *state){
	FILE *file=fopen(DATAPATH"d01","wb");
	if(!file){
		logcat("Could not write to "DATAPATH"d01");
		return;
	}
	fwrite(&state->musicenabled,sizeof(unsigned char),1,file);
	fwrite(&state->showtut,sizeof(unsigned char),1,file);
	fclose(file);
}
int readhsdata(unsigned *highscore){
	FILE *file=fopen(DATAPATH"d02","rb");
	if(!file)return false;
	fread(highscore,sizeof(unsigned),5,file);
	fclose(file);
	return true;
}
void savehsdata(unsigned *highscore){
	FILE *file=fopen(DATAPATH"d02","wb");
	if(!file){
		logcat("Could not write to "DATAPATH"d02");
		return;
	}
	fwrite(highscore,sizeof(unsigned),5,file);
	fclose(file);
}

void drawslot(struct state *state,struct slot *slot){
	const float spritewidth=1.0/6.0f;
	float left=spritewidth*slot->id;
	glUniform4f(state->uniform.texcoords,left,left+spritewidth,0.0f,1.0f);
	glUniform2f(state->uniform.vector,slot->base.x,slot->base.y);
	glUniform2f(state->uniform.size,slot->base.w,slot->base.h);
	glDrawArrays(GL_TRIANGLE_STRIP,0,4);
}
void draw(struct state *state,struct base *target){
	glUniform4f(state->uniform.texcoords,0.0f,1.0f,0.0f,1.0f);
	glUniform2f(state->uniform.vector,target->x,target->y);
	glUniform2f(state->uniform.size,target->w,target->h);
	glDrawArrays(GL_TRIANGLE_STRIP,0,4);
}