//Planned Changes
//Restructure struct classes


#pragma once
#include <windows.h>
#include <windowsx.h>
#include <d3d9.h>
#include <D3dx9tex.h>
#include <string.h>
#include <libxml/parser.h>
#include "dict.h"

#pragma comment (lib, "libxml2.lib")
#pragma comment(lib, "dict.lib")
#define ENV_FLOOR 135

struct SpriteSheetFrame;
struct SpriteSheet;
struct Sprite;
void ContPlayer(int CurrTime, Sprite* self);

struct SpriteSheetFrame {
	int x, y, w, h;
	SpriteSheet *Sheet;
};

struct SpriteSheet {
	char* FileName;
	LPDIRECT3DTEXTURE9 TextureLink;
	Dict *FrameDict;
	Dict *ActDict; //Contains Pointers to SpriteActionStep Roots
	int SSHeight;
	int SSWidth;
};

struct SpriteActionStep {
	SpriteSheetFrame *Frame; //null at root
	SpriteActionStep *ActionStepArray;
	SpriteActionStep *NextAction;
	int Delay; //Time Delay before moving to next step, in milliseconds
	bool RootYN;
};

struct SpritePhysicsStatus {
	float XAcceleration;
	float IdleDeclerate;
	float FallSpeed;
	float MaxSpeed;
	float JumpSpeed;
	float Xspd, Yspd;
	int FacingDirection;
	bool OnGround;
};

struct Sprite {
	int Xs, Ys;
	int Height, Width;
	SpritePhysicsStatus Phys;
	SpriteSheet *SS;
	SpriteActionStep *CurrActImage;
	char CurrActString[80];
	char ssid[80];
	int CurrAct;
	int PriorAct;
	int Time;
	bool EnemyYN;
	bool PlayerYN;
	bool ProjectileYN;
	bool BackgroundYN;
	void(*Controller)(int Frame, Sprite* self);
};

//Stack to be used for looping through all sprites in main code - Next = nullptr as terminator
struct SpriteStack {
	Sprite *s;
	SpriteStack *Next;
};

//Stack Functions----------------------------------------------------------
SpriteStack *SpriteStackCreate() {
	SpriteStack *temp;
	temp = (SpriteStack*)malloc(sizeof(SpriteStack));
	temp->Next = nullptr;
	temp->s = nullptr;
	return temp;
}

void SpriteStackAdd(Sprite* s, SpriteStack* Stck) {
	SpriteStack *temp;
	temp = SpriteStackCreate();
	temp->s = s;
	while (Stck->Next != nullptr) Stck = Stck->Next;
	Stck->Next = temp;
}

void SpriteStackRelease(SpriteStack* s) {
	if (s->Next != nullptr) SpriteStackRelease(s->Next);
	free(s);
}
//Stack Functions----------------------------------------------------------

#define ACTION_JUMP 0
#define ACTION_MOVE_RIGHT 1
#define ACTION_MOVE_LEFT 2
#define ACTION_IDLE 3

#define FACING_LEFT 0
#define FACING_RIGHT 1

//return vertex information for directx
//CUSTOMVERTEX *SpriteGetVertices(Sprite *s);
//initialize action dict, given a file with inputs
//
//

void SpriteSheetOpen(SpriteSheet *SS, LPDIRECT3DDEVICE9 d3ddev) 
{
	HRESULT X;
	X = D3DXCreateTextureFromFile(d3ddev,
		SS->FileName,
		&SS->TextureLink);

}
	


xmlNodePtr getNext(xmlNodePtr cur, const char *key) {
	while (cur != nullptr) {
		if ((!xmlStrcmp(cur->name, (const xmlChar *)key))) {
			return cur;
		}
		cur = cur->next;
	}
	return cur;
}


int xmlGetInt(xmlNodePtr node, const char * key)
{
	static xmlNodePtr temp;
	temp = getNext(node->children, key);
	return atoi((char*)temp->children->content);
}

float xmlGetFloat(xmlNodePtr node, const char * key)
{
	static xmlNodePtr temp;
	temp = getNext(node->children, key);
	return (float)atof((char*)temp->children->content);
}

void xmlGetString(char* str, xmlNodePtr node, const char * key) //assumes str buffer exists and is large enough
{
	static xmlNodePtr temp;
	temp = getNext(node->children, key);
	int len = strlen((char*)temp->children->content);
	strcpy_s(str, len*sizeof(char) + 1, (char*)temp->children->content);
}

void xmlSpriteConfig(const char *docname, LPDIRECT3DDEVICE9 d3ddev, Dict *SpriteSheetDict, SpriteStack *SpriteStackMain);
// Requires well formed and valid xml file
// Guarentees
//Dict *SpriteSheetDict //Sprite Sheet Dictionary - One record for each Spritesheet. SSHeight, SSWidth, and Texture are initialized in this implimentation
//Dict *ActionDict //Action Sheet Dictionary - One record for each action a Sprite can take. Provides direct pointers to frames within Spritesheets, their dimensions, the delay to the next frame, and a pointer to the next frame (which can be followed indefinately.
//Will fail in unpredictable ways if inputs are not well formed and valid

void xmlSpriteConfig(const char *docname, LPDIRECT3DDEVICE9 d3ddev, Dict *SpriteSheetDict, SpriteStack *SpriteStackMain) {
	xmlDocPtr doc;
	xmlNodePtr root, cur1, cur2, cur3, sheetid;
	char strtemp[256];
	char *next_token = NULL;
	xmlKeepBlanksDefault(0);
	doc = xmlParseFile(docname);

	if (doc == NULL) {
		fprintf(stderr, "Document not parsed successfully. \n");
		return;
	}

	root = xmlDocGetRootElement(doc);

	if (root == NULL) {
		fprintf(stderr, "empty document\n");
		xmlFreeDoc(doc);
		return;
	}

	if (xmlStrcmp(root->name, (const xmlChar *) "spritesheet")) {
		fprintf(stderr, "document of the wrong type, root node != spritesheet");
		xmlFreeDoc(doc);
		return;
	}
	//End Error Checking

	//initialize spritesheet dict
	if (SpriteSheetDict == nullptr) {
		fprintf(stderr, "initialize SpriteSheetDict before calling\n");
		xmlFreeDoc(doc);
		return;
	}

	for (cur1 = root->children; (cur1 = getNext(cur1, "sheet")) != nullptr; cur1 = cur1->next)
	{
		SpriteSheet *SStemp;
		cur2 = cur1->children;
		cur2 = getNext(cur2, "id");
		//add record to spritesheet dict, get handle for remainder of initialization
		SStemp = (SpriteSheet*)DictAdd(SpriteSheetDict, sizeof(SpriteSheet), (char*)cur2->children->content);
		SStemp->ActDict = DictCreate();

		for (cur2 = cur1->children; (cur2 = getNext(cur2, "filename")) != nullptr; cur2 = cur2->next)
		{
			//parse id from node->child->content
			SStemp->FileName = (char*)malloc(strlen((char*)cur2->children->content) + 1);
			strcpy_s(SStemp->FileName, strlen((char*)cur2->children->content) + 1, (char*)cur2->children->content);
			break;
		}

		xmlGetString(strtemp, cur1, "filesize");
		SStemp->SSWidth = atoi(strtok_s(strtemp, ",", &next_token));
		SStemp->SSHeight = atoi(strtok_s(NULL, ",", &next_token));

		//initialize texture object
		SpriteSheetOpen(SStemp, d3ddev); 
		//initialize spritesheetframe dict, assign to spritesheet object
		SStemp->FrameDict = DictCreate();


		for (cur2 = cur1->children; (cur2 = getNext(cur2, "frame")) != nullptr; cur2 = cur2->next)
		{
			SpriteSheetFrame *SSFtemp;
			cur3 = cur2->children;
			cur3 = getNext(cur3, "id");
			//parse id from node->child->content
			//add record to spritesheet frame dict; get pointer for remainder of loop initialization
			SSFtemp = (SpriteSheetFrame*)DictAdd(SStemp->FrameDict, sizeof(SpriteSheetFrame), (char*)cur3->children->content);
			SSFtemp->Sheet = SStemp;

			xmlGetString(strtemp, cur2, "cord");
			//parse 2 comma delinlited dimensions from node->child->Content
			SSFtemp->x = atoi(strtok_s(strtemp, ",", &next_token));
			SSFtemp->y = atoi(strtok_s(NULL, ",", &next_token));
			next_token = NULL;


			xmlGetString(strtemp, cur2, "dimensions");
			//parse 2 comma delinlited dimensions from node->child->Content
			SSFtemp->w = atoi(strtok_s(strtemp, ",", &next_token));
			SSFtemp->h = atoi(strtok_s(NULL, ",", &next_token));
			next_token = NULL;

		}

	}

	//spritesheet and frames initialization complete

	//begin action initialization

	//initialize action dict

	for (cur1 = root->children; (cur1 = getNext(cur1, "action")) != nullptr; cur1 = cur1->next)
	{
		//add entry to action dict
		SpriteActionStep *SAStemp;
		xmlGetString(strtemp, cur1, "actionid");
	
		cur2 = cur1->children;
		sheetid = getNext(cur2, "sheetid");

		SpriteSheet *SSTemp = ((SpriteSheet*)DictLookUp(SpriteSheetDict, (char*)sheetid->children->content));
		//add record to spritesheet action dict, get handle for remainder of initialization
		SAStemp = (SpriteActionStep*)DictAdd(SSTemp->ActDict, sizeof(SpriteActionStep), strtemp);
		SAStemp->RootYN = true;

		//Count the # of action steps exist within the action 
		int count_steps = 0; 
		for (cur2 = cur1->children; (cur2 = getNext(cur2, "actionstep")) != nullptr; cur2 = cur2->next)	count_steps++;

		//initialize array of action step pointers of size count*sizeof(actionsteppointer)
		SAStemp->ActionStepArray = (SpriteActionStep*)malloc(count_steps*sizeof(SpriteActionStep));
		count_steps = 0;

		for (cur2 = cur1->children; (cur2 = getNext(cur2, "actionstep")) != nullptr; cur2 = cur2->next)
		{

			//get frame object pointer based on sheet and frame id
			//assign frame pointer to this action step [count]
			xmlGetString(strtemp, cur2, "frameid");
			Dict *tempFrameDict;
			tempFrameDict = SSTemp->FrameDict;//Get Frame Dict Pointer
			(SAStemp->ActionStepArray + count_steps)->Frame = (SpriteSheetFrame*)DictLookUp(tempFrameDict, strtemp);

			(SAStemp->ActionStepArray + count_steps)->Delay = xmlGetInt(cur2, "delay");
			(SAStemp->ActionStepArray + count_steps)->RootYN = false;

			//if actionstep > 0, assign pointer of prior action step to this one
			if (count_steps > 0) (SAStemp->ActionStepArray + count_steps - 1)->NextAction = (SAStemp->ActionStepArray + count_steps);
			count_steps++;
		}
		//point next action step pointer of action[max] to action[0]
		(SAStemp->ActionStepArray + count_steps - 1)->NextAction = (SAStemp->ActionStepArray);
	}


	for (cur1 = root->children; (cur1 = getNext(cur1, "sprite")) != nullptr; cur1 = cur1->next)
	{
		Sprite* S;
		S = (Sprite*)malloc(sizeof(Sprite));
		SpriteStackAdd(S, SpriteStackMain);
		S->Time = 0;
		S->Height = xmlGetInt(cur1, "Height");
		if (S->Height == -1) S->Height = SCREEN_HEIGHT;
		S->Width = xmlGetInt(cur1, "Width");
		if (S->Width == -1) S->Width = SCREEN_WIDTH;
		S->Ys = xmlGetInt(cur1, "StartY");
		S->Xs = xmlGetInt(cur1, "StartX");
		S->Phys.Xspd = xmlGetFloat(cur1, "Xspd");
		S->Phys.Yspd = xmlGetFloat(cur1, "Yspd");
		S->Phys.XAcceleration = xmlGetFloat(cur1, "XAcceleration");
		S->Phys.IdleDeclerate = xmlGetFloat(cur1, "IdleDeclerate");
		S->Phys.FallSpeed = xmlGetFloat(cur1, "FallSpeed");
		S->Phys.MaxSpeed = xmlGetFloat(cur1, "MaxSpeed");
		S->Phys.JumpSpeed = xmlGetFloat(cur1, "JumpSpeed");
		S->Phys.OnGround = (xmlGetInt(cur1, "OnGround")==1);
		S->Phys.FacingDirection = xmlGetInt(cur1, "FacingDirection");
		S->CurrAct = xmlGetInt(cur1, "CurrAct");
		S->EnemyYN = (xmlGetInt(cur1, "EnemyYN") == 1);
		S->PlayerYN = (xmlGetInt(cur1, "PlayerYN") == 1);
		S->ProjectileYN = (xmlGetInt(cur1, "ProjectileYN") == 1);
		S->BackgroundYN = (xmlGetInt(cur1, "BackgroundYN") == 1);
		xmlGetString(S->CurrActString, cur1, "CurrActionStr");
		xmlGetString(S->ssid, cur1, "ssid");
		S->SS = (SpriteSheet*)DictLookUp(SpriteSheetDict, S->ssid);
		S->CurrActImage = ((SpriteActionStep*)DictLookUp(S->SS->ActDict, S->CurrActString))->ActionStepArray;
		if (!S->BackgroundYN) S->Controller = &ContPlayer;
		else  S->Controller = nullptr;
	}
	//Actions can now be linked to sprite objects for quick reference to their frame ids
	xmlFreeDoc(doc);
	return;
}

// Deletes the Sprite Sheet Dict
void SpriteSheetDictRelease(Dict *SSDict)
{
	SpriteSheet *SS;
	DictNode *temp;
	temp = SSDict->First;
		
	for (temp = SSDict->First; temp != nullptr; temp = temp->Next) {
		SS = (SpriteSheet*)temp->Object;
		free(SS->FileName);
		DictRelease(SS->FrameDict);
		DictRelease(SS->ActDict);
		SS->TextureLink->Release();
	}
	DictRelease(SSDict);
}; 

void ContPlayer(int CurrTime, Sprite* self)
{
	static bool FrameDirection = 1;
	int TimeDiff = CurrTime - self->Time;
	self->Time = CurrTime;
	for (int i = 0; i < TimeDiff; i++)
	{
		switch (self->CurrAct)
		{
		case ACTION_JUMP:
			if (self->Phys.OnGround == 0)
			{
				break;
			};
			self->Phys.Yspd = self->Phys.JumpSpeed*-1;
			self->Phys.OnGround = 0;
			self->CurrAct = self->PriorAct;
			break;
		case ACTION_MOVE_RIGHT:
			self->Phys.FacingDirection = FACING_RIGHT;
			if (self->Phys.Xspd < self->Phys.MaxSpeed)
				self->Phys.Xspd += self->Phys.XAcceleration;
			//self->CurrActImage = (SpriteActionStep*)DictLookUp(self->ActDict, "ACTION_MOVE_RIGHT"); //Update to run this in response to event, instead of physics, logic
			break;
		case ACTION_MOVE_LEFT:
			self->Phys.FacingDirection = FACING_LEFT;
			if (self->Phys.Xspd >(self->Phys.MaxSpeed*-1))
				self->Phys.Xspd -= self->Phys.XAcceleration;
			break;
		case ACTION_IDLE:
			if (self->Phys.Xspd > self->Phys.IdleDeclerate)
				self->Phys.Xspd -= self->Phys.IdleDeclerate;
			else if (self->Phys.Xspd < -self->Phys.IdleDeclerate)
				self->Phys.Xspd += self->Phys.IdleDeclerate;
			else self->Phys.Xspd = 0;
			break;
		};

		self->Xs += int(self->Phys.Xspd);
		self->Ys += int(self->Phys.Yspd);
		if (self->Ys >(SCREEN_HEIGHT - self->Height - ENV_FLOOR))
		{
			self->Ys = (SCREEN_HEIGHT - self->Height - ENV_FLOOR);
			self->Phys.Yspd = 0;
			self->Phys.OnGround = 1;
		}
		if (!self->Phys.OnGround)
		{
			self->Phys.Yspd += self->Phys.FallSpeed;
		};
	};
	//self->CurrAct = ACTION_IDLE;

	if (CurrTime % self->CurrActImage->Delay == 0)
	{
		self->CurrActImage = self->CurrActImage->NextAction;

	}


	/*Rewrite to reflect the frame pointer approach
	if (CurrTime % self == 0)
	{
		if (FrameDirection)
		{
			if (self->FrameState == (self->SS->NFrames - 1))
			{
				FrameDirection = 0;
				self->FrameState = self->FrameState - 1;
			}
			else
			{
				self->FrameState = self->FrameState + 1;
			}
		}
		else
		{
			if (self->FrameState == 0)
			{
				FrameDirection = 1;
				self->FrameState = self->FrameState + 1;
			}
			else
			{
				self->FrameState = self->FrameState - 1;
			}
		}
	};*/
};
